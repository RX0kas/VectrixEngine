#include "Json.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <variant>

#include "Memory.h"
#include "Vectrix/Core/Log.h"

namespace Vectrix {
    std::pair<VectrixResult,JsonValue> Json::load(const std::string& filePath) {
        return parse(readUTF8(filePath));
    }

    VectrixResult Json::save(const std::string& filePath, const JsonObject& object) {
        std::string out;
        writeValue(JsonValue(object), out, 0);
        out += '\n';

        std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            VC_CORE_ERROR("Can't open for writing: {}", filePath);
            return UNKNOWN_ERROR;
        }
        file << out;
        if (!file.good()) {
            VC_CORE_ERROR("Failed while writing: {}", filePath);
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
        // Grow the precision until the text parses back to the same double.
        char buffer[32];
        for (int precision = 6; precision <= 17; ++precision) {
            std::snprintf(buffer, sizeof(buffer), "%.*g", precision, value);
            if (std::strtod(buffer, nullptr) == value)
                break;
        }
        out += buffer;
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
        size_t pos = 0;
        JsonValue v = parseValue(fileData,pos);
        while (pos < fileData.size() && std::isspace(fileData[pos])) pos++;
        if (pos != fileData.size()) {
            return {FORMATING_ERROR, JsonValue("Unexpected trailing content at pos " + std::to_string(pos))};
        }
        return {SUCCESS, v};
    }

    std::string Json::parseString(const std::string& src, size_t& pos) {
        std::string content;
        consume(src,pos);
        while (peek(src,pos) != '"') {
            content += consume(src,pos);
        }
        consume(src,pos);
        return content;
    }

    double Json::parseNumber(const std::string& src, size_t& pos) {
        std::string content;
        while (std::ranges::count(number_characters,peek(src,pos))) {
            content += consume(src,pos);
        }

        return std::stod(content);
    }

    std::variant<bool,std::nullptr_t> Json::parseBool(const std::string& src, size_t& pos) {
        const auto word = src.substr(pos, 4);
        if (word=="true") {
            pos+=4;
            return true;
        }
        if (word=="null") {
            pos+=4;
            return nullptr;
        }
        if (src.substr(pos, 5)=="false") {
            pos+=5;
            return false;
        }
        VC_CORE_ERROR("Unexpected token at pos {}",pos);
    }

    JsonArray Json::parseArray(const std::string& src, size_t& pos) {
        JsonArray arr{};
        consume(src,pos);

        bool done = false;
        while (!done) {
            JsonValue val = parseValue(src,pos);

            if (peek(src,pos)!=',') {
                done = true;
            } else {
                consume(src,pos);
            }
            arr.push_back(val);
        }

        if (peek(src,pos) == ']') consume(src,pos);
        return arr;
    }

    JsonObject Json::parseObject(const std::string& src, size_t& pos) {
        JsonObject obj{};

        consume(src,pos);

        bool done = false;
        while (!done) {
            std::string name = parseString(src,pos);
            consume(src,pos);

            JsonValue val = parseValue(src,pos);

            if (peek(src,pos)!=',') {
                done = true;
            } else {
                consume(src,pos);
            }
            obj.emplace(name,val);
        }

        if (peek(src,pos) == '}') consume(src,pos);
        return obj;
    }

    JsonValue Json::parseValue(const std::string& src, size_t& pos) {
        const char next = peek(src,pos);

        if (next=='"') // String
            return JsonValue(parseString(src,pos));

        if (std::isdigit(next) || next == '-') // Number
            return parseNumber(src,pos);

        if (next=='{') // Object
            return parseObject(src,pos);

        if (next=='t' || next == 'f' || next == 'n') {
            // Bool or null
            auto b = parseBool(src,pos);
            if (std::holds_alternative<bool>(b))
                return std::get<bool>(b);

            return {};
        }
        if (next=='[')  // Array
            return parseArray(src,pos);

        VC_CORE_WARN("Can't parse value at pos {}",pos);
        return {};
    }
} // Vectrix