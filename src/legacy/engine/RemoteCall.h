#pragma once
#include "legacy/api/APIHelp.h"

#include <string>

//////////////////// Funcs ////////////////////

bool LLSEExportFunc(ScriptEngine* engine, Local<Function> const& func, std::string const& exportName);
bool LLSERemoveAllExportedFuncs(std::shared_ptr<ScriptEngine> const& engine);

class ModuleMessage;
void RemoteSyncCallRequest(ModuleMessage& msg);
void RemoteSyncCallReturn(ModuleMessage const& msg);
