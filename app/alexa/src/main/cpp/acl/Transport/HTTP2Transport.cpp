#include <algorithm>
#include <chrono>
#include <functional>
#include <random>
#include <util/HTTP/HttpResponseCode.h>
#include <util/HTTP2/HTTP2MimeRequestEncoder.h>
#include <util/HTTP2/HTTP2MimeResponseDecoder.h>
#include <logger/Logger.h>
#include <timing/TimeUtils.h>
#include "PostConnectInterface.h"
#include "DownchannelHandler.h"
#include "HTTP2Transport.h"
#include "MessageRequestHandler.h"
#include "PingHandler.h"
#include "TransportDefines.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace utils;
        using namespace logger;
        using namespace sdkInterfaces;
        using namespace avs;
        using namespace attachment;
        using namespace http2;
        using namespace metrics;
        static const string TAG("HTTP2Transport");
        #define LX(event) LogEntry(TAG, event)
        #define LX_P(event) LX(event).p("this", this)
        static const int MAX_STREAMS = 10;
        static const int MAX_MESSAGE_HANDLERS = MAX_STREAMS - 2;
        static minutes INACTIVITY_TIMEOUT{5};
        static const seconds MESSAGE_QUEUE_TIMEOUT = seconds(15);
        ostream& operator<<(ostream& stream, HTTP2Transport::State state) {
            switch (state) {
                case HTTP2Transport::State::INIT: return stream << "INIT";
                case HTTP2Transport::State::AUTHORIZING: return stream << "AUTHORIZING";
                case HTTP2Transport::State::CONNECTING: return stream << "CONNECTING";
                case HTTP2Transport::State::WAITING_TO_RETRY_CONNECTING: return stream << "WAITING_TO_RETRY_CONNECTING";
                case HTTP2Transport::State::POST_CONNECTING: return stream << "POST_CONNECTING";
                case HTTP2Transport::State::CONNECTED: return stream << "CONNECTED";
                case HTTP2Transport::State::SERVER_SIDE_DISCONNECT: return stream << "SERVER_SIDE_DISCONNECT";
                case HTTP2Transport::State::DISCONNECTING: return stream << "DISCONNECTING";
                case HTTP2Transport::State::SHUTDOWN: return stream << "SHUTDOWN";
            }
            return stream << "";
        }
        HTTP2Transport::Configuration::Configuration() : inactivityTimeout{INACTIVITY_TIMEOUT} {}
        shared_ptr<HTTP2Transport> HTTP2Transport::create(shared_ptr<AuthDelegateInterface> authDelegate, const string& avsGateway,
                                                          shared_ptr<HTTP2ConnectionInterface> http2Connection, shared_ptr<MessageConsumerInterface> messageConsumer,
                                                          shared_ptr<AttachmentManager> attachmentManager, shared_ptr<TransportObserverInterface> transportObserver,
                                                          shared_ptr<PostConnectFactoryInterface> postConnectFactory,
                                                          shared_ptr<SynchronizedMessageRequestQueue> sharedRequestQueue, Configuration configuration,
                                                          shared_ptr<MetricRecorderInterface> metricRecorder, shared_ptr<EventTracerInterface> eventTracer) {
            ACSDK_DEBUG5(LX(__func__).d("authDelegate", authDelegate.get()).d("avsGateway", avsGateway).d("http2Connection", http2Connection.get())
                               .d("messageConsumer", messageConsumer.get()).d("attachmentManager", attachmentManager.get())
                               .d("transportObserver", transportObserver.get()).d("postConnectFactory", postConnectFactory.get())
                               .d("sharedRequestQueue", sharedRequestQueue.get()));
            if (!authDelegate) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullAuthDelegate"));
                return nullptr;
            }
            if (avsGateway.empty()) {
                ACSDK_ERROR(LX("createFailed").d("reason", "emptyAVSGateway"));
                return nullptr;
            }
            if (!http2Connection) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullHTTP2ConnectionInterface"));
                return nullptr;
            }
            if (!messageConsumer) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageConsumer"));
                return nullptr;
            }
            if (!attachmentManager) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullAttachmentManager"));
                return nullptr;
            }
            if (!postConnectFactory) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullPostConnectFactory"));
                return nullptr;
            }
            if (!sharedRequestQueue) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullSharedRequestQueue"));
                return nullptr;
            }
            auto transport = std::shared_ptr<HTTP2Transport>(new HTTP2Transport(move(authDelegate), avsGateway, move(http2Connection), move(messageConsumer),
                                                             move(attachmentManager), move(transportObserver), move(postConnectFactory), move(sharedRequestQueue),
                                                             configuration, move(metricRecorder), move(eventTracer)));
            return transport;
        }
        HTTP2Transport::HTTP2Transport(shared_ptr<AuthDelegateInterface> authDelegate, const string& avsGateway, shared_ptr<HTTP2ConnectionInterface> http2Connection,
                                       shared_ptr<MessageConsumerInterface> messageConsumer, shared_ptr<AttachmentManager> attachmentManager,
                                       shared_ptr<TransportObserverInterface> transportObserver, shared_ptr<PostConnectFactoryInterface> postConnectFactory,
                                       shared_ptr<SynchronizedMessageRequestQueue> sharedRequestQueue, Configuration configuration,
                                       shared_ptr<MetricRecorderInterface> metricRecorder, shared_ptr<EventTracerInterface> eventTracer) :
                                       m_metricRecorder{move(metricRecorder)}, m_state{State::INIT}, m_authDelegate{move(authDelegate)}, m_avsGateway{avsGateway},
                                       m_http2Connection{move(http2Connection)}, m_messageConsumer{move(messageConsumer)},
                                       m_attachmentManager{move(attachmentManager)}, m_postConnectFactory{move(postConnectFactory)},
                                       m_sharedRequestQueue{move(sharedRequestQueue)}, m_eventTracer{move(eventTracer)},
                                       m_connectRetryCount{0}, m_countOfUnfinishedMessageHandlers{0}, m_postConnected{false}, m_configuration{configuration},
                                       m_disconnectReason{ConnectionStatusObserverInterface::ChangedReason::NONE} {
            m_observers.insert(transportObserver);
        }
        void HTTP2Transport::addObserver(shared_ptr<TransportObserverInterface> transportObserver) {
            ACSDK_DEBUG7(LX_P(__func__).d("transportObserver", transportObserver.get()));
            if (!transportObserver) {
                ACSDK_ERROR(LX_P("addObserverFailed").d("reason", "nullObserver"));
                return;
            }
            lock_guard<mutex> lock{m_observerMutex};
            m_observers.insert(transportObserver);
        }
        void HTTP2Transport::removeObserver(shared_ptr<TransportObserverInterface> transportObserver) {
            ACSDK_DEBUG7(LX_P(__func__).d("transportObserver", transportObserver.get()));
            if (!transportObserver) {
                ACSDK_ERROR(LX_P("removeObserverFailed").d("reason", "nullObserver"));
                return;
            }
            lock_guard<mutex> lock{m_observerMutex};
            m_observers.erase(transportObserver);
        }
        shared_ptr<HTTP2ConnectionInterface> HTTP2Transport::getHTTP2Connection() {
            return m_http2Connection;
        }
        bool HTTP2Transport::connect() {
            ACSDK_INFO(LX_P(__func__));
            if (!setState(State::AUTHORIZING, ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST)) {
                ACSDK_ERROR(LX_P("connectFailed").d("reason", "setStateFailed"));
                return false;
            }
            m_thread = std::thread(&HTTP2Transport::mainLoop, this);
            return true;
        }
        void HTTP2Transport::disconnect() {
            ACSDK_INFO(LX_P(__func__));
            thread localThread;
            {
                lock_guard<std::mutex> lock(m_mutex);
                if (State::SHUTDOWN != m_state) {
                    setStateLocked(State::DISCONNECTING, ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST);
                }
                swap(m_thread, localThread);
            }
            if (localThread.joinable()) localThread.join();
        }
        bool HTTP2Transport::isConnected() {
            lock_guard<mutex> lock(m_mutex);
            return State::CONNECTED == m_state;
        }
        void HTTP2Transport::onRequestEnqueued() {
            ACSDK_DEBUG7(LX_P(__func__));
            lock_guard<mutex> lock(m_mutex);
            m_wakeEvent.notify_all();
        }
        void HTTP2Transport::onWakeConnectionRetry() {
            ACSDK_INFO(LX_P(__func__));
            lock_guard<mutex> lock(m_mutex);
            if (State::WAITING_TO_RETRY_CONNECTING != m_state) return;
            if (!setStateLocked(State::CONNECTING, ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST)) {
                ACSDK_ERROR(LX_P("onWakeRetryConnectingFailed").d("reason", "setStateFailed"));
            }
        }
        void HTTP2Transport::onWakeVerifyConnectivity() {
            ACSDK_INFO(LX_P(__func__));
            lock_guard<mutex> lock(m_mutex);
            if (!m_pingHandler) {
                m_timeOfLastActivity = m_timeOfLastActivity.min();
                m_wakeEvent.notify_all();
            }
        }
        void HTTP2Transport::sendMessage(shared_ptr<MessageRequest> request) {
            ACSDK_DEBUG7(LX_P(__func__));
            if (!request) {
                ACSDK_ERROR(LX_P("enqueueRequestFailed").d("reason", "nullRequest"));
            }
            unique_lock<mutex> lock(m_mutex);
            bool allowed = false;
            switch(m_state) {
                case State::INIT: case State::AUTHORIZING: case State::CONNECTING: case State::WAITING_TO_RETRY_CONNECTING: case State::POST_CONNECTING:
                    allowed = true;
                    break;
                case State::CONNECTED: case State::SERVER_SIDE_DISCONNECT: case State::DISCONNECTING: case State::SHUTDOWN:
                    allowed = false;
                    break;
            }
            if (allowed) {
                m_requestQueue.enqueueRequest(request);
                m_wakeEvent.notify_all();
            } else {
                ACSDK_ERROR(LX_P("enqueueRequestFailed").d("reason", "notInAllowedState").d("m_state", m_state));
                lock.unlock();
                request->sendCompleted(MessageRequestObserverInterface::Status::NOT_CONNECTED);
            }
        }
        void HTTP2Transport::onPostConnected() {
            ACSDK_INFO(LX_P(__func__));
            lock_guard<mutex> lock(m_mutex);
            switch(m_state) {
                case State::INIT: case State::AUTHORIZING: case State::CONNECTING: case State::WAITING_TO_RETRY_CONNECTING: m_postConnected = true; break;
                case State::CONNECTED: ACSDK_ERROR(LX_P("onPostConnectFailed").d("reason", "unexpectedState")); break;
                case State::POST_CONNECTING:
                    m_postConnected = true;
                    if (!setStateLocked(State::CONNECTED, ConnectionStatusObserverInterface::ChangedReason::SUCCESS)) {
                        ACSDK_ERROR(LX_P("onPostConnectFailed").d("reason", "setState(CONNECTED)Failed"));
                    }
                    break;
                case State::SERVER_SIDE_DISCONNECT: case State::DISCONNECTING: case State::SHUTDOWN: break;
            }
        }
        void HTTP2Transport::onUnRecoverablePostConnectFailure() {
            ACSDK_INFO(LX_P(__func__));
            lock_guard<mutex> lock(m_mutex);
            switch(m_state) {
                case State::INIT: case State::AUTHORIZING: case State::CONNECTING: case State::WAITING_TO_RETRY_CONNECTING: case State::POST_CONNECTING:
                    if (!setStateLocked(State::SHUTDOWN, ConnectionStatusObserverInterface::ChangedReason::UNRECOVERABLE_ERROR)) {
                        ACSDK_ERROR(LX_P("onUnRecoverablePostConnectFailure").d("reason", "setState(SHUTDOWN)Failed"));
                    }
                    break;
                case State::CONNECTED:
                    ACSDK_ERROR(LX_P("onUnRecoverablePostConnectFailure").d("reason", "unexpectedState"));
                    if (!setStateLocked(State::SHUTDOWN, ConnectionStatusObserverInterface::ChangedReason::UNRECOVERABLE_ERROR)) {
                        ACSDK_ERROR(LX_P("onUnRecoverablePostConnectFailure").d("reason", "setState(SHUTDOWN)Failed"));
                    }
                    break;
                case State::SERVER_SIDE_DISCONNECT: case State::DISCONNECTING: case State::SHUTDOWN: break;
            }
        }
        void HTTP2Transport::onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error error) {
            ACSDK_INFO(LX_P(__func__).d("newState", newState).d("error", error));
            lock_guard<mutex> lock(m_mutex);
            switch(newState) {
                case AuthObserverInterface::State::UNINITIALIZED: case AuthObserverInterface::State::EXPIRED:
                    if (State::WAITING_TO_RETRY_CONNECTING == m_state) {
                        ACSDK_DEBUG0(LX_P("revertToAuthorizing").d("reason", "authorizationExpiredBeforeConnected"));
                        setStateLocked(State::AUTHORIZING, ConnectionStatusObserverInterface::ChangedReason::INVALID_AUTH);
                    }
                    return;
                case AuthObserverInterface::State::REFRESHED:
                    if (State::AUTHORIZING == m_state) {
                        setStateLocked(State::CONNECTING, ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                    }
                    return;
                case AuthObserverInterface::State::UNRECOVERABLE_ERROR:
                    ACSDK_ERROR(LX_P("shuttingDown").d("reason", "unrecoverableAuthError"));
                    setStateLocked(State::SHUTDOWN, ConnectionStatusObserverInterface::ChangedReason::UNRECOVERABLE_ERROR);
                    return;
            }
            ACSDK_ERROR(LX_P("shuttingDown").d("reason", "unknownAuthStatus").d("newState", static_cast<int>(newState)));
            setStateLocked(State::SHUTDOWN, ConnectionStatusObserverInterface::ChangedReason::UNRECOVERABLE_ERROR);
        }
        void HTTP2Transport::doShutdown() {
            ACSDK_INFO(LX_P(__func__));
            setState(State::SHUTDOWN, ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST);
            disconnect();
            m_authDelegate->removeAuthObserver(shared_from_this());
            m_pingHandler.reset();
            m_http2Connection.reset();
            m_messageConsumer.reset();
            m_attachmentManager.reset();
            m_postConnectFactory.reset();
            m_postConnect.reset();
            m_observers.clear();
        }
        void HTTP2Transport::onDownchannelConnected() {
            ACSDK_INFO(LX_P(__func__));
            setState(State::POST_CONNECTING, ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
        }
        void HTTP2Transport::onDownchannelFinished() {
            ACSDK_INFO(LX_P(__func__));
            lock_guard<mutex> lock(m_mutex);
            switch (m_state) {
                case State::INIT: case State::AUTHORIZING: case State::WAITING_TO_RETRY_CONNECTING:
                    ACSDK_ERROR(LX_P("onDownchannelFinishedFailed").d("reason", "unexpectedState"));
                    break;
                case State::CONNECTING:
                    setStateLocked(State::WAITING_TO_RETRY_CONNECTING, ConnectionStatusObserverInterface::ChangedReason::NONE);
                    break;
                case State::POST_CONNECTING: case State::CONNECTED:
                    setStateLocked(State::SERVER_SIDE_DISCONNECT,ConnectionStatusObserverInterface::ChangedReason::SERVER_SIDE_DISCONNECT);
                    break;
                case State::SERVER_SIDE_DISCONNECT: case State::DISCONNECTING: case State::SHUTDOWN: break;
            }
        }
        void HTTP2Transport::onMessageRequestSent(const std::shared_ptr<avsCommon::avs::MessageRequest>& request) {
            lock_guard<mutex> lock(m_mutex);
            if (request->getIsSerialized()) m_sharedRequestQueue->setWaitingForSendAcknowledgement();
            m_countOfUnfinishedMessageHandlers++;
            ACSDK_DEBUG7(LX_P(__func__).d("countOfUnfinishedMessageHandlers", m_countOfUnfinishedMessageHandlers));
        }
        void HTTP2Transport::onMessageRequestTimeout() {
            ACSDK_INFO(LX_P(__func__));
            onWakeVerifyConnectivity();
        }
        void HTTP2Transport::onMessageRequestAcknowledged(const std::shared_ptr<avsCommon::avs::MessageRequest>& request) {
            ACSDK_DEBUG7(LX_P(__func__));
            std::lock_guard<std::mutex> lock(m_mutex);
            if (request->getIsSerialized()) m_sharedRequestQueue->clearWaitingForSendAcknowledgement();
            m_wakeEvent.notify_all();
        }
        void HTTP2Transport::onMessageRequestFinished() {
            lock_guard<mutex> lock(m_mutex);
            --m_countOfUnfinishedMessageHandlers;
            ACSDK_DEBUG7(LX_P(__func__).d("countOfUnfinishedMessageHandlers", m_countOfUnfinishedMessageHandlers));
            m_wakeEvent.notify_all();
        }
        void HTTP2Transport::onPingRequestAcknowledged(bool success) {
            ACSDK_DEBUG7(LX_P(__func__).d("success", success));
            lock_guard<mutex> lock(m_mutex);
            m_pingHandler.reset();
            if (!success) setStateLocked(State::SERVER_SIDE_DISCONNECT, ConnectionStatusObserverInterface::ChangedReason::SERVER_SIDE_DISCONNECT);
            m_wakeEvent.notify_all();
        }
        void HTTP2Transport::onPingTimeout() {
            ACSDK_WARN(LX_P(__func__));
            lock_guard<mutex> lock(m_mutex);
            m_pingHandler.reset();
            setStateLocked(State::SHUTDOWN, ConnectionStatusObserverInterface::ChangedReason::PING_TIMEDOUT);
            m_wakeEvent.notify_all();
        }
        void HTTP2Transport::onActivity() {
            ACSDK_DEBUG9(LX_P(__func__));
            lock_guard<mutex> lock(m_mutex);
            m_timeOfLastActivity = steady_clock::now();
        }
        void HTTP2Transport::onForbidden(const string& authToken) {
            ACSDK_INFO(LX_P(__func__));
            m_authDelegate->onAuthFailure(authToken);
        }
        shared_ptr<HTTP2RequestInterface> HTTP2Transport::createAndSendRequest(const HTTP2RequestConfig& cfg) {
            ACSDK_DEBUG7(LX_P(__func__).d("type", cfg.getRequestType()).sensitive("url", cfg.getUrl()));
            return m_http2Connection->createAndSendRequest(cfg);
        }
        string HTTP2Transport::getAVSGateway() {
            return m_avsGateway;
        }
        void HTTP2Transport::onGoawayReceived() {
            ACSDK_INFO(LX_P(__func__));
        }
        void HTTP2Transport::mainLoop() {
            ACSDK_DEBUG7(LX_P(__func__));
            m_http2Connection->addObserver(shared_from_this());
            m_postConnect = m_postConnectFactory->createPostConnect();
            if (!m_postConnect || !m_postConnect->doPostConnect(shared_from_this(), shared_from_this())) {
                ACSDK_ERROR(LX_P("mainLoopFailed").d("reason", "createPostConnectFailed"));
                lock_guard<mutex> lock(m_mutex);
                setStateLocked(State::SHUTDOWN, ConnectionStatusObserverInterface::ChangedReason::INTERNAL_ERROR);
            }
            m_timeOfLastActivity = steady_clock::now();
            State nextState = getState();
            while(nextState != State::SHUTDOWN) {
                switch(nextState) {
                    case State::INIT: nextState = handleInit(); break;
                    case State::AUTHORIZING: nextState = handleAuthorizing(); break;
                    case State::CONNECTING: nextState = handleConnecting(); break;
                    case State::WAITING_TO_RETRY_CONNECTING: nextState = handleWaitingToRetryConnecting(); break;
                    case State::POST_CONNECTING: nextState = handlePostConnecting(); break;
                    case State::CONNECTED: nextState = handleConnected(); break;
                    case State::SERVER_SIDE_DISCONNECT: nextState = handleServerSideDisconnect(); break;
                    case State::DISCONNECTING: nextState = handleDisconnecting(); break;
                    case State::SHUTDOWN: break;
                }
            }
            handleShutdown();
            ACSDK_DEBUG7(LX_P("mainLoopExiting"));
        }
        HTTP2Transport::State HTTP2Transport::handleInit() {
            ACSDK_CRITICAL(LX_P(__func__).d("reason", "unexpectedState"));
            lock_guard<mutex> lock(m_mutex);
            setStateLocked(State::SHUTDOWN, ConnectionStatusObserverInterface::ChangedReason::INTERNAL_ERROR);
            return m_state;
        }
        HTTP2Transport::State HTTP2Transport::handleAuthorizing() {
            ACSDK_INFO(LX_P(__func__));
            m_authDelegate->addAuthObserver(shared_from_this());
            return monitorSharedQueueWhileWaiting(State::AUTHORIZING);
        }
        HTTP2Transport::State HTTP2Transport::handleConnecting() {
            ACSDK_INFO(LX_P(__func__));
            auto authToken = m_authDelegate->getAuthToken();
            if (authToken.empty()) {
                ACSDK_DEBUG0(LX_P("Empty AuthToken"));
                lock_guard<mutex> lock(m_mutex);
                setStateLocked(State::WAITING_TO_RETRY_CONNECTING, ConnectionStatusObserverInterface::ChangedReason::INVALID_AUTH);
                return m_state;
            }
            auto downchannelHandler = DownchannelHandler::create(shared_from_this(), authToken, m_messageConsumer,
                                                                 m_attachmentManager);
            if (!downchannelHandler) {
                ACSDK_ERROR(LX_P("handleConnectingFailed").d("reason", "createDownchannelHandlerFailed"));
                lock_guard<mutex> lock(m_mutex);
                setStateLocked(State::WAITING_TO_RETRY_CONNECTING, ConnectionStatusObserverInterface::ChangedReason::INTERNAL_ERROR);
                return m_state;
            }
            return monitorSharedQueueWhileWaiting(State::CONNECTING);
        }
        HTTP2Transport::State HTTP2Transport::handleWaitingToRetryConnecting() {
            auto timeout = TransportDefines::getRetryTimer().calculateTimeToRetry(m_connectRetryCount);
            ACSDK_INFO(LX_P(__func__).d("connectRetryCount", m_connectRetryCount).d("timeout", timeout.count()));
            m_connectRetryCount++;
            auto wakeTime = std::chrono::steady_clock::now() + timeout;
            monitorSharedQueueWhileWaiting(State::WAITING_TO_RETRY_CONNECTING, wakeTime);
            lock_guard<mutex> lock(m_mutex);
            if (State::WAITING_TO_RETRY_CONNECTING == m_state) {
                setStateLocked(State::CONNECTING, ConnectionStatusObserverInterface::ChangedReason::NONE);
            }
            return m_state;
        }
        HTTP2Transport::State HTTP2Transport::handlePostConnecting() {
            ACSDK_INFO(LX_P(__func__));
            if (m_postConnected) {
                setState(State::CONNECTED, ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                return State::CONNECTED;
            }
            return sendMessagesAndPings(State::POST_CONNECTING, m_requestQueue);
        }
        HTTP2Transport::State HTTP2Transport::handleConnected() {
            ACSDK_INFO(LX_P(__func__));
            if (m_postConnect) m_postConnect.reset();
            notifyObserversOnConnected();
            return sendMessagesAndPings(State::CONNECTED, *m_sharedRequestQueue);
        }
        HTTP2Transport::State HTTP2Transport::handleServerSideDisconnect() {
            ACSDK_INFO(LX_P(__func__));
            notifyObserversOnServerSideDisconnect();
            return State::DISCONNECTING;
        }
        HTTP2Transport::State HTTP2Transport::handleDisconnecting() {
            ACSDK_INFO(LX_P(__func__));
            unique_lock<mutex> lock(m_mutex);
            while(State::DISCONNECTING == m_state && m_countOfUnfinishedMessageHandlers > 0) m_wakeEvent.wait(lock);
            setStateLocked(State::SHUTDOWN, ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
            return m_state;
        }
        HTTP2Transport::State HTTP2Transport::handleShutdown() {
            ACSDK_INFO(LX_P(__func__));
            {
                lock_guard<mutex> lock(m_mutex);
                m_sharedRequestQueue->clearWaitingForSendAcknowledgement();
                while(!m_requestQueue.empty()) {
                    auto request = m_requestQueue.dequeueOldestRequest();
                    if (request != nullptr) request->sendCompleted(MessageRequestObserverInterface::Status::NOT_CONNECTED);
                }
                m_requestQueue.clear();
            }
            m_http2Connection->removeObserver(shared_from_this());
            m_http2Connection->disconnect();
            notifyObserversOnDisconnect(m_disconnectReason);
            return State::SHUTDOWN;
        }
        HTTP2Transport::State HTTP2Transport::monitorSharedQueueWhileWaiting(HTTP2Transport::State whileState, time_point<steady_clock> maxWakeTime) {
            while(true) {
                auto wakeTime = maxWakeTime;
                while(true) {
                    auto messageRequestTime = m_sharedRequestQueue->peekRequestTime();
                    if (!messageRequestTime.hasValue()) break;
                    auto messageTimeoutTime = messageRequestTime.value() + MESSAGE_QUEUE_TIMEOUT;
                    if (messageTimeoutTime > steady_clock::now()) {
                        wakeTime = min(messageTimeoutTime, wakeTime);
                        break;
                    }
                    auto request = m_sharedRequestQueue->dequeueOldestRequest();
                    request->sendCompleted(MessageRequestObserverInterface::Status::TIMEDOUT);
                }
                auto messageRequestTime = m_sharedRequestQueue->peekRequestTime();
                unique_lock<mutex> lock(m_mutex);
                m_wakeEvent.wait_until(lock, wakeTime, [this, whileState, messageRequestTime] {
                    return m_state != whileState || m_sharedRequestQueue->peekRequestTime() != messageRequestTime;
                });
                if (whileState != m_state || steady_clock::now() >= maxWakeTime) return m_state;
            }
        }
        HTTP2Transport::State HTTP2Transport::sendMessagesAndPings(HTTP2Transport::State whileState, MessageRequestQueueInterface& requestQueue) {
            ACSDK_DEBUG7(LX_P(__func__).d("whileState", whileState));
            unique_lock<mutex> lock(m_mutex);
            auto canSendMessage = [this, &requestQueue] {
                return (requestQueue.isMessageRequestAvailable() && (m_countOfUnfinishedMessageHandlers < MAX_MESSAGE_HANDLERS));
            };
            auto wakePredicate = [this, whileState, canSendMessage] {
                return whileState != m_state || canSendMessage() || (steady_clock::now() > m_timeOfLastActivity + m_configuration.inactivityTimeout);
            };
            auto pingWakePredicate = [this, whileState, canSendMessage] { return whileState != m_state || !m_pingHandler || canSendMessage(); };
            while(true) {
                if (m_pingHandler) m_wakeEvent.wait(lock, pingWakePredicate);
                else m_wakeEvent.wait_until(lock, m_timeOfLastActivity + m_configuration.inactivityTimeout, wakePredicate);
                if (m_state != whileState) break;
                if (canSendMessage()) {
                    auto messageRequest = requestQueue.dequeueSendableRequest();
                    lock.unlock();
                    auto authToken = m_authDelegate->getAuthToken();
                    if (!authToken.empty()) {
                        auto handler = MessageRequestHandler::create(shared_from_this(), authToken, messageRequest,m_messageConsumer,
                                                    m_attachmentManager,m_metricRecorder,m_eventTracer);
                        if (!handler) messageRequest->sendCompleted(MessageRequestObserverInterface::Status::INTERNAL_ERROR);
                    } else {
                        ACSDK_ERROR(LX_P("failedToCreateMessageHandler").d("reason", "invalidAuth"));
                        messageRequest->sendCompleted(MessageRequestObserverInterface::Status::INVALID_AUTH);
                    }
                    lock.lock();
                } else if (steady_clock::now() > m_timeOfLastActivity + m_configuration.inactivityTimeout) {
                    if (!m_pingHandler) {
                        lock.unlock();
                        auto authToken = m_authDelegate->getAuthToken();
                        if (!authToken.empty()) m_pingHandler = PingHandler::create(shared_from_this(), authToken);
                        else {
                            ACSDK_ERROR(LX_P("failedToCreatePingHandler").d("reason", "invalidAuth"));
                        }
                        if (!m_pingHandler) {
                            ACSDK_ERROR(LX_P("shutDown").d("reason", "failedToCreatePingHandler"));
                            setState(State::SHUTDOWN, ConnectionStatusObserverInterface::ChangedReason::PING_TIMEDOUT);
                        }
                        lock.lock();
                    } else {
                        ACSDK_DEBUG7(LX_P("m_pingHandler != nullptr"));
                    }
                }
            }
            return m_state;
        }
        bool HTTP2Transport::setState(State newState, ConnectionStatusObserverInterface::ChangedReason changedReason) {
            std::lock_guard<std::mutex> lock(m_mutex);
            return setStateLocked(newState, changedReason);
        }
        bool HTTP2Transport::setStateLocked(State newState, ConnectionStatusObserverInterface::ChangedReason changedReason) {
            ACSDK_INFO(LX_P(__func__).d("currentState", m_state).d("newState", newState).d("changedReason", changedReason));
            if (newState == m_state) {
                ACSDK_DEBUG7(LX_P("alreadyInNewState"));
                return true;
            }
            bool allowed = false;
            switch(newState) {
                case State::INIT: allowed = false; break;
                case State::AUTHORIZING: allowed = State::INIT == m_state || State::WAITING_TO_RETRY_CONNECTING == m_state; break;
                case State::CONNECTING: allowed = State::AUTHORIZING == m_state || State::WAITING_TO_RETRY_CONNECTING == m_state; break;
                case State::WAITING_TO_RETRY_CONNECTING: allowed = State::CONNECTING == m_state; break;
                case State::POST_CONNECTING: allowed = State::CONNECTING == m_state; break;
                case State::CONNECTED: allowed = State::POST_CONNECTING == m_state; break;
                case State::SERVER_SIDE_DISCONNECT: allowed = m_state != State::DISCONNECTING && m_state != State::SHUTDOWN; break;
                case State::DISCONNECTING: allowed = m_state != State::SHUTDOWN; break;
                case State::SHUTDOWN: allowed = true; break;
            }
            if (!allowed) {
                ACSDK_ERROR(LX_P("stateChangeNotAllowed").d("oldState", m_state).d("newState", newState));
                return false;
            }
            switch(newState) {
                case State::INIT: case State::AUTHORIZING: case State::CONNECTING: case State::WAITING_TO_RETRY_CONNECTING: case State::POST_CONNECTING:
                case State::CONNECTED:
                    break;
                case State::SERVER_SIDE_DISCONNECT: case State::DISCONNECTING: case State::SHUTDOWN:
                    if (ConnectionStatusObserverInterface::ChangedReason::NONE == m_disconnectReason) m_disconnectReason = changedReason;
                    break;
            }
            m_state = newState;
            m_wakeEvent.notify_all();
            return true;
        }
        void HTTP2Transport::notifyObserversOnConnected() {
            ACSDK_DEBUG7(LX_P(__func__));
            unique_lock<mutex> lock{m_observerMutex};
            auto observers = m_observers;
            lock.unlock();
            for (const auto& observer : observers) observer->onConnected(shared_from_this());
        }
        void HTTP2Transport::notifyObserversOnDisconnect(ConnectionStatusObserverInterface::ChangedReason reason) {
            ACSDK_DEBUG7(LX_P(__func__));
            if (m_postConnect) {
                m_postConnect->onDisconnect();
                m_postConnect.reset();
            }
            unique_lock<mutex> lock{m_observerMutex};
            auto observers = m_observers;
            lock.unlock();
            for (const auto& observer : observers) observer->onDisconnected(shared_from_this(), reason);
        }
        void HTTP2Transport::notifyObserversOnServerSideDisconnect() {
            ACSDK_DEBUG7(LX_P(__func__));
            if (m_postConnect) {
                m_postConnect->onDisconnect();
                m_postConnect.reset();
            }
            unique_lock<mutex> lock{m_observerMutex};
            auto observers = m_observers;
            lock.unlock();
            for (const auto& observer : observers) {
                observer->onServerSideDisconnect(shared_from_this());
            }
        }
        HTTP2Transport::State HTTP2Transport::getState() {
            lock_guard<mutex> lock(m_mutex);
            return m_state;
        }
    }
}