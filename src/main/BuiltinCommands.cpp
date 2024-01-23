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
    return PythonHelper::processPythonDebugEngine(cmd);
#endif

    if (cmd == LLSE_DEBUG_CMD) {
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
        return false;
    }
    if (isInConsoleDebugMode) {
        EngineScope enter(debugEngine);
        try {
            if (cmd == "stop") {
                return true;
            } else {
                auto result = debugEngine->eval(cmd);
                PrintValue(std::cout, result);
                std::cout << std::endl;
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
        "lsedebug",
        "Debug LegacyScriptEngine",
        CommandPermissionLevel::Owner
    );
    command->mandatory("eval", DynamicCommand::ParameterType::RawText);
    command->addOverload("eval");
    command->setCallback([](DynamicCommand const&                                    cmd,
                            CommandOrigin const&                                     origin,
                            CommandOutput&                                           output,
                            std::unordered_map<std::string, DynamicCommand::Result>& results) {
        EngineScope enter(debugEngine);
        try {
            auto result = debugEngine->eval(results["eval"].getRaw<std::string>());
            PrintValue(std::cout, result);
            std::cout << std::endl;
        } catch (Exception& e) {
            PrintException(e);
        }
    });
    DynamicCommand::setup(ll::service::getCommandRegistry(), std::move(command));
}
