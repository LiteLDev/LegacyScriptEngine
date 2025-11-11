#pragma once
#include "api/APIHelp.h"
#include "mc/server/commands/CommandOrigin.h"

class CommandOriginClass;
extern ClassDefine<void>               OriginTypeStaticBuilder;
extern ClassDefine<CommandOriginClass> CommandOriginClassBuilder;

class CommandOriginClass : public ScriptClass {
public:
    std::shared_ptr<CommandOrigin> ptr;
    inline CommandOrigin*    get() { return ptr.get(); }

public:
    CommandOriginClass(std::shared_ptr<CommandOrigin> p);
    static std::shared_ptr<CommandOrigin> extract(Local<Value> v);
    Local<Value>                          getOriginType();
    Local<Value>                          getOriginTypeName();
    Local<Value>                          getOriginName();
    Local<Value>                          getBlockPosition();
    Local<Value>                          getPosition();
    Local<Value>                          getEntity();
    Local<Value>                          getPlayer();
    Local<Value>                          getNbt(const Arguments& args);
    Local<Value>                          toString();
};
