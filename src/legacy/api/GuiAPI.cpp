#include "api/GuiAPI.h"

#include "api/APIHelp.h"
#include "api/McAPI.h"
#include "api/PlayerAPI.h"
#include "engine/GlobalShareData.h"
#include "ll/api/form/FormBase.h"
#include "ll/api/form/SimpleForm.h"
#include "ll/api/service/GamingStatus.h"
#include "mc/world/actor/player/Player.h"
#include "nlohmann/json_fwd.hpp"

using ll::form::FormCancelReason;

//////////////////// Class Definition ////////////////////

ClassDefine<SimpleFormClass> SimpleFormClassBuilder = defineClass<SimpleFormClass>("LLSE_SimpleForm")
                                                          .constructor(nullptr)
                                                          .instanceFunction("setTitle", &SimpleFormClass::setTitle)
                                                          .instanceFunction("setContent", &SimpleFormClass::setContent)
                                                          .instanceFunction("addButton", &SimpleFormClass::addButton)
                                                          .instanceFunction("addHeader", &SimpleFormClass::addHeader)
                                                          .instanceFunction("addLabel", &SimpleFormClass::addLabel)
                                                          .instanceFunction("addDivider", &SimpleFormClass::addDivider)
                                                          .build();

ClassDefine<CustomFormClass> CustomFormClassBuilder =
    defineClass<CustomFormClass>("LLSE_CustomForm")
        .constructor(nullptr)
        .instanceFunction("setTitle", &CustomFormClass::setTitle)
        .instanceFunction("addHeader", &CustomFormClass::addHeader)
        .instanceFunction("addLabel", &CustomFormClass::addLabel)
        .instanceFunction("addDivider", &CustomFormClass::addDivider)
        .instanceFunction("addInput", &CustomFormClass::addInput)
        .instanceFunction("addSwitch", &CustomFormClass::addSwitch)
        .instanceFunction("addDropdown", &CustomFormClass::addDropdown)
        .instanceFunction("addSlider", &CustomFormClass::addSlider)
        .instanceFunction("addStepSlider", &CustomFormClass::addStepSlider)
        .instanceFunction("setSubmitButton", &CustomFormClass::setSubmitButton)
        .build();

//////////////////// Simple Form ////////////////////

SimpleFormClass::SimpleFormClass() : ScriptClass(ScriptClass::ConstructFromCpp<SimpleFormClass>{}), form("", "") {}

// 生成函数
Local<Object> SimpleFormClass::newForm() {
    auto newp = new SimpleFormClass();
    return newp->getScriptObject();
}

ll::form::SimpleForm* SimpleFormClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<SimpleFormClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<SimpleFormClass>(v)->get();
    else return nullptr;
}

void SimpleFormClass::sendForm(
    ll::form::SimpleForm*    form,
    Player*                  player,
    script::Local<Function>& callback,
    bool                     update
) {
    script::Global<Function> callbackFunc{callback};
    auto                     cb = [engine{EngineScope::currentEngine()},
               callback{std::move(callbackFunc)}](Player& pl, int chosen, FormCancelReason reason) {
        if ((ll::getGamingStatus() != ll::GamingStatus::Running)) return;
        if (!EngineManager::isValid(engine)) return;
        if (callback.isEmpty()) return;

        EngineScope scope(engine);
        try {
            callback.get().call(
                {},
                PlayerClass::newPlayer(&pl),
                chosen >= 0 ? Number::newNumber(chosen) : Local<Value>(),
                reason.has_value() ? Number::newNumber((uchar)reason.value()) : Local<Value>()
            );
        }
        CATCH_IN_CALLBACK("sendForm")
    };
    if (update) form->sendUpdate(*player, std::move(cb));
    else form->sendTo(*player, std::move(cb));
}

// 成员函数
Local<Value> SimpleFormClass::setTitle(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        form.setTitle(args[0].asString().toString());
        return this->getScriptObject();
    }
    CATCH("Fail in setTitle!")
}

Local<Value> SimpleFormClass::setContent(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        form.setContent(args[0].asString().toString());
        return this->getScriptObject();
    }
    CATCH("Fail in setTitle!")
}

Local<Value> SimpleFormClass::addButton(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        std::string image = args.size() >= 2 ? args[1].asString().toString() : "";
        std::string type  = image.starts_with("http") ? "url" : "path";
        form.appendButton(args[0].asString().toString(), image, type);
        return this->getScriptObject();
    }
    CATCH("Fail in addButton!")
}

Local<Value> SimpleFormClass::addHeader(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        form.appendLabel(args[0].asString().toString());
        return this->getScriptObject();
    }
    CATCH("Fail in addHeader!")
}

Local<Value> SimpleFormClass::addLabel(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        form.appendLabel(args[0].asString().toString());
        return this->getScriptObject();
    }
    CATCH("Fail in addLabel!")
}

Local<Value> SimpleFormClass::addDivider(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 0);

    try {
        form.appendDivider();
        return this->getScriptObject();
    }
    CATCH("Fail in addDivider!")
}

//////////////////// Custom Form ////////////////////

CustomFormClass::CustomFormClass() : ScriptClass(ScriptClass::ConstructFromCpp<CustomFormClass>{}), form() {}

// 生成函数
Local<Object> CustomFormClass::newForm() {
    auto newp = new CustomFormClass();
    return newp->getScriptObject();
}

lse::form::RawCustomForm* CustomFormClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<CustomFormClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<CustomFormClass>(v)->get();
    else return nullptr;
}

// 成员函数
void CustomFormClass::sendForm(
    lse::form::RawCustomForm* form,
    Player*                   player,
    script::Local<Function>&  callback,
    bool                      update
) {
    script::Global<Function> callbackFunc{callback};
    auto                     cb = [engine{EngineScope::currentEngine()},
               callback{std::move(callbackFunc)
               }](Player& player, lse::form::CustomFormResult const& data, FormCancelReason reason) {
        if (ll::getGamingStatus() != ll::GamingStatus::Running) return;
        if (!EngineManager::isValid(engine)) return;
        if (callback.isEmpty()) return;

        EngineScope  scope(engine);
        Local<Value> result;
        if (data) {
            auto dataJson = nlohmann::ordered_json::parse(*data);
            result        = JsonToValue(dataJson);
            if (result.isNull()) result = Array::newArray();
        }
        auto reasonVal = reason.has_value() ? Number::newNumber((uchar)reason.value()) : Local<Value>();
        try {
            callback.get().call({}, PlayerClass::newPlayer(&player), result, reasonVal);
        }
        CATCH_IN_CALLBACK("sendForm")
    };
    if (update) form->sendRawUpdate(*player, std::move(cb));
    else form->sendRawTo(*player, std::move(cb));
}

Local<Value> CustomFormClass::setTitle(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        form.setTitle(args[0].asString().toString());
        return this->getScriptObject();
    }
    CATCH("Fail in setTitle!")
}

Local<Value> CustomFormClass::setSubmitButton(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        form.setSubmitButton(args[0].asString().toString());
        return this->getScriptObject();
    }
    CATCH("Fail in setSubmitButton!")
}

Local<Value> CustomFormClass::addHeader(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        form.appendLabel(args[0].asString().toString());
        return this->getScriptObject();
    }
    CATCH("Fail in addHeader!")
}

Local<Value> CustomFormClass::addLabel(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        form.appendLabel(args[0].asString().toString());
        return this->getScriptObject();
    }
    CATCH("Fail in addLabel!")
}

Local<Value> CustomFormClass::addDivider(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 0)

    try {
        form.appendDivider();
        return this->getScriptObject();
    }
    CATCH("Fail in addDivider!")
}

Local<Value> CustomFormClass::addInput(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kString);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kString);
    if (args.size() >= 4) CHECK_ARG_TYPE(args[3], ValueKind::kString);

    try {
        std::string placeholder = args.size() >= 2 ? args[1].asString().toString() : "";
        std::string def         = args.size() >= 3 ? args[2].asString().toString() : "";
        std::string tooltip     = args.size() >= 4 ? args[3].asString().toString() : "";

        form.appendInput("", args[0].asString().toString(), placeholder, def, tooltip);
        return this->getScriptObject();
    }
    CATCH("Fail in addInput!")
}

Local<Value> CustomFormClass::addSwitch(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    if (args.size() >= 2) {
        if (!args[1].isBoolean() && !args[1].isNumber()) {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
    }
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kString);

    try {
        bool def =
            args.size() >= 2 ? args[1].isBoolean() ? args[1].asBoolean().value() : args[1].asNumber().toInt32() : false;
        std::string tooltip = args.size() >= 3 ? args[2].asString().toString() : "";

        form.appendToggle("", args[0].asString().toString(), def, tooltip);
        return this->getScriptObject();
    }
    CATCH("Fail in addSwitch!")
}

Local<Value> CustomFormClass::addDropdown(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    CHECK_ARG_TYPE(args[1], ValueKind::kArray);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    if (args.size() >= 4) CHECK_ARG_TYPE(args[3], ValueKind::kString);

    try {
        auto                     optionsArr = args[1].asArray();
        std::vector<std::string> options;
        options.reserve(optionsArr.size());
        for (size_t i = 0; i < optionsArr.size(); ++i) {
            options.emplace_back(optionsArr.get(i).asString().toString());
        }

        int         def     = args.size() >= 3 ? args[2].asNumber().toInt32() : 0;
        std::string tooltip = args.size() >= 4 ? args[3].asString().toString() : "";

        form.appendDropdown("", args[0].asString().toString(), options, def, tooltip);
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
    if (args.size() >= 6) CHECK_ARG_TYPE(args[5], ValueKind::kString);

    try {
        int minValue = args[1].asNumber().toInt32();
        int maxValue = args[2].asNumber().toInt32();
        if (minValue >= maxValue) maxValue = minValue + 1;

        int step     = args.size() >= 4 ? args[3].asNumber().toInt32() : 1;
        int defValue = args.size() >= 5 ? args[4].asNumber().toInt32() : minValue;
        if (defValue < minValue || defValue > maxValue) defValue = minValue;
        std::string tooltip = args.size() >= 6 ? args[5].asString().toString() : "";

        form.appendSlider("", args[0].asString().toString(), minValue, maxValue, step, defValue, tooltip);
        return this->getScriptObject();
    }
    CATCH("Fail in addSlider!")
}

Local<Value> CustomFormClass::addStepSlider(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    CHECK_ARG_TYPE(args[1], ValueKind::kArray);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    if (args.size() >= 4) CHECK_ARG_TYPE(args[3], ValueKind::kString);

    try {
        auto                     stepsArr = args[1].asArray();
        std::vector<std::string> steps;
        steps.reserve(stepsArr.size());
        for (size_t i = 0; i < stepsArr.size(); ++i) steps.push_back(stepsArr.get(i).asString().toString());

        int         defIndex = args.size() >= 3 ? args[2].asNumber().toInt32() : 0;
        std::string tooltip  = args.size() >= 4 ? args[3].asString().toString() : "";

        form.appendStepSlider("", args[0].asString().toString(), steps, defIndex, tooltip);
        return this->getScriptObject();
    }
    CATCH("Fail in addStepSlider!")
}

//////////////////// APIs ////////////////////

Local<Value> McClass::newSimpleForm(const Arguments&) { return SimpleFormClass::newForm(); }

Local<Value> McClass::newCustomForm(const Arguments&) { return CustomFormClass::newForm(); }
