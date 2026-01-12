#define NEW_DEFINES
#include "api/InternationalAPI.h"

#include "ll/api/i18n/I18n.h"

#include <FMT/args.h>

void FormatHelper(
    std::vector<Local<Value>>&                          args,
    const std::vector<char*>&                           names,
    bool                                                enableObject,
    fmt::dynamic_format_arg_store<fmt::format_context>& s,
    std::vector<std::unique_ptr<char[]>>&               delList
) {
    for (size_t i = 0; i < args.size(); ++i) {
        auto&       arg  = args[i];
        const char* name = (i < names.size()) ? names[i] : nullptr;

        auto pushArg = [&](auto value) {
            if (name) {
                s.push_back(fmt::arg(name, value));
            } else {
                s.push_back(value);
            }
        };

        switch (arg.getKind()) {
        case ValueKind::kBoolean:
            pushArg(arg.asBoolean().value());
            break;
        case ValueKind::kNumber:
            pushArg(CheckIsFloat(arg) ? arg.asNumber().toDouble() : arg.asNumber().toInt64());
            break;
        case ValueKind::kString:
            pushArg(arg.asString().toString());
            break;
        case ValueKind::kArray:
            if (enableObject) {
                auto                      arr = arg.asArray();
                std::vector<Local<Value>> vals(arr.size());
                for (size_t j = 0; j < arr.size(); ++j) {
                    vals[j] = arr.get(j);
                }
                FormatHelper(vals, {}, false, s, delList);
            }
            break;
        case ValueKind::kObject:
            if (enableObject) {
                auto                      obj  = arg.asObject();
                auto                      keys = obj.getKeys();
                std::vector<Local<Value>> vals(keys.size());
                std::vector<char*>        nextNames(keys.size());
                for (size_t j = 0; j < keys.size(); ++j) {
                    auto key   = keys[j].toString();
                    auto cName = std::make_unique<char[]>(key.size() + 1);
                    std::memcpy(cName.get(), key.c_str(), key.size() + 1);
                    delList.push_back(std::move(cName));
                    nextNames[j] = delList.back().get();
                    vals[j]      = obj.get(keys[j]);
                }
                FormatHelper(vals, nextNames, false, s, delList);
            }
            break;
        default:
            pushArg(ValueToString(arg));
            break;
        }
    }
}

Local<Value> TrFormat(const Arguments& args, size_t offset, std::string key, const std::string& localeName = "") {
    try {
        size_t argsLength = args.size() - offset;
        key               = getEngineOwnData()->i18n.get(key, localeName);
        // realFormatStr = FixCurlyBracket(realFormatStr);
        if (0ULL == argsLength) {
            // Avoid fmt if only one argument
            return String::newString(key);
        } else {
            fmt::dynamic_format_arg_store<fmt::format_context> s;
            std::vector<std::unique_ptr<char[]>>               delList;
            std::vector<Local<Value>>                          vals;
            for (auto i = offset; i < args.size(); ++i) {
                vals.push_back(args[i]);
            }
            FormatHelper(vals, {}, true, s, delList);
            auto result = String::newString(fmt::vformat(key, s));
            return result;
        }
    } catch (const fmt::format_error&) {
        return String::newString(key);
    }
}

ClassDefine<void> I18nClassBuilder = defineClass<void>("i18n")
                                         .function("tr", &I18nClass::tr)
                                         .function("trl", &I18nClass::trl)
                                         .function("get", &I18nClass::get)
                                         .function("load", &I18nClass::load)
                                         .build();

Local<Value> I18nClass::tr(const Arguments& args) {
    CHECK_ARGS_LEAST_COUNT(1); // At least 1
    CHECK_ARG_TYPE(0, kString);

    try {
        return TrFormat(args, 1ULL, args[0].asString().toString());
    }
    CATCH_AND_THROW;
}

Local<Value> I18nClass::trl(const Arguments& args) {
    CHECK_ARGS_LEAST_COUNT(2);
    CHECK_ARG_TYPE(0, kString);
    CHECK_ARG_TYPE(1, kString);

    try {
        return TrFormat(args, 2ULL, args[1].asString().toString(), args[0].asString().toString());
    }
    CATCH_AND_THROW;
}

Local<Value> I18nClass::get(const Arguments& args) {
    CHECK_ARGS_LEAST_COUNT(1);
    CHECK_ARG_TYPE(0, kString);
    if (args.size() == 2) {
        CHECK_ARG_TYPE(1, kString);
    } else {
        CHECK_ARGS_COUNT(1);
    }

    try {
        auto        key        = args[0].asString().toString();
        std::string localeName = getEngineOwnData()->defaultLocaleName;
        if (args.size() == 2) {
            localeName = args[1].asString().toString();
        }
        return String::newString(getEngineOwnData()->i18n.get(key, localeName));
    }
    CATCH_AND_THROW;
}

Local<Value> I18nClass::load(const Arguments& args) {
    CHECK_ARGS_LEAST_COUNT(1);
    CHECK_ARG_TYPE(0, kString);
    if (args.size() > 1) {
        CHECK_ARG_TYPE(1, kString);
    }
    if (args.size() > 2) {
        CHECK_ARG_TYPE(2, kObject);
    }

    try {
        auto path = args[0].asString().toString();
        if (args.size() > 1) {
            getEngineOwnData()->defaultLocaleName = args[1].asString().toString();
        }

        if (args.size() > 2) {
            auto rawLangData     = args[2].asObject();
            auto rawLangDataKeys = rawLangData.getKeys();

            for (auto i = 0ULL; i < rawLangDataKeys.size(); ++i) {
                auto localeName = rawLangDataKeys[i].toString();
                auto val        = rawLangData.get(rawLangDataKeys[i]);
                if (val.getKind() != ValueKind::kObject) {
                    throw Exception("Value in LangData must be an object");
                }

                auto obj     = val.asObject();
                auto objKeys = obj.getKeys();
                for (auto j = 0ULL; j < objKeys.size(); ++j) {
                    auto str = obj.get(objKeys[j]);
                    if (str.getKind() != ValueKind::kString) {
                        throw Exception("Value in SubLangData must be a string");
                    }
                    getEngineOwnData()->i18n.set(localeName, objKeys[j].toString(), str.asString().toString());
                }
            }
        }

        if (auto result = getEngineOwnData()->i18n.load(path); !result) {
            return Boolean::newBoolean(false);
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW;
}