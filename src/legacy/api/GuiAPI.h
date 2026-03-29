#pragma once
#include "api/APIHelp.h"
#include "ll/api/form/SimpleForm.h"
#include "lse/api/helper/RawCustomForm.h"

//////////////////// Classes ////////////////////

class SimpleFormClass : public ScriptClass {
    ll::form::SimpleForm form;

public:
    SimpleFormClass();

    ll::form::SimpleForm* get() { return &form; }

    static Local<Object>         newForm();
    static ll::form::SimpleForm* extract(Local<Value> const& v);
    static void
    sendForm(ll::form::SimpleForm* form, Player* player, Local<Function> const& callback, bool update = false);

    Local<Value> setTitle(Arguments const& args);
    Local<Value> setContent(Arguments const& args);
    Local<Value> addButton(Arguments const& args);
    Local<Value> addHeader(Arguments const& args);
    Local<Value> addLabel(Arguments const& args);
    Local<Value> addDivider(Arguments const& args);
};
extern ClassDefine<SimpleFormClass> SimpleFormClassBuilder;

class CustomFormClass : public ScriptClass {
    lse::form::RawCustomForm form;

public:
    CustomFormClass();

    lse::form::RawCustomForm* get() { return &form; }

    static Local<Object>             newForm();
    static lse::form::RawCustomForm* extract(Local<Value> const& v);
    static void
    sendForm(lse::form::RawCustomForm* form, Player* player, Local<Function> const& callback, bool update = true);

    Local<Value> setTitle(Arguments const& args);
    Local<Value> addHeader(Arguments const& args);
    Local<Value> addLabel(Arguments const& args);
    Local<Value> addDivider(Arguments const& args);
    Local<Value> addInput(Arguments const& args);
    Local<Value> addSwitch(Arguments const& args);
    Local<Value> addDropdown(Arguments const& args);
    Local<Value> addSlider(Arguments const& args);
    Local<Value> addStepSlider(Arguments const& args);
    Local<Value> setSubmitButton(Arguments const& args);
};
extern ClassDefine<CustomFormClass> CustomFormClassBuilder;
