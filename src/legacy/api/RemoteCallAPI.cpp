#include "api/APIHelp.h"
#include "api/LlAPI.h"
#include "engine/GlobalShareData.h"
#include "engine/MessageSystem.h"
#include "ll/api/service/ServerInfo.h"

#include <process.h>
#include <string>

#define DEFAULT_REMOTE_CALL_NAME_SPACE "LLSEGlobal"

Local<Value> MakeRemoteCall(const string& nameSpace, const string& funcName, const Arguments& args) {
    auto data = globalShareData->exportedFuncs.find(nameSpace + ":" + funcName);
    if (data == globalShareData->exportedFuncs.end()) {
        lse::getSelfPluginInstance().getLogger().error(
            "Fail to import! Function [{}::{}] has not been exported!",
            nameSpace,
            funcName
        );
        lse::getSelfPluginInstance().getLogger().error("In plugin <{}>", ENGINE_OWN_DATA()->pluginName);
        return Local<Value>();
    }

    std::vector<std::string> params;
    for (int i = 0; i < args.size(); ++i) {
        params.emplace_back(ValueToJson(args[i]));
    }
    return JsonToValue(data->second.callback(std::move(params)));
}

bool LLSEExportFunc(
    ScriptEngine*          engine,
    const Local<Function>& func,
    const string&          nameSpace,
    const string&          funcName
) {
    std::string       exportName = nameSpace + ":" + funcName;
    ExportedFuncData* funcData   = &(globalShareData->exportedFuncs)[exportName];
    if (funcData->engine) return false;
    funcData->engine         = engine;
    funcData->func           = script::Global<Function>(func);
    funcData->fromEngineType = LLSE_MODULE_TYPE;
    funcData->callback       = [exportName](std::vector<std::string> params) -> std::string {
        auto data = globalShareData->exportedFuncs.find(exportName);
        if (data == globalShareData->exportedFuncs.end()) {
            lse::getSelfPluginInstance().getLogger().error("Exported function \"{}\" not found", exportName);
            return "";
        }
        auto engine = data->second.engine;
        if ((ll::getServerStatus() != ll::ServerStatus::Running) || !EngineManager::isValid(engine)
            || engine->isDestroying())
            return "";
        EngineScope                       enter(data->second.engine);
        std::vector<script::Local<Value>> scriptParams;
        for (auto& param : params) {
            scriptParams.emplace_back(JsonToValue(param));
        }
        return ValueToJson(data->second.func.get().call({}, scriptParams));
    };
    return true;
}

bool LLSERemoveAllExportedFuncs(ScriptEngine* engine) {
    // enter scope to prevent crash in script::Global::~Global()
    EngineScope                                      enter(engine);
    std::vector<std::pair<std::string, std::string>> funcs;
    for (auto& [key, data] : ENGINE_GET_DATA(engine)->exportFuncs) {
        funcs.emplace_back(data.nameSpace, data.funcName);
    }
    ENGINE_GET_DATA(engine)->exportFuncs.clear();
    return true;
}

//////////////////// APIs ////////////////////

Local<Value> LlClass::exportFunc(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kFunction);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        std::string nameSpace;
        std::string funcName;
        if (args.size() > 2) {
            CHECK_ARG_TYPE(args[2], ValueKind::kString);
            nameSpace = args[1].toStr();
            funcName  = args[2].toStr();
        } else {
            nameSpace = DEFAULT_REMOTE_CALL_NAME_SPACE;
            funcName  = args[1].toStr();
        }
        return Boolean::newBoolean(
            LLSEExportFunc(EngineScope::currentEngine(), args[0].asFunction(), nameSpace, funcName)
        );
    }
    CATCH("Fail in LLSEExport!");
}

Local<Value> LlClass::importFunc(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        std::string nameSpace;
        std::string funcName;
        if (args.size() > 1) {
            CHECK_ARG_TYPE(args[1], ValueKind::kString);
            nameSpace = args[0].toStr();
            funcName  = args[1].toStr();
        } else {
            nameSpace = DEFAULT_REMOTE_CALL_NAME_SPACE;
            funcName  = args[0].toStr();
        }

        // 远程调用
        return Function::newFunction([nameSpace{nameSpace}, funcName{funcName}](const Arguments& args) -> Local<Value> {
            return MakeRemoteCall(nameSpace, funcName, args);
        });
    }
    CATCH("Fail in LLSEImport!")
}

Local<Value> LlClass::hasFuncExported(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        std::string nameSpace;
        std::string funcName;
        if (args.size() > 1) {
            CHECK_ARG_TYPE(args[1], ValueKind::kString);
            nameSpace = args[0].toStr();
            funcName  = args[1].toStr();
        } else {
            nameSpace = DEFAULT_REMOTE_CALL_NAME_SPACE;
            funcName  = args[0].toStr();
        }

        return Boolean::newBoolean(
            EngineOwnData().exportFuncs.find(nameSpace + ":" + funcName) != EngineOwnData().exportFuncs.end()
        );
    }
    CATCH("Fail in LLSEHasExported!")
}
