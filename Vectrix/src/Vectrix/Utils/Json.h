#ifndef VECTRIXWORKSPACE_JSON_H
#define VECTRIXWORKSPACE_JSON_H
#include <any>
#include <cstddef>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "Vectrix/Core/Log.h"


namespace Vectrix {
    class JsonValue;

    using JsonArray = std::vector<JsonValue>;
    using JsonObject = std::map<std::string, JsonValue>;

    class JsonValue {
    public:
        JsonValue() : m_data(nullptr) {}
        JsonValue(const std::string& value) : m_data(value) {}
        JsonValue(double value) : m_data(value) {}
        explicit JsonValue(bool value) : m_data(value) {}
        JsonValue(const JsonArray& value) : m_data(value) {}
        JsonValue(const JsonObject& value) : m_data(value) {}

        [[nodiscard]] std::string getString() const {
            if (std::holds_alternative<std::string>(m_data))
                return std::get<std::string>(m_data);
            VC_CORE_ERROR("JsonValue is not a string");
        }

        template<typename T>
        [[nodiscard]] bool isType() const {
            return std::holds_alternative<T>(m_data);
        }

        [[nodiscard]] double getDouble() const {
            if (std::holds_alternative<double>(m_data))
                return std::get<double>(m_data);
            VC_CORE_ERROR("JsonValue is not a double");
        }

        [[nodiscard]] bool getBool() const {
            if (std::holds_alternative<bool>(m_data))
                return std::get<bool>(m_data);
            VC_CORE_ERROR("JsonValue is not a boolean");
        }

        [[nodiscard]] bool isNull() const {
            return std::holds_alternative<nullptr_t>(m_data);
        }

        JsonValue operator[](const std::string& key) {
            if (std::holds_alternative<JsonObject>(m_data))
                return std::get<JsonObject>(m_data)[key];
            VC_CORE_ERROR("JsonValue is not an object");
        }
        JsonValue operator[](size_t index) {
            if (std::holds_alternative<JsonArray>(m_data))
                return std::get<JsonArray>(m_data)[index];
            VC_CORE_ERROR("JsonValue is not an array");
        }

        [[nodiscard]] bool contains(const std::string& name) const {
            if (std::holds_alternative<JsonObject>(m_data)) {
                const auto& o = std::get<JsonObject>(m_data);
                return o.find(name)!=o.end();
            }
            VC_CORE_ERROR("JsonValue is not an object");
        }
    private:
        std::variant<std::string, double, bool, nullptr_t, JsonArray, JsonObject> m_data;
    };

    class Json {
    public:
        static JsonValue parse(const std::string& fileData);
        static JsonValue load(const std::string& filePath);
    private:
        static char peek(const std::string& src, size_t& pos) {
            while (pos < src.size() && std::isspace(src[pos])) pos++;
            return src[pos];
        }

        static char consume(const std::string& src, size_t& pos) {
            return src[pos++];
        }
        static std::string parseString(const std::string& src, size_t& pos);
        static double parseNumber(const std::string& src, size_t& pos);
        static std::variant<bool,nullptr_t> parseBool(const std::string& src, size_t& pos);
        static JsonArray parseArray(const std::string& src, size_t& pos);
        static JsonObject parseObject(const std::string& src, size_t& pos);
        static JsonValue parseValue(const std::string& src, size_t& pos);
        static constexpr std::string_view number_characters = "0123456789-+e.";
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_JSON_H