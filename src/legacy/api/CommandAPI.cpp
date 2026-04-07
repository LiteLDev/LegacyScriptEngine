#include "legacy/api/CommandAPI.h"

#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/CommandOriginAPI.h"
#include "legacy/api/CommandOutputAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/McAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "legacy/engine/EngineManager.h"
#include "legacy/engine/EngineOwnData.h"
#include "legacy/engine/GlobalShareData.h"
#include "legacy/engine/LocalShareData.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "magic_enum.hpp"
#include "mc/_HeaderOutputPredefine.h"
#include "mc/deps/core/utility/MCRESULT.h"
#include "mc/deps/json/FastWriter.h"
#include "mc/deps/json/Value.h"
#include "mc/locale/I18n.h"
#include "mc/locale/Localization.h"
#include "mc/server/commands/CommandBlockName.h"
#include "mc/server/commands/CommandBlockNameResult.h"
#include "mc/server/commands/CommandContext.h"
#include "mc/server/commands/CommandOutputType.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/server/commands/CommandVersion.h"
#include "mc/server/commands/CurrentCmdVersion.h"
#include "mc/server/commands/GenerateMessageResult.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/ServerCommandOrigin.h"
#include "mc/world/Minecraft.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/dimension/Dimension.h"

#include <string>
#include <vector>

using namespace ll::command;

//////////////////// Class Definition ////////////////////

ClassDefine<void> PermissionStaticBuilder  = EnumDefineBuilder<OldCommandPermissionLevel>::build("PermType");
ClassDefine<void> ParamTypeStaticBuilder   = EnumDefineBuilder<OldParameterType>::build("ParamType");
ClassDefine<void> ParamOptionStaticBuilder = EnumDefineBuilder<CommandParameterOption>::build("ParamOption");

ClassDefine<CommandClass> CommandClassBuilder =
    defineClass<CommandClass>("LLSE_Command")
        .constructor(nullptr)
        .instanceProperty("name", &CommandClass::getName)
        .instanceProperty("registered", &CommandClass::isRegistered)

        .instanceFunction("setEnum", &CommandClass::setEnum)
        .instanceFunction("setAlias", &CommandClass::setAlias)
        //.instanceFunction("newParameter", &CommandClass::newParameter)
        .instanceFunction("mandatory", &CommandClass::mandatory)
        .instanceFunction("optional", &CommandClass::optional)
        .instanceFunction("setSoftEnum", &CommandClass::setSoftEnum)
        .instanceFunction("addSoftEnumValues", &CommandClass::addSoftEnumValues)
        .instanceFunction("removeSoftEnumValues", &CommandClass::removeSoftEnumValues)
        .instanceFunction("getSoftEnumValues", &CommandClass::getSoftEnumValues)
        .instanceFunction("getSoftEnumNames", &CommandClass::getSoftEnumNames)
        .instanceFunction("overload", &CommandClass::addOverload)
        .instanceFunction("setCallback", &CommandClass::setCallback)
        .instanceFunction("setup", &CommandClass::setup)

        .build();

//////////////////// Helper ////////////////////

bool LLSERemoveCmdCallback(std::shared_ptr<script::ScriptEngine> engine) {
    std::erase_if(localShareData->commandCallbacks, [&engine](auto& data) { return data.second.fromEngine == engine; });
    return true;
}

Local<Value> convertResult(ParamStorageType const& result, CommandOrigin const& origin) {
    if (!result.has_value()) return {};
    if (result.hold(ParamKind::Kind::Enum)) {
        return String::newString(std::get<RuntimeEnum>(result.value()).name);
    }
    if (result.hold(ParamKind::Kind::SoftEnum)) {
        return String::newString(std::get<RuntimeSoftEnum>(result.value()));
    }
    if (result.hold(ParamKind::Kind::BlockName)) {
        return BlockClass::newBlock(
            *std::get<CommandBlockName>(result.value()).resolveBlock(0).mBlock,
            BlockPos::MIN(),
            -1
        );
    }
    if (result.hold(ParamKind::Kind::Item)) {
        auto item = std::make_unique<ItemStack>();
        item->reinit(
            ll::service::getLevel()->getItemRegistry().getNameFromLegacyID(std::get<CommandItem>(result.value()).mId),
            1,
            0
        );
        return ItemClass::newItem(std::move(item));
    }
    if (result.hold(ParamKind::Kind::Actor)) {
        auto arr = Array::newArray();
        for (auto i : std::get<CommandSelector<Actor>>(result.value()).results(origin)) {
            arr.add(EntityClass::newEntity(i));
        }
        return arr;
    }
    if (result.hold(ParamKind::Kind::Player)) {
        auto arr = Array::newArray();
        for (auto i : std::get<CommandSelector<Player>>(result.value()).results(origin)) {
            arr.add(PlayerClass::newPlayer(i));
        }
        return arr;
    }
    if (result.hold(ParamKind::Kind::BlockPos)) {
        auto dim = origin.getDimension();
        return IntPos::newPos(
            std::get<CommandPosition>(result.value())
                .getBlockPos(static_cast<int>(CurrentCmdVersion::Latest), origin, Vec3::ZERO()),
            dim ? dim->getDimensionId().id : -1
        );
    }
    if (result.hold(ParamKind::Kind::Vec3)) {
        auto dim = origin.getDimension();
        return FloatPos::newPos(
            std::get<CommandPositionFloat>(result.value())
                .getPosition(static_cast<int>(CurrentCmdVersion::Latest), origin, Vec3::ZERO()),
            dim ? dim->getDimensionId().id : -1
        );
    }
    if (result.hold(ParamKind::Kind::Message)) {
        return String::newString(
            std::get<CommandMessage>(result.value())
                .generateMessage(origin, static_cast<int>(CurrentCmdVersion::Latest))
                .mMessage->c_str()
        );
    }
    if (result.hold(ParamKind::Kind::RawText)) {
        return String::newString(std::get<CommandRawText>(result.value()).mText);
    }
    if (result.hold(ParamKind::Kind::JsonValue)) {
        return String::newString(Json::FastWriter().write(std::get<Json::Value>(result.value())));
    }
    if (result.hold(ParamKind::Kind::Effect)) {
        return String::newString(std::get<MobEffect const*>(result.value())->mResourceName);
    }
    if (result.hold(ParamKind::Kind::Command)) {
        return String::newString(std::get<std::unique_ptr<::Command>>(result.value())->getCommandName());
    }
    if (result.hold(ParamKind::Kind::ActorType)) {
        return String::newString(
            std::get<ActorDefinitionIdentifier const*>(result.value())->mCanonicalName->getString()
        );
    }
    if (result.hold(ParamKind::Kind::Bool)) {
        return Boolean::newBoolean(std::get<bool>(result.value()));
    }
    if (result.hold(ParamKind::Kind::Int)) {
        return Number::newNumber(std::get<int>(result.value()));
    }
    if (result.hold(ParamKind::Kind::Float)) {
        return Number::newNumber(std::get<float>(result.value()));
    }
    if (result.hold(ParamKind::Kind::String)) {
        return String::newString(std::get<std::string>(result.value()));
    }
    return {};
}

template <typename T>
std::enable_if_t<std::is_enum_v<T>, T> parseEnum(Local<Value> const& value) {
    if (value.isString()) {
        auto tmp = magic_enum::enum_cast<T>(value.asString().toString());
        if (!tmp.has_value()) throw std::runtime_error("Unable to parse Enum value");
        return tmp.value();
    }
    if (value.isNumber()) {
        return static_cast<T>(value.asNumber().toInt32());
    }
    throw std::runtime_error("Unable to parse Enum value");
}

//////////////////// MC APIs ////////////////////

Local<Value> McClass::runcmd(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    CommandContext context = CommandContext(
        args[0].asString().toString(),
        std::make_unique<ServerCommandOrigin>(
            "Server",
            ll::service::getLevel()->asServer(),
            CommandPermissionLevel::Owner,
            0
        ),
        static_cast<int>(CurrentCmdVersion::Latest)
    );
    try {
        return Boolean::newBoolean(ll::service::getMinecraft()->mCommands->executeCommand(context, false).mSuccess);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::runcmdEx(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    try {
        std::string outputStr;
        auto        origin =
            ServerCommandOrigin("Server", ll::service::getLevel()->asServer(), CommandPermissionLevel::Owner, 0);
        auto command = ll::service::getMinecraft()->mCommands->compileCommand(
            args[0].asString().toString(),
            origin,
            static_cast<CurrentCmdVersion>(static_cast<int>(CurrentCmdVersion::Latest)),
            [&](std::string const& err) { outputStr.append(err).append("\n"); }
        );
        Local<Object> resObj = Object::newObject();
        if (command) {
            CommandOutput output(CommandOutputType::AllOutput);
            command->run(origin, output);
            static std::shared_ptr<Localization const> localization =
                getI18n().getLocaleFor(getI18n().getCurrentLanguage()->mCode);
            for (auto& msg : output.mMessages) {
                outputStr += getI18n().get(msg.mMessageId, msg.mParams, localization).append("\n");
            }
            if (outputStr.ends_with('\n')) {
                outputStr.pop_back();
            }
            resObj.set("success", output.mSuccessCount ? true : false);
            resObj.set("output", outputStr);
            return resObj;
        }
        if (outputStr.ends_with('\n')) {
            outputStr.pop_back();
        }
        resObj.set("success", false);
        resObj.set("output", outputStr);
        return resObj;
    }
    CATCH_AND_THROW
}

// name, description, permission, flag, alias
Local<Value> McClass::newCommand(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        auto name = args[0].asString().toString();

        auto                   desc       = args[1].asString().toString();
        CommandPermissionLevel permission = CommandPermissionLevel::Admin;
        CommandFlag            flag       = {static_cast<CommandFlagValue>(0x80)};
        std::string            alias;
        if (args.size() > 2) {
            permission = static_cast<CommandPermissionLevel>(parseEnum<OldCommandPermissionLevel>(args[2]));
            if (args.size() > 3) {
                CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
                flag = {static_cast<CommandFlagValue>(args[3].asNumber().toInt32())};
                if (args.size() > 4) {
                    CHECK_ARG_TYPE(args[4], ValueKind::kString);
                    alias = args[4].asString().toString();
                }
            }
        }
        auto newCommandFunc = [](std::string const&            name,
                                 std::string const&            desc,
                                 CommandPermissionLevel const& permission,
                                 CommandFlag const&            flag,
                                 std::string const&            alias) {
            if (auto registry = ll::service::getCommandRegistry()) {
                if (registry->findCommand(name)) {
                    lse::LegacyScriptEngine::getLogger().warn(
                        "Runtime command {} already exists, changes will not beapplied except for setOverload!"_tr(name)
                    );
                }
            }
            auto& command = CommandRegistrar::getInstance(false).getOrCreateCommand(name, desc, permission, flag);
            if (!alias.empty()) {
                command.alias(alias);
            }
        };
        if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
            ll::coro::keepThis([name, desc, permission, flag, alias, newCommandFunc]() -> ll::coro::CoroTask<> {
                newCommandFunc(name, desc, permission, flag, alias);
                co_return;
            }).launch(ll::thread::ServerThreadExecutor::getDefault());
        } else {
            newCommandFunc(name, desc, permission, flag, alias);
        }
        return CommandClass::newCommand(name);
    }
    CATCH_AND_THROW
}

//////////////////// Command APIs ////////////////////

CommandClass::CommandClass(std::string const& name)
: ScriptClass(ScriptClass::ConstructFromCpp<CommandClass>{}),
  commandName(name) {};

Local<Object> CommandClass::newCommand(std::string const& name) {
    auto newp = new CommandClass(name);
    return newp->getScriptObject();
}

Local<Value> CommandClass::getName() const {
    try {
        return String::newString(commandName);
    }
    CATCH_AND_THROW
}

Local<Value> CommandClass::setAlias(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    try {
        std::string alias = args[0].asString().toString();
        if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
            ll::coro::keepThis([commandName(commandName), alias]() -> ll::coro::CoroTask<> {
                CommandRegistrar::getInstance(false).getOrCreateCommand(commandName).alias(alias);
                co_return;
            }).launch(ll::thread::ServerThreadExecutor::getDefault());
            return Boolean::newBoolean(true);
        }
        get().alias(alias);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

// string, vector<string>
Local<Value> CommandClass::setEnum(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    CHECK_ARG_TYPE(args[1], ValueKind::kArray)
    try {
        auto enumName = args[0].asString().toString();
        auto enumArr  = args[1].asArray();
        if (enumArr.size() == 0 || !enumArr.get(0).isString()) return {};
        std::vector<std::pair<std::string, uint64>> enumValues;
        for (int i = 0; i < enumArr.size(); ++i) {
            enumValues.push_back({enumArr.get(i).asString().toString(), i});
        }
        if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
            ll::coro::keepThis([enumName, enumValues]() -> ll::coro::CoroTask<> {
                CommandRegistrar::getInstance(false).tryRegisterRuntimeEnum(enumName, std::move(enumValues));
                co_return;
            }).launch(ll::thread::ServerThreadExecutor::getDefault());
            return String::newString(enumName);
        }
        if (CommandRegistrar::getInstance(false).tryRegisterRuntimeEnum(enumName, std::move(enumValues))) {
            return String::newString(enumName);
        }
        return {};
    }
    CATCH_AND_THROW
}

void onExecute(CommandOrigin const& origin, CommandOutput& output, RuntimeCommand const& runtime) {
    std::string commandName = runtime.getCommandName();
    if (!localShareData->commandCallbacks.contains(commandName)) {
        lse::LegacyScriptEngine::getLogger().warn(
            "Command {} failed to execute, is the plugin unloaded?"_tr(commandName)
        );
        return;
    }
    EngineScope enter(localShareData->commandCallbacks[commandName].fromEngine.get());
    try {
        Local<Object> args = Object::newObject();
        auto          cmd  = CommandClass::newCommand(commandName);
        auto*         ori  = new CommandOriginClass(origin.clone());
        auto*         outp = new CommandOutputClass(std::make_shared<CommandOutput>(output), ori->get());

        auto& registeredCommands = getEngineOwnData()->plugin->registeredCommands;
        if (!registeredCommands.contains(commandName)) {
            lse::LegacyScriptEngine::getLogger().warn("Could not find {} in registered commands."_tr(commandName));
            return;
        }
        for (auto& info : registeredCommands[commandName]) {
            try {
                if (!info.name.empty()) {
                    if (info.type == ParamKind::Kind::Enum || info.type == ParamKind::Kind::SoftEnum) {
                        auto& param = runtime[info.enumName];
                        args.set(info.name, convertResult(param, origin));
                        if (!info.identifier.empty()
                            && info.identifier != info.name) { // Keep compatibility with old plugins
                            args.set(info.identifier, convertResult(param, origin));
                        }
                    } else {
                        auto& param = runtime[info.name];
                        args.set(info.name, convertResult(param, origin));
                        if (!info.identifier.empty()
                            && info.identifier != info.name) { // Keep compatibility with old plugins
                            args.set(info.identifier, convertResult(param, origin));
                        }
                    }
                }
            } catch (std::out_of_range&) {}
        }
        localShareData->commandCallbacks[commandName].func.get().call({}, cmd, ori, outp, args);
        std::swap(output.mMessages, outp->output->mMessages);
        output.mSuccessCount = outp->output->mSuccessCount;
        outp->isAsync        = true;
    }
    CATCH_WITH_MESSAGE("Fail in executing command \"{}\"!", commandName)
}

// name, type, optional, description, identifier, option
// name, type, description, identifier, option
// name, type, optional, description, option
// name, type, description, option
Local<Value> CommandClass::newParameter(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto                   name       = args[0].asString().toString();
        ParamKind::Kind        type       = static_cast<ParamKind::Kind>(parseEnum<OldParameterType>(args[1]));
        std::string            enumName   = "";
        bool                   optional   = false;
        std::string            identifier = "";
        size_t                 index      = 2;
        CommandParameterOption option     = CommandParameterOption::None;
        if (args.size() > index && args[index].isBoolean()) optional = args[index++].asBoolean().value();
        if (args.size() > index && args[index].isString()) enumName = args[index++].asString().toString();
        if (args.size() > index && args[index].isString()) identifier = args[index++].asString().toString();
        if (args.size() > index && args[index].isNumber())
            option = static_cast<CommandParameterOption>(args[index++].asNumber().toInt32());
        if (index != args.size()) throw std::runtime_error("Error Argument in newParameter");

        getEngineOwnData()->plugin->registeredCommands[commandName].push_back(
            {name, type, optional, enumName, option, identifier}
        ); // Stores the parameter name for onExecute use

        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

// name, type, description, identifier, option
// name, type, description, option
Local<Value> CommandClass::mandatory(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto                   name       = args[0].asString().toString();
        ParamKind::Kind        type       = static_cast<ParamKind::Kind>(parseEnum<OldParameterType>(args[1]));
        std::string            enumName   = "";
        std::string            identifier = "";
        size_t                 index      = 2;
        CommandParameterOption option     = CommandParameterOption::None;
        if (args.size() > index && args[index].isString()) enumName = args[index++].asString().toString();
        if (args.size() > index && args[index].isString()) identifier = args[index++].asString().toString();
        if (args.size() > index && args[index].isNumber())
            option = static_cast<CommandParameterOption>(args[index++].asNumber().toInt32());
        if (index != args.size()) throw std::runtime_error("Error Argument in newParameter");

        getEngineOwnData()->plugin->registeredCommands[commandName].push_back(
            {name, type, false, enumName, option, identifier}
        ); // Stores the parameter name for onExecute use

        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

// name, type, description, identifier, option
// name, type, description, option
auto CommandClass::optional(Arguments const& args) const -> Local<Value> {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto                   name       = args[0].asString().toString();
        ParamKind::Kind        type       = static_cast<ParamKind::Kind>(parseEnum<OldParameterType>(args[1]));
        std::string            enumName   = "";
        std::string            identifier = "";
        size_t                 index      = 2;
        CommandParameterOption option     = CommandParameterOption::None;
        if (args.size() > index && args[index].isString()) enumName = args[index++].asString().toString();
        if (args.size() > index && args[index].isString()) identifier = args[index++].asString().toString();
        if (args.size() > index && args[index].isNumber())
            option = static_cast<CommandParameterOption>(args[index++].asNumber().toInt32());
        if (index != args.size()) throw std::runtime_error("Error Argument in newParameter");

        getEngineOwnData()->plugin->registeredCommands[commandName].push_back(
            {name, type, true, enumName, option, identifier}
        ); // Stores the parameter name for onExecute use

        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

// vector<identifier>
// vector<index>
Local<Value> CommandClass::addOverload(Arguments const& args) {
    try {
        auto overloadFunc = [e(
                                EngineManager::checkAndGet(EngineScope::currentEngine(), true)
                            )](RuntimeOverload& cmd, std::string const& commandName, std::string const& paramName) {
            auto& paramList = getEngineData(e)->plugin->registeredCommands[commandName];
            for (auto& info : paramList) {
                if (info.name == paramName || info.enumName == paramName || info.identifier == paramName) {
                    if (info.optional) {
                        if (info.type == ParamKind::Kind::Enum || info.type == ParamKind::Kind::SoftEnum) {
                            cmd.optional(info.enumName, info.type, info.enumName).option(info.option);
                        } else {
                            cmd.optional(info.name, info.type).option(info.option);
                        }
                    } else {
                        if (info.type == ParamKind::Kind::Enum || info.type == ParamKind::Kind::SoftEnum) {
                            cmd.required(info.enumName, info.type, info.enumName).option(info.option);
                        } else {
                            cmd.required(info.name, info.type).option(info.option);
                        }
                    }
                }
            }
        };
        auto delayRegFunc = [this, &overloadFunc](std::vector<std::string>& paramNames) {
            ll::coro::keepThis(
                [paramNames,
                 commandName(commandName),
                 overloadFunc,
                 e(EngineManager::checkAndGet(EngineScope::currentEngine(), true))]() -> ll::coro::CoroTask<> {
                    auto cmd = CommandRegistrar::getInstance(false)
                                   .getOrCreateCommand(commandName)
                                   .runtimeOverload(getEngineData(e)->plugin);
                    for (auto& paramName : paramNames) {
                        overloadFunc(cmd, commandName, paramName);
                    }
                    cmd.execute(onExecute);
                    co_return;
                }
            ).launch(ll::thread::ServerThreadExecutor::getDefault());
        };
        if (args.size() == 0) {
            if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
                ll::coro::keepThis(
                    [commandName(commandName),
                     e(EngineManager::checkAndGet(EngineScope::currentEngine(), true))]() -> ll::coro::CoroTask<> {
                        getEngineData(e)->plugin->registeredCommands[commandName].push_back({});
                        auto cmd = CommandRegistrar::getInstance(false)
                                       .getOrCreateCommand(commandName)
                                       .runtimeOverload(getEngineData(e)->plugin);
                        cmd.execute(onExecute);
                        co_return;
                    }
                ).launch(ll::thread::ServerThreadExecutor::getDefault());

            } else {
                getEngineOwnData()->plugin->registeredCommands[commandName].push_back({});
                auto cmd = get().runtimeOverload(getEngineOwnData()->plugin);
                cmd.execute(onExecute);
            }
            return Boolean::newBoolean(true);
        }
        if (args[0].isNumber()) {
            if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
                std::vector<std::string> paramNames;
                for (int i = 0; i < args.size(); ++i) {
                    CHECK_ARG_TYPE(args[i], ValueKind::kNumber);
                    paramNames.push_back(std::to_string(args[i].asNumber().toInt32()));
                }
                delayRegFunc(paramNames);
            } else {
                auto cmd = get().runtimeOverload(getEngineOwnData()->plugin);
                for (int i = 0; i < args.size(); ++i) {
                    CHECK_ARG_TYPE(args[i], ValueKind::kNumber);
                    std::string paramName = std::to_string(args[i].asNumber().toInt32());
                    overloadFunc(cmd, commandName, paramName);
                }
                cmd.execute(onExecute);
            }
            return Boolean::newBoolean(true);
        }
        if (args[0].isString()) {
            if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
                std::vector<std::string> paramNames;
                for (int i = 0; i < args.size(); ++i) {
                    CHECK_ARG_TYPE(args[i], ValueKind::kString);
                    paramNames.push_back(args[i].asString().toString());
                }
                delayRegFunc(paramNames);
            } else {
                auto cmd = get().runtimeOverload(getEngineOwnData()->plugin);
                for (int i = 0; i < args.size(); ++i) {
                    CHECK_ARG_TYPE(args[i], ValueKind::kString);
                    std::string paramName = args[i].asString().toString();
                    overloadFunc(cmd, commandName, paramName);
                }
                cmd.execute(onExecute);
            }
            return Boolean::newBoolean(true);
        }
        if (args[0].isArray()) {
            auto arr = args[0].asArray();
            if (arr.size() == 0) {
                if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
                    ll::coro::keepThis(
                        [commandName(commandName),
                         e(EngineManager::checkAndGet(EngineScope::currentEngine(), true))]() -> ll::coro::CoroTask<> {
                            getEngineData(e)->plugin->registeredCommands[commandName].push_back({});
                            auto cmd = CommandRegistrar::getInstance(false)
                                           .getOrCreateCommand(commandName)
                                           .runtimeOverload(getEngineData(e)->plugin);
                            cmd.execute(onExecute);
                            co_return;
                        }
                    ).launch(ll::thread::ServerThreadExecutor::getDefault());
                } else {
                    getEngineOwnData()->plugin->registeredCommands[commandName].push_back({});
                    auto cmd = get().runtimeOverload(getEngineOwnData()->plugin);
                    cmd.execute(onExecute);
                }
                return Boolean::newBoolean(true);
            }
            if (arr.get(0).isNumber()) {
                if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
                    std::vector<std::string> paramNames;
                    for (int i = 0; i < arr.size(); ++i) {
                        CHECK_ARG_TYPE(arr.get(i), ValueKind::kNumber);
                        paramNames.push_back(std::to_string(arr.get(i).asNumber().toInt32()));
                    }
                    delayRegFunc(paramNames);
                } else {
                    auto cmd = get().runtimeOverload(getEngineOwnData()->plugin);
                    for (int i = 0; i < arr.size(); ++i) {
                        CHECK_ARG_TYPE(arr.get(i), ValueKind::kNumber);
                        std::string paramName = std::to_string(arr.get(i).asNumber().toInt32());
                        overloadFunc(cmd, commandName, paramName);
                    }
                    cmd.execute(onExecute);
                }
                return Boolean::newBoolean(true);
            }
            if (arr.get(0).isString()) {
                if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
                    std::vector<std::string> paramNames;
                    for (int i = 0; i < arr.size(); ++i) {
                        CHECK_ARG_TYPE(arr.get(i), ValueKind::kString);
                        paramNames.push_back(arr.get(i).asString().toString());
                    }
                    delayRegFunc(paramNames);
                } else {
                    auto cmd = get().runtimeOverload(getEngineOwnData()->plugin);
                    for (int i = 0; i < arr.size(); ++i) {
                        CHECK_ARG_TYPE(arr.get(i), ValueKind::kString);
                        std::string paramName = arr.get(i).asString().toString();
                        overloadFunc(cmd, commandName, paramName);
                    }
                    cmd.execute(onExecute);
                }
                return Boolean::newBoolean(true);
            }
        }
        throw WrongArgTypeException(__FUNCTION__);
    }
    CATCH_AND_THROW
}

// function (command, origin, output, results){}
Local<Value> CommandClass::setCallback(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kFunction);
    try {
        auto func                                     = args[0].asFunction();
        localShareData->commandCallbacks[commandName] = {
            EngineManager::checkAndGet(EngineScope::currentEngine()),
            CommandPermissionLevel::Any,
            script::Global<Function>(func)
        };
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

// setup(Function<Command, Origin, Output, Map<String, Any>>)
Local<Value> CommandClass::setup(Arguments const& args) const {
    try {
        if (args.size() > 0) {
            return setCallback(args);
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> CommandClass::isRegistered() { return Boolean::newBoolean(true); }

Local<Value> CommandClass::toString(Arguments const&) {
    try {
        return String::newString(fmt::format("<Command({})>", commandName));
    }
    CATCH_AND_THROW
}

Local<Value> CommandClass::setSoftEnum(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kArray);
    try {
        auto name  = args[0].asString().toString();
        auto enums = parseStringList(args[1].asArray());
        if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
            ll::coro::keepThis([name, enums]() -> ll::coro::CoroTask<> {
                CommandRegistrar::getInstance(false).tryRegisterSoftEnum(name, std::move(enums));
                co_return;
            }).launch(ll::thread::ServerThreadExecutor::getDefault());
        } else {
            CommandRegistrar::getInstance(false).tryRegisterSoftEnum(name, std::move(enums));
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> CommandClass::addSoftEnumValues(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kArray);
    try {
        auto name  = args[0].asString().toString();
        auto enums = parseStringList(args[1].asArray());
        if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
            ll::coro::keepThis([name, enums]() -> ll::coro::CoroTask<> {
                CommandRegistrar::getInstance(false).addSoftEnumValues(name, std::move(enums));
                co_return;
            }).launch(ll::thread::ServerThreadExecutor::getDefault());
        } else {
            CommandRegistrar::getInstance(false).addSoftEnumValues(name, std::move(enums));
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> CommandClass::removeSoftEnumValues(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kArray);
    try {
        auto name  = args[0].asString().toString();
        auto enums = parseStringList(args[1].asArray());
        if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
            ll::coro::keepThis([name, enums]() -> ll::coro::CoroTask<> {
                CommandRegistrar::getInstance(false).removeSoftEnumValues(name, std::move(enums));
                co_return;
            }).launch(ll::thread::ServerThreadExecutor::getDefault());
        } else {
            CommandRegistrar::getInstance(false).removeSoftEnumValues(name, std::move(enums));
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> CommandClass::getSoftEnumValues(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto name = args[0].asString().toString();
        if (auto registry = ll::service::getCommandRegistry()) {
            auto& lookup    = registry->mSoftEnumLookup;
            auto& softEnums = registry->mSoftEnums;
            if (lookup.contains(name)) {
                return getStringArray(softEnums[lookup[name]].mValues);
            }
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> CommandClass::getSoftEnumNames(Arguments const&) {
    try {
        auto registry = ll::service::getCommandRegistry();
        if (!registry) return {};
        auto&                    lookup = registry->mSoftEnums;
        std::vector<std::string> names;
        for (auto& [name, _] : lookup) {
            names.push_back(name);
        }
        return getStringArray(names);
    }
    CATCH_AND_THROW
}
