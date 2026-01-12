#include "legacyapi/db/Any.h"

namespace DB {

Any::Any() {
    type  = Type::Null;
    value = false;
}
Any::Any(bool v) {
    type  = Type::Boolean;
    value = v;
}
Any::Any(int64_t v) {
    type  = Type::Integer;
    value = v;
}
Any::Any(uint64_t v) {
    type  = Type::UInteger;
    value = v;
}
Any::Any(double v) {
    type  = Type::Floating;
    value = v;
}
Any::Any(const std::string& v) {
    type  = Type::String;
    value = v;
}
Any::Any(const char* v) {
    type  = Type::String;
    value = v;
}
Any::Any(char* v, size_t len) {
    type  = Type::String;
    value = std::string(v, len);
}
Any::Any(const Date& v) {
    type  = Type::Date;
    value = v;
}
Any::Any(const Time& v) {
    type  = Type::Time;
    value = v;
}
Any::Any(const DateTime& v) {
    type  = Type::DateTime;
    value = v;
}
Any::Any(char v) {
    type  = Type::Integer;
    value = v;
}
Any::Any(unsigned char v) {
    type  = Type::UInteger;
    value = static_cast<uint64_t>(v);
}
Any::Any(short v) {
    type  = Type::Integer;
    value = v;
}
Any::Any(unsigned short v) {
    type  = Type::UInteger;
    value = static_cast<uint64_t>(v);
}
Any::Any(int v) {
    type  = Type::Integer;
    value = v;
}
Any::Any(unsigned int v) {
    type  = Type::UInteger;
    value = static_cast<uint64_t>(v);
}
Any::Any(long v) {
    type  = Type::Integer;
    value = v;
}
Any::Any(unsigned long v) {
    type  = Type::UInteger;
    value = static_cast<uint64_t>(v);
}
Any::Any(float v) {
    type  = Type::Floating;
    value = v;
}
Any::Any(const ByteArray& v) {
    type  = Type::Blob;
    value = v;
}
Any::Any(const Any& v) { *this = v; }

Any& Any::operator=(const Any& v) {
    if (this == &v) return *this;
    type  = v.type;
    value = v.value;
    return *this;
}

bool Any::is_null() const { return type == Type::Null; }
bool Any::is_boolean() const { return type == Type::Boolean; }
bool Any::is_integer() const { return type == Type::Integer || type == Type::UInteger; }
bool Any::is_uinteger() const { return type == Type::UInteger; }
bool Any::is_floating() const { return type == Type::Floating; }
bool Any::is_string() const { return type == Type::String; }
bool Any::is_date() const { return type == Type::Date; }
bool Any::is_time() const { return type == Type::Time; }
bool Any::is_datetime() const { return type == Type::DateTime; }
bool Any::is_number() const { return type == Type::Integer || type == Type::UInteger || type == Type::Floating; }
bool Any::is_blob() const { return type == Type::Blob; }

std::string Any::type2str(Any::Type type) {
    switch (type) {
    case Type::Null:
        return "null";
    case Type::Boolean:
        return "boolean";
    case Type::Integer:
        return "integer";
    case Type::UInteger:
        return "uinteger";
    case Type::Floating:
        return "floating";
    case Type::String:
        return "string";
    case Type::Date:
        return "date";
    case Type::Time:
        return "time";
    case Type::DateTime:
        return "datetime";
    case Type::Blob:
        return "blob";
    default:
        return "unknown";
    }
}

Any Any::str2any(const std::string& str) {
    if (str.empty()) return Any();
    bool isInteger  = true;
    bool isFloating = false;
    bool first      = true;
    for (auto& ch : str) {
        if (first && ch == '-') {
            first = false;
            continue;
        }
        if (ch >= '0' && ch <= '9') continue;
        if (ch == '.') {
            if (isFloating) isFloating = false;
            else {
                isInteger  = false;
                isFloating = true;
            }
            continue;
        }
    }
    if (isFloating) return Any(std::stod(str));
    else if (isInteger) {
        auto floating = std::stod(str);
        if (floating > ULLONG_MAX || floating < LLONG_MIN) return Any(floating);
        else if (floating > LLONG_MAX) return Any(std::stoull(str));
        return Any(std::stoll(str));
    } else return Any(str);
}

} // namespace DB
