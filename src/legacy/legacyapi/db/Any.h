#pragma once
#include "legacyapi/db/Types.h"

#include <list>
#include <map>
#include <set>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#pragma region AnyConversion

namespace DB {
// Declare Any class
class Any;
} // namespace DB

/**
 * @brief Function to convert Any to T.
 *
 * @tparam T The type to convert to
 * @param v  The Any object
 * @return T The converted value
 */
template <typename T>
inline T any_to(const DB::Any& v) {
    throw std::bad_cast();
}

template <typename T>
inline std::vector<DB::Any> to_any_container(const std::vector<T>& v);
template <typename T>
inline std::set<DB::Any> to_any_container(const std::set<T>& v);
template <typename T>
inline std::list<DB::Any> to_any_container(const std::list<T>& v);
template <typename T>
inline std::unordered_set<DB::Any> to_any_container(const std::unordered_set<T>& v);
template <typename K, typename V>
inline std::map<K, DB::Any> to_any_container(const std::map<K, V>& v);
template <typename K, typename V>
inline std::unordered_map<K, DB::Any> to_any_unordered_map(const std::unordered_map<K, V>& v);

#pragma endregion

namespace DB {

class Any {

public:
    std::variant<bool, int64_t, uint64_t, double, std::string, Date, Time, DateTime,
                 ByteArray> value; ///< Value

    enum class Type : char {
        Null     = 0,
        Boolean  = 1,
        Integer  = 2,
        UInteger = 3,
        Floating = 4,
        String   = 5,
        Date     = 6,
        Time     = 7,
        DateTime = 8,
        Blob     = 9
    } type = Type::Null; ///< Type of the value

    /**
     * @brief Construct a new Any object with null value.
     *
     */
    Any();
    /**
     * @brief Construct a new Any object with boolean value.
     *
     * @param v The boolean value
     */
    Any(bool v);
    /**
     * @brief Construct a new Any object with int64 value.
     *
     * @param v The integer value
     */
    Any(int64_t v);
    /**
     * @brief Construct a new Any object with uint64 value.
     *
     * @param v The unsigned integer value
     */
    Any(uint64_t v);
    /**
     * @brief Construct a new Any object with double value.
     *
     * @param v The floating value
     */
    Any(double v);
    /**
     * @brief Construct a new Any object with string value.
     *
     * @param v The string value
     */
    Any(const std::string& v);
    /**
     * @brief Construct a new Any object with const char* value.
     *
     * @param v The const char* value
     */
    Any(const char* v);
    /**
     * @brief Construct a new Any object with char* value.
     *
     * @param v   The char* value
     * @param len The length of the char* value
     */
    Any(char* v, size_t len);
    /**
     * @brief Construct a new Any object with date value.
     *
     * @param v The Date object
     */
    Any(const Date& v);
    /**
     * @brief Construct a new Any object with time value.
     *
     * @param v The Time object
     */
    Any(const Time& v);
    /**
     * @brief Construct a new Any object with date time value.
     *
     * @param v The DateTime object
     */
    Any(const DateTime& v);
    /**
     * @brief Construct a new Any object with int8(char) value.
     *
     * @param v The char value
     */
    Any(char v);
    /**
     * @brief Construct a new Any object with uint8(unsigned char) value.
     *
     * @param v The unsigned char value
     */
    Any(unsigned char v);
    /**
     * @brief Construct a new Any object with int16(short) value.
     *
     * @param v The short value
     */
    Any(short v);
    /**
     * @brief Construct a new Any object with uint16(unsigned short) value.
     *
     * @param v The unsigned short value
     */
    Any(unsigned short v);
    /**
     * @brief Construct a new Any object with int32(int) value.
     *
     * @param v The int value
     */
    Any(int v);
    /**
     * @brief Construct a new Any object with uint32(unsigned int) value.
     *
     * @param v The unsigned int value
     */
    Any(unsigned int v);
    /**
     * @brief Construct a new Any object with long value.
     *
     * @param v The long value
     */
    Any(long v);
    /**
     * @brief Construct a new Any object with unsigned long value.
     *
     * @param v The unsigned long value
     */
    Any(unsigned long v);
    /**
     * @brief Construct a new Any object with float value.
     *
     * @param v The float value
     */
    Any(float v);
    /**
     * @brief Construct a new Any object with byte array value.
     *
     * @param v The byte array value
     */
    Any(const ByteArray& v);
    /// Copy constructor
    Any(const Any& v);
    /// Copy assignment operator
    Any& operator=(const Any& v);

    /**
     * @brief Get if the value is null.
     *
     */
    bool is_null() const;
    /**
     * @brief Get if the value is boolean.
     *
     */
    bool is_boolean() const;
    /**
     * @brief Get if the value is (unsigned) integer.
     *
     */
    bool is_integer() const;
    /**
     * @brief Get if the value is unsigned integer.
     *
     */
    bool is_uinteger() const;
    /**
     * @brief Get if the value is floating.
     *
     */
    bool is_floating() const;
    /**
     * @brief Get if the value is string.
     *
     */
    bool is_string() const;
    /**
     * @brief Get if the value is date.
     *
     */
    bool is_date() const;
    /**
     * @brief Get if the value is time.
     *
     */
    bool is_time() const;
    /**
     * @brief Get if the value is date time.
     *
     */
    bool is_datetime() const;
    /**
     * @brief Get if the value is blob.
     *
     */
    bool is_blob() const;
    /**
     * @brief Get if the value is floating or (unsigned) integer.
     *
     */
    bool is_number() const;

    /**
     * @brief Get the number value as T
     *
     * @tparam T             The C++ basic number type to convert to, such as int,
     * long, double, etc.
     * @return T             The value
     * @throws std::bad_cast If the value cannot be converted to T or the value is
     * not a number
     * @note   You can use Any::is_number() to check if the value is a number
     * before calling this function.
     * @see    is_number()
     */
    template <typename T>
    inline T get_number() const {
        switch (type) {
#if !defined(DBANY_NO_NULL_CONVERSION)
        case Type::Null:
            return 0;
#endif
        case Type::Boolean:
            return std::get<bool>(value);
        case Type::Integer:
            return std::get<int64_t>(value);
        case Type::UInteger:
            return std::get<uint64_t>(value);
        case Type::Floating:
            return std::get<double>(value);
        case Type::String:
        case Type::Date:
        case Type::Time:
        case Type::DateTime:
        case Type::Blob:
        default:
            throw std::bad_cast();
        }
    }

    /**
     * @brief Get the value as T.
     *
     * @tparam T  The type of the value
     * @return T  The value
     * @throws std::bad_cast  If the value cannot be converted to T
     * @par Custom Type Conversion
     * Define a custom type conversion function for the type T
     * @code
     * template <>
     * MyClass any_to(const Any& v) {
     *     MyClass result;
     *     switch (v.type) {
     *     case Any::Type::String:
     *         result.a = *v.value.string;
     *     default:
     *         throw std::bad_cast();
     *     }
     *     return result;
     * }
     * @endcode
     * @note  You can use `#define DBANY_NO_NULL_CONVERSION` to disable null
     * conversion. (throw an exception when trying converting from a null type
     * value)
     * @see any_to
     */
    template <typename T>
    inline T get() const {
        return any_to<T>(*this);
    }

    /**
     * @brief Convert Any::Type to string.
     *
     * @param  type         The Any::Type value
     * @return std::string  The string value
     */
    static std::string type2str(Any::Type type);

    /**
     * @brief Convert string to Any.
     *
     * @param  str  The string
     * @return Any  The converted value
     */
    static Any str2any(const std::string& str);
};

} // namespace DB

// Explicit specializations of Any::get<T>() must be defined at namespace scope.
namespace DB {

template <>
inline bool Any::get<bool>() const {
    switch (type) {
#if !defined(DBANY_NO_NULL_CONVERSION)
    case Type::Null:
        return false;
#endif
    case Type::Boolean:
        return std::get<bool>(value);
    case Type::Integer:
        return static_cast<bool>(std::get<int64_t>(value));
    case Type::UInteger:
        return static_cast<bool>(std::get<uint64_t>(value));
    case Type::Floating:
        return static_cast<bool>(std::get<double>(value));
    default:
        throw std::bad_cast();
    }
}

template <>
inline char Any::get<char>() const {
    return get_number<char>();
}

template <>
inline unsigned char Any::get<unsigned char>() const {
    return get_number<unsigned char>();
}

template <>
inline short Any::get<short>() const {
    return get_number<short>();
}

template <>
inline unsigned short Any::get<unsigned short>() const {
    return get_number<unsigned short>();
}

template <>
inline int Any::get<int>() const {
    return get_number<int>();
}

template <>
inline unsigned int Any::get<unsigned int>() const {
    return get_number<unsigned int>();
}

template <>
inline long Any::get<long>() const {
    return get_number<long>();
}

template <>
inline unsigned long Any::get<unsigned long>() const {
    return get_number<unsigned long>();
}

template <>
inline long long Any::get<long long>() const {
    return get_number<long long>();
}

template <>
inline unsigned long long Any::get<unsigned long long>() const {
    return get_number<unsigned long long>();
}

template <>
inline double Any::get<double>() const {
    return get_number<double>();
}

template <>
inline float Any::get<float>() const {
    return get_number<float>();
}

template <>
inline std::string Any::get<std::string>() const {
    switch (type) {
#if !defined(DBANY_NO_NULL_CONVERSION)
    case Type::Null:
        return "";
#endif
    case Type::Boolean:
        return std::get<bool>(value) ? "true" : "false";
    case Type::Integer:
        return std::to_string(std::get<int64_t>(value));
    case Type::UInteger:
        return std::to_string(std::get<uint64_t>(value));
    case Type::Floating:
        return std::to_string(std::get<double>(value));
    case Type::String:
        return std::get<std::string>(value);
    case Type::Date:
        return std::to_string(std::get<Date>(value).year) + "-" + std::to_string(std::get<Date>(value).month) + "-"
             + std::to_string(std::get<Date>(value).day);
    case Type::Time:
        return std::to_string(std::get<Time>(value).hour) + ":" + std::to_string(std::get<Time>(value).minute) + ":"
             + std::to_string(std::get<Time>(value).second);
    case Type::DateTime:
        return std::to_string(std::get<DateTime>(value).date.year) + "-"
             + std::to_string(std::get<DateTime>(value).date.month) + "-"
             + std::to_string(std::get<DateTime>(value).date.day) + " "
             + std::to_string(std::get<DateTime>(value).time.hour) + ":"
             + std::to_string(std::get<DateTime>(value).time.minute) + ":"
             + std::to_string(std::get<DateTime>(value).time.second);
    case Type::Blob: {
        auto blob = std::get<ByteArray>(value);
        return std::string(blob.begin(), blob.end());
    }
    default:
        throw std::bad_cast();
    }
}

template <>
inline Date Any::get<Date>() const {
    switch (type) {
    case Type::Date:
        return std::get<Date>(value);
    case Type::DateTime:
        return std::get<DateTime>(value).date;
    default:
        throw std::bad_cast();
    }
}

template <>
inline Time Any::get<Time>() const {
    switch (type) {
    case Type::Time:
        return std::get<Time>(value);
    case Type::DateTime:
        return std::get<DateTime>(value).time;
    default:
        throw std::bad_cast();
    }
}

template <>
inline DateTime Any::get<DateTime>() const {
    switch (type) {
    case Type::DateTime:
        return std::get<DateTime>(value);
    default:
        throw std::bad_cast();
    }
}

template <>
inline ByteArray Any::get<ByteArray>() const {
    switch (type) {
    case Type::Blob:
        return std::get<ByteArray>(value);
    case Type::String: {
        auto str = std::get<std::string>(value);
        return ByteArray((unsigned char*)str.data(), (unsigned char*)str.data() + str.size());
    }
    default:
        throw std::bad_cast();
    }
}

} // namespace DB

template <typename T>
inline std::vector<DB::Any> to_any_container(const std::vector<T>& v) {
    std::vector<DB::Any> result;
    for (auto& i : v) {
        result.push_back(DB::Any(i));
    }
    return result;
}

template <typename T>
inline std::set<DB::Any> to_any_container(const std::set<T>& v) {
    std::set<DB::Any> result;
    for (auto& i : v) {
        result.insert(DB::Any(i));
    }
    return result;
}

template <typename T>
inline std::list<DB::Any> to_any_container(const std::list<T>& v) {
    std::list<DB::Any> result;
    for (auto& i : v) {
        result.push_back(DB::Any(i));
    }
    return result;
}

template <typename T>
inline std::unordered_set<DB::Any> to_any_container(const std::unordered_set<T>& v) {
    std::unordered_set<DB::Any> result;
    for (auto& i : v) {
        result.insert(DB::Any(i));
    }
    return result;
}

template <typename K, typename V>
inline std::map<K, DB::Any> to_any_container(const std::map<K, V>& v) {
    std::map<K, DB::Any> result;
    for (auto& i : v) {
        result.insert(std::make_pair(i.first, DB::Any(i.second)));
    }
    return result;
}

template <typename K, typename V>
inline std::unordered_map<K, DB::Any> to_any_unordered_map(const std::unordered_map<K, V>& v) {
    std::unordered_map<K, DB::Any> result;
    for (auto& i : v) {
        result.insert(std::make_pair(i.first, DB::Any(i.second)));
    }
    return result;
}
