#include "VaillanI18n.h"

#include <mc/locale/I18n.h>
#include <mc/locale/Localization.h>

ClassDefine<void> VaillanI18nClassBuilder =
    defineClass("VaillanI18n")
        .function("setCurrentLanguage", &VaillanI18nClass::setCurrentLanguage)
        .function("getCurrentLanguage", &VaillanI18nClass::getCurrentLanguage)
        .function("getSupportedLanguages", &VaillanI18nClass::getSupportedLanguages)
        .function("translate", &VaillanI18nClass::translate)
        .function("loadLanguage", &VaillanI18nClass::loadLanguage)
        .function("loadLanguageFromFile", &VaillanI18nClass::loadLanguageFromFile)
        .function("loadLanguagesFromDirectory", &VaillanI18nClass::loadLanguagesFromDirectory)
        .build();

Local<Value> VaillanI18nClass::setCurrentLanguage(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    std::string lang = args[0].asString().toString();
    try {
        getI18n().chooseLanguage(lang);
    }
    CATCH_AND_THROW
    return Boolean::newBoolean(true);
}

Local<Value> VaillanI18nClass::getCurrentLanguage(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 0);
    try {
        return String::newString(*getI18n().getCurrentLanguage()->mCode);
    }
    CATCH_AND_THROW
}

Local<Value> VaillanI18nClass::getSupportedLanguages(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 0);
    try {
        auto arr = Array::newArray();
        for (auto& lang : getI18n().getSupportedLanguageCodes()) {
            arr.add(String::newString(lang));
        }
        return arr;
    }
    CATCH_AND_THROW
}

Local<Value> VaillanI18nClass::translate(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    std::string key = args[0].asString().toString();
    std::string lang;

    std::vector<std::string> params;
    if (args.size() >= 2) {
        CHECK_ARG_TYPE(args[1], ValueKind::kArray);
        auto arr = args[1].asArray();
        for (size_t i = 0; i < arr.size(); i++) {
            CHECK_ARG_TYPE(arr.get(i), ValueKind::kString);
            params.push_back(arr.get(i).asString().toString());
        }
    }

    if (args.size() >= 3) {
        CHECK_ARG_TYPE(args[2], ValueKind::kString);
        lang = args[2].asString().toString();
    }

    try {
        return String::newString(getI18n().get(key, params, getI18n().getLocaleFor(lang)));
    }
    CATCH_AND_THROW
}

Local<Value> VaillanI18nClass::loadLanguage(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kObject);
    std::string lang = args[0].asString().toString();
    auto        obj  = args[1].asObject();

    try {
        std::unordered_map<std::string, std::string> map;
        for (auto& prop : obj.getKeys()) {
            CHECK_ARG_TYPE(obj.get(prop), ValueKind::kString);
            map[prop.toString()] = obj.get(prop).asString().toString();
        }
        auto loc = std::const_pointer_cast<Localization>(getI18n().getLocaleFor(lang));
        if (!loc) getI18n().appendAdditionalTranslations(map, lang);
        else
            for (auto const& [key, value] : map) loc->mStrings->insert_or_assign(key, value);
    }
    CATCH_AND_THROW
    return Boolean::newBoolean(true);
}

Local<Value> VaillanI18nClass::loadLanguageFromFile(Arguments const& args) {
    // TODO: Load language from file
    return Boolean::newBoolean(true);
}

Local<Value> VaillanI18nClass::loadLanguagesFromDirectory(Arguments const& args) {
    // TODO: Load languages from directory
    return Boolean::newBoolean(true);
}