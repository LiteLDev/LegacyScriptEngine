#include "FormUI.h"

#include "FormPacketHelper.h"
#include "lse/Entry.h"
#include "mc/network/packet/ModalFormRequestPacket.h"
#include "mc/world/actor/player/Player.h"

#include <memory>
#include <nlohmann/json.hpp>
#include <utility>

namespace lse::form {
//////////////////////////////// Simple Form ////////////////////////////////
std::string Button::serialize() {
    try {
        nlohmann::json button;
        button["text"] = text;
        if (!image.empty()) {
            nlohmann::json imageObj;
            imageObj["type"] = image.find("textures/") == 0 ? "path" : "url";
            imageObj["data"] = image;
            button["image"]  = imageObj;
        }
        return button.dump();
    } catch (const nlohmann::json::exception&) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to generate Button in Simple Form serialize!"
        );
        return "";
    }
}

SimpleForm& SimpleForm::setTitle(const std::string& newTitle) {
    this->title = newTitle;
    return *this;
}

SimpleForm& SimpleForm::setContent(const std::string& newContent) {
    this->content = newContent;
    return *this;
}

SimpleForm& SimpleForm::addButton(std::string text, std::string image, Button::ButtonCallback cb) {
    return append(Button(text, image, cb));
}

SimpleForm& SimpleForm::append(const Button& element) {
    elements.emplace_back(std::make_shared<Button>(element));
    return *this;
}

std::string SimpleForm::serialize() {
    try {
        nlohmann::json form = nlohmann::json::parse(R"({"title":"","content":"","buttons":[],"type":"form"})");
        form["title"]       = title;
        form["content"]     = content;
        for (auto& e : elements) {
            std::string element = e->serialize();
            if (!element.empty()) form["buttons"].push_back(nlohmann::json::parse(element));
        }
        return form.dump();
    } catch (const nlohmann::json::exception&) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to generate Simple Form in serialize!"
        );
        return "";
    }
}

void SimpleForm::sendTo(Player* player, Callback cb) {
    unsigned id    = NewFormId();
    this->callback = std::move(cb);
    SetSimpleFormBuilderData(id, std::make_shared<SimpleForm>(*this));

    std::string data = serialize();
    if (data.empty()) {
        return;
    }
    ModalFormRequestPacket packet(id, data);
    player->sendNetworkPacket(packet);
}

//////////////////////////////// Modal Form ////////////////////////////////
ModalForm& ModalForm::setTitle(const std::string& newTitle) {
    this->title = newTitle;
    return *this;
}

ModalForm& ModalForm::setContent(const std::string& newContent) {
    this->content = newContent;
    return *this;
}

ModalForm& ModalForm::setConfirmButton(const std::string& text) {
    this->confirmButton = text;
    return *this;
}

ModalForm& ModalForm::setCancelButton(const std::string& text) {
    this->cancelButton = text;
    return *this;
}

std::string ModalForm::serialize() {
    try {
        nlohmann::json form =
            nlohmann::json::parse(R"({"title":"","content":"","button1":"","button2":"","type":"modal"})");
        form["title"]   = title;
        form["content"] = content;
        form["button1"] = confirmButton;
        form["button2"] = cancelButton;
        return form.dump();
    } catch (const nlohmann::json::exception&) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("Fail to generate Modal Form in serialize!");
        return "";
    }
}

void ModalForm::sendTo(Player* player, Callback cb) {
    unsigned id    = NewFormId();
    this->callback = std::move(cb);
    SetModalFormBuilderData(id, std::make_shared<ModalForm>(*this));

    std::string data = serialize();
    if (data.empty()) {
        return;
    }
    ModalFormRequestPacket packet(id, data);
    player->sendNetworkPacket(packet);
}

//////////////////////////////// Custom Form ////////////////////////////////
std::string CustomFormElement::getString() { return value; }

int CustomFormElement::getNumber() { return getInt(); }

int CustomFormElement::getInt() {
    try {
        return stoi(value);
    } catch (...) {
        return 0;
    }
}

float CustomFormElement::getFloat() {
    try {
        return stof(value);
    } catch (...) {
        return 0;
    }
}

double CustomFormElement::getDouble() {
    try {
        return stod(value);
    } catch (...) {
        return 0;
    }
}

bool CustomFormElement::getBool() {
    if (value.empty() || value == "0" || value == "false" || value == "False" || value == "FALSE") return false;
    return true;
}

std::string Label::serialize() {
    try {
        nlohmann::json itemAdd;
        itemAdd["type"] = "label";
        itemAdd["text"] = text;
        return itemAdd.dump();
    } catch (const nlohmann::json::exception&) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to generate Label in Custom Form serialize!"
        );
        return "";
    }
}

std::string Input::serialize() {
    try {
        nlohmann::json itemAdd;
        itemAdd["type"] = "input";
        itemAdd["text"] = title;
        if (!placeholder.empty()) itemAdd["placeholder"] = placeholder;
        if (!def.empty()) itemAdd["default"] = def;
        return itemAdd.dump();
    } catch (const nlohmann::json::exception&) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to generate Input in Custom Form serialize!"
        );
        return "";
    }
}

std::string Toggle::serialize() {
    try {
        nlohmann::json itemAdd;
        itemAdd["type"] = "toggle";
        itemAdd["text"] = title;
        if (def) itemAdd["default"] = def;
        return itemAdd.dump();
    } catch (const nlohmann::json::exception&) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to generate Toggle in Custom Form serialize!"
        );
        return "";
    }
}

std::string Dropdown::serialize() {
    try {
        nlohmann::json itemAdd;
        itemAdd["type"] = "dropdown";
        itemAdd["text"] = title;

        nlohmann::json items = nlohmann::json::array();
        for (auto& str : options) items.push_back(str);
        itemAdd["options"] = items;

        if (def > 0) itemAdd["default"] = def;
        return itemAdd.dump();
    } catch (const nlohmann::json::exception&) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to generate Dropdown in Custom Form serialize!"
        );
        return "";
    }
}

std::string Slider::serialize() {
    try {
        nlohmann::json itemAdd;
        itemAdd["type"] = "slider";
        itemAdd["text"] = title;

        if (minValue > maxValue) {
            const auto t = maxValue;
            maxValue     = minValue;
            minValue     = t;
        }
        itemAdd["min"]     = minValue;
        itemAdd["max"]     = maxValue;
        itemAdd["step"]    = step > 0 ? step : maxValue - minValue;
        itemAdd["default"] = std::max(std::min(def, maxValue), minValue);

        return itemAdd.dump();
    } catch (const nlohmann::json::exception&) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to generate Slider in Custom Form serialize!"
        );
        return "";
    }
}

std::string StepSlider::serialize() {
    try {
        nlohmann::json itemAdd;
        itemAdd["type"] = "step_slider";
        itemAdd["text"] = title;

        nlohmann::json items = nlohmann::json::array();
        for (auto& str : options) items.push_back(str);
        itemAdd["steps"] = items;

        size_t maxIndex = items.size() - 1;
        if (def >= 0 && def <= maxIndex) {
            itemAdd["default"] = def;
        }
        return itemAdd.dump();
    } catch (const nlohmann::json::exception&) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to generate StepSlider in Custom Form serialize!"
        );
        return "";
    }
}

CustomForm& CustomForm::setTitle(const std::string& newTitle) {
    this->title = newTitle;
    return *this;
}

CustomForm& CustomForm::addLabel(const std::string& name, std::string text) { return append(Label(name, text)); }

CustomForm&
CustomForm::addInput(const std::string& name, std::string iTitle, std::string placeholder, std::string def) {
    return append(Input(name, iTitle, placeholder, def));
}

CustomForm& CustomForm::addToggle(const std::string& name, std::string tTitle, bool def) {
    return append(Toggle(name, tTitle, def));
}

CustomForm& CustomForm::addDropdown(
    const std::string&              name,
    std::string                     dTitle,
    const std::vector<std::string>& options,
    int                             defId
) {
    return append(Dropdown(name, dTitle, options, defId));
}

CustomForm&
CustomForm::addSlider(const std::string& name, std::string sTitle, double min, double max, double step, double def) {
    return append(Slider(name, sTitle, min, max, step, def));
}

CustomForm& CustomForm::addStepSlider(
    const std::string&              name,
    std::string                     sTitle,
    const std::vector<std::string>& options,
    int                             defId
) {
    return append(StepSlider(name, sTitle, options, defId));
}

CustomForm& CustomForm::append(const Label& element) {
    elements.emplace_back(element.name, std::make_shared<Label>(element));
    return *this;
}

CustomForm& CustomForm::append(const Input& element) {
    elements.emplace_back(element.name, std::make_shared<Input>(element));
    return *this;
}

CustomForm& CustomForm::append(const Toggle& element) {
    elements.emplace_back(element.name, std::make_shared<Toggle>(element));
    return *this;
}

CustomForm& CustomForm::append(const Dropdown& element) {
    elements.emplace_back(element.name, std::make_shared<Dropdown>(element));
    return *this;
}

CustomForm& CustomForm::append(const Slider& element) {
    elements.emplace_back(element.name, std::make_shared<Slider>(element));
    return *this;
}

CustomForm& CustomForm::append(const StepSlider& element) {
    elements.emplace_back(element.name, std::make_shared<StepSlider>(element));
    return *this;
}

std::string CustomForm::serialize() {
    try {
        nlohmann::json form =
            nlohmann::json::parse(R"({ "title":"", "type":"custom_form", "content":[], "buttons":[] })");
        form["title"] = title;
        for (auto& [k, v] : elements) {
            std::string element = v->serialize();
            if (!element.empty()) form["content"].push_back(nlohmann::json::parse(element));
        }
        return form.dump();
    } catch (const nlohmann::json::exception&) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to generate Custom Form in serialize!"
        );
        return "";
    }
}

void CustomForm::sendTo(Player* player, Callback cb) {
    unsigned id    = NewFormId();
    this->callback = std::move(cb);
    SetCustomFormBuilderData(id, std::make_shared<CustomForm>(*this));

    std::string data = serialize();
    if (data.empty()) {
        return;
    }

    ModalFormRequestPacket packet(id, data);
    player->sendNetworkPacket(packet);
}

void CustomForm::sendToForRawJson(Player* player, Callback2 cb) {
    unsigned id = NewFormId();
    // this->callback = callback;
    this->callback = nullptr;
    SetCustomFormPacketCallback(id, cb);

    std::string data = serialize();
    if (data.empty()) {
        return;
    }

    ModalFormRequestPacket packet(id, data);
    player->sendNetworkPacket(packet);
}

std::string CustomForm::getString(const std::string& name) {
    const auto element = getElement(name);
    return element != nullptr ? element->getString() : "";
}

int CustomForm::getNumber(const std::string& name) { return getInt(name); }

int CustomForm::getInt(const std::string& name) {
    const auto element = getElement(name);
    return element != nullptr ? element->getInt() : 0;
}

float CustomForm::getFloat(const std::string& name) {
    const auto element = getElement(name);
    return element != nullptr ? element->getFloat() : 0;
}

double CustomForm::getDouble(const std::string& name) {
    const auto element = getElement(name);
    return element != nullptr ? element->getDouble() : 0;
}

bool CustomForm::getBool(const std::string& name) {
    const auto element = getElement(name);
    return element != nullptr ? element->getBool() : false;
}

std::string CustomForm::getString(int index) {
    const auto element = getElement(index);
    return element != nullptr ? element->getString() : "";
}

int CustomForm::getNumber(int index) { return getInt(index); }

int CustomForm::getInt(int index) {
    const auto element = getElement(index);
    return element != nullptr ? element->getInt() : 0;
}

float CustomForm::getFloat(int index) {
    const auto element = getElement(index);
    return element != nullptr ? element->getFloat() : 0;
}

double CustomForm::getDouble(int index) {
    const auto element = getElement(index);
    return element != nullptr ? element->getDouble() : 0;
}

bool CustomForm::getBool(int index) {
    const auto element = getElement(index);
    return element != nullptr ? element->getBool() : false;
}

CustomFormElement* CustomForm::getElement(const std::string& name) {
    for (auto& [k, v] : elements)
        if (k == name) return v.get();
    return nullptr;
}

CustomFormElement* CustomForm::getElement(int index) {
    return elements.size() > index ? elements[index].second.get() : nullptr;
}

CustomFormElement::Type CustomForm::getType(int index) { return elements[index].second->getType(); }

} // namespace lse::form
