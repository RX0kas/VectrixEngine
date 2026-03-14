#ifndef VECTRIXWORKSPACE_JSON_H
#define VECTRIXWORKSPACE_JSON_H
#include <any>
#include <cstddef>
#include <cstddef>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "Vectrix/Core/Log.h"


/**
 * @file Json.h
 * @brief JSON parsing and value representation classes
 * @ingroup utilities
 *
 * This file provides a simple JSON parser and value container.
 * It supports all standard JSON types: null, boolean, number, string, array, and object.
 */

namespace Vectrix {
    class JsonValue;

    /**
     * @brief Type alias for a JSON array (vector of JsonValue)
     */
    using JsonArray = std::vector<JsonValue>;

    /**
     * @brief Type alias for a JSON object (map of string to JsonValue)
     */
    using JsonObject = std::map<std::string, JsonValue>;

    /**
     * @class JsonValue
     * @brief Represents any JSON value (null, boolean, number, string, array, object)
     */
    class JsonValue {
    public:
        /**
         * @brief Constructs a null JSON value (default)
         */
        JsonValue() : m_data(nullptr) {}

        /**
         * @brief Constructs a JSON string value
         * @param value The string value
         */
        JsonValue(const std::string& value) : m_data(value) {}

        /**
         * @brief Constructs a JSON number value
         * @param value The double value
         */
        JsonValue(double value) : m_data(value) {}

        /**
         * @brief Constructs a JSON boolean value
         * @param value The boolean value
         * @note This constructor is explicit to avoid unintended conversions.
         */
        explicit JsonValue(bool value) : m_data(value) {}

        /**
         * @brief Constructs a JSON array value
         * @param value The array of JsonValue elements
         */
        JsonValue(const JsonArray& value) : m_data(value) {}

        /**
         * @brief Constructs a JSON object value
         * @param value The object (map of string to JsonValue)
         */
        JsonValue(const JsonObject& value) : m_data(value) {}

        /**
         * @brief Retrieves the value as a string.
         * @return The string value if the stored type is string.
         * @note If the value is not a string, an error is logged and the return value is undefined.
         */
        [[nodiscard]] std::string getString() const {
            if (std::holds_alternative<std::string>(m_data))
                return std::get<std::string>(m_data);
            VC_CORE_ERROR("JsonValue is not a string");
        }

        /**
         * @brief Checks if the stored value is of a specific type.
         * @tparam T The type to check (e.g., std::string, double, bool, nullptr_t, JsonArray, JsonObject).
         * @return true if the value holds type T, false otherwise.
         */
        template<typename T>
        [[nodiscard]] bool isType() const {
            return std::holds_alternative<T>(m_data);
        }

        /**
         * @brief Retrieves the value as a double.
         * @return The double value if the stored type is double.
         * @note If the value is not a number, an error is logged and the return value is undefined.
         */
        [[nodiscard]] double getDouble() const {
            if (std::holds_alternative<double>(m_data))
                return std::get<double>(m_data);
            VC_CORE_ERROR("JsonValue is not a double");
        }

        /**
         * @brief Retrieves the value as a boolean.
         * @return The boolean value if the stored type is bool.
         * @note If the value is not a boolean, an error is logged and the return value is undefined.
         */
        [[nodiscard]] bool getBool() const {
            if (std::holds_alternative<bool>(m_data))
                return std::get<bool>(m_data);
            VC_CORE_ERROR("JsonValue is not a boolean");
        }

        /**
         * @brief Checks if the value is null.
         * @return true if the value is null, false otherwise.
         */
        [[nodiscard]] bool isNull() const {
            return std::holds_alternative<std::nullptr_t>(m_data);
        }

        /**
         * @brief Accesses a member of a JSON object by key.
         * @param key The object key.
         * @return The JsonValue associated with the key.
         * @note If the value is not an object, an error is logged and the returned value is undefined.
         */
        JsonValue operator[](const std::string& key) {
            if (std::holds_alternative<JsonObject>(m_data))
                return std::get<JsonObject>(m_data)[key];
            VC_CORE_ERROR("JsonValue is not an object");
        }

        /**
         * @brief Accesses an element of a JSON array by index.
         * @param index The zero-based index.
         * @return The JsonValue at the specified index.
         * @note If the value is not an array, an error is logged and the returned value is undefined.
         *       If the index is out of bounds, std::vector::operator[] behavior is undefined.
         */
        JsonValue operator[](size_t index) {
            if (std::holds_alternative<JsonArray>(m_data))
                return std::get<JsonArray>(m_data)[index];
            VC_CORE_ERROR("JsonValue is not an array");
        }

        /**
         * @brief Checks if a JSON object contains a specific key.
         * @param name The key to look for.
         * @return true if the object contains the key, false otherwise.
         * @note If the value is not an object, an error is logged and the return value is undefined.
         */
        [[nodiscard]] bool contains(const std::string& name) const {
            if (std::holds_alternative<JsonObject>(m_data)) {
                const auto& o = std::get<JsonObject>(m_data);
                return o.find(name) != o.end();
            }
            VC_CORE_ERROR("JsonValue is not an object");
        }

    private:
        std::variant<std::string, double, bool, std::nullptr_t, JsonArray, JsonObject> m_data;
    };

    /**
     * @class Json
     * @brief Provides static methods for parsing JSON from strings or files.
     * @note All methods are static; no instance of JSON is needed.
     */
    class Json {
    public:
        /**
         * @brief Parses a JSON string and returns the root JsonValue.
         * @param fileData The JSON string to parse.
         * @return The root JsonValue representing the parsed JSON.
         * @note On parsing error, the behavior is undefined (likely an assertion or error log).
         */
        static JsonValue parse(const std::string& fileData);

        /**
         * @brief Loads a JSON file from disk and parses its contents.
         * @param filePath Path to the JSON file.
         * @return The root JsonValue representing the parsed JSON.
         * @note If the file cannot be opened or parsed, an error is logged.
         */
        static JsonValue load(const std::string& filePath);

    private:
        /**
         * @brief Skips whitespace and returns the next character without consuming it.
         * @param src The source string.
         * @param pos Current parsing position (updated to skip whitespace).
         * @return The next non-whitespace character.
         */
        static char peek(const std::string& src, size_t& pos) {
            while (pos < src.size() && std::isspace(src[pos])) pos++;
            return src[pos];
        }

        /**
         * @brief Consumes and returns the current character, advancing the position.
         * @param src The source string.
         * @param pos Current parsing position.
         * @return The character at the current position before advancing.
         */
        static char consume(const std::string& src, size_t& pos) {
            return src[pos++];
        }

        /**
         * @brief Parses a JSON string enclosed in double quotes.
         * @param src The source string.
         * @param pos Current parsing position (updated to after the string).
         * @return The parsed string content (without quotes, with escape sequences resolved).
         */
        static std::string parseString(const std::string& src, size_t& pos);

        /**
         * @brief Parses a JSON number.
         * @param src The source string.
         * @param pos Current parsing position (updated to after the number).
         * @return The parsed double value.
         */
        static double parseNumber(const std::string& src, size_t& pos);

        /**
         * @brief Parses a JSON boolean (true/false) or null.
         * @param src The source string.
         * @param pos Current parsing position (updated to after the token).
         * @return A variant holding either bool or nullptr_t.
         */
        static std::variant<bool, std::nullptr_t> parseBool(const std::string& src, size_t& pos);

        /**
         * @brief Parses a JSON array (e.g., [value, value, ...]).
         * @param src The source string.
         * @param pos Current parsing position (updated to after the array).
         * @return The parsed JsonArray.
         */
        static JsonArray parseArray(const std::string& src, size_t& pos);

        /**
         * @brief Parses a JSON object (e.g., {"key": value, ...}).
         * @param src The source string.
         * @param pos Current parsing position (updated to after the object).
         * @return The parsed JsonObject.
         */
        static JsonObject parseObject(const std::string& src, size_t& pos);

        /**
         * @brief Parses any JSON value (string, number, bool, null, array, object).
         * @param src The source string.
         * @param pos Current parsing position (updated to after the value).
         * @return The parsed JsonValue.
         */
        static JsonValue parseValue(const std::string& src, size_t& pos);

        /** @brief Characters allowed in a JSON number (including sign and exponent) */
        static constexpr std::string_view number_characters = "0123456789-+e.";
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_JSON_H