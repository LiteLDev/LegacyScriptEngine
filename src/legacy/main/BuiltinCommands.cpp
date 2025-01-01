#include "main/BuiltinCommands.h"

#include "api/APIHelp.h"
#include "ll/api/io/Logger.h"
#include "ll/api/service/Bedrock.h"
#include "lse/Entry.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "ll/api/command/Command.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "mc/server/commands/CommandOutput.h"

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

struct EngineDebugCommand {
    std::string eval;
};

void RegisterDebugCommand() {
    auto& command = ll::command::CommandRegistrar::getInstance()
                        .getOrCreateCommand(LLSE_DEBUG_CMD, "Debug LegacyScriptEngine", CommandPermissionLevel::Owner);
    command.overload<EngineDebugCommand>().optional("eval").execute(
        [](CommandOrigin const&, CommandOutput& output, EngineDebugCommand const& param) {
            if (!param.eval.empty()) {
                EngineScope enter(debugEngine);
                try {
                    auto               result = debugEngine->eval(param.eval);
                    std::ostringstream sout;
                    PrintValue(sout, result);
                    output.success(sout.str());
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
        }
    );
}
