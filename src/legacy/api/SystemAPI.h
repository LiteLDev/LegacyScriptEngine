#pragma once
#include "legacy/api/APIHelp.h"

//////////////////// System Static ////////////////////

class SystemClass {
public:
    static Local<Value> getTimeStr(Arguments const& args);
    static Local<Value> getTimeObj(Arguments const& args);
    static Local<Value> randomGuid(Arguments const& args);

    static Local<Value> cmd(Arguments const& args);
    static Local<Value> newProcess(Arguments const& args);
};
extern ClassDefine<> SystemClassBuilder;
