#include "api/APIHelp.h"
#include "api/McAPI.h"
#include "api/PlayerAPI.h"
#include "engine/EngineOwnData.h"
#include "engine/GlobalShareData.h"
#include "engine/LocalShareData.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/service/Bedrock.h"
#include "main/Global.h"
#include "utils/Utils.h"

#include <string>
#include <vector>

//////////////////// Helper ////////////////////

void RegisterCmd(const std::string& cmd, const std::string& describe, int cmdLevel) {
    auto& registry = ll::service::getCommandRegistry().get();
    registry.registerCommand(
        cmd,
        describe.c_str(),
        (CommandPermissionLevel)cmdLevel,
        {(CommandFlagValue)0},
        {(CommandFlagValue)0x80}
    );
}
// Helper
void LLSERegisterNewCmd(
    bool               isPlayerCmd,
    std::string        cmd,
    const std::string& describe,
    int                level,
    Local<Function>    func
) {
    if (cmd[0] == '/') cmd = cmd.erase(0, 1);

    if (isPlayerCmd) {
        localShareData->playerCmdCallbacks[cmd] = {EngineScope::currentEngine(), level, script::Global<Function>(func)};
        globalShareData->playerRegisteredCmd[cmd] = LLSE_BACKEND_TYPE;
    } else {
        localShareData
            ->consoleCmdCallbacks[cmd] = {EngineScope::currentEngine(), level, script::Global<Function>(func)};
        globalShareData->consoleRegisteredCmd[cmd] = LLSE_BACKEND_TYPE;
    }

    // 延迟注册
    if (isCmdRegisterEnabled) RegisterCmd(cmd, describe, level);
    else toRegCmdQueue.push_back({cmd, describe, level});
}

bool LLSERemoveCmdRegister(ScriptEngine* engine) {
    std::erase_if(localShareData->playerCmdCallbacks, [&engine](auto& data) {
        return data.second.fromEngine == engine;
    });
    std::erase_if(localShareData->consoleCmdCallbacks, [&engine](auto& data) {
        return data.second.fromEngine == engine;
    });
    return true;
}
// Helper

Local<Value> McClass::regPlayerCmd(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kFunction);
    if (args.size() >= 4) CHECK_ARG_TYPE(args[3], ValueKind::kNumber);

    try {
        std::string cmd      = args[0].asString().toString();
        std::string describe = args[1].asString().toString();
        int         level    = 0;

        if (args.size() >= 4) {
            int newLevel = args[3].asNumber().toInt32();
            if (newLevel >= 0 && newLevel <= 3) level = newLevel;
        }

        LLSERegisterNewCmd(true, cmd, describe, level, args[2].asFunction());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in RegisterPlayerCmd!");
}

Local<Value> McClass::regConsoleCmd(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kFunction);

    try {
        std::string cmd      = args[0].asString().toString();
        std::string describe = args[1].asString().toString();

        LLSERegisterNewCmd(false, cmd, describe, 4, args[2].asFunction());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in RegisterConsoleCmd!");
}

// Helper
bool SendCmdOutput(const std::string& output) {
    lse::LegacyScriptEngine::getInstance().getSelf().getLogger().info(output);
    return true;
}
// Helper

Local<Value> McClass::sendCmdOutput(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        return Boolean::newBoolean(SendCmdOutput(args[0].asString().toString()));
    }
    CATCH("Fail in SendCmdOutput!");
}

void ProcessRegCmdQueue() {
    for (auto& cmdData : toRegCmdQueue) {
        RegisterCmd(cmdData.cmd, cmdData.describe, cmdData.level);
    }
    toRegCmdQueue.clear();
}

std::string LLSEFindCmdReg(
    bool                      isPlayerCmd,
    const std::string&        cmd,
    std::vector<std::string>& receiveParas,
    bool*                     fromOtherEngine
) {
    std::unordered_map<std::string, std::string>& registeredMap =
        isPlayerCmd ? globalShareData->playerRegisteredCmd : globalShareData->consoleRegisteredCmd;
    for (auto& [prefix, fromEngine] : registeredMap) {
        if (cmd == prefix || (cmd.find(prefix) == 0 && cmd[prefix.size()] == ' '))
        // 如果命令与注册前缀全匹配，或者目标前缀后面为空格
        {
            // Matched
            if (fromEngine != LLSE_BACKEND_TYPE) {
                *fromOtherEngine = true;
                return {};
            }
        }
    }

    std::map<std::string, CmdCallbackData, CmdCallbackMapCmp>& cmdMap =
        isPlayerCmd ? localShareData->playerCmdCallbacks : localShareData->consoleCmdCallbacks;

    for (auto& cmdData : cmdMap) {
        std::string prefix = cmdData.first;
        if (cmd == prefix || (cmd.find(prefix) == 0 && cmd[prefix.size()] == ' '))
        // 如果命令与注册前缀全匹配，或者目标前缀后面为空格
        {
            // Matched
            if (cmd.size() > prefix.size()) {
                // 除了注册前缀之外还有额外参数
                receiveParas = SplitCmdLine(cmd.substr(prefix.size() + 1));
            } else receiveParas = std::vector<std::string>();

            return prefix;
        }
    }
    return {};
}

bool CallPlayerCmdCallback(Player* player, const std::string& cmdPrefix, const std::vector<std::string>& paras) {
    EngineScope  enter(localShareData->playerCmdCallbacks[cmdPrefix].fromEngine);
    auto         cmdData = localShareData->playerCmdCallbacks[cmdPrefix];
    Local<Value> res{};
    try {
        Local<Array> args = Array::newArray();
        for (auto& para : paras) args.add(String::newString(para));
        res = cmdData.func.get().call({}, PlayerClass::newPlayer(player), args);
    }
    CATCH_IN_CALLBACK("PlayerCmd");
    if (res.isNull() || (res.isBoolean() && res.asBoolean().value() == false)) return false;

    return true;
}

bool CallServerCmdCallback(const std::string& cmdPrefix, const std::vector<std::string>& paras) {
    EngineScope  enter(localShareData->consoleCmdCallbacks[cmdPrefix].fromEngine);
    auto         cmdData = localShareData->consoleCmdCallbacks[cmdPrefix];
    Local<Value> res{};
    try {
        Local<Array> args = Array::newArray();
        for (auto& para : paras) args.add(String::newString(para));
        res = cmdData.func.get().call({}, args);
    }
    CATCH_IN_CALLBACK("ServerCmd");
    if (res.isNull() || (res.isBoolean() && res.asBoolean().value() == false)) return false;

    return true;
}
