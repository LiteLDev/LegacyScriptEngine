#pragma once
#include "legacy/main/Global.h"
#include "lse/Plugin.h"
#include "utils/UsingScriptX.h"

#include <ll/api/Expected.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/io/Logger.h>
#include <ll/api/io/LoggerRegistry.h>
#include <string>
#include <unordered_map>

struct FormCallbackData {
    script::ScriptEngine*            engine;
    script::Global<script::Function> func;
};

struct RemoteCallData {
    std::string              nameSpace;
    std::string              funcName;
    script::Global<Function> callback;
};

// It is similar to ll::mod::Mod, it stores data of an engine(usually a plugin).
struct EngineOwnData {
    // Basic information
    std::string pluginName = {};
    std::string engineType = LLSE_BACKEND_TYPE;

    // RemoteCall Exported Functions: unordered_map<nameSpace, funcName>
    std::unordered_map<std::string, RemoteCallData> exportFuncs;

    // I18nAPI
    ll::i18n::I18n i18n;
    std::string    defaultLocaleName;

    std::shared_ptr<lse::Plugin> plugin;

    // Use standalone logger in EngineOwnData instead of plugin.getLogger() for allowing modify logger title.
    std::shared_ptr<ll::io::Logger> logger;

    // Player binding data
    std::unordered_map<std::string, script::Global<Value>> playerDataDB;

    // Unload Callbacks, use for close database...
    int                                                         index = 0;
    std::unordered_map<int, std::function<void(ScriptEngine*)>> unloadCallbacks;
    inline int addUnloadCallback(std::function<void(ScriptEngine*)>&& cb) {
        unloadCallbacks[++index] = cb;
        return index;
    }
    inline bool removeUnloadCallback(int pIndex) { return unloadCallbacks.erase(pIndex); }

    static void clearEngineObjects(ScriptEngine* engine) {
        EngineScope scope(engine);
        auto        data = std::static_pointer_cast<EngineOwnData>(engine->getData());
        data->playerDataDB.clear();
        data->unloadCallbacks.clear();
        // LLSERemoveAllExportedFuncs(engine);
        assert(data->exportFuncs.empty());
    }
};

// Engine additional data
inline std::shared_ptr<EngineOwnData> getEngineOwnData() {
    return std::static_pointer_cast<EngineOwnData>(EngineScope::currentEngine()->getData());
}

inline std::shared_ptr<EngineOwnData> getEngineData(script::ScriptEngine* engine) {
    return std::static_pointer_cast<EngineOwnData>(engine->getData());
}