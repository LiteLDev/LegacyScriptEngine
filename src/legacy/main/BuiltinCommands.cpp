#include "main/BuiltinCommands.h"

#include "api/APIHelp.h"
#include "legacyapi/command/DynamicCommand.h"
#include "ll/api/io/Logger.h"
#include "ll/api/service/Bedrock.h"
#include "lse/Entry.h"
#include "mc/server/commands/CommandPermissionLevel.h"

#include <string>

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON
#include "PythonHelper.h"
#endif

extern bool          isInConsoleDebugMode;
extern ScriptEngine* debugEngine;

bool ProcessDebugEngine(const std::string& cmd) {
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON
    // process python debug seperately
    return PythonHelper::processPythonDebugEngine(cmd);
#endif
    if (isInConsoleDebugMode) {
        EngineScope enter(debugEngine);
        try {
            if (cmd == "stop" || cmd == LLSE_DEBUG_CMD) {
                return true;
            } else {
                auto               result = debugEngine->eval(cmd);
                std::ostringstream sout;
                PrintValue(sout, result);
                lse::getSelfPluginInstance().getLogger().info(sout.str());
                std::cout << "> " << std::flush;
            }
        } catch (Exception& e) {
            PrintException(e);
            std::cout << "> " << std::flush;
        }
        return false;
    }
    return true;
}

void RegisterDebugCommand() {
    auto command =
        DynamicCommand::createCommand(LLSE_DEBUG_CMD, "Debug LegacyScriptEngine", CommandPermissionLevel::Owner);
    command->optional("eval", DynamicCommand::ParameterType::RawText);
    command->addOverload("eval");
    command->setCallback([](DynamicCommand const&,
                            CommandOrigin const&,
                            CommandOutput&,
                            std::unordered_map<std::string, DynamicCommand::Result>& results) {
        if (results["eval"].isSet) {
            EngineScope enter(debugEngine);
            try {
                auto               result = debugEngine->eval(results["eval"].getRaw<std::string>());
                std::ostringstream sout;
                PrintValue(sout, result);
                lse::getSelfPluginInstance().getLogger().info(sout.str());
            } catch (Exception& e) {
                PrintException(e);
            }
        } else {
            if (isInConsoleDebugMode) {
                // EndDebug
                lse::getSelfPluginInstance().getLogger().info("Debug mode ended");
                isInConsoleDebugMode = false;
            } else {
                // StartDebug
                lse::getSelfPluginInstance().getLogger().info("Debug mode begins");
                isInConsoleDebugMode = true;
                std::cout << "> " << std::flush;
            }
        }
    });
    DynamicCommand::setup(ll::service::getCommandRegistry(), std::move(command));
}
