#include "legacy/api/LlAPI.h"

#include "legacy/api/APIHelp.h"
#include "legacy/engine/EngineOwnData.h"
#include "legacy/engine/GlobalShareData.h"
#include "ll/api/data/Version.h"
#include "ll/api/mod/Mod.h"
#include "ll/api/mod/ModManagerRegistry.h"
#include "ll/api/reflection/Serialization.h"
#include "ll/api/utils/SystemUtils.h"

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
                                       .property("pluginsRoot", &LlClass::getPluginsRoot)

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
                                       .function("getCurrentPluginInfo", &LlClass::getCurrentPluginInfo)
                                       .function("checkVersion", &LlClass::requireVersion)
                                       .function("onUnload", &LlClass::onUnload)

                                       // For Compatibility
                                       .function("version", &LlClass::version)
                                       .function("versionStatus", &LlClass::getVersionStatus)

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
        return String::newString(ll::i18n::getDefaultLocaleCode());
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::isWine() {
    try {
        return Boolean::newBoolean(ll::sys_utils::isWine());
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::isDebugMode() {
#ifdef NDEBUG
    return Boolean::newBoolean(false);
#else
    return Boolean::newBoolean(true);
#endif
}

Local<Value> LlClass::isRelease() {
    try {
        if (auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version) {
            return Boolean::newBoolean(!ver->preRelease.has_value() && !ver->build.has_value());
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::isBeta() {
    try {
        if (auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version) {
            return Boolean::newBoolean(ver->preRelease.has_value());
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::isDev() {
    try {
        if (auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version) {
            return Boolean::newBoolean(ver->build.has_value());
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::getMajorVersion() {
    try {
        if (auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version) {
            return Number::newNumber(ver->major);
        }
        return Number::newNumber(0);
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::getMinorVersion() {
    try {
        if (auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version) {
            return Number::newNumber(ver->minor);
        }
        return Number::newNumber(0);
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::getRevisionVersion() {
    try {
        if (auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version) {
            return Number::newNumber(ver->patch);
        }
        return Number::newNumber(0);
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::getScriptEngineVersion() {
    try {
        return String::newString(EngineScope::currentEngine()->getEngineVersion());
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::getVersionStatus() {
    if (auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version) {
        if (ver->preRelease.has_value()) {
            return Number::newNumber(1);
        }
        if (ver->build.has_value()) {
            return Number::newNumber(2);
        }
        return Number::newNumber(0);
    }
    return Number::newNumber(0);
}

Local<Value> LlClass::getPluginsRoot() {
    try {
        return String::newString(ll::mod::getModsRoot().u8string());
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::registerPlugin(Arguments const& args) {
    if (args.size() == 0) {
        return Boolean::newBoolean(true);
    }
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kString);
    if (args.size() >= 4) CHECK_ARG_TYPE(args[3], ValueKind::kObject);

    try {
        std::string desc = args.size() >= 2 ? args[1].asString().toString() : "";

        ll::data::Version ver = ll::data::Version(1, 0, 0);
        if (args.size() >= 3) {
            if (args[2].isArray()) { // like [1,0,0].
                Local<Array> verInfo = args[2].asArray();
                if (verInfo.size() >= 1) {
                    Local<Value> major = verInfo.get(0);
                    if (major.isNumber()) ver.major = major.asNumber().toInt32();
                }
                if (verInfo.size() >= 2) {
                    Local<Value> minor = verInfo.get(1);
                    if (minor.isNumber()) ver.minor = minor.asNumber().toInt32();
                }
                if (verInfo.size() >= 3) {
                    Local<Value> revision = verInfo.get(2);
                    if (revision.isNumber()) ver.patch = revision.asNumber().toInt32();
                }
                if (verInfo.size() >= 4) { // script: Version Enum.
                    Local<Value> status = verInfo.get(3);
                    if (status.isNumber()) {
                        switch (status.asNumber().toInt32()) {
                        case 0:
                            ver.preRelease = ll::data::PreRelease();
                            ver.preRelease->from_string("dev");
                            break;
                        case 1:
                            ver.preRelease = ll::data::PreRelease();
                            ver.preRelease->from_string("beta");
                            break;
                        default:
                            break;
                        }
                    }
                }
            } else if (args[2].isObject()) { // like { major: 1, minor:0, revision:0 }
                Local<Object> verInfo = args[2].asObject();
                if (verInfo.has("major")) {
                    Local<Value> major = verInfo.get("major");
                    if (major.isNumber()) ver.major = major.asNumber().toInt32();
                }
                if (verInfo.has("minor")) {
                    Local<Value> minor = verInfo.get("minor");
                    if (minor.isNumber()) ver.minor = minor.asNumber().toInt32();
                }
                if (verInfo.has("revision")) {
                    Local<Value> revision = verInfo.get("revision");
                    if (revision.isNumber()) ver.patch = revision.asNumber().toInt32();
                }
                if (verInfo.has("status")) {
                    Local<Value> status = verInfo.get("status");
                    if (status.isNumber()) {
                        switch (status.asNumber().toInt32()) {
                        case 0:
                            ver.preRelease = ll::data::PreRelease();
                            ver.preRelease->from_string("dev");
                            break;
                        case 1:
                            ver.preRelease = ll::data::PreRelease();
                            ver.preRelease->from_string("beta");
                            break;
                        default:
                            break;
                        }
                    }
                }
            } else {
                throw WrongArgTypeException("ll::registerPlugin");
            }
        }

        if (getEngineOwnData()->plugin->getManifest().version.value_or(ll::data::Version(0, 0, 0)) != ver) {
            auto newManifest        = getEngineOwnData()->plugin->getManifest();
            newManifest.description = desc;
            newManifest.version     = ver;
            if (args.size() >= 4) {
                Local<Object> otherInfo = args[3].asObject();
                auto          keys      = otherInfo.getKeyNames();
                if (!newManifest.extraInfo) {
                    newManifest.extraInfo = ll::SmallDenseMap<std::string, std::string>();
                }
                for (auto& key : keys) {
                    if (ll::string_utils::toLowerCase(key) == "author") {
                        newManifest.author = otherInfo.get(key).asString().toString();
                        continue;
                    }
                    newManifest.extraInfo->insert({key, otherInfo.get(key).asString().toString()});
                }
            }
            ll::file_utils::writeFile(
                getEngineOwnData()->plugin->getModDir() / "manifest.json",
                ll::reflection::serialize<ordered_json>(newManifest)->dump(4)
            );
        }

        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::getPluginInfo(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto pluginName = args[0].asString().toString();
        auto plugin     = lse::LegacyScriptEngine::getInstance().getManager().getMod(pluginName);
        if (!plugin && pluginName == getEngineOwnData()->pluginName) {
            plugin = getEngineOwnData()->plugin;
        }
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
                for (auto const& [k, v] : plugin->getManifest().extraInfo.value()) {
                    others.set(k, v);
                }
                result.set("others", others);
            }

            return result;
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::getCurrentPluginInfo(Arguments const& args) {
    try {
        if (auto plugin = getEngineOwnData()->plugin) {
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
                for (auto const& [k, v] : plugin->getManifest().extraInfo.value()) {
                    others.set(k, v);
                }
                result.set("others", others);
            }

            return result;
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::versionString(Arguments const&) {
    try {
        if (auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version) {
            return String::newString(ver->to_string());
        }
        return String::newString("0.0.0");
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::requireVersion(Arguments const&) { return Boolean::newBoolean(true); }

Local<Value> LlClass::getAllPluginInfo(Arguments const&) {
    try {
        Local<Array> plugins = Array::newArray();

        for (auto& mod : ll::mod::ModManagerRegistry::getInstance().mods()) {

            // Create plugin object
            auto pluginObject = Object::newObject();

            pluginObject.set("name", mod.getManifest().name);
            if (mod.getManifest().description.has_value()) {
                pluginObject.set("desc", mod.getManifest().description.value());
            }
            pluginObject.set("type", mod.getType());

            auto ver = Array::newArray();
            ver.add(Number::newNumber(mod.getManifest().version->major));
            ver.add(Number::newNumber(mod.getManifest().version->minor));
            ver.add(Number::newNumber(mod.getManifest().version->patch));

            pluginObject.set("version", ver);
            pluginObject.set("versionStr", mod.getManifest().version->to_string());
            pluginObject.set("filePath", mod.getManifest().entry);

            if (mod.getManifest().extraInfo.has_value()) {
                auto others = Object::newObject();
                for (auto const& [k, v] : mod.getManifest().extraInfo.value()) {
                    others.set(k, v);
                }
                pluginObject.set("others", others);
            }

            // Add plugin object to list
            plugins.add(pluginObject);
        }
        return plugins;
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::onUnload(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kFunction);

    try {
        getEngineOwnData()->addUnloadCallback(
            [func = script::Global(args[0].asFunction())](std::shared_ptr<ScriptEngine> engine) {
                EngineScope enter(engine.get());
                func.get().call();
            }
        );
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::listPlugins(Arguments const&) {
    try {
        Local<Array> plugins = Array::newArray();

        for (auto& mod : ll::mod::ModManagerRegistry::getInstance().mods()) {
            plugins.add(String::newString(mod.getName()));
        }
        return plugins;
    }
    CATCH_AND_THROW
}

Local<Value> LlClass::require(Arguments const&) { return Boolean::newBoolean(true); }

Local<Value> LlClass::eval(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        return EngineScope::currentEngine()->eval(args[0].asString().toString());
    }
    CATCH_AND_THROW
}

// For Compatibility
Local<Value> LlClass::version(Arguments const&) {
    try {
        if (auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version) {
            auto version = Object::newObject();
            version.set("major", ver->major);
            version.set("minor", ver->minor);
            version.set("revision", ver->patch);
            version.set("isBeta", ver->preRelease.has_value());
            version.set("isRelease", !ver->preRelease.has_value());
            version.set("isDev", ver->to_string().find("+") != std::string::npos);
            return version;
        }
        return Object::newObject();
    }
    CATCH_AND_THROW
}
