#include "api/APIHelp.h"
#include "api/LlAPI.h"
#include "engine/GlobalShareData.h"
#include "engine/EngineOwnData.h"
#include <ll/api/utils/WinUtils.h>
#include "utils/Utils.h"
#include <string>
#include <filesystem>
#include <map>
using namespace std;


//////////////////// Classes ////////////////////

ClassDefine<void> LlClassBuilder =
    defineClass("ll")
        .property("major", &LlClass::getMajorVersion)
        .property("minor", &LlClass::getMinorVersion)
        .property("revision", &LlClass::getRevisionVersion)
        .property("status", &LlClass::getVersionStatus)
        .property("scriptEngineVersion", &LlClass::getScriptEngineVersion)
        .property("language", &LlClass::getLanguage)
        .property("isWine", &LlClass::isWine)
        .property("isDebugMode", &LlClass::isDebugMode)
        .property("isRelease", &LlClass::isRelease)
        .property("isBeta", &LlClass::isBeta)
        .property("isDev", &LlClass::isDev)
        
        .function("versionString", &LlClass::versionString)
        .function("requireVersion", &LlClass::requireVersion)
        .function("listPlugins", &LlClass::listPlugins)
        .function("getAllPluginInfo", &LlClass::getAllPluginInfo)
        .function("imports", &LlClass::importFunc)
        .function("exports", &LlClass::exportFunc)
        .function("hasExported", &LlClass::hasFuncExported)
        .function("require", &LlClass::require)
        .function("eval", &LlClass::eval)
        .function("registerPlugin", &LlClass::registerPlugin)
        .function("getPluginInfo", &LlClass::getPluginInfo)
        .function("checkVersion", &LlClass::requireVersion)

        // For Compatibility
        .function("version", &LlClass::version)
        .function("isDebugMode", &LlClass::isDebugModeFunction)
        .function("versionStatus", &LlClass::getVersionStatusFunction)
        .function("scriptEngineVersion", &LlClass::getScriptEngineVersionFunction)

        .build();

ClassDefine<void> VersionClassBuilder =
    defineClass("Version")
        // wait for: constructor, setter, converter(toString, toArray, toObject)...
        .property("Dev", []() { return Number::newNumber(0); })
        .property("Beta", []() { return Number::newNumber(1); })
        .property("Release", []() { return Number::newNumber(2); })
        .build();

Local<Value> LlClass::getLanguage() {
    try {
        return String::newString(ll::win_utils::getSystemLocaleName());
    }
    CATCH("Fail in LLSEGetLanguage")
}

Local<Value> LlClass::isWine() {
    try {
        return Boolean::newBoolean(ll::win_utils::isWine());
    }
    CATCH("Fail in LLSEIsWine")
}

Local<Value> LlClass::isDebugMode() {
    try {
        #ifdef LL_DEBUG
        return Boolean::newBoolean(true);
#else
        return Boolean::newBoolean(false);
#endif
    }
    CATCH("Fail in LLSEIsDebugMode")
}

Local<Value> LlClass::isRelease() {
    try {
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LLSEIsRelease")
}

Local<Value> LlClass::isBeta() {
    try {
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in LLSEIsBeta")
}

Local<Value> LlClass::isDev() {
    try {
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in LLSEIsDev")
}

Local<Value> LlClass::getMajorVersion() {
    try {
        return Number::newNumber(3);
    }
    CATCH("Fail in LLSEGetMajorVersion")
}

Local<Value> LlClass::getMinorVersion() {
    try {
        return Number::newNumber(0);
    }
    CATCH("Fail in LLSEGetMinorVersion")
}

Local<Value> LlClass::getRevisionVersion() {
    try {
        return Number::newNumber(0);
    }
    CATCH("Fail in LLSEGetRevisionVersion")
}

Local<Value> LlClass::getScriptEngineVersion() {
    try {
        return String::newString(EngineScope::currentEngine()->getEngineVersion());
    }
    CATCH("Fail in LLSEGetScriptEngineVerison")
}

Local<Value> LlClass::getVersionStatus() {
    try {
        return Number::newNumber(0);
    }
    CATCH("Fail in LLSEGetVersionStatus")
}

Local<Value> LlClass::registerPlugin(const Arguments& args) { try{
        return {};
    }
    CATCH("Fail in LLAPI");
}
Local<Value> LlClass::getPluginInfo(const Arguments& args) {
    try {
        return {};
    }
    CATCH("Fail in LLAPI");
}
Local<Value> LlClass::versionString(const Arguments& args) {
    try {
        return String::newString("3.0.0");
    }
    CATCH("Fail in LLSEGetVersionString!")
}

Local<Value> LlClass::requireVersion(const Arguments& args) {

    try {
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LLSERequireVersion!")
}

Local<Value> LlClass::getAllPluginInfo(const Arguments& args) {
    try {
        return {};
    }
    CATCH("Fail in LLAPI");
}
// For Compatibility
Local<Value> LlClass::listPlugins(const Arguments& args) {
    try {
        return {};
    }
    CATCH("Fail in LLAPI");
}

Local<Value> LlClass::require(const Arguments& args) {
    try {
        return {};
    }
    CATCH("Fail in LLAPI");
}
Local<Value> LlClass::eval(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        return EngineScope::currentEngine()->eval(args[0].toStr());
    }
    CATCH("Fail in LLSEEval!")
}

// For Compatibility
Local<Value> LlClass::getVersionStatusFunction(const Arguments& args) { return Number::newNumber(0); }

// For Compatibility
Local<Value> LlClass::version(const Arguments& args) {
    try {
        Local<Object> ver = Object::newObject();
        ver.set("major", 3);
        ver.set("minor", 0);
        ver.set("revision", 0);
        ver.set("isBeta", false);
        ver.set("isRelease",true);
        ver.set("isDev", false);
        return ver;
    }
    CATCH("Fail in LLSEGetVersion!")
}

// For Compatibility
Local<Value> LlClass::isDebugModeFunction(const Arguments& args) {
    try {
#ifdef LL_DEBUG
        return Boolean::newBoolean(true);
#else
        return Boolean::newBoolean(false);
#endif
    }
    CATCH("Fail in LLSEGetIsDebugModeFunction")
}

// For Compatibility
Local<Value> LlClass::getScriptEngineVersionFunction(const Arguments& args) {
    try {
        return String::newString(EngineScope::currentEngine()->getEngineVersion());
    }
    CATCH("Fail in LLSEGetScriptEngineVerison")
}