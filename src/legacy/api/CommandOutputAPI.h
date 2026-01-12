#pragma once
#include "api/APIHelp.h"
#include "mc/server/commands/CommandOutput.h"

class CommandOutputClass;
extern ClassDefine<CommandOutputClass> CommandOutputClassBuilder;

class CommandOutputClass : public ScriptClass {
public:
    // Storing CommandOutput and CommandOrigin as shared_ptr is for asynchronous command processing.
    std::shared_ptr<CommandOutput>        output;
    std::shared_ptr<CommandOrigin const>  origin;
    bool                                  isAsync;
    inline std::shared_ptr<CommandOutput> get() { return output; }

public:
    CommandOutputClass(std::shared_ptr<CommandOutput> output, std::shared_ptr<CommandOrigin const> origin);

    Local<Value> empty();
    Local<Value> getSuccessCount();
    Local<Value> success(const Arguments& args);
    Local<Value> addMessage(const Arguments& args);
    Local<Value> error(const Arguments& args);
    void         send();
    Local<Value> toString(const Arguments& args);
};
