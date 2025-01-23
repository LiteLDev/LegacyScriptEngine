#pragma once
////////////////////////////////////////////////////////////////////////
//  Form UI Utility - Help you to build forms and options more easily
//
//  [Example - Simple Form]
//  - Form that contains several buttons (with optional image)
//  - Let the player choose an option from multiple options
//
//  SimpleForm form("Welcome to shop", "Choose what you want to do...");     // Initialize the form with title and
//  content form.addButton("Buy", "textures/items/apple",                            // Add a button "Buy" with texture
//  image
//      [](Player* pl) { pl->sendText("To buy something..."); })             // Buy's callback function
//
//      .addButton("Sell", "https://xxx.com/xxx.png",                        // Add a button "Sell" with online image
//          [](Player* pl) { pl->sendText("To sell something..."); })        // Sell's callback function
//
//      .addButton("Settings", "",                                           // Add a button "Settings" with no image
//          [](Player* pl) { pl->sendText("Get into settings..."); })        // Settings's callback function
//
//      .addButton("Exit")                                                   // Add a single button "Exit"
//      .sendTo(Level::getPlayer("John"));                                   // Send the form to a player called "John"
//
//
//  [Example - Modal Form]
//  - Form with Confirm and Cancel buttons
//  - Let the player confirm or cancel an action
//
//  ModalForm form("Confirm the action", "Do you want that?", "Yes", "Nope");     // Initialize the form with title,
//  content and two buttons ("Yes", "Nope") form.sendTo(Level::getPlayer("S3v3N1ce"), // Send the form to a player
//  called "S3v3N1ce"
//      [](Player* player, bool isConfirm)                                        // Callback function to process the
//      result
//          {
//              if (isConfirm)                                                    // Player pressed button "Yes"
//                  player->sendText("Okay, let's go");
//              else                                                              // Player pressed button "Nope"
//                  player->sendText("Hm, suit yourself");
//          });
//
//
//  [Example - Custom Form]
//  - Form that contains some kinds of elements (like input line, toggle, dropdown, ....)
//  - Let the player provide some detailed information
//
//  CustomForm form2("Information Collection Form");                               // Initialize the form with title
//  form2.addLabel("label1", "Personal Information")                               // Add a label shows "Personal
//  Information"
//      .addInput("username", "Your Name")                                         // Add an input line to gather
//      player's name .addDropdown("sex", "Your Sex", { "Male","Female","Secret" })              // Add a dropdown to
//      gather player's sex .addSlider("age", "Your Age", 3, 100)                                      // Add a slider
//      to gather player's age
//
//      .addLabel("label2", "MC Information")                                      // Add a label shows "MC Information"
//      .addToggle("licensed", "Purchased a licensed Minecraft?", true)            // Add a toggle about whether he buys
//      a licensed mc or not .addStepSlider("skill", "Skill Lvl", { "Beginner", "Amateur", "Pro" })     // Add a step
//      slider shows his game skill level
//
//      .sendTo(Level::getPlayer("yqs112358"),                                     // Send the form to a player called
//      "yqs112358"
//          [](Player* player, auto result)                                        // Callback function to process the
//          result
//          {
//              if (result.empty())                                                // Player cancelled the form
//                  return;
//              player->sendText("You have commited the form.");
//              player->sendFormattedText("Your name: {}", result["username"]->getString());
//              player->sendFormattedText("Your sex: {}", result["sex"]->getString());
//              player->sendFormattedText("Your age: {}", result["age"]->getNumber());
//              player->sendFormattedText("Your license: {}", result["licensed"]->getBool() ? "yes" : "no");
//              player->sendFormattedText("Your skill level: {}:", result["skill"]->getString());
//          });
//
//
// Tips:  The <key> of std::map "result" equals the first argument "name" you pass to these elements
//         So, "name" must be *unique* or error will occur
//
////////////////////////////////////////////////////////////////////////

#include "mc/network/packet/ModalFormCancelReason.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>

class ServerPlayer;
class Player;

namespace lse::form {

using FormCancelReason = std::optional<ModalFormCancelReason>;

//////////////////////////////// Simple Form Elements ////////////////////////////////
class SimpleFormElement {
protected:
    virtual std::string serialize() = 0;
    virtual ~SimpleFormElement()    = default;
    friend class SimpleForm;
};

class Button : public SimpleFormElement {
protected:
    std::string serialize() override;

public:
    using ButtonCallback = std::function<void(Player*, FormCancelReason reason)>;
    std::string    text, image;
    ButtonCallback callback;

public:
    inline explicit Button(std::string text, std::string image = "", ButtonCallback callback = ButtonCallback())
    : text(std::move(text)),
      image(std::move(image)),
      callback(std::move(callback)) {}
    inline void setText(const std::string& _text) { this->text = _text; }
    inline void setImage(const std::string& _image) { this->image = _image; }
    inline void setCallback(ButtonCallback _callback) { this->callback = std::move(_callback); }
};

//////////////////////////////// Custom Form Elements ////////////////////////////////
class CustomFormElement {
protected:
    virtual std::string serialize() = 0;
    friend class CustomForm;
    virtual ~CustomFormElement() = default;

public:
    enum class Type { Label, Input, Toggle, Dropdown, Slider, StepSlider };
    std::string                             name;
    std::string                             value;
    Type                                    type{};
    inline void                             setName(const std::string& _name) { this->name = _name; }
    inline virtual Type                     getType() = 0;
    std::string                             getString();
    [[deprecated("Please use getInt")]] int getNumber();
    int                                     getInt();
    float                                   getFloat();
    double                                  getDouble();
    bool                                    getBool();
};

class Label : public CustomFormElement {
protected:
    std::string serialize() override;

public:
    std::string text;

public:
    inline Label(const std::string& name, std::string text) : text(std::move(text)) { setName(name); }
    inline Type getType() override { return Type::Label; }
    inline void setText(const std::string& _text) { this->text = _text; }
};

class Input : public CustomFormElement {
protected:
    std::string serialize() override;

public:
    std::string title, placeholder, def;
    inline Input(const std::string& name, std::string title, std::string placeholder = "", std::string def = "")
    : title(std::move(title)),
      placeholder(std::move(placeholder)),
      def(std::move(def)) {
        setName(name);
    }
    inline Type getType() override { return Type::Input; }
    inline void setTitle(const std::string& _title) { this->title = _title; }
    inline void setPlaceHolder(const std::string& _placeholder) { this->placeholder = _placeholder; }
    inline void setDefault(const std::string& _def) { this->def = _def; }
};

class Toggle : public CustomFormElement {
protected:
    std::string serialize() override;

public:
public:
    std::string title;
    bool        def;

public:
    inline Toggle(const std::string& name, std::string title, bool def = false) : title(std::move(title)), def(def) {
        setName(name);
    }
    inline virtual Type getType() override { return Type::Toggle; }
    inline void         setTitle(const std::string& _title) { this->title = _title; }
    inline void         setDefault(bool _def) { this->def = _def; }
};

class Dropdown : public CustomFormElement {
protected:
    std::string serialize() override;

public:
    std::string              title;
    std::vector<std::string> options;
    int                      def;

public:
    inline Dropdown(const std::string& name, std::string title, const std::vector<std::string>& options, int defId = 0)
    : title(std::move(title)),
      options(options),
      def(defId) {
        setName(name);
    }
    inline Type getType() override { return Type::Dropdown; }
    inline void setTitle(const std::string& _title) { this->title = _title; }
    inline void setOptions(const std::vector<std::string>& _options) { this->options = _options; }
    inline void addOption(const std::string& option) { options.push_back(option); }
    inline void setDefault(int defId) { this->def = defId; }
};

class Slider : public CustomFormElement {
protected:
    std::string serialize() override;

public:
    std::string title;
    double      minValue, maxValue, step, def;

public:
    inline Slider(
        const std::string& name,
        std::string        title,
        double             minValue,
        double             maxValue,
        double             step = 1,
        double             def  = 0
    )
    : title(std::move(title)),
      minValue(minValue),
      maxValue(maxValue),
      step(step),
      def(def) {
        setName(name);
    }
    inline Type getType() override { return Type::Slider; }
    inline void setTitle(const std::string& _title) { this->title = _title; }
    inline void setMin(double _minValue) { this->minValue = _minValue; }
    inline void setMax(double _maxValue) { this->maxValue = _maxValue; }
    inline void setStep(double _step) { this->step = _step; }
    inline void setDefault(double _def) { this->def = _def; }
};

class StepSlider : public CustomFormElement {
protected:
    std::string serialize() override;

public:
    std::string              title;
    std::vector<std::string> options;
    int                      def;

public:
    inline StepSlider(
        const std::string&              name,
        std::string                     title,
        const std::vector<std::string>& options,
        int                             defId = 0
    )
    : title(std::move(title)),
      options(options),
      def(defId) {
        setName(name);
    }
    inline Type getType() override { return Type::StepSlider; }
    inline void setTitle(const std::string& _title) { this->title = _title; }
    inline void setOptions(const std::vector<std::string>& _options) { this->options = _options; }
    inline void addOption(const std::string& option) { options.push_back(option); }
    inline void setDefault(int defId) { this->def = defId; }
};

//////////////////////////////// Forms ////////////////////////////////
class FormImpl {
protected:
    // fifo_json json;
    virtual std::string serialize() = 0;
    virtual ~FormImpl()             = default;
};

class SimpleForm : public FormImpl {
protected:
    std::string serialize() override;

public:
    using Callback = std::function<void(Player*, int, FormCancelReason reason)>;
    std::string                                     title, content;
    std::vector<std::shared_ptr<SimpleFormElement>> elements;
    Callback                                        callback;

public:
    SimpleForm(std::string title, std::string content) : title(std::move(title)), content(std::move(content)) {}
    template <typename T, typename... Args>
    SimpleForm(const std::string& title, const std::string& content, T element, Args... args) {
        append(element);
        SimpleForm(title, content, args...);
    }
    SimpleForm& setTitle(const std::string& title);
    SimpleForm& setContent(const std::string& content);
    SimpleForm&
    addButton(std::string text, std::string image = "", Button::ButtonCallback callback = Button::ButtonCallback());
    SimpleForm& append(const Button& element);
    void        sendTo(Player* player, Callback callback = Callback());
};

class ModalForm : public FormImpl {
protected:
    std::string serialize() override;

public:
    using Callback = std::function<void(Player*, bool, FormCancelReason reason)>;
    std::string title, content, confirmButton, cancelButton;
    Callback    callback;

public:
    ModalForm(std::string title, std::string content, std::string button1, std::string button2)
    : title(std::move(title)),
      content(std::move(content)),
      confirmButton(std::move(button1)),
      cancelButton(std::move(button2)) {}
    template <typename T, typename... Args>
    ModalForm(
        const std::string& title,
        const std::string& content,
        const std::string& confirmButton,
        const std::string& cancelButton,
        Args... args
    ) {
        ModalForm(title, content, confirmButton, cancelButton, args...);
    }
    ModalForm& setTitle(const std::string& title);
    ModalForm& setContent(const std::string& content);
    ModalForm& setConfirmButton(const std::string& text);
    ModalForm& setCancelButton(const std::string& text);
    void       sendTo(Player* player, Callback callback = Callback());
};

class CustomForm : public FormImpl {
protected:
    std::string serialize() override;

public:
    using Callback = std::function<
        void(Player*, std::map<std::string, std::shared_ptr<CustomFormElement>>, FormCancelReason reason)>;
    using Callback2 = std::function<void(Player*, std::string, FormCancelReason reason)>;
    std::string                                                             title;
    std::vector<std::pair<std::string, std::shared_ptr<CustomFormElement>>> elements;
    Callback                                                                callback;

public:
    explicit CustomForm(std::string title) : title(std::move(title)) {}
    template <typename T, typename... Args>
    CustomForm(const std::string& title, T element, Args... args) {
        append(element);
        CustomForm(title, args...);
    }
    CustomForm& setTitle(const std::string& title);

    CustomForm& addLabel(const std::string& name, std::string text);
    CustomForm&
    addInput(const std::string& name, std::string title, std::string placeholder = "", std::string def = "");
    CustomForm& addToggle(const std::string& name, std::string title, bool def = false);
    CustomForm&
    addDropdown(const std::string& name, std::string title, const std::vector<std::string>& options, int defId = 0);
    CustomForm&
    addSlider(const std::string& name, std::string title, double min, double max, double step = 1, double def = 0);
    CustomForm&
    addStepSlider(const std::string& name, std::string title, const std::vector<std::string>& options, int defId = 0);

    CustomForm& append(const Label& element);
    CustomForm& append(const Input& element);
    CustomForm& append(const Toggle& element);
    CustomForm& append(const Dropdown& element);
    CustomForm& append(const Slider& element);
    CustomForm& append(const StepSlider& element);

    void                    sendTo(Player* player, Callback callback);
    void                    sendToForRawJson(Player* player, Callback2 callback);
    CustomFormElement*      getElement(const std::string& name);
    CustomFormElement*      getElement(int index);
    CustomFormElement::Type getType(int index);

    std::string                             getString(const std::string& name);
    [[deprecated("Please use getInt")]] int getNumber(const std::string& name);
    int                                     getInt(const std::string& name);
    float                                   getFloat(const std::string& name);
    double                                  getDouble(const std::string& name);
    bool                                    getBool(const std::string& name);
    std::string                             getString(int index);
    [[deprecated("Please use getInt")]] int getNumber(int index);
    int                                     getInt(int index);
    float                                   getFloat(int index);
    double                                  getDouble(int index);
    bool                                    getBool(int index);

    // Tool Functions
    template <typename T>
    inline void setValue(int index, T value) {
        elements[index].second->value = std::to_string(value);
    }
    inline void setValue(int index, std::string value) { elements[index].second->value = value; }
};
} // namespace lse::form
