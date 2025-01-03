#pragma once
#include "api/CommandCompatibleAPI.h"
#include "ll/api/command/CommandHandle.h"

extern ClassDefine<void> ParamTypeStaticBuilder;
extern ClassDefine<void> PermissionStaticBuilder;
extern ClassDefine<void> ParamOptionStaticBuilder;

bool LLSERemoveCmdCallback(script::ScriptEngine* engine);

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
    std::string                        commandName;
    std::string                        description;
    inline ll::command::CommandHandle& get() {
        return ll::command::CommandRegistrar::getInstance().getOrCreateCommand(commandName);
    }
    inline std::vector<std::string> parseStringList(Local<Array> arr) {
        if (arr.size() == 0 || !arr.get(0).isString()) return {};
        std::vector<std::string> strs;
        for (size_t i = 0; i < arr.size(); ++i) {
            strs.push_back(arr.get(i).asString().toString());
        }
        return std::move(strs);
    }
    inline Local<Value> getStringArray(std::vector<std::string> values) {
        Local<Array> arr = Array::newArray(values.size());
        for (auto& str : values) {
            arr.add(String::newString(str));
        }
        return arr;
    }

public:
    CommandClass(std::string& name);
    static Local<Object> newCommand(std::string& name);
    Local<Value>         getName();
    Local<Value>         setAlias(const Arguments& args);
    Local<Value>         setEnum(const Arguments& args);
    Local<Value>         newParameter(const Arguments& args);
    Local<Value>         mandatory(const Arguments& args);
    Local<Value>         optional(const Arguments& args);
    Local<Value>         addOverload(const Arguments& args);
    Local<Value>         setCallback(const Arguments& args);
    Local<Value>         setup(const Arguments& args);
    Local<Value>         isRegistered();
    Local<Value>         toString(const Arguments& args);
    Local<Value>         setSoftEnum(const Arguments& args);
    Local<Value>         addSoftEnumValues(const Arguments& args);
    Local<Value>         removeSoftEnumValues(const Arguments& args);
    Local<Value>         getSoftEnumValues(const Arguments& args);
    Local<Value>         getSoftEnumNames(const Arguments& args);
};

extern ClassDefine<CommandClass> CommandClassBuilder;
