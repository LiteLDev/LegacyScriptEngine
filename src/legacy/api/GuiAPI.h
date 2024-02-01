#pragma once
#include "api/APIHelp.h"
#include "legacyapi/form/FormUI.h"

//////////////////// Classes ////////////////////

class SimpleFormClass : public ScriptClass {
private:
    lse::form::SimpleForm form;

public:
    SimpleFormClass();

    lse::form::SimpleForm* get() { return &form; }

    static Local<Object>          newForm();
    static lse::form::SimpleForm* extract(Local<Value> v);
    static void sendForm(lse::form::SimpleForm* form, Player* player, script::Local<Function>& callback);

    Local<Value> setTitle(const Arguments& args);
    Local<Value> setContent(const Arguments& args);
    Local<Value> addButton(const Arguments& args);
};
extern ClassDefine<SimpleFormClass> SimpleFormClassBuilder;

class CustomFormClass : public ScriptClass {
private:
    lse::form::CustomForm form;

public:
    CustomFormClass();

    lse::form::CustomForm* get() { return &form; }

    static Local<Object>          newForm();
    static lse::form::CustomForm* extract(Local<Value> v);
    static void sendForm(lse::form::CustomForm* form, Player* player, script::Local<Function>& callback);

    Local<Value> setTitle(const Arguments& args);
    Local<Value> addLabel(const Arguments& args);
    Local<Value> addInput(const Arguments& args);
    Local<Value> addSwitch(const Arguments& args);
    Local<Value> addDropdown(const Arguments& args);
    Local<Value> addSlider(const Arguments& args);
    Local<Value> addStepSlider(const Arguments& args);
};
extern ClassDefine<CustomFormClass> CustomFormClassBuilder;
