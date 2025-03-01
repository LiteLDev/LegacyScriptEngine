#pragma once
#include "api/APIHelp.h"
#include "mc/server/commands/CommandOutput.h"

class CommandOutputClass;
extern ClassDefine<CommandOutputClass> CommandOutputClassBuilder;

class CommandOutputClass : public ScriptClass {
    CommandOutput*        ptr;
    inline CommandOutput* get() { return ptr; }

public:
    CommandOutputClass(CommandOutput* p);
    static Local<Object> newCommandOutput(CommandOutput* p);

    Local<Value> empty();

    Local<Value> getSuccessCount();

    Local<Value> success(const Arguments& args);

    Local<Value> addMessage(const Arguments& args);

    Local<Value> error(const Arguments& args);

    // Local<Value> setHasPlayerText()
    //{
    //     try
    //     {
    //         get()->setHasPlayerText();
    //         return Boolean::newBoolean(true);
    //     }
    //     CATCH("Fail in getBlockPosition!");
    // };

    // Local<Value> wantsData()
    //{
    //     try
    //     {
    //         return Boolean::newBoolean(get()->wantsData());
    //     }
    //     CATCH("Fail in getBlockPosition!");
    // };

    // Local<Value> addToResultList(const Arguments& args);

    // Local<Value> forceOutput(const Arguments& args);

    // Local<Value> getData() const;

    // Local<Value> getMessages() const;

    // Local<Value> load(const Arguments& args);

    Local<Value> toString(const Arguments& args);
};
