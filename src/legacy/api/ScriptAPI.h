#pragma once
#include "api/APIHelp.h"

//////////////////// APIs ////////////////////

Local<Value> Log(Arguments const& args);
Local<Value> ColorLog(Arguments const& args);
Local<Value> FastLog(Arguments const& args);

Local<Value> SetTimeout(Arguments const& args);
Local<Value> SetInterval(Arguments const& args);
Local<Value> ClearInterval(Arguments const& args);
