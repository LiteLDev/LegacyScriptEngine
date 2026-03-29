#pragma once

#include "utils/UsingScriptX.h"

//////////////////// LLSE Static ////////////////////

class LlClass {
public:
    static Local<Value> getLanguage();
    static Local<Value> getMajorVersion();
    static Local<Value> getMinorVersion();
    static Local<Value> getRevisionVersion();
    static Local<Value> getScriptEngineVersion();
    static Local<Value> getVersionStatus();
    static Local<Value> isRelease();
    static Local<Value> isBeta();
    static Local<Value> isDev();
    static Local<Value> isWine();
    static Local<Value> isDebugMode();
    static Local<Value> getPluginsRoot();

    static Local<Value> registerPlugin(Arguments const& args);
    static Local<Value> versionString(Arguments const& args);
    static Local<Value> requireVersion(Arguments const& args);
    static Local<Value> getAllPluginInfo(Arguments const& args);
    static Local<Value> listPlugins(Arguments const& args);
    static Local<Value> exportFunc(Arguments const& args);
    static Local<Value> importFunc(Arguments const& args);
    static Local<Value> hasFuncExported(Arguments const& args);
    static Local<Value> require(Arguments const& args);
    static Local<Value> eval(Arguments const& args);
    static Local<Value> getPluginInfo(Arguments const& args);
    static Local<Value> getCurrentPluginInfo(Arguments const& args);
    static Local<Value> onUnload(Arguments const& args);

    // For Compatibility
    static Local<Value> version(Arguments const& args);
};
extern ClassDefine<void> LlClassBuilder;
extern ClassDefine<void> VersionClassBuilder;
