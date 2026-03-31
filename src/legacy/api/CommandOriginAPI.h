#pragma once
#include "legacy/api/APIHelp.h"
#include "mc/server/commands/CommandOrigin.h"

class CommandOriginClass;
extern ClassDefine<void>               OriginTypeStaticBuilder;
extern ClassDefine<CommandOriginClass> CommandOriginClassBuilder;

class CommandOriginClass : public ScriptClass {
public:
    // Storing CommandOrigin as shared_ptr is for asynchronous command processing.
    std::shared_ptr<CommandOrigin const> origin;
    std::shared_ptr<CommandOrigin const> get() { return origin; }

public:
    CommandOriginClass(std::shared_ptr<CommandOrigin const> const& ori);
    Local<Value> getOriginType();
    Local<Value> getOriginTypeName();
    Local<Value> getOriginName();
    Local<Value> getBlockPosition();
    Local<Value> getPosition();
    Local<Value> getEntity();
    Local<Value> getPlayer();
    Local<Value> getNbt(Arguments const& args);
    Local<Value> toString();
};
