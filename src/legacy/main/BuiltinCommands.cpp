#include "main/BuiltinCommands.h"

#include "api/APIHelp.h"
#include "ll/api/command/Command.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/io/Logger.h"
#include "lse/Entry.h"
#include "lse/api/DirectFormatter.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"

#include <string>

#ifdef LSE_BACKEND_PYTHON
#include "PythonHelper.h"
#endif

extern bool                          InConsoleDebugMode;
extern std::shared_ptr<ScriptEngine> DebugEngine;
std::shared_ptr<ll::io::Logger>      DebugCmdLogger = ll::io::LoggerRegistry::getInstance().getOrCreate("LSEDEBUG");

inline void PrintDebugSign() { DebugCmdLogger->info("> "); }

bool ProcessDebugEngine(const std::string& cmd) {
#ifdef LSE_BACKEND_PYTHON
    // process python debug seperately
    return PythonHelper::processPythonDebugEngine(cmd);
#endif
    if (InConsoleDebugMode) {
        EngineScope enter(DebugEngine.get());
        auto&       logger = lse::LegacyScriptEngine::getInstance().getSelf().getLogger();
        try {
            if (cmd == "stop" || cmd == LLSE_DEBUG_CMD) {
                return true;
            } else {
                auto               result = DebugEngine->eval(cmd);
                std::ostringstream sout;
                PrintValue(sout, result);
                logger.info(sout.str());
                PrintDebugSign();
            }
        } catch (...) {
            ll::error_utils::printCurrentException(logger);
            PrintDebugSign();
        }
        return false;
    }
    return true;
}

struct EngineDebugCommand {
    CommandRawText eval;
};

void RegisterDebugCommand() {
    DebugCmdLogger->setFormatter(ll::makePolymorphic<lse::io::DirectFormatter>());
    // Node.js engine doesn't support debug engine, Python engine don't need to register command.
#if (!defined LSE_BACKEND_NODEJS) && (!defined LSE_BACKEND_PYTHON)
    auto& command = ll::command::CommandRegistrar::getInstance(false).getOrCreateCommand(
        LLSE_DEBUG_CMD,
        "Debug LegacyScriptEngine",
        static_cast<CommandPermissionLevel>(lse::LegacyScriptEngine::getInstance().getConfig().debugCommandLevel)
    );
    command.overload<EngineDebugCommand>().optional("eval").execute(
        [](CommandOrigin const&, CommandOutput& output, EngineDebugCommand const& param) {
            auto& logger = lse::LegacyScriptEngine::getInstance().getSelf().getLogger();
            if (!param.eval.mText.empty()) {
                EngineScope enter(DebugEngine.get());
                try {
                    auto               result = DebugEngine->eval(param.eval.mText);
                    std::ostringstream sout;
                    PrintValue(sout, result);
                    output.success(sout.str());
                } catch (...) {
                    ll::error_utils::printCurrentException(logger);
                }
            } else {
                if (InConsoleDebugMode) {
                    // EndDebug
                    logger.info("Debug mode ended");
                    InConsoleDebugMode = false;
                } else {
                    // StartDebug
                    logger.info("Debug mode begins");
                    InConsoleDebugMode = true;
                    PrintDebugSign();
                }
            }
        }
    );
#endif
}
