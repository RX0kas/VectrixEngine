#ifndef VECTRIXWORKSPACE_JSON_H
#define VECTRIXWORKSPACE_JSON_H
#include <any>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "Result.h"
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
        explicit JsonValue(const std::string& value) : m_data(value) {}

        /**
         * @brief Constructs a JSON string value from a C string
         * @param value The null-terminated string
         * @note Non-explicit on purpose, so a string literal binds here instead of
         *       collapsing to the bool constructor.
         */
        JsonValue(const char* value) : m_data(std::string(value)) {}

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
        JsonValue(bool value) : m_data(value) {}

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
            return {};
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
            return 0.0;
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
            return false;
        }

        /**
         * @brief Retrieves the value as @p T, if it holds a matching type.
         * @tparam T Either a stored alternative (std::string, bool, JsonArray, JsonObject,
         *           std::nullptr_t) or any arithmetic type. JSON numbers are stored as
         *           double, so an arithmetic T is read from that double and cast to T.
         * @return The value, or std::nullopt when the stored type does not match.
         */
        template<typename T>
        [[nodiscard]] std::optional<T> getAs() const {
            if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>) {
                if (std::holds_alternative<double>(m_data))
                    return static_cast<T>(std::get<double>(m_data));
                return std::nullopt;
            } else {
                if (std::holds_alternative<T>(m_data))
                    return std::get<T>(m_data);
                return std::nullopt;
            }
        }

        /**
         * @brief Checks if the value is null.
         * @return true if the value is null, false otherwise.
         */
        [[nodiscard]] bool isNull() const {
            return std::holds_alternative<std::nullptr_t>(m_data);
        }

        /**
         * @brief Number of elements held by an array, or members held by an object.
         * @return The element/member count, or 0 for any scalar or null value.
         */
        [[nodiscard]] size_t size() const {
            if (const auto* array = std::get_if<JsonArray>(&m_data))
                return array->size();
            if (const auto* object = std::get_if<JsonObject>(&m_data))
                return object->size();
            return 0;
        }

        /**
         * @brief Accesses or creates a member of a JSON object by key.
         * @param key The object key.
         * @return A reference to the value stored under @p key, which can be assigned to.
         *         A null value is turned into an empty object first, and a missing key is
         *         inserted with a null value. Use contains() to probe without inserting.
         * @note If the value holds some other type an error is logged and a reference to a
         *       shared throwaway value is returned.
         */
        JsonValue& operator[](const std::string& key) {
            if (std::holds_alternative<std::nullptr_t>(m_data))
                m_data = JsonObject{};
            if (auto* object = std::get_if<JsonObject>(&m_data))
                return (*object)[key];
            VC_CORE_ERROR("JsonValue is not an object");
            return nullSink();
        }

        /**
         * @brief Reads a member of a JSON object by key without altering the value.
         * @param key The object key.
         * @return The value stored under @p key, or a null value when the key is missing
         *         or the value is not an object.
         */
        const JsonValue& operator[](const std::string& key) const {
            if (const auto* object = std::get_if<JsonObject>(&m_data)) {
                const auto it = object->find(key);
                return it != object->end() ? it->second : nullValue();
            }
            return nullValue();
        }

        /**
         * @brief Accesses an element of a JSON array by index.
         * @param index The zero-based index.
         * @return A reference to the element, which can be assigned to.
         * @note If the value is not an array, or the index is out of range, an error is
         *       logged and a reference to a shared throwaway value is returned.
         */
        JsonValue& operator[](size_t index) {
            if (auto* array = std::get_if<JsonArray>(&m_data)) {
                if (index < array->size())
                    return (*array)[index];
                VC_CORE_ERROR("JsonValue array index {} is out of range", index);
                return nullSink();
            }
            VC_CORE_ERROR("JsonValue is not an array");
            return nullSink();
        }

        /**
         * @brief Reads an element of a JSON array by index without altering the value.
         * @param index The zero-based index.
         * @return The element, or a null value when the index is out of range or the
         *         value is not an array.
         */
        const JsonValue& operator[](size_t index) const {
            if (const auto* array = std::get_if<JsonArray>(&m_data)) {
                if (index < array->size())
                    return (*array)[index];
                return nullValue();
            }
            return nullValue();
        }

        /**
         * @brief Stores a value under a key, creating the object when the value is null.
         * @param key The object key.
         * @param value The value to store.
         */
        void set(const std::string& key, JsonValue value) {
            (*this)[key] = std::move(value);
        }

        /**
         * @brief Appends a value, turning a null value into an array first.
         * @param value The value to append.
         * @note If the value holds some other type an error is logged and nothing is added.
         */
        void pushBack(JsonValue value) {
            if (std::holds_alternative<std::nullptr_t>(m_data))
                m_data = JsonArray{};
            if (auto* array = std::get_if<JsonArray>(&m_data)) {
                array->push_back(std::move(value));
                return;
            }
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
                return o.contains(name);
            }
            VC_CORE_ERROR("JsonValue is not an object");
            return false;
        }

        [[nodiscard]] JsonObject getAsObject() const {
            if (std::holds_alternative<JsonObject>(m_data)) {
                return std::get<JsonObject>(m_data);
            }
            VC_CORE_ERROR("JsonValue is not an object");
            return {};
        }

        /**
         * @brief Returns the underlying object by reference, for in-place editing.
         * @return A reference to the stored object. A null value becomes an empty object
         *         first; any other type logs an error and yields a shared empty object.
         */
        JsonObject& asObject() {
            if (std::holds_alternative<std::nullptr_t>(m_data))
                m_data = JsonObject{};
            if (auto* object = std::get_if<JsonObject>(&m_data))
                return *object;
            VC_CORE_ERROR("JsonValue is not an object");
            static JsonObject s_discard;
            s_discard.clear();
            return s_discard;
        }

        /**
         * @brief Returns the underlying array by reference, for in-place editing.
         * @return A reference to the stored array. A null value becomes an empty array
         *         first; any other type logs an error and yields a shared empty array.
         */
        JsonArray& asArray() {
            if (std::holds_alternative<std::nullptr_t>(m_data))
                m_data = JsonArray{};
            if (auto* array = std::get_if<JsonArray>(&m_data))
                return *array;
            VC_CORE_ERROR("JsonValue is not an array");
            static JsonArray s_discard;
            s_discard.clear();
            return s_discard;
        }

    private:
        /**
         * @brief Shared writable value handed back when a mutating accessor is misused.
         * @note Reset to null on each call; writes to it are intentionally discarded.
         */
        static JsonValue& nullSink() {
            static JsonValue s_sink;
            s_sink = JsonValue{};
            return s_sink;
        }

        /**
         * @brief Shared null value handed back when a const accessor misses or is misused.
         */
        static const JsonValue& nullValue() {
            static const JsonValue s_null;
            return s_null;
        }

        std::variant<std::string, double, bool, std::nullptr_t, JsonArray, JsonObject> m_data;

        friend class Json;
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
        [[nodiscard]] static std::pair<VectrixResult,JsonValue> parse(const std::string& fileData);

        /**
         * @brief Loads a JSON file from disk and parses its contents.
         * @param filePath Path to the JSON file.
         * @return The root JsonValue representing the parsed JSON.
         * @note If the file cannot be opened or parsed, an error is logged.
         */
        [[nodiscard]] static std::pair<VectrixResult,JsonValue> load(const std::string& filePath);

        /**
         * @brief Serializes a JSON object and writes it to disk as pretty-printed text.
         * @param filePath Path of the file to write (created or truncated).
         * @param object The root object to serialize.
         * @return #SUCCESS when the file was written, #UNKNOWN_ERROR when it could not be opened or written.
         * @note The output uses a two-space indent. Map keys are written in sorted order.
         *       NaN and infinite numbers are written as @c null since JSON cannot represent them.
         */
        [[nodiscard]] static VectrixResult save(const std::string& filePath,const JsonObject& object);
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

        /**
         * @brief Appends the textual form of any JSON value to a buffer.
         * @param value The value to serialize.
         * @param out The buffer being built.
         * @param depth Current nesting depth, used to indent nested arrays and objects.
         */
        static void writeValue(const JsonValue& value, std::string& out, size_t depth);

        /**
         * @brief Appends a JSON string literal (quoted and escaped) to a buffer.
         * @param str The raw string content.
         * @param out The buffer being built.
         */
        static void writeString(const std::string& str, std::string& out);

        /**
         * @brief Appends a JSON number to a buffer.
         * @param value The number to write.
         * @param out The buffer being built.
         * @note Integral values are written without a fractional part; NaN and infinities become @c null.
         */
        static void writeNumber(double value, std::string& out);

        /** @brief Characters allowed in a JSON number (including sign and exponent) */
        static constexpr std::string_view number_characters = "0123456789-+e.";
    };
} // Vectrix

#endif //VECTRIXWORKSPACE_JSON_H