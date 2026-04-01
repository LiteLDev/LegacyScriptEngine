#include "legacy/api/APIHelp.h"
#include "legacy/api/McAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "legacy/engine/EngineManager.h"
#include "legacy/engine/EngineOwnData.h"
#include "legacy/engine/GlobalShareData.h"
#include "legacy/engine/LocalShareData.h"
#include "legacy/utils/Utils.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "mc/server/commands/CommandOutput.h"

#include <ranges>
#include <string>
#include <vector>

namespace lse::legacy_command {
void registerLegacyCommand(
    std::string const&                             name,
    std::string const&                             description,
    CommandPermissionLevel const                   level,
    std::shared_ptr<ScriptEngine> const&           engine,
    std::optional<script::Global<Function>> const& playerFunc,
    std::optional<script::Global<Function>> const& consoleFunc
) {
    using namespace ll::command;

    EngineScope enter(engine.get());
    auto&       plugin = getEngineData(engine)->plugin;

    auto  cmdParas    = ll::string_utils::splitByPattern(name, " ");
    bool  singleParam = cmdParas.size() == 1;
    auto& command =
        CommandRegistrar::getInstance(false)
            .getOrCreateCommand(std::string(cmdParas[0]), description, level, CommandFlagValue::NotCheat, plugin);
    auto overload      = command.runtimeOverload(plugin);
    bool hasSubCommand = false;
    if (!singleParam) {
        for (size_t i = 1; i < cmdParas.size(); ++i) {
            overload.text(cmdParas[i]);
        }
    }
    // To ensure compatibility for https://github.com/bricktea/iLand-Core
    hasSubCommand = std::ranges::any_of(localShareData->fakeCommandsMap | std::views::keys, [&](auto const& cmd) {
        return cmd.starts_with(name + " ");
    });
    if (!hasSubCommand) {
        overload.optional("args", ParamKind::RawText);
    }
    // Workaround to add description for sub commands
    if (!singleParam) {
        overload.optional("§e" + description + "§r", ParamKind::Bool);
    }
    overload.execute([playerFunc, consoleFunc, engine, hasSubCommand](
                         CommandOrigin const&  origin,
                         CommandOutput&        output,
                         RuntimeCommand const& command
                     ) {
        if (!engine || (!playerFunc && !consoleFunc)) return;
        EngineScope  enterInner(engine.get());
        Local<Array> params = Array::newArray();
        if (!hasSubCommand && command["args"].hold(ParamKind::RawText)) {
            for (auto& para :
                 ll::string_utils::splitByPattern(std::get<CommandRawText>(command["args"].value()).mText, " ")) {
                params.add(String::newString(para));
            }
        }
        if (origin.getOriginType() == CommandOriginType::Player && playerFunc) {
            if (origin.getPermissionsLevel() < command.mPermissionLevel) {
                output.error("You don't have permission to use this command."_tr());
                return;
            }
            playerFunc->get().call({}, PlayerClass::newPlayer(static_cast<Player*>(origin.getEntity())), params);
        }
        if (origin.getOriginType() == CommandOriginType::DedicatedServer && consoleFunc) {
            consoleFunc->get().call({}, params);
        }
    });
}

void newLegacyCommand(
    bool                   isPlayer,
    std::string            name,
    std::string const&     description,
    CommandPermissionLevel level,
    Local<Function> const& func
) {
    if (name[0] == '/') name = name.erase(0, 1);
    auto& fakeCommands = localShareData->fakeCommandsMap;
    if (isPlayer) {
        if (fakeCommands.contains(name)) {
            fakeCommands[name].playerFunc = script::Global(func);
        } else {
            fakeCommands[name] = {
                description,
                level,
                EngineManager::checkAndGet(EngineScope::currentEngine()),
                script::Global(func),
                {}
            };
        }
    } else {
        if (fakeCommands.contains(name)) {
            fakeCommands[name].consoleFunc = script::Global(func);
        } else {
            fakeCommands[name] = {
                description,
                level,
                EngineManager::checkAndGet(EngineScope::currentEngine()),
                {},
                script::Global(func)
            };
        }
    }
}

void registerLegacyCommands() {
    for (auto& [name, data] : localShareData->fakeCommandsMap) {
        try {
            registerLegacyCommand(name, data.description, data.level, data.engine, data.playerFunc, data.consoleFunc);
        } catch (...) {
            LegacyScriptEngine::getLogger().error("Failed to register legacy command: {}"_tr(name));
            ll::error_utils::printCurrentException(LegacyScriptEngine::getLogger());
        }
    }
    localShareData->fakeCommandsMap.clear();
}
} // namespace lse::legacy_command

Local<Value> McClass::regPlayerCmd(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kFunction);
    if (args.size() >= 4) CHECK_ARG_TYPE(args[3], ValueKind::kNumber);

    try {
        int level = 0;

        if (args.size() >= 4) {
            int newLevel = args[3].asNumber().toInt32();
            if (newLevel >= 0 && newLevel <= 5) level = newLevel;
        }

        lse::legacy_command::newLegacyCommand(
            true,
            args[0].asString().toString(),
            args[1].asString().toString(),
            static_cast<CommandPermissionLevel>(level),
            args[2].asFunction()
        );
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::regConsoleCmd(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kFunction);

    try {
        lse::legacy_command::newLegacyCommand(
            false,
            args[0].asString().toString(),
            args[1].asString().toString(),
            CommandPermissionLevel::Owner,
            args[2].asFunction()
        );
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::sendCmdOutput(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        lse::LegacyScriptEngine::getLogger().info(args[0].asString().toString());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}
