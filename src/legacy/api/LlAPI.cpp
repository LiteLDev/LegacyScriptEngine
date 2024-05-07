#include "api/LlAPI.h"

#include "api/APIHelp.h"
#include "engine/GlobalShareData.h"
#include "ll/api/plugin/PluginManager.h"
#include "ll/api/plugin/PluginManagerRegistry.h"
#include "ll/api/service/ServerInfo.h"
#include "ll/api/utils/WinUtils.h"
#include "lse/PluginManager.h"

#include <string>

//////////////////// Classes ////////////////////

ClassDefine<void> LlClassBuilder = defineClass("ll")
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
                                       .function("versionStatus", &LlClass::getVersionStatusFunction)
                                       .function("scriptEngineVersion", &LlClass::getScriptEngineVersionFunction)

                                       .build();

ClassDefine<void> VersionClassBuilder = defineClass("Version")
                                            // wait for: constructor, setter, converter(toString, toArray,
                                            // toObject)...
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

auto LlClass::isDebugMode() -> Local<Value> {
#ifdef NDEBUG
    return Boolean::newBoolean(false);
#else
    return Boolean::newBoolean(true);
#endif
}

Local<Value> LlClass::isRelease() {
    try {
        return Boolean::newBoolean(!ll::getLoaderVersion().preRelease.has_value());
    }
    CATCH("Fail in LLSEIsRelease")
}

Local<Value> LlClass::isBeta() {
    try {
        return Boolean::newBoolean(ll::getLoaderVersion().preRelease.has_value());
    }
    CATCH("Fail in LLSEIsBeta")
}

Local<Value> LlClass::isDev() { return Boolean::newBoolean(false); }

Local<Value> LlClass::getMajorVersion() {
    try {
        return Number::newNumber(ll::getLoaderVersion().major);
    }
    CATCH("Fail in LLSEGetMajorVersion")
}

Local<Value> LlClass::getMinorVersion() {
    try {
        return Number::newNumber(ll::getLoaderVersion().minor);
    }
    CATCH("Fail in LLSEGetMinorVersion")
}

Local<Value> LlClass::getRevisionVersion() {
    try {
        return Number::newNumber(ll::getLoaderVersion().patch);
    }
    CATCH("Fail in LLSEGetRevisionVersion")
}

Local<Value> LlClass::getScriptEngineVersion() {
    try {
        return String::newString(EngineScope::currentEngine()->getEngineVersion());
    }
    CATCH("Fail in LLSEGetScriptEngineVerison")
}

Local<Value> LlClass::getVersionStatus() { return Number::newNumber(0); }

Local<Value> LlClass::registerPlugin(const Arguments& args) { return Boolean::newBoolean(true); }
Local<Value> LlClass::getPluginInfo(const Arguments& args) {
    try {
        auto plugin = lse::getPluginManager().getPlugin(args[0].asString().toString());
        if (plugin) {
            auto result = Object::newObject();

            result.set("name", plugin->getManifest().name);
            if (plugin->getManifest().description.has_value()) {
                result.set("desc", plugin->getManifest().description.value());
            }

            auto ver = Array::newArray();
            ver.add(Number::newNumber(plugin->getManifest().version->major));
            ver.add(Number::newNumber(plugin->getManifest().version->minor));
            ver.add(Number::newNumber(plugin->getManifest().version->patch));

            result.set("version", ver);
            result.set("versionStr", plugin->getManifest().version->to_string());
            result.set("filePath", plugin->getManifest().entry);
            if (plugin->getManifest().extraInfo.has_value()) {
                auto others = Object::newObject();
                for (const auto& [k, v] : plugin->getManifest().extraInfo.value()) {
                    others.set(k, v);
                }
                result.set("others", others);
            }

            return result;
        }
        return {};
    }
    CATCH("Fail in LLAPI");
}
Local<Value> LlClass::versionString(const Arguments& args) {
    try {
        return String::newString(ll::getLoaderVersion().to_string());
    }
    CATCH("Fail in LLSEGetVersionString!")
}

Local<Value> LlClass::requireVersion(const Arguments& args) { return Boolean::newBoolean(true); }

Local<Value> LlClass::getAllPluginInfo(const Arguments& args) {
    try {
        Local<Array> plugins = Array::newArray();
        ll::plugin::PluginManagerRegistry::getInstance().forEachPluginWithType(
            [&](std::string_view type, std::string_view name, ll::plugin::Plugin& plugin) {
                // Create plugin object
                auto pluginObject = Object::newObject();

                pluginObject.set("name", plugin.getManifest().name);
                if (plugin.getManifest().description.has_value()) {
                    pluginObject.set("desc", plugin.getManifest().description.value());
                }
                pluginObject.set("type", type);

                auto ver = Array::newArray();
                ver.add(Number::newNumber(plugin.getManifest().version->major));
                ver.add(Number::newNumber(plugin.getManifest().version->minor));
                ver.add(Number::newNumber(plugin.getManifest().version->patch));

                pluginObject.set("version", ver);
                pluginObject.set("versionStr", plugin.getManifest().version->to_string());
                pluginObject.set("filePath", plugin.getManifest().entry);

                if (plugin.getManifest().extraInfo.has_value()) {
                    auto others = Object::newObject();
                    for (const auto& [k, v] : plugin.getManifest().extraInfo.value()) {
                        others.set(k, v);
                    }
                    pluginObject.set("others", others);
                }

                // Add plugin object to list
                plugins.add(pluginObject);
                return true;
            }
        );
        return plugins;
    }
    CATCH("Fail in LLAPI");
}
// For Compatibility
Local<Value> LlClass::listPlugins(const Arguments& args) {
    try {
        Local<Array> plugins = Array::newArray();
        ll::plugin::PluginManagerRegistry::getInstance().forEachPluginWithType(
            [&](std::string_view type, std::string_view name, ll::plugin::Plugin&) {
                plugins.add(String::newString(name));
                return true;
            }
        );
        return plugins;
    }
    CATCH("Fail in LLAPI");
}

Local<Value> LlClass::require(const Arguments& args) { return Boolean::newBoolean(true); }

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
        ver.set("major", ll::getLoaderVersion().major);
        ver.set("minor", ll::getLoaderVersion().minor);
        ver.set("revision", ll::getLoaderVersion().patch);
        ver.set("isBeta", !ll::getLoaderVersion().preRelease.has_value());
        ver.set("isRelease", ll::getLoaderVersion().preRelease.has_value());
        ver.set("isDev", false);
        return ver;
    }
    CATCH("Fail in LLSEGetVersion!")
}

// For Compatibility
Local<Value> LlClass::isDebugModeFunction(const Arguments& args) {
#ifdef LEGACYSCRIPTENGINE_DEBUG
    return Boolean::newBoolean(true);
#else
    return Boolean::newBoolean(false);
#endif
}

// For Compatibility
Local<Value> LlClass::getScriptEngineVersionFunction(const Arguments& args) {
    try {
        return String::newString(EngineScope::currentEngine()->getEngineVersion());
    }
    CATCH("Fail in LLSEGetScriptEngineVerison")
}
