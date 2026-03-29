#pragma once
#include "api/APIHelp.h"

class I18nClass {
public:
    static Local<Value> tr(Arguments const& args);
    static Local<Value> trl(Arguments const& args);
    static Local<Value> get(Arguments const& args);
    static Local<Value> load(Arguments const& args);
};
extern ClassDefine<void> I18nClassBuilder;
