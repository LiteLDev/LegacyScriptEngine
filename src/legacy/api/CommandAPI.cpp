#include "api/CommandAPI.h"

#include "api/BaseAPI.h"
#include "api/BlockAPI.h"
#include "api/CommandOriginAPI.h"
#include "api/CommandOutputAPI.h"
#include "api/EntityAPI.h"
#include "api/ItemAPI.h"
#include "api/McAPI.h"
#include "api/PlayerAPI.h"
#include "engine/EngineOwnData.h"
#include "engine/GlobalShareData.h"
#include "engine/LocalShareData.h"
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
    std::erase_if(localShareData->commandCallbacks, [&engine](auto& data) {
        return data.second.fromEngine == engine.get();
    });
    return true;
}

Local<Value> convertResult(ParamStorageType const& result, CommandOrigin const& origin, CommandOutput& output) {
    if (!result.has_value()) return {};
    if (result.hold(ParamKind::Kind::Enum)) {
        return String::newString(std::get<RuntimeEnum>(result.value()).name);
    } else if (result.hold(ParamKind::Kind::SoftEnum)) {
        return String::newString(std::get<RuntimeSoftEnum>(result.value()));
    } else if (result.hold(ParamKind::Kind::BlockName)) {
        return BlockClass::newBlock(
            *std::get<CommandBlockName>(result.value()).resolveBlock(0).mBlock,
            BlockPos::MIN(),
            -1
        );
    } else if (result.hold(ParamKind::Kind::Item)) {
        return ItemClass::newItem(
            std::make_unique<ItemStack>(ll::service::getLevel()->getItemRegistry().getNameFromLegacyID(
                std::get<CommandItem>(result.value()).mId
            ))
        );
    } else if (result.hold(ParamKind::Kind::Actor)) {
        auto arr = Array::newArray();
        for (auto i : std::get<CommandSelector<Actor>>(result.value()).results(origin)) {
            arr.add(EntityClass::newEntity(i));
        }
        return arr;
    } else if (result.hold(ParamKind::Kind::Player)) {
        auto arr = Array::newArray();
        for (auto i : std::get<CommandSelector<Player>>(result.value()).results(origin)) {
            arr.add(PlayerClass::newPlayer(i));
        }
        return arr;
    } else if (result.hold(ParamKind::Kind::BlockPos)) {
        auto dim = origin.getDimension();
        return IntPos::newPos(
            std::get<CommandPosition>(result.value())
                .getBlockPos(CommandVersion::CurrentVersion(), origin, Vec3::ZERO()),
            dim ? dim->getDimensionId().id : -1
        );
    } else if (result.hold(ParamKind::Kind::Vec3)) {
        auto dim = origin.getDimension();
        return FloatPos::newPos(
            std::get<CommandPositionFloat>(result.value())
                .getPosition(CommandVersion::CurrentVersion(), origin, Vec3::ZERO()),
            dim ? dim->getDimensionId().id : -1
        );
    } else if (result.hold(ParamKind::Kind::Message)) {
        return String::newString(
            std::get<CommandMessage>(result.value())
                .generateMessage(origin, CommandVersion::CurrentVersion())
                .mMessage->c_str()
        );
    } else if (result.hold(ParamKind::Kind::RawText)) {
        return String::newString(std::get<CommandRawText>(result.value()).mText);
    } else if (result.hold(ParamKind::Kind::JsonValue)) {
        return String::newString(Json::FastWriter().write(std::get<Json::Value>(result.value())));
    } else if (result.hold(ParamKind::Kind::Effect)) {
        return String::newString(std::get<MobEffect const*>(result.value())->mResourceName);
    } else if (result.hold(ParamKind::Kind::Command)) {
        return String::newString(std::get<std::unique_ptr<::Command>>(result.value())->getCommandName());
    } else if (result.hold(ParamKind::Kind::ActorType)) {
        return String::newString(
            std::get<ActorDefinitionIdentifier const*>(result.value())->mCanonicalName->getString()
        );
    } else if (result.hold(ParamKind::Kind::Bool)) {
        return Boolean::newBoolean(std::get<bool>(result.value()));
    } else if (result.hold(ParamKind::Kind::Int)) {
        return Number::newNumber(std::get<int>(result.value()));
    } else if (result.hold(ParamKind::Kind::Float)) {
        return Number::newNumber(std::get<float>(result.value()));
    } else if (result.hold(ParamKind::Kind::String)) {
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
    } else if (value.isNumber()) {
        return (T)value.asNumber().toInt32();
    }
    throw std::runtime_error("Unable to parse Enum value");
}

//////////////////// MC APIs ////////////////////

Local<Value> McClass::runcmd(const Arguments& args) {
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
        CommandVersion::CurrentVersion()
    );
    try {
        return Boolean::newBoolean(ll::service::getMinecraft()->mCommands->executeCommand(context, false).mSuccess);
    }
    CATCH("Fail in RunCmd!")
}

Local<Value> McClass::runcmdEx(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    try {
        std::string outputStr;
        auto        origin =
            ServerCommandOrigin("Server", ll::service::getLevel()->asServer(), CommandPermissionLevel::Owner, 0);
        auto command = ll::service::getMinecraft()->mCommands->compileCommand(
            args[0].asString().toString(),
            origin,
            (CurrentCmdVersion)CommandVersion::CurrentVersion(),
            [&](std::string const& err) { outputStr.append(err).append("\n"); }
        );
        Local<Object> resObj = Object::newObject();
        if (command) {
            CommandOutput output(CommandOutputType::AllOutput);
            command->run(origin, output);
            static std::shared_ptr<const Localization> localization =
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
    CATCH("Fail in RunCmdEx!")
}

// name, description, permission, flag, alias
Local<Value> McClass::newCommand(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        auto name = args[0].asString().toString();

        auto                   desc       = args[1].asString().toString();
        CommandPermissionLevel permission = CommandPermissionLevel::Admin;
        CommandFlag            flag       = {(CommandFlagValue)0x80};
        std::string            alias;
        if (args.size() > 2) {
            permission = (CommandPermissionLevel)parseEnum<OldCommandPermissionLevel>(args[2]);
            if (args.size() > 3) {
                CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
                flag = {(CommandFlagValue)args[3].asNumber().toInt32()};
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
            auto registry = ll::service::getCommandRegistry();
            if (registry) {
                auto instance = registry->findCommand(name);
                if (instance) {
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
    CATCH("Fail in newCommand!")
}

//////////////////// Command APIs ////////////////////

CommandClass::CommandClass(std::string& name)
: ScriptClass(ScriptClass::ConstructFromCpp<CommandClass>{}),
  commandName(name) {};

Local<Object> CommandClass::newCommand(std::string& name) {
    auto newp = new CommandClass(name);
    return newp->getScriptObject();
}

Local<Value> CommandClass::getName() {
    try {
        return String::newString(commandName);
    }
    CATCH("Fail in getCommandName!")
}

Local<Value> CommandClass::setAlias(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    try {
        std::string alias = args[0].asString().toString();
        if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
            ll::coro::keepThis([commandName(commandName), alias]() -> ll::coro::CoroTask<> {
                ll::command::CommandRegistrar::getInstance(false).getOrCreateCommand(commandName).alias(alias);
                co_return;
            }).launch(ll::thread::ServerThreadExecutor::getDefault());
            return Boolean::newBoolean(true);
        } else {
            get().alias(alias);
        }
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setAlias!")
}

// string, vector<string>
Local<Value> CommandClass::setEnum(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    CHECK_ARG_TYPE(args[1], ValueKind::kArray)
    try {
        auto enumName = args[0].asString().toString();
        auto enumArr  = args[1].asArray();
        if (enumArr.size() == 0 || !enumArr.get(0).isString()) return Local<Value>();
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
        } else {
            if (CommandRegistrar::getInstance(false).tryRegisterRuntimeEnum(enumName, std::move(enumValues))) {
                return String::newString(enumName);
            }
        }
        return {};
    }
    CATCH("Fail in setEnum!")
}

void onExecute(CommandOrigin const& origin, CommandOutput& output, RuntimeCommand const& runtime) {
    std::string commandName = runtime.getCommandName();
    if (localShareData->commandCallbacks.find(commandName) == localShareData->commandCallbacks.end()) {
        lse::LegacyScriptEngine::getLogger().warn(
            "Command {} failed to execute, is the plugin unloaded?"_tr(commandName)
        );
        return;
    }
    EngineScope enter(localShareData->commandCallbacks[commandName].fromEngine);
    try {
        Local<Object> args = Object::newObject();
        auto          cmd  = CommandClass::newCommand(commandName);
        auto*         ori  = new CommandOriginClass(origin.clone());
        auto*         outp = new CommandOutputClass(std::make_shared<CommandOutput>(output), ori->get());

        auto& registeredCommands = getEngineOwnData()->plugin->registeredCommands;
        if (registeredCommands.find(commandName) == registeredCommands.end()) {
            lse::LegacyScriptEngine::getLogger().warn(
                "Could not find {} in registered commands."_tr(commandName)
            );
            return;
        }
        for (auto& info : registeredCommands[commandName]) {
            try {
                if (!info.name.empty()) {
                    if (info.type == ParamKind::Kind::Enum || info.type == ParamKind::Kind::SoftEnum) {
                        auto& param = runtime[info.enumName];
                        args.set(info.name, convertResult(param, origin, output));
                        if (!info.identifier.empty()
                            && info.identifier != info.name) { // Keep compatibility with old plugins
                            args.set(info.identifier, convertResult(param, origin, output));
                        }
                    } else {
                        auto& param = runtime[info.name];
                        args.set(info.name, convertResult(param, origin, output));
                        if (!info.identifier.empty()
                            && info.identifier != info.name) { // Keep compatibility with old plugins
                            args.set(info.identifier, convertResult(param, origin, output));
                        }
                    }
                }
            } catch (std::out_of_range&) {
                continue;
            }
        }
        localShareData->commandCallbacks[commandName].func.get().call({}, cmd, ori, outp, args);
        std::swap(output.mMessages, outp->output->mMessages);
        output.mSuccessCount = outp->output->mSuccessCount;
        outp->isAsync        = true;
    }
    CATCH_WITHOUT_RETURN("Fail in executing command \"" + commandName + "\"!")
}

// name, type, optional, description, identifier, option
// name, type, description, identifier, option
// name, type, optional, description, option
// name, type, description, option
Local<Value> CommandClass::newParameter(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto                   name       = args[0].asString().toString();
        ParamKind::Kind        type       = (ParamKind::Kind)parseEnum<OldParameterType>(args[1]);
        std::string            enumName   = "";
        bool                   optional   = false;
        std::string            identifier = "";
        size_t                 index      = 2;
        CommandParameterOption option     = CommandParameterOption::None;
        if (args.size() > index && args[index].isBoolean()) optional = args[index++].asBoolean().value();
        if (args.size() > index && args[index].isString()) enumName = args[index++].asString().toString();
        if (args.size() > index && args[index].isString()) identifier = args[index++].asString().toString();
        if (args.size() > index && args[index].isNumber())
            option = (CommandParameterOption)args[index++].asNumber().toInt32();
        if (index != args.size()) throw std::runtime_error("Error Argument in newParameter");

        getEngineOwnData()->plugin->registeredCommands[commandName].push_back(
            {name, type, optional, enumName, option, identifier}
        ); // Stores the parameter name for onExecute use

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in newParameter!")
}

// name, type, description, identifier, option
// name, type, description, option
Local<Value> CommandClass::mandatory(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto                   name       = args[0].asString().toString();
        ParamKind::Kind        type       = (ParamKind::Kind)parseEnum<OldParameterType>(args[1]);
        std::string            enumName   = "";
        std::string            identifier = "";
        size_t                 index      = 2;
        CommandParameterOption option     = CommandParameterOption::None;
        if (args.size() > index && args[index].isString()) enumName = args[index++].asString().toString();
        if (args.size() > index && args[index].isString()) identifier = args[index++].asString().toString();
        if (args.size() > index && args[index].isNumber())
            option = (CommandParameterOption)args[index++].asNumber().toInt32();
        if (index != args.size()) throw std::runtime_error("Error Argument in newParameter");

        getEngineOwnData()->plugin->registeredCommands[commandName].push_back(
            {name, type, false, enumName, option, identifier}
        ); // Stores the parameter name for onExecute use

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in newParameter!")
}

// name, type, description, identifier, option
// name, type, description, option
Local<Value> CommandClass::optional(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto                   name       = args[0].asString().toString();
        ParamKind::Kind        type       = (ParamKind::Kind)parseEnum<OldParameterType>(args[1]);
        std::string            enumName   = "";
        std::string            identifier = "";
        size_t                 index      = 2;
        CommandParameterOption option     = CommandParameterOption::None;
        if (args.size() > index && args[index].isString()) enumName = args[index++].asString().toString();
        if (args.size() > index && args[index].isString()) identifier = args[index++].asString().toString();
        if (args.size() > index && args[index].isNumber())
            option = (CommandParameterOption)args[index++].asNumber().toInt32();
        if (index != args.size()) throw std::runtime_error("Error Argument in newParameter");

        getEngineOwnData()->plugin->registeredCommands[commandName].push_back(
            {name, type, true, enumName, option, identifier}
        ); // Stores the parameter name for onExecute use

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in newParameter!")
}

// vector<identifier>
// vector<index>
Local<Value> CommandClass::addOverload(const Arguments& args) {
    try {
        auto overloadFunc = [e(
                                EngineScope::currentEngine()
                            )](RuntimeOverload& cmd, std::string const& commandName, std::string const& paramName) {
            auto& paramList = getEngineData(e)->plugin->registeredCommands[commandName];
            for (auto& info : paramList) {
                if (info.name == paramName || info.enumName == paramName || info.identifier == paramName) {
                    if (info.optional) {
                        if (info.type == ParamKind::Kind::Enum || info.type == ParamKind::Kind::SoftEnum) {
                            (void)cmd.optional(info.enumName, info.type, info.enumName).option(info.option);
                        } else {
                            (void)cmd.optional(info.name, info.type).option(info.option);
                        }
                    } else {
                        if (info.type == ParamKind::Kind::Enum || info.type == ParamKind::Kind::SoftEnum) {
                            (void)cmd.required(info.enumName, info.type, info.enumName).option(info.option);
                        } else {
                            (void)cmd.required(info.name, info.type).option(info.option);
                        }
                    }
                }
            }
        };
        auto delayRegFunc = [this, &overloadFunc](std::vector<std::string>& paramNames) {
            ll::coro::keepThis(
                [paramNames, commandName(commandName), overloadFunc, e(EngineScope::currentEngine())]()
                    -> ll::coro::CoroTask<> {
                    auto cmd = ll::command::CommandRegistrar::getInstance(false)
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
                    [commandName(commandName), e(EngineScope::currentEngine())]() -> ll::coro::CoroTask<> {
                        getEngineData(e)->plugin->registeredCommands[commandName].push_back({});
                        auto cmd = ll::command::CommandRegistrar::getInstance(false)
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
        } else if (args[0].isString()) {
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
        } else if (args[0].isArray()) {
            auto arr = args[0].asArray();
            if (arr.size() == 0) {
                if (ll::getGamingStatus() == ll::GamingStatus::Starting) {
                    ll::coro::keepThis(
                        [commandName(commandName), e(EngineScope::currentEngine())]() -> ll::coro::CoroTask<> {
                            getEngineData(e)->plugin->registeredCommands[commandName].push_back({});
                            auto cmd = ll::command::CommandRegistrar::getInstance(false)
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
            } else if (arr.get(0).isString()) {
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
        LOG_WRONG_ARG_TYPE(__FUNCTION__);
        return Local<Value>();
    }
    CATCH("Fail in addOverload!")
}

// function (command, origin, output, results){}
Local<Value> CommandClass::setCallback(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kFunction);
    try {
        auto func = args[0].asFunction();
        localShareData
            ->commandCallbacks[commandName] = {EngineScope::currentEngine(), 0, script::Global<Function>(func)};
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setCallback!")
}

// setup(Function<Command, Origin, Output, Map<String, Any>>)
Local<Value> CommandClass::setup(const Arguments& args) {
    try {
        if (args.size() > 0) {
            setCallback(args);
        }
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setup!")
}

Local<Value> CommandClass::isRegistered() { return Boolean::newBoolean(true); }

Local<Value> CommandClass::toString(const Arguments&) {
    try {
        return String::newString(fmt::format("<Command({})>", commandName));
    }
    CATCH("Fail in toString!");
}

Local<Value> CommandClass::setSoftEnum(const Arguments& args) {
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
    CATCH("Fail in setSoftEnum!");
}

Local<Value> CommandClass::addSoftEnumValues(const Arguments& args) {
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
    CATCH("Fail in addSoftEnumValues!");
}

Local<Value> CommandClass::removeSoftEnumValues(const Arguments& args) {
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
    CATCH("Fail in removeSoftEnumValues!");
}

Local<Value> CommandClass::getSoftEnumValues(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto name     = args[0].asString().toString();
        auto registry = ll::service::getCommandRegistry();
        if (registry) {
            auto& lookup    = registry->mSoftEnumLookup;
            auto& softEnums = registry->mSoftEnums;
            if (lookup.find(name) != lookup.end()) {
                return getStringArray(softEnums[lookup[name]].mValues);
            }
        }
        return {};
    }
    CATCH("Fail in getSoftEnumValues");
}

Local<Value> CommandClass::getSoftEnumNames(const Arguments&) {
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
    CATCH("Fail in getSoftEnumNames");
}
