#pragma once
#include "api/APIHelp.h"
#include "ll/api/form/SimpleForm.h"
#include "lse/api/helper/CustomFormWrapper.h"

//////////////////// Classes ////////////////////

class SimpleFormClass : public ScriptClass {
private:
    ll::form::SimpleForm form;

public:
    SimpleFormClass();

    ll::form::SimpleForm* get() { return &form; }

    static Local<Object>         newForm();
    static ll::form::SimpleForm* extract(Local<Value> v);
    static void sendForm(ll::form::SimpleForm* form, Player* player, script::Local<Function>& callback);

    Local<Value> setTitle(const Arguments& args);
    Local<Value> setContent(const Arguments& args);
    Local<Value> addButton(const Arguments& args);
};
extern ClassDefine<SimpleFormClass> SimpleFormClassBuilder;

class CustomFormClass : public ScriptClass {
private:
    lse::form::CustomFormWrapper form;

public:
    CustomFormClass();

    lse::form::CustomFormWrapper* get() { return &form; }

    static Local<Object>                 newForm();
    static lse::form::CustomFormWrapper* extract(Local<Value> v);
    static void sendForm(lse::form::CustomFormWrapper* form, Player* player, script::Local<Function>& callback);

    Local<Value> setTitle(const Arguments& args);
    Local<Value> addLabel(const Arguments& args);
    Local<Value> addInput(const Arguments& args);
    Local<Value> addSwitch(const Arguments& args);
    Local<Value> addDropdown(const Arguments& args);
    Local<Value> addSlider(const Arguments& args);
    Local<Value> addStepSlider(const Arguments& args);
};
extern ClassDefine<CustomFormClass> CustomFormClassBuilder;
