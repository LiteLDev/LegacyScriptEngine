#pragma once
#include "legacy/api/APIHelp.h"

class VaillanI18nClass : public ScriptClass {
public:
    static Local<Value> setCurrentLanguage(Arguments const& args);
    static Local<Value> getCurrentLanguage(Arguments const& args);
    static Local<Value> getSupportedLanguages(Arguments const& args);
    static Local<Value> translate(Arguments const& args);
    static Local<Value> loadLanguage(Arguments const& args);
    static Local<Value> loadLanguageFromFile(Arguments const& args);
    static Local<Value> loadLanguagesFromDirectory(Arguments const& args);

    static void loadLanguageFile(std::string const& language, std::filesystem::path const& path);
};

extern ClassDefine<VaillanI18nClass> VaillanI18nClassBuilder;
