#include "api/LlAPI.h"

#include "api/APIHelp.h"
#include "engine/GlobalShareData.h"
#include "ll/api/Versions.h"
#include "ll/api/data/Version.h"
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
    CATCH("Fail in getLanguage")
}

Local<Value> LlClass::isWine() {
    try {
        return Boolean::newBoolean(ll::sys_utils::isWine());
    }
    CATCH("Fail in isWine")
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
        auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version;
        if (ver) {
            return Boolean::newBoolean(!ver->preRelease.has_value());
        } else {
            return Boolean::newBoolean(false);
        }
    }
    CATCH("Fail in isRelease")
}

Local<Value> LlClass::isBeta() {
    try {
        auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version;
        if (ver) {
            return Boolean::newBoolean(ver->preRelease.has_value());
        } else {
            return Boolean::newBoolean(false);
        }
    }
    CATCH("Fail in isBeta")
}

Local<Value> LlClass::isDev() {
    try {
        auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version;
        if (ver) {
            return Boolean::newBoolean(ver->to_string().find("+") != std::string::npos);
        } else {
            return Boolean::newBoolean(true);
        }
    }
    CATCH("Fail in isDev");
}

Local<Value> LlClass::getMajorVersion() {
    try {
        auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version;
        if (ver) {
            return Number::newNumber(ver->major);
        } else {
            return Number::newNumber(0);
        }
    }
    CATCH("Fail in getMajorVersion")
}

Local<Value> LlClass::getMinorVersion() {
    try {
        auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version;
        if (ver) {
            return Number::newNumber(ver->minor);
        } else {
            return Number::newNumber(0);
        }
    }
    CATCH("Fail in getMinorVersion")
}

Local<Value> LlClass::getRevisionVersion() {
    try {
        auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version;
        if (ver) {
            return Number::newNumber(ver->patch);
        } else {
            return Number::newNumber(0);
        }
    }
    CATCH("Fail in getRevisionVersion")
}

Local<Value> LlClass::getScriptEngineVersion() {
    try {
        return String::newString(EngineScope::currentEngine()->getEngineVersion());
    }
    CATCH("Fail in getScriptEngineVersion")
}

Local<Value> LlClass::getVersionStatus() {
    auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version;
    if (ver) {
        if (ver->to_string().find("+") != std::string::npos) {
            return Number::newNumber(0);
        } else if (ver->preRelease.has_value()) {
            return Number::newNumber(1);
        } else {
            return Number::newNumber(2);
        }
    } else {
        return Number::newNumber(0);
    }
}

Local<Value> LlClass::registerPlugin(const Arguments& args) {
    if (args.size() == 0) {
        return Boolean::newBoolean(true);
    } else {
        CHECK_ARG_TYPE(args[0], ValueKind::kString);
    }
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
                LOG_WRONG_ARG_TYPE();
                return Boolean::newBoolean(false);
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
                ll::reflection::serialize<nlohmann::ordered_json>(newManifest)->dump(4)
            );
        }

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in registerPlugin");
}

Local<Value> LlClass::getPluginInfo(const Arguments& args) {
    try {
        auto plugin = lse::LegacyScriptEngine::getInstance().getManager().getMod(args[0].asString().toString());
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
    CATCH("Fail in getPluginInfo");
}
Local<Value> LlClass::versionString(const Arguments&) {
    try {
        auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version;
        if (ver) {
            return String::newString(ver->to_string());
        } else {
            return String::newString("0.0.0");
        }
    }
    CATCH("Fail in versionString!")
}

Local<Value> LlClass::requireVersion(const Arguments&) { return Boolean::newBoolean(true); }

Local<Value> LlClass::getAllPluginInfo(const Arguments&) {
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
                for (const auto& [k, v] : mod.getManifest().extraInfo.value()) {
                    others.set(k, v);
                }
                pluginObject.set("others", others);
            }

            // Add plugin object to list
            plugins.add(pluginObject);
        }
        return plugins;
    }
    CATCH("Fail in getAllPluginInfo");
}

Local<Value> LlClass::onUnload(const script::Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kFunction);

    try {
        getEngineOwnData()->addUnloadCallback([func = script::Global<Function>(args[0].asFunction()
                                               )](ScriptEngine* engine) {
            EngineScope enter(engine);
            func.get().call();
        });
        return {};
    }
    CATCH("Fail in onUnload");
}

Local<Value> LlClass::listPlugins(const Arguments&) {
    try {
        Local<Array> plugins = Array::newArray();

        for (auto& mod : ll::mod::ModManagerRegistry::getInstance().mods()) {
            plugins.add(String::newString(mod.getName()));
        }
        return plugins;
    }
    CATCH("Fail in listPlugins");
}

Local<Value> LlClass::require(const Arguments&) { return Boolean::newBoolean(true); }

Local<Value> LlClass::eval(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        return EngineScope::currentEngine()->eval(args[0].asString().toString());
    }
    CATCH("Fail in eval!")
}

// For Compatibility
Local<Value> LlClass::version(const Arguments&) {
    try {
        auto& ver = lse::LegacyScriptEngine::getInstance().getSelf().getManifest().version;
        if (ver) {
            auto version = Object::newObject();
            version.set("major", ver->major);
            version.set("minor", ver->minor);
            version.set("revision", ver->patch);
            version.set("isBeta", ver->preRelease.has_value());
            version.set("isRelease", !ver->preRelease.has_value());
            version.set("isDev", ver->to_string().find("+") != std::string::npos);
            return version;
        } else {
            return Object::newObject();
        }
    }
    CATCH("Fail in version!")
}
