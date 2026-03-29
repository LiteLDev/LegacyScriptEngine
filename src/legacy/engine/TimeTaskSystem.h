#pragma once
#include "api/APIHelp.h"
#include "engine/MessageSystem.h"

#include <vector>

///////////////////////// API /////////////////////////

// void NewTimeout_s(
//     script::Global<Function>     func,
//     vector<script::Local<Value>> paras,
//     int                          timeout,
//     ScriptEngine*                engine = EngineScope::currentEngine()
// );

int  NewTimeout(Local<Function> const& func, std::vector<Local<Value>> paras, int timeout);
int  NewTimeout(Local<String> const& func, int timeout);
int  NewInterval(Local<Function> const& func, std::vector<Local<Value>> paras, int timeout);
int  NewInterval(Local<String> const& func, int timeout);
bool ClearTimeTask(int const& id);
bool CheckTimeTask(int const& id);

///////////////////////// Func /////////////////////////

void LLSERemoveTimeTaskData(std::shared_ptr<ScriptEngine> const& engine);
