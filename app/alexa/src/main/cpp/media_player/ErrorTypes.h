#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_ERRORTYPES_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_ERRORTYPES_H_

#include <ostream>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace mediaPlayer {
                enum class ErrorType {
                    NONE,
                    MEDIA_ERROR_UNKNOWN,
                    MEDIA_ERROR_INVALID_REQUEST,
                    MEDIA_ERROR_SERVICE_UNAVAILABLE,
                    MEDIA_ERROR_INTERNAL_SERVER_ERROR,
                    MEDIA_ERROR_INTERNAL_DEVICE_ERROR,
                    MEDIA_ERROR_HTTP_BAD_REQUEST,
                    MEDIA_ERROR_HTTP_UNAUTHORIZED,
                    MEDIA_ERROR_HTTP_FORBIDDEN,
                    MEDIA_ERROR_HTTP_NOT_FOUND,
                    MEDIA_ERROR_HTTP_CONFLICT,
                    MEDIA_ERROR_HTTP_INVALID_RANGE,
                    MEDIA_ERROR_HTTP_TOO_MANY_REQUESTS,
                    MEDIA_ERROR_HTTP_BAD_GATEWAY,
                    MEDIA_ERROR_HTTP_SERVICE_UNAVAILABLE,
                    MEDIA_ERROR_HTTP_GATEWAY_TIMEOUT,
                    MEDIA_ERROR_TIMED_OUT,
                    MEDIA_ERROR_URL_MALFORMED,
                    MEDIA_ERROR_COULD_NOT_RESOLVE_HOST,
                    MEDIA_ERROR_COULD_NOT_CONNECT,
                    MEDIA_ERROR_NOT_SEEKABLE,
                    MEDIA_ERROR_UNSUPPORTED,
                    MEDIA_ERROR_POSSIBLY_TEXT,
                    MEDIA_ERROR_IO,
                    MEDIA_ERROR_INVALID_COMMAND,
                    MEDIA_ERROR_PLAYLIST_ERROR

                };
                inline std::string_view errorTypeToString(ErrorType errorType) {
                    switch (errorType) {
                        case ErrorType::NONE: return "NONE";
                        case ErrorType::MEDIA_ERROR_UNKNOWN: return "MEDIA_ERROR_UNKNOWN";
                        case ErrorType::MEDIA_ERROR_INVALID_REQUEST: return "MEDIA_ERROR_INVALID_REQUEST";
                        case ErrorType::MEDIA_ERROR_SERVICE_UNAVAILABLE: return "MEDIA_ERROR_SERVICE_UNAVAILABLE";
                        case ErrorType::MEDIA_ERROR_INTERNAL_SERVER_ERROR: return "MEDIA_ERROR_INTERNAL_SERVER_ERROR";
                        case ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR: return "MEDIA_ERROR_INTERNAL_DEVICE_ERROR";
                        case ErrorType::MEDIA_ERROR_HTTP_BAD_REQUEST: return "MEDIA_ERROR_HTTP_BAD_REQUEST";
                        case ErrorType::MEDIA_ERROR_HTTP_UNAUTHORIZED: return "MEDIA_ERROR_HTTP_UNAUTHORIZED";
                        case ErrorType::MEDIA_ERROR_HTTP_FORBIDDEN: return "MEDIA_ERROR_HTTP_FORBIDDEN";
                        case ErrorType::MEDIA_ERROR_HTTP_NOT_FOUND: return "MEDIA_ERROR_HTTP_NOT_FOUND";
                        case ErrorType::MEDIA_ERROR_HTTP_CONFLICT: return "MEDIA_ERROR_HTTP_CONFLICT";
                        case ErrorType::MEDIA_ERROR_HTTP_INVALID_RANGE: return "MEDIA_ERROR_HTTP_INVALID_RANGE";
                        case ErrorType::MEDIA_ERROR_HTTP_TOO_MANY_REQUESTS: return "MEDIA_ERROR_HTTP_TOO_MANY_REQUESTS";
                        case ErrorType::MEDIA_ERROR_HTTP_BAD_GATEWAY: return "MEDIA_ERROR_HTTP_BAD_GATEWAY";
                        case ErrorType::MEDIA_ERROR_HTTP_SERVICE_UNAVAILABLE: return "MEDIA_ERROR_HTTP_SERVICE_UNAVAILABLE";
                        case ErrorType::MEDIA_ERROR_HTTP_GATEWAY_TIMEOUT: return "MEDIA_ERROR_HTTP_GATEWAY_TIMEOUT";
                        case ErrorType::MEDIA_ERROR_TIMED_OUT: return "MEDIA_ERROR_TIMED_OUT";
                        case ErrorType::MEDIA_ERROR_URL_MALFORMED: return "MEDIA_ERROR_URL_MALFORMED";
                        case ErrorType::MEDIA_ERROR_COULD_NOT_RESOLVE_HOST: return "MEDIA_ERROR_COULD_NOT_RESOLVE_HOST";
                        case ErrorType::MEDIA_ERROR_COULD_NOT_CONNECT: return "MEDIA_ERROR_COULD_NOT_CONNECT";
                        case ErrorType::MEDIA_ERROR_NOT_SEEKABLE: return "MEDIA_ERROR_NOT_SEEKABLE";
                        case ErrorType::MEDIA_ERROR_UNSUPPORTED: return "MEDIA_ERROR_UNSUPPORTED";
                        case ErrorType::MEDIA_ERROR_POSSIBLY_TEXT: return "MEDIA_ERROR_TEXT_PLAYBACK";
                        case ErrorType::MEDIA_ERROR_IO: return "MEDIA_ERROR_IO";
                        case ErrorType::MEDIA_ERROR_INVALID_COMMAND: return "MEDIA_ERROR_INVALID_COMMAND";
                        case ErrorType::MEDIA_ERROR_PLAYLIST_ERROR: return "MEDIA_ERROR_PLAYLIST_ERROR";
                    }
                    return "unknown ErrorType";
                }
                inline std::ostream& operator<<(std::ostream& stream, const ErrorType& errorType) {
                    return stream << errorTypeToString(errorType);
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_MEDIAPLAYER_ERRORTYPES_H_
