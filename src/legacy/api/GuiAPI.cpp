#include "api/GuiAPI.h"

#include "api/APIHelp.h"
#include "api/McAPI.h"
#include "api/PlayerAPI.h"
#include "engine/EngineOwnData.h"
#include "engine/GlobalShareData.h"

// #include <llapi/SendPacketAPI.h> //todo
#include "ll/api/service/ServerInfo.h"

#include <cstdlib>
#include <iostream>
#include <mc/world/actor/player/Player.h>

//////////////////// Class Definition ////////////////////

ClassDefine<SimpleFormClass> SimpleFormClassBuilder = defineClass<SimpleFormClass>("LLSE_SimpleForm")
                                                          .constructor(nullptr)
                                                          .instanceFunction("setTitle", &SimpleFormClass::setTitle)
                                                          .instanceFunction("setContent", &SimpleFormClass::setContent)
                                                          .instanceFunction("addButton", &SimpleFormClass::addButton)
                                                          .build();

ClassDefine<CustomFormClass> CustomFormClassBuilder =
    defineClass<CustomFormClass>("LLSE_CustomForm")
        .constructor(nullptr)
        .instanceFunction("setTitle", &CustomFormClass::setTitle)
        .instanceFunction("addLabel", &CustomFormClass::addLabel)
        .instanceFunction("addInput", &CustomFormClass::addInput)
        .instanceFunction("addSwitch", &CustomFormClass::addSwitch)
        .instanceFunction("addDropdown", &CustomFormClass::addDropdown)
        .instanceFunction("addSlider", &CustomFormClass::addSlider)
        .instanceFunction("addStepSlider", &CustomFormClass::addStepSlider)
        .build();

//////////////////// Simple Form ////////////////////

SimpleFormClass::SimpleFormClass() : ScriptClass(ScriptClass::ConstructFromCpp<SimpleFormClass>{}), form("", "") {}

// 生成函数
Local<Object> SimpleFormClass::newForm() {
    auto newp = new SimpleFormClass();
    return newp->getScriptObject();
}

lse::form::SimpleForm* SimpleFormClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<SimpleFormClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<SimpleFormClass>(v)->get();
    else return nullptr;
}

void SimpleFormClass::sendForm(lse::form::SimpleForm* form, Player* player, script::Local<Function>& callback) {
    script::Global<Function> callbackFunc{callback};

    form->sendTo(
        player,
        [engine{EngineScope::currentEngine()}, callback{std::move(callbackFunc)}](Player* pl, int chosen) {
            if ((ll::getServerStatus() != ll::ServerStatus::Running)) return;
            if (!EngineManager::isValid(engine)) return;
            if (callback.isEmpty()) return;

            EngineScope scope(engine);
            try {
                if (chosen < 0) callback.get().call({}, PlayerClass::newPlayer(pl), Local<Value>());
                else callback.get().call({}, PlayerClass::newPlayer(pl), Number::newNumber(chosen));
            }
            CATCH_IN_CALLBACK("sendForm")
        }
    );
}

// 成员函数
Local<Value> SimpleFormClass::setTitle(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        form.setTitle(args[0].toStr());
        return this->getScriptObject();
    }
    CATCH("Fail in setTitle!")
}

Local<Value> SimpleFormClass::setContent(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        form.setContent(args[0].toStr());
        return this->getScriptObject();
    }
    CATCH("Fail in setTitle!")
}

Local<Value> SimpleFormClass::addButton(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        string image = args.size() >= 2 ? args[1].toStr() : "";
        form.addButton(args[0].toStr(), image);
        return this->getScriptObject();
    }
    CATCH("Fail in addButton!")
}

//////////////////// Custom Form ////////////////////

CustomFormClass::CustomFormClass() : ScriptClass(ScriptClass::ConstructFromCpp<CustomFormClass>{}), form("") {}

// 生成函数
Local<Object> CustomFormClass::newForm() {
    auto newp = new CustomFormClass();
    return newp->getScriptObject();
}

lse::form::CustomForm* CustomFormClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<CustomFormClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<CustomFormClass>(v)->get();
    else return nullptr;
}

// 成员函数
void CustomFormClass::sendForm(lse::form::CustomForm* form, Player* player, script::Local<Function>& callback) {
    script::Global<Function> callbackFunc{callback};

    form->sendToForRawJson(
        player,
        [engine{EngineScope::currentEngine()}, callback{std::move(callbackFunc)}](Player* player, std::string data) {
            if (ll::getServerStatus() != ll::ServerStatus::Running) return;
            if (!EngineManager::isValid(engine)) return;
            if (callback.isEmpty()) return;

            EngineScope scope(engine);
            try {
                callback.get().call({}, PlayerClass::newPlayer(player), JsonToValue(data));
            }
            CATCH_IN_CALLBACK("sendForm")
        }
    );
}

Local<Value> CustomFormClass::setTitle(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        form.setTitle(args[0].toStr());
        return this->getScriptObject();
    }
    CATCH("Fail in setTitle!")
}

Local<Value> CustomFormClass::addLabel(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        form.addLabel(args[0].toStr(), args[0].toStr());
        return this->getScriptObject();
    }
    CATCH("Fail in addLabel!")
}

Local<Value> CustomFormClass::addInput(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kString);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kString);

    try {
        string placeholder = args.size() >= 2 ? args[1].toStr() : "";
        string def         = args.size() >= 3 ? args[2].toStr() : "";

        form.addInput(args[0].toStr(), args[0].toStr(), placeholder, def);
        return this->getScriptObject();
    }
    CATCH("Fail in addInput!")
}

Local<Value> CustomFormClass::addSwitch(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    if (args.size() >= 2) {
        if (!args[1].isBoolean() && !args[1].isNumber()) {
            LOG_WRONG_ARG_TYPE();
            return Local<Value>();
        }
    }

    try {
        bool def =
            args.size() >= 2 ? args[1].isBoolean() ? args[1].asBoolean().value() : args[1].asNumber().toInt32() : false;

        form.addToggle(args[0].toStr(), args[0].toStr(), def);
        return this->getScriptObject();
    }
    CATCH("Fail in addSwitch!")
}

Local<Value> CustomFormClass::addDropdown(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    CHECK_ARG_TYPE(args[1], ValueKind::kArray);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);

    try {
        auto           optionsArr = args[1].asArray();
        vector<string> options;
        for (int i = 0; i < optionsArr.size(); ++i) options.push_back(optionsArr.get(i).toStr());

        int def = args.size() >= 3 ? args[2].asNumber().toInt32() : 0;

        form.addDropdown(args[0].toStr(), args[0].toStr(), options, def);
        return this->getScriptObject();
    }
    CATCH("Fail in addDropdown!")
}

Local<Value> CustomFormClass::addSlider(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 3)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    if (args.size() >= 4) CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
    if (args.size() >= 5) CHECK_ARG_TYPE(args[4], ValueKind::kNumber);

    try {
        int minValue = args[1].asNumber().toInt32();
        int maxValue = args[2].asNumber().toInt32();
        if (minValue >= maxValue) maxValue = minValue + 1;

        int step     = args.size() >= 4 ? args[3].asNumber().toInt32() : 1;
        int defValue = args.size() >= 5 ? args[4].asNumber().toInt32() : minValue;
        if (defValue < minValue || defValue > maxValue) defValue = minValue;

        form.addSlider(args[0].toStr(), args[0].toStr(), minValue, maxValue, step, defValue);
        return this->getScriptObject();
    }
    CATCH("Fail in addSlider!")
}

Local<Value> CustomFormClass::addStepSlider(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    CHECK_ARG_TYPE(args[1], ValueKind::kArray);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);

    try {
        auto           stepsArr = args[1].asArray();
        vector<string> steps;
        for (int i = 0; i < stepsArr.size(); ++i) steps.push_back(stepsArr.get(i).toStr());

        int defIndex = args.size() >= 3 ? args[2].asNumber().toInt32() : 0;

        form.addStepSlider(args[0].toStr(), args[0].toStr(), steps, defIndex);
        return this->getScriptObject();
    }
    CATCH("Fail in addStepSlider!")
}

//////////////////// APIs ////////////////////

Local<Value> McClass::newSimpleForm(const Arguments&) { return SimpleFormClass::newForm(); }

Local<Value> McClass::newCustomForm(const Arguments&) { return CustomFormClass::newForm(); }
