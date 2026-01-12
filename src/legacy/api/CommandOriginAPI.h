#pragma once
#include "api/APIHelp.h"
#include "mc/server/commands/CommandOrigin.h"

class CommandOriginClass;
extern ClassDefine<void>               OriginTypeStaticBuilder;
extern ClassDefine<CommandOriginClass> CommandOriginClassBuilder;

class CommandOriginClass : public ScriptClass {
public:
    CommandOrigin const&        origin;
    inline CommandOrigin const& get() { return origin; }

public:
    CommandOriginClass(CommandOrigin const& ori);
    Local<Value> getOriginType();
    Local<Value> getOriginTypeName();
    Local<Value> getOriginName();
    Local<Value> getBlockPosition();
    Local<Value> getPosition();
    Local<Value> getEntity();
    Local<Value> getPlayer();
    Local<Value> getNbt(const Arguments& args);
    Local<Value> toString();
};
