#pragma once

#include "api/APIHelp.h"

//////////////////// Classes ////////////////////

class LoggerClass {
public:
    static Local<Value> log(Arguments const& args);
    static Local<Value> debug(Arguments const& args);
    static Local<Value> info(Arguments const& args);
    static Local<Value> warn(Arguments const& args);
    static Local<Value> error(Arguments const& args);
    static Local<Value> fatal(Arguments const& args);

    static Local<Value> setTitle(Arguments const& args);
    static Local<Value> setConsole(Arguments const& args);
    static Local<Value> setFile(Arguments const& args);
    static Local<Value> setPlayer(Arguments const& args);

    static Local<Value> setLogLevel(Arguments const& args);
};
extern ClassDefine<> LoggerClassBuilder;
