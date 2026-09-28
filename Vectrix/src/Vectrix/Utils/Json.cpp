#include "Json.h"

#include <charconv>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <variant>

#include "Vectrix/Core/Log.h"

namespace Vectrix {
    namespace {
        /// Nesting depth past which a document is rejected instead of risking a stack overflow
        constexpr size_t MAX_JSON_DEPTH = 256;

        bool isJsonSpace(const char c) {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r';
        }

        int hexValue(const char c) {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        }

        void appendUTF8(std::string& out, const uint32_t codePoint) {
            if (codePoint < 0x80) {
                out += static_cast<char>(codePoint);
            } else if (codePoint < 0x800) {
                out += static_cast<char>(0xC0 | (codePoint >> 6));
                out += static_cast<char>(0x80 | (codePoint & 0x3F));
            } else if (codePoint < 0x10000) {
                out += static_cast<char>(0xE0 | (codePoint >> 12));
                out += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
                out += static_cast<char>(0x80 | (codePoint & 0x3F));
            } else {
                out += static_cast<char>(0xF0 | (codePoint >> 18));
                out += static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
                out += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
                out += static_cast<char>(0x80 | (codePoint & 0x3F));
            }
        }

        /**
         * Recursive descent parser over a whole document. Every read is bounds checked and any
         * malformed input stops the parse with a message, rather than yielding a partial value.
         */
        class Parser {
        public:
            explicit Parser(const std::string& src) : m_src(src) {}

            bool parseDocument(JsonValue& out) {
                if (m_src.compare(0, 3, "\xEF\xBB\xBF") == 0)
                    m_pos = 3; // UTF-8 byte order mark, as some Windows editors write
                if (!parseValue(out, 0)) return false;
                skipWhitespace();
                if (!atEnd()) return fail("Unexpected trailing content");
                return true;
            }

            [[nodiscard]] const std::string& error() const { return m_error; }

        private:
            const std::string& m_src;
            size_t m_pos = 0;
            std::string m_error;

            bool fail(const std::string& message) {
                if (m_error.empty())
                    m_error = message + " at position " + std::to_string(m_pos);
                return false;
            }

            [[nodiscard]] bool atEnd() const { return m_pos >= m_src.size(); }

            void skipWhitespace() {
                while (!atEnd() && isJsonSpace(m_src[m_pos])) m_pos++;
            }

            bool expect(const char c) {
                skipWhitespace();
                if (atEnd() || m_src[m_pos] != c)
                    return fail(std::string("Expected '") + c + "'");
                m_pos++;
                return true;
            }

            bool parseValue(JsonValue& out, const size_t depth) {
                if (depth > MAX_JSON_DEPTH) return fail("Nesting too deep");
                skipWhitespace();
                if (atEnd()) return fail("Unexpected end of input");

                switch (m_src[m_pos]) {
                    case '"': {
                        std::string content;
                        if (!parseString(content)) return false;
                        out = JsonValue(std::move(content));
                        return true;
                    }
                    case '{': return parseObject(out, depth);
                    case '[': return parseArray(out, depth);
                    case 't': return parseLiteral("true", JsonValue(true), out);
                    case 'f': return parseLiteral("false", JsonValue(false), out);
                    case 'n': return parseLiteral("null", JsonValue(), out);
                    default:  return parseNumber(out);
                }
            }

            bool parseLiteral(const std::string_view word, JsonValue value, JsonValue& out) {
                if (m_src.compare(m_pos, word.size(), word) != 0)
                    return fail("Unexpected token");
                m_pos += word.size();
                out = std::move(value);
                return true;
            }

            bool parseNumber(JsonValue& out) {
                const size_t start = m_pos;
                while (!atEnd()) {
                    const char c = m_src[m_pos];
                    if (!((c >= '0' && c <= '9') || c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E'))
                        break;
                    m_pos++;
                }
                if (start == m_pos) return fail("Unexpected character");

                // from_chars ignores the C locale: strtod/stod would read "0.5" as 0 under a locale whose
                // decimal separator is a comma (e.g. fr_FR, which GTK installs on Linux once a file dialog opened)
                double value = 0.0;
                const char* last = m_src.data() + m_pos;
                const auto [ptr, ec] = std::from_chars(m_src.data() + start, last, value);
                if (ec != std::errc() || ptr != last) {
                    m_pos = start;
                    return fail("Invalid number");
                }
                out = JsonValue(value);
                return true;
            }

            bool parseHex4(uint32_t& out) {
                if (m_src.size() - m_pos < 4) return fail("Truncated \\u escape");
                out = 0;
                for (int i = 0; i < 4; ++i) {
                    const int digit = hexValue(m_src[m_pos++]);
                    if (digit < 0) return fail("Invalid \\u escape");
                    out = (out << 4) | static_cast<uint32_t>(digit);
                }
                return true;
            }

            bool parseString(std::string& out) {
                if (!expect('"')) return false;
                while (true) {
                    if (atEnd()) return fail("Unterminated string");
                    const char c = m_src[m_pos++];
                    if (c == '"') return true;
                    if (c != '\\') {
                        out += c;
                        continue;
                    }

                    if (atEnd()) return fail("Unterminated string");
                    switch (m_src[m_pos++]) {
                        case '"':  out += '"';  break;
                        case '\\': out += '\\'; break;
                        case '/':  out += '/';  break;
                        case 'b':  out += '\b'; break;
                        case 'f':  out += '\f'; break;
                        case 'n':  out += '\n'; break;
                        case 'r':  out += '\r'; break;
                        case 't':  out += '\t'; break;
                        case 'u': {
                            uint32_t codePoint = 0;
                            if (!parseHex4(codePoint)) return false;
                            // A code point above U+FFFF is spelled as a UTF-16 surrogate pair, over two escapes
                            if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {
                                if (m_src.compare(m_pos, 2, "\\u") != 0) return fail("Unpaired surrogate");
                                m_pos += 2;
                                uint32_t low = 0;
                                if (!parseHex4(low)) return false;
                                if (low < 0xDC00 || low > 0xDFFF) return fail("Invalid surrogate pair");
                                codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (low - 0xDC00);
                            }
                            appendUTF8(out, codePoint);
                            break;
                        }
                        default:
                            m_pos--;
                            return fail("Invalid escape sequence");
                    }
                }
            }

            bool parseArray(JsonValue& out, const size_t depth) {
                m_pos++; // '['
                JsonArray array;
                skipWhitespace();
                if (!atEnd() && m_src[m_pos] == ']') {
                    m_pos++;
                    out = JsonValue(std::move(array));
                    return true;
                }

                while (true) {
                    JsonValue element;
                    if (!parseValue(element, depth + 1)) return false;
                    array.push_back(std::move(element));

                    skipWhitespace();
                    if (atEnd()) return fail("Unterminated array");
                    const char c = m_src[m_pos++];
                    if (c == ']') break;
                    if (c != ',') {
                        m_pos--;
                        return fail("Expected ',' or ']'");
                    }
                }
                out = JsonValue(std::move(array));
                return true;
            }

            bool parseObject(JsonValue& out, const size_t depth) {
                m_pos++; // '{'
                JsonObject object;
                skipWhitespace();
                if (!atEnd() && m_src[m_pos] == '}') {
                    m_pos++;
                    out = JsonValue(std::move(object));
                    return true;
                }

                while (true) {
                    std::string key;
                    if (!parseString(key)) return false;
                    if (!expect(':')) return false;
                    JsonValue value;
                    if (!parseValue(value, depth + 1)) return false;
                    object.emplace(std::move(key), std::move(value)); // first occurrence of a duplicate key wins

                    skipWhitespace();
                    if (atEnd()) return fail("Unterminated object");
                    const char c = m_src[m_pos++];
                    if (c == '}') break;
                    if (c != ',') {
                        m_pos--;
                        return fail("Expected ',' or '}'");
                    }
                }
                out = JsonValue(std::move(object));
                return true;
            }
        };
    }

    std::pair<VectrixResult,JsonValue> Json::load(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open())
            return {NOT_FOUND, JsonValue("Can't open " + filePath)};

        std::stringstream buffer;
        buffer << file.rdbuf();
        return parse(buffer.str());
    }

    VectrixResult Json::save(const std::string& filePath, const JsonObject& object) {
        std::string out;
        writeValue(JsonValue(object), out, 0);
        out += '\n';

        // Reported through the return value, which the callers show to the user: not an abort
        std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            VC_CORE_ERROR_NO_EXIT("Can't open for writing: {}", filePath);
            return UNKNOWN_ERROR;
        }
        file << out;
        if (!file.good()) {
            VC_CORE_ERROR_NO_EXIT("Failed while writing: {}", filePath);
            return UNKNOWN_ERROR;
        }
        return SUCCESS;
    }

    void Json::writeString(const std::string& str, std::string& out) {
        out += '"';
        for (const char c : str) {
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\b': out += "\\b";  break;
                case '\f': out += "\\f";  break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        char buffer[8];
                        std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned char>(c));
                        out += buffer;
                    } else {
                        out += c;
                    }
            }
        }
        out += '"';
    }

    void Json::writeNumber(double value, std::string& out) {
        if (!std::isfinite(value)) {
            // JSON has no way to spell NaN or infinity.
            out += "null";
            return;
        }
        // Whole numbers within the exact range of a double are written without a fractional part.
        if (value == std::floor(value) && std::abs(value) < 1e15) {
            out += std::to_string(static_cast<long long>(value));
            return;
        }
        // to_chars ignores the C locale (snprintf("%g") would write "0,5" under fr_FR) and gives the
        // shortest text that reads back to the same double
        char buffer[32];
        const auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
        out.append(buffer, ptr);
    }

    void Json::writeValue(const JsonValue& value, std::string& out, size_t depth) {
        const std::string indent((depth + 1) * 2, ' ');
        const std::string closingIndent(depth * 2, ' ');

        if (std::holds_alternative<std::nullptr_t>(value.m_data)) {
            out += "null";
        } else if (std::holds_alternative<bool>(value.m_data)) {
            out += std::get<bool>(value.m_data) ? "true" : "false";
        } else if (std::holds_alternative<double>(value.m_data)) {
            writeNumber(std::get<double>(value.m_data), out);
        } else if (std::holds_alternative<std::string>(value.m_data)) {
            writeString(std::get<std::string>(value.m_data), out);
        } else if (std::holds_alternative<JsonArray>(value.m_data)) {
            const auto& arr = std::get<JsonArray>(value.m_data);
            if (arr.empty()) {
                out += "[]";
                return;
            }
            out += "[\n";
            for (size_t i = 0; i < arr.size(); ++i) {
                out += indent;
                writeValue(arr[i], out, depth + 1);
                if (i + 1 < arr.size()) out += ',';
                out += '\n';
            }
            out += closingIndent;
            out += ']';
        } else if (std::holds_alternative<JsonObject>(value.m_data)) {
            const auto& obj = std::get<JsonObject>(value.m_data);
            if (obj.empty()) {
                out += "{}";
                return;
            }
            out += "{\n";
            size_t written = 0;
            for (const auto& [key, val] : obj) {
                out += indent;
                writeString(key, out);
                out += ": ";
                writeValue(val, out, depth + 1);
                if (++written < obj.size()) out += ',';
                out += '\n';
            }
            out += closingIndent;
            out += '}';
        }
    }

    std::pair<VectrixResult,JsonValue> Json::parse(const std::string& fileData) {
        Parser parser(fileData);
        JsonValue root;
        if (!parser.parseDocument(root))
            return {FORMATING_ERROR, JsonValue(parser.error())};
        return {SUCCESS, std::move(root)};
    }
} // Vectrix
