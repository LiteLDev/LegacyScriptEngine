#include "legacy/api/VaillanI18n.h"

#include <mc/locale/I18n.h>
#include <mc/locale/Localization.h>

ClassDefine<VaillanI18nClass> VaillanI18nClassBuilder =
    defineClass<VaillanI18nClass>("VaillanI18n")
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
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
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

    auto key      = args[0].asString().toString();
    auto language = getI18n().getCurrentLanguage().get();

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
        if (auto res = getI18n().getLocaleFor(args[2].asString().toString()); res) {
            language = res;
        }
    }

    try {
        return String::newString(getI18n().get(key, params, language));
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
        if (!loc) {
            getI18n().appendAdditionalTranslations(map, lang);
        } else {
            for (auto& [key, value] : map) {
                loc->mStrings->insert_or_assign(key, std::move(value));
            }
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> VaillanI18nClass::loadLanguageFromFile(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        auto path = std::filesystem::path{args[1].asString().toU8string()};
        if (!std::filesystem::is_regular_file(path)) return Boolean::newBoolean(false);
        loadLanguageFile(args[0].asString().toString(), path);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> VaillanI18nClass::loadLanguagesFromDirectory(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        for (auto& entry :  std::filesystem::directory_iterator(args[0].asString().toU8string())) {
            if (!entry.is_regular_file()) continue;
            auto file = ll::string_utils::u8str2str(entry.path().filename().u8string());
            if (!file.ends_with(".lang")) continue;
            loadLanguageFile(file.substr(0, file.size() - 5), entry.path());
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

void VaillanI18nClass::loadLanguageFile(std::string const& language, std::filesystem::path const& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;
    std::unordered_map<std::string, std::string> map;

    for (std::string line; std::getline(file, line); ) {
        if (line.empty()) continue;
        ll::string_utils::replaceAll(line, "\t", "");
        ll::string_utils::replaceAll(line, "\\n", "\n");
        if (auto equalPos = line.find('='); equalPos != std::string::npos) {
            auto key = line.substr(0, equalPos);
            ll::string_utils::replaceAll(key, " ", "");

            auto value = line.substr(equalPos + 1, line.find('#', equalPos + 1));
            while (value.ends_with(' ')) value.erase(value.size() - 1);

            if (!key.empty() && !value.empty()) map.insert_or_assign(key, value);
        }
    }

    file.close();
    auto loc = std::const_pointer_cast<Localization>(getI18n().getLocaleFor(language));
    if (!loc) {
        getI18n().appendAdditionalTranslations(map, language);
    } else {
        for (auto& [key, value] : map) {
            loc->mStrings->insert_or_assign(key, std::move(value));
        }
    }
}