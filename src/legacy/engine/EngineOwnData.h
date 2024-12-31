#pragma once
#include "dyncall/dyncall.h"
#include "legacy/main/Global.h"
#include "ll/api/base/Macro.h"
#include "ll/api/mod/Mod.h"
#include "lse/Entry.h"
#include "utils/UsingScriptX.h"
#include "lse/Plugin.h"

#include <ll/api/Expected.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/io/Logger.h>
#include <ll/api/io/LoggerRegistry.h>
#include <map>
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

/*
struct SimpleCallbackData
{
    ScriptEngine* engine;
    script::Global<script::Function> func;
    std::vector<script::Global<Value>> values;
};
*/

// It is similar to ll::mod::Mod, it stores data of an engine(usually a plugin).
struct EngineOwnData {
    // 基础信息
    std::string pluginName          = "";
    std::string pluginFileOrDirPath = "";
    std::string engineType          = LLSE_BACKEND_TYPE;

    // 表单回调
    std::map<unsigned, FormCallbackData> formCallbacks;

    // RemoteCall Exported Functions: unordered_map<nameSpace, funcName>
    std::unordered_map<std::string, RemoteCallData> exportFuncs;

    /*
    uint64_t simpleCallbackIndex = 0;
    std::unordered_map<uint64_t, SimpleCallbackData> simpleCallbacks;

    inline uint64_t addSimpleCallback(script::Local<Function> func,
    std::vector<script::Local<Value>> values)
    {
        auto index = ++simpleCallbackIndex;
        std::vector<script::Global<Value>> globalValues;
        for (auto& value : values)
            globalValues.emplace_back(value);
        SimpleCallbackData data{EngineScope::currentEngine(),
    script::Global<Function>(func), std::move(globalValues)};
        simpleCallbacks.emplace(index, std::move(data));
        return index;
    }
    inline bool removeSimpleCallback(uint64_t index)
    {
        return simpleCallbacks.erase(index);
    }
    */

    // I18nAPI
    std::shared_ptr<ll::i18n::I18n> i18n = nullptr;
    std::shared_ptr<lse::Plugin>    plugin;

    // 玩家绑定数据
    std::unordered_map<std::string, script::Global<Value>> playerDataDB;

    // Dynamic Call vm for NativeFFI
    DCCallVM* dynamicCallVM;

    // Unload Callbacks, use for close database...
    int                                                         index = 0;
    std::unordered_map<int, std::function<void(ScriptEngine*)>> unloadCallbacks;
    inline int addUnloadCallback(std::function<void(ScriptEngine*)>&& cb) {
        unloadCallbacks[++index] = cb;
        return index;
    }
    inline bool removeUnloadCallback(int pIndex) { return unloadCallbacks.erase(pIndex); }

    // Init
    EngineOwnData() {
        dynamicCallVM = dcNewCallVM(4096);
        dcMode(dynamicCallVM, DC_CALL_C_DEFAULT);
    }
};

// 引擎附加数据
inline std::shared_ptr<EngineOwnData> getEngineOwnData() {
    return std::static_pointer_cast<EngineOwnData>(EngineScope::currentEngine()->getData());
}

inline std::shared_ptr<EngineOwnData> getEngineData(script::ScriptEngine* engine) {
    return std::static_pointer_cast<EngineOwnData>(engine->getData());
}