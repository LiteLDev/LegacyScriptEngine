#pragma once

#include "engine/EngineOwnData.h"
#include "ll/api/utils/ErrorUtils.h"
#include "lse/Entry.h"
#include "mc/world/level/Level.h"
#include "utils/JsonHelper.h"
#include "utils/UsingScriptX.h"

#include <exception>
#include <magic_enum.hpp>
#include <string>

// 实例类类型检查
template <typename T>
bool IsInstanceOf(Local<Value> v) {
    return EngineScope::currentEngine()->isInstanceOf<T>(v);
}

std::string ValueKindToString(const ValueKind& kind);

// 输出脚本调用堆栈，API名称，以及插件名
inline Exception CREATE_EXCEPTION_WITH_SCRIPT_INFO(std::string const& func, std::string const& msg) {
    return Exception(fmt::format("In API: {}, In Plugin: {}, {}", func, getEngineOwnData()->pluginName, msg));
}

inline void LOG_ERROR_WITH_SCRIPT_INFO(std::string const& func) {
    lse::LegacyScriptEngine::getLogger().error("In API: {}, In Plugin: {}", func, getEngineOwnData()->pluginName);
}

// 参数类型错误输出
inline void THROW_WRONG_ARG_TYPE(std::string const& func) {
    throw CREATE_EXCEPTION_WITH_SCRIPT_INFO(func, "Wrong type of argument!");
}

// 参数数量错误输出
inline void THROW_TOO_FEW_ARGS(std::string const& func) {
    throw CREATE_EXCEPTION_WITH_SCRIPT_INFO(func, "Too Few arguments!");
}

// 参数数量错误输出
inline void THROW_WRONG_ARGS_COUNT(std::string const& func) {
    throw CREATE_EXCEPTION_WITH_SCRIPT_INFO(func, "Wrong number of arguments!");
}

// 至少COUNT个参数
#define CHECK_ARGS_COUNT(ARGS, COUNT)                                                                                  \
    if (ARGS.size() < COUNT) {                                                                                         \
        THROW_TOO_FEW_ARGS(__FUNCTION__);                                                                              \
    }

// 检查是否TYPE类型
#define CHECK_ARG_TYPE(ARG, TYPE)                                                                                      \
    if (ARG.getKind() != TYPE) {                                                                                       \
        THROW_WRONG_ARG_TYPE(__FUNCTION__);                                                                            \
    }

// 截获引擎异常
#define CATCH_AND_THROW                                                                                                \
    catch (Exception const& e) {                                                                                       \
        throw e;                                                                                                       \
    }                                                                                                                  \
    catch (std::exception const& e) {                                                                                  \
        throw Exception(e.what());                                                                                     \
    }                                                                                                                  \
    catch (...) {                                                                                                      \
        throw Exception(                                                                                               \
            fmt::format("{}\nfunction: {}", ll::error_utils::makeExceptionString(std::current_exception()), __func__)  \
        );                                                                                                             \
    }

#define CATCH                                                                                                          \
    catch (const Exception& e) {                                                                                       \
        ll::error_utils::printException(e, lse::LegacyScriptEngine::getLogger());                                      \
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__);                                                                      \
    }                                                                                                                  \
    catch (...) {                                                                                                      \
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getLogger());                                  \
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__);                                                                      \
    }

#define CATCH_WITH_MESSAGE(LOG)                                                                                        \
    catch (const Exception& e) {                                                                                       \
        lse::LegacyScriptEngine::getLogger().error(LOG);                                                               \
        ll::error_utils::printException(e, lse::LegacyScriptEngine::getLogger());                                      \
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__);                                                                      \
    }                                                                                                                  \
    catch (...) {                                                                                                      \
        lse::LegacyScriptEngine::getLogger().error(LOG);                                                               \
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getLogger());                                  \
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__);                                                                      \
    }

// 截获回调函数异常
#define CATCH_IN_CALLBACK(callback)                                                                                    \
    catch (const Exception& e) {                                                                                       \
        ll::error_utils::printException(e, lse::LegacyScriptEngine::getLogger());                                      \
        lse::LegacyScriptEngine::getLogger().error(std::string("In callback for ") + callback);                        \
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__);                                                                      \
    }                                                                                                                  \
    catch (...) {                                                                                                      \
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getLogger());                                  \
        lse::LegacyScriptEngine::getLogger().error(std::string("In callback for ") + callback);                        \
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__);                                                                      \
    }

// 判断是否为浮点数
bool CheckIsFloat(const Local<Value>& num);

// 序列化
template <typename T>
void PrintValue(T& out, Local<Value> v);

std::string ValueToString(Local<Value> const& v);

// Json 序列化 反序列化
Local<Value> JsonToValue(std::string jsonStr);
Local<Value> JsonToValue(ordered_json j);
std::string  ValueToJson(Local<Value> v, int formatIndent = -1);

// Get the enum's ClassDefine<void> object
// Limitation: enum values must be in range of [-128, 128)
template <typename Type>
struct EnumDefineBuilder {
    inline static Local<Value> keys() {
        try {
            auto arr = Array::newArray();
            for (auto& name : magic_enum::enum_names<Type>()) {
                arr.add(String::newString(name));
            }
            return arr;
        } catch (const std::exception&) {
            lse::LegacyScriptEngine::getLogger().error("Error in " __FUNCTION__);
        }
        return {};
    }

    inline static Local<Value> toObject() {
        try {
            auto obj = Object::newObject();
            for (auto& [value, name] : magic_enum::enum_entries<Type>()) {
                obj.set(String::newString(name), Number::newNumber(static_cast<int64_t>(value)));
            }
            return obj;
        } catch (const std::exception&) {
            lse::LegacyScriptEngine::getLogger().error("Error in " __FUNCTION__);
        }
        return {};
    }

    inline static Local<Value> getName(const Arguments& args) {
        try {
            if (args.size() < 1) return {};
            if (args[0].isString())
                return magic_enum::enum_cast<Type>(args[0].asString().toString()).has_value() ? args[0]
                                                                                              : Local<Value>();
            if (args[0].isNumber())
                return String::newString(magic_enum::enum_name(static_cast<Type>(args[0].asNumber().toInt32())));
            return {};
        } catch (const std::exception&) {
            lse::LegacyScriptEngine::getLogger().error("Error in " __FUNCTION__);
        }
        return {};
    }

    inline static Local<Value> toString() {
        try {
            return String::newString(typeid(Type).name() + 5);
        } catch (const std::exception&) {
            lse::LegacyScriptEngine::getLogger().error("Error in " __FUNCTION__);
        }
        return {};
    }

    inline static ClassDefine<void> build(std::string const& enumName) {
        script::ClassDefineBuilder<void> builder = defineClass(enumName);

        for (auto& [val, name] : magic_enum::enum_entries<Type>()) {
            builder.property(std::string(name), [enumName, val, name{std::string{name}}]() -> Local<Value> {
                try {
                    return Number::newNumber(static_cast<int64_t>(val));
                } catch (const std::exception&) {
                    lse::LegacyScriptEngine::getLogger().error("Error in get {}.{}", enumName, name);
                }
                return {};
            });
        }

        builder.property("keys", &keys);
        // builder.property("object", &toObject);
        builder.function("getName", &getName);
        // fmt::print("\n");
        return builder.build();
    }
};
