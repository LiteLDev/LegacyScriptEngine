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

int  NewTimeout(Local<Function> func, std::vector<Local<Value>> paras, int timeout);
int  NewTimeout(Local<String> func, int timeout);
int  NewInterval(Local<Function> func, std::vector<Local<Value>> paras, int timeout);
int  NewInterval(Local<String> func, int timeout);
bool ClearTimeTask(int const& id);
bool CheckTimeTask(int const& id);

///////////////////////// Func /////////////////////////

void LLSERemoveTimeTaskData(ScriptEngine* engine);
