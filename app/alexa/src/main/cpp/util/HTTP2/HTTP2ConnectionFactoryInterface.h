#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2CONNECTIONFACTORYINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2CONNECTIONFACTORYINTERFACE_H_

#include <memory>
#include "HTTP2ConnectionInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                class HTTP2ConnectionFactoryInterface {
                public:
                    virtual ~HTTP2ConnectionFactoryInterface() = default;
                    virtual std::shared_ptr<HTTP2ConnectionInterface> createHTTP2Connection() = 0;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2CONNECTIONFACTORYINTERFACE_H_
