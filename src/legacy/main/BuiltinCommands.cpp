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

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON
#include "PythonHelper.h"
#endif

extern bool                     isInConsoleDebugMode;
extern ScriptEngine*            debugEngine;
std::shared_ptr<ll::io::Logger> debugLogger = ll::io::LoggerRegistry::getInstance().getOrCreate("LSEDEBUG");
inline void                     printConsoleSymbol() { debugLogger->info("> "); }

bool ProcessDebugEngine(const std::string& cmd) {
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON
    // process python debug seperately
    return PythonHelper::processPythonDebugEngine(cmd);
#endif
    if (isInConsoleDebugMode) {
        EngineScope enter(debugEngine);
        auto&       logger = lse::LegacyScriptEngine::getInstance().getSelf().getLogger();
        try {
            if (cmd == "stop" || cmd == LLSE_DEBUG_CMD) {
                return true;
            } else {
                auto               result = debugEngine->eval(cmd);
                std::ostringstream sout;
                PrintValue(sout, result);
                logger.info(sout.str());
                printConsoleSymbol();
            }
        } catch (...) {
            ll::error_utils::printCurrentException(logger);
            printConsoleSymbol();
        }
        return false;
    }
    return true;
}

struct EngineDebugCommand {
    CommandRawText eval;
};

void RegisterDebugCommand() {
    debugLogger->setFormatter(ll::makePolymorphic<lse::io::DirectFormatter>());
    auto& command = ll::command::CommandRegistrar::getInstance()
                        .getOrCreateCommand(LLSE_DEBUG_CMD, "Debug LegacyScriptEngine", CommandPermissionLevel::Owner);
    command.overload<EngineDebugCommand>().optional("eval").execute(
        [](CommandOrigin const&, CommandOutput& output, EngineDebugCommand const& param) {
            auto& logger = lse::LegacyScriptEngine::getInstance().getSelf().getLogger();
            if (!param.eval.getText().empty()) {
                EngineScope enter(debugEngine);
                try {
                    auto               result = debugEngine->eval(param.eval.getText());
                    std::ostringstream sout;
                    PrintValue(sout, result);
                    output.success(sout.str());
                } catch (...) {
                    ll::error_utils::printCurrentException(logger);
                }
            } else {
                if (isInConsoleDebugMode) {
                    // EndDebug
                    logger.info("Debug mode ended");
                    isInConsoleDebugMode = false;
                } else {
                    // StartDebug
                    logger.info("Debug mode begins");
                    isInConsoleDebugMode = true;
                    printConsoleSymbol();
                }
            }
        }
    );
}
