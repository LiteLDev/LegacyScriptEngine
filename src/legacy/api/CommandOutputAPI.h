#pragma once
#include "api/APIHelp.h"
#include "mc/server/commands/CommandOutput.h"

class CommandOutputClass;
extern ClassDefine<CommandOutputClass> CommandOutputClassBuilder;

class CommandOutputClass : public ScriptClass {
public:
    CommandOutput&        output;
    CommandOrigin const&  origin;
    bool                  isAsync;
    inline CommandOutput& get() { return output; }

public:
    CommandOutputClass(CommandOutput& output, CommandOrigin const& origin);

    Local<Value> empty();

    Local<Value> getSuccessCount();

    Local<Value> success(const Arguments& args);

    Local<Value> addMessage(const Arguments& args);

    Local<Value> error(const Arguments& args);

    void send();

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
