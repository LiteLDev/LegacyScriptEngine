#pragma once
#include "api/APIHelp.h"
#include "ll/api/form/SimpleForm.h"
#include "lse/api/helper/RawCustomForm.h"

//////////////////// Classes ////////////////////

class SimpleFormClass : public ScriptClass {
private:
    ll::form::SimpleForm form;

public:
    SimpleFormClass();

    ll::form::SimpleForm* get() { return &form; }

    static Local<Object>         newForm();
    static ll::form::SimpleForm* extract(Local<Value> v);
    static void
    sendForm(ll::form::SimpleForm* form, Player* player, script::Local<Function>& callback, bool update = false);

    Local<Value> setTitle(const Arguments& args);
    Local<Value> setContent(const Arguments& args);
    Local<Value> addButton(const Arguments& args);
    Local<Value> addHeader(const Arguments& args);
    Local<Value> addLabel(const Arguments& args);
    Local<Value> addDivider(const Arguments& args);
};
extern ClassDefine<SimpleFormClass> SimpleFormClassBuilder;

class CustomFormClass : public ScriptClass {
private:
    lse::form::RawCustomForm form;

public:
    CustomFormClass();

    lse::form::RawCustomForm* get() { return &form; }

    static Local<Object>                 newForm();
    static lse::form::RawCustomForm* extract(Local<Value> v);
    static void
    sendForm(lse::form::RawCustomForm* form, Player* player, script::Local<Function>& callback, bool update = true);

    Local<Value> setTitle(const Arguments& args);
    Local<Value> addHeader(const Arguments& args);
    Local<Value> addLabel(const Arguments& args);
    Local<Value> addDivider(const Arguments& args);
    Local<Value> addInput(const Arguments& args);
    Local<Value> addSwitch(const Arguments& args);
    Local<Value> addDropdown(const Arguments& args);
    Local<Value> addSlider(const Arguments& args);
    Local<Value> addStepSlider(const Arguments& args);
    Local<Value> setSubmitButton(const Arguments& args);
};
extern ClassDefine<CustomFormClass> CustomFormClassBuilder;
