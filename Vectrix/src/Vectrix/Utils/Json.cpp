#include "Json.h"

#include <variant>

#include "Memory.h"
#include "Vectrix/Core/Log.h"

namespace Vectrix {
    JsonValue Json::load(const std::string& filePath) {
        return parse(readUTF8(filePath));
    }

    JsonValue Json::parse(const std::string& fileData) {
        size_t pos = 0;
        JsonValue v = parseValue(fileData,pos);
        while (pos < fileData.size() && std::isspace(fileData[pos])) pos++;
        if (pos != fileData.size()) {
            VC_CORE_ERROR("Unexpected trailing content at pos " + std::to_string(pos));
        }
        return v;
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
        while (std::count(number_characters.begin(),number_characters.end(),peek(src,pos))) {
            content += consume(src,pos);
        }

        return std::stod(content);
    }

    std::variant<bool,nullptr_t> Json::parseBool(const std::string& src, size_t& pos) {
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
            return {parseString(src,pos)};

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