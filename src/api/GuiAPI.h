#pragma once
#include "api/APIHelp.h"

#include "ll/api/form/CustomForm.h"
#include "ll/api/form/FormBase.h"
#include "ll/api/form/SimpleForm.h"

//////////////////// Classes ////////////////////

class SimpleFormClass : public ScriptClass {
private:
  ll::form::SimpleForm form;

public:
  SimpleFormClass();

  ll::form::SimpleForm *get() { return &form; }

  static Local<Object> newForm();
  static ll::form::SimpleForm *extract(Local<Value> v);
  static bool sendForm(ll::form::SimpleForm *form, Player *player,
                       script::Local<Function> &callback);

  Local<Value> setTitle(const Arguments &args);
  Local<Value> setContent(const Arguments &args);
  Local<Value> addButton(const Arguments &args);
};
extern ClassDefine<SimpleFormClass> SimpleFormClassBuilder;

class CustomFormClass : public ScriptClass {
private:
  ll::form::CustomForm form;

public:
  CustomFormClass();

  ll::form::CustomForm *get() { return &form; }

  static Local<Object> newForm();
  static ll::form::CustomForm *extract(Local<Value> v);
  static bool sendForm(ll::form::CustomForm *form, Player *player,
                       script::Local<Function> &callback);

  Local<Value> setTitle(const Arguments &args);
  Local<Value> addLabel(const Arguments &args);
  Local<Value> addInput(const Arguments &args);
  Local<Value> addSwitch(const Arguments &args);
  Local<Value> addDropdown(const Arguments &args);
  Local<Value> addSlider(const Arguments &args);
  Local<Value> addStepSlider(const Arguments &args);
};
extern ClassDefine<CustomFormClass> CustomFormClassBuilder;