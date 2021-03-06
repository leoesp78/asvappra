#include "LogEntry.h"
#include <cstring>
#include <iomanip>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                static const char* RESERVED_METADATA_CHARS = R"(\,=:)";
                static const std::string ESCAPED_METADATA_ESCAPE = R"(\\)";
                static const std::string ESCAPED_PAIR_SEPARATOR = R"(\,)";
                static const std::string ESCAPED_SECTION_SEPARATOR = R"(\:)";
                static const std::string ESCAPED_KEY_VALUE_SEPARATOR = R"(\=)";
                static const char METADATA_ESCAPE = '\\';
                static const char PAIR_SEPARATOR = ',';
                static const char SECTION_SEPARATOR = ':';
                static const std::string BOOL_TRUE = "true";
                static const std::string BOOL_FALSE = "false";
                LogEntry::LogEntry(const char *source, const char* event) : m_hasMetadata(false) {
                    m_stream << source << SECTION_SEPARATOR;
                    if (event) m_stream << event;
                }
                LogEntry::LogEntry(const std::string& source, const std::string& event) : m_hasMetadata(false) {
                    m_stream << source << SECTION_SEPARATOR << event;
                }
                LogEntry& LogEntry::d(const std::string& key, const char* value) {
                    return d(key.c_str(), value);
                }
                LogEntry& LogEntry::d(const char* key, char* value) {
                    return d(key, static_cast<const char*>(value));
                }
                LogEntry& LogEntry::d(const char* key, const char* value) {
                    prefixKeyValuePair();
                    if (!key) key = "";
                    m_stream << key << KEY_VALUE_SEPARATOR;
                    appendEscapedString(value);
                    return *this;
                }
                LogEntry& LogEntry::d(const std::string& key, const std::string& value) {
                    return d(key.c_str(), value.c_str());
                }
                LogEntry& LogEntry::d(const char* key, const std::string& value) {
                    return d(key, value.c_str());
                }
                LogEntry& LogEntry::d(const std::string& key, bool value) {
                    return d(key.c_str(), value);
                }
                LogEntry& LogEntry::d(const char* key, bool value) {
                    return d(key, value ? BOOL_TRUE : BOOL_FALSE);
                }
                LogEntry& LogEntry::m(const char* message) {
                    prefixMessage();
                    if (message) m_stream << message;
                    return *this;
                }
                LogEntry& LogEntry::m(const std::string& message) {
                    prefixMessage();
                    m_stream << message;
                    return *this;
                }
                LogEntry& LogEntry::p(const char* key, void* ptr) {
                    return d(key, ptr);
                }
                const char* LogEntry::c_str() const {
                    return m_stream.c_str();
                }
                void LogEntry::prefixKeyValuePair() {
                    if (m_hasMetadata) m_stream << PAIR_SEPARATOR;
                    else {
                        m_stream << SECTION_SEPARATOR;
                        m_hasMetadata = true;
                    }
                }
                void LogEntry::prefixMessage() {
                    if (!m_hasMetadata) m_stream << SECTION_SEPARATOR;
                    m_stream << SECTION_SEPARATOR;
                }
                void LogEntry::appendEscapedString(const char* in) {
                    if (in == NULL) return;
                    auto pos = in;
                    auto maxCount = strlen(in);
                    while (maxCount-- > 0 && *pos != 0) {
                        auto next = strpbrk(pos, RESERVED_METADATA_CHARS);
                        if (next) {
                            m_stream.write(pos, next - pos);
                            switch (*next) {
                                case METADATA_ESCAPE: m_stream << ESCAPED_METADATA_ESCAPE; break;
                                case PAIR_SEPARATOR: m_stream << ESCAPED_PAIR_SEPARATOR; break;
                                case SECTION_SEPARATOR: m_stream << ESCAPED_SECTION_SEPARATOR; break;
                                case KEY_VALUE_SEPARATOR: m_stream << ESCAPED_KEY_VALUE_SEPARATOR; break;
                            }
                            pos = next + 1;
                        } else {
                            m_stream << pos;
                            return;
                        }
                    }
                }
            }
        }
    }
}
