#include "main/BuiltinCommands.h"

#include "api/APIHelp.h"
#include "ll/api/Logger.h"
#include "ll/api/command/DynamicCommand.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/utils/HashUtils.h"
#include "ll/api/utils/StringUtils.h"
#include "mc/server/commands/CommandParameterOption.h"
#include "mc/server/commands/CommandPermissionLevel.h"

#include <string>


#ifdef LLSE_BACKEND_PYTHON
#include "PythonHelper.h"
#endif

extern ll::Logger    logger;
extern bool          isInConsoleDebugMode;
extern ScriptEngine* debugEngine;

#define OUTPUT_DEBUG_SIGN() std::cout << "> " << std::flush

bool ProcessDebugEngine(const std::string& cmd) {
#ifdef LLSE_BACKEND_PYTHON
    // process python debug seperately
    return PythonHelper::processPythonDebugEngine(cmd;
#endif
    if (isInConsoleDebugMode) {
        EngineScope enter(debugEngine);
        try {
            if (cmd == "stop") {
                return true;
            } else {
                auto               result = debugEngine->eval(cmd);
                std::ostringstream sout;
                PrintValue(sout, result);
                logger.info(sout.str());
                OUTPUT_DEBUG_SIGN();
            }
        } catch (Exception& e) {
            PrintException(e);
            OUTPUT_DEBUG_SIGN();
        }
        return false;
    }
    return true;
}

void RegisterDebugCommand() {
    auto command = DynamicCommand::createCommand(
        ll::service::getCommandRegistry(),
        LLSE_DEBUG_CMD,
        "Debug LegacyScriptEngine",
        CommandPermissionLevel::Owner
    );
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
                logger.info(sout.str());
            } catch (Exception& e) {
                PrintException(e);
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
                OUTPUT_DEBUG_SIGN();
            }
        }
    });
    DynamicCommand::setup(ll::service::getCommandRegistry(), std::move(command));
}
