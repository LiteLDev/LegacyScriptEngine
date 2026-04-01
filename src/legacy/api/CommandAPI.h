#pragma once

#include "legacy/utils/UsingScriptX.h"
#include "ll/api/command/CommandHandle.h"

extern ClassDefine<void> ParamTypeStaticBuilder;
extern ClassDefine<void> PermissionStaticBuilder;
extern ClassDefine<void> ParamOptionStaticBuilder;

bool LLSERemoveCmdCallback(std::shared_ptr<script::ScriptEngine> engine);

enum class OldCommandPermissionLevel : schar {
    Any         = 0x0,
    GameMasters = 0x1,
    Admin       = 0x2,
    HostPlayer  = 0x3,
    Console     = 0x4,
    Internal    = 0x5,
};

enum class OldParameterType : size_t {
    Int,
    Bool,
    Float,
    Dimension,
    String,
    Enum,
    SoftEnum,
    Actor,
    Player,
    BlockPos,
    Vec3,
    RawText,
    Message,
    JsonValue,
    Item,
    Block,
    BlockState,
    Effect,
    ActorType,
    Command,
    RelativeFloat,
    IntegerRange,
    FilePath,
    WildcardInt,
    WildcardActor,
    // New types can
    Count,
};

class CommandClass : public ScriptClass {
    std::string                 commandName;
    std::string                 description;
    ll::command::CommandHandle& get() const {
        return ll::command::CommandRegistrar::getInstance(false).getOrCreateCommand(commandName);
    }
    static std::vector<std::string> parseStringList(Local<Array> const& arr) {
        if (arr.size() == 0 || !arr.get(0).isString()) return {};
        std::vector<std::string> strs;
        for (size_t i = 0; i < arr.size(); ++i) {
            strs.push_back(arr.get(i).asString().toString());
        }
        return std::move(strs);
    }
    static Local<Value> getStringArray(std::vector<std::string> const& values) {
        Local<Array> arr = Array::newArray(values.size());
        for (auto& str : values) {
            arr.add(String::newString(str));
        }
        return arr;
    }

public:
    CommandClass(std::string const& name);
    static Local<Object> newCommand(std::string const& name);
    Local<Value>         getName() const;
    Local<Value>         setAlias(Arguments const& args);
    Local<Value>         setEnum(Arguments const& args);
    Local<Value>         newParameter(Arguments const& args) const;
    Local<Value>         mandatory(Arguments const& args) const;
    Local<Value>         optional(Arguments const& args) const;
    Local<Value>         addOverload(Arguments const& args);
    Local<Value>         setCallback(Arguments const& args) const;
    Local<Value>         setup(Arguments const& args) const;
    Local<Value>         isRegistered();
    Local<Value>         toString(Arguments const& args);
    Local<Value>         setSoftEnum(Arguments const& args);
    Local<Value>         addSoftEnumValues(Arguments const& args);
    Local<Value>         removeSoftEnumValues(Arguments const& args);
    Local<Value>         getSoftEnumValues(Arguments const& args);
    Local<Value>         getSoftEnumNames(Arguments const& args);
};

extern ClassDefine<CommandClass> CommandClassBuilder;
