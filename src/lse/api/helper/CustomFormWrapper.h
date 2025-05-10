#pragma once

#include "ll/api/form/CustomForm.h"
#include "ll/api/form/FormBase.h"
#include "nlohmann/json_fwd.hpp"

namespace lse::form {

using CustomFormResult = std::optional<nlohmann::ordered_json>;

class CustomFormWrapper {
    ll::form::CustomForm form;
    std::vector<int>     resultIndices{};
    size_t               resultIndex = 0;

public:
    using Callback = std::function<void(Player&, CustomFormResult const&, ll::form::FormCancelReason)>;

    [[nodiscard]] CustomFormWrapper() : form() {};

    [[nodiscard]] explicit CustomFormWrapper(std::string const& title) : form(title) {};

    ~CustomFormWrapper() = default;

    inline CustomFormWrapper& setTitle(std::string const& title) {
        form.setTitle(title);
        return *this;
    };

    inline CustomFormWrapper& setSubmitButton(std::string const& text) {
        form.setSubmitButton(text);
        return *this;
    };

    inline CustomFormWrapper& appendHeader(std::string const& text) {
        form.appendHeader(text);
        resultIndices.emplace_back(-1);
        return *this;
    };

    inline CustomFormWrapper& appendLabel(std::string const& text) {
        form.appendLabel(text);
        resultIndices.emplace_back(-1);
        return *this;
    };

    inline CustomFormWrapper& appendDivider() {
        form.appendDivider();
        resultIndices.emplace_back(-1);
        return *this;
    };

    inline CustomFormWrapper&
    appendInput(std::string const& text, std::string const& placeholder = {}, std::string const& defaultVal = {}) {
        form.appendInput(std::to_string(resultIndex), text, placeholder, defaultVal);
        resultIndices.emplace_back(resultIndex++);
        return *this;
    };

    inline CustomFormWrapper& appendToggle(std::string const& text, bool defaultVal = false) {
        form.appendToggle(std::to_string(resultIndex), text, defaultVal);
        resultIndices.emplace_back(resultIndex++);
        return *this;
    };

    inline CustomFormWrapper&
    appendDropdown(std::string const& text, std::vector<std::string> const& options, size_t defaultVal = 0) {
        form.appendDropdown(std::to_string(resultIndex), text, options, defaultVal);
        resultIndices.emplace_back(resultIndex++);
        return *this;
    };

    inline CustomFormWrapper&
    appendSlider(std::string const& text, double min, double max, double step = 0.0, double defaultVal = 0.0) {
        form.appendSlider(std::to_string(resultIndex), text, min, max, step, defaultVal);
        resultIndices.emplace_back(resultIndex++);
        return *this;
    };

    inline CustomFormWrapper&
    appendStepSlider(std::string const& text, std::vector<std::string> const& steps, size_t defaultVal = 0) {
        form.appendStepSlider(std::to_string(resultIndex), text, steps, defaultVal);
        resultIndices.emplace_back(resultIndex++);
        return *this;
    };

    static CustomFormResult
    convertResult(std::optional<std::string> const& result, std::vector<int> const& resultIndices);

    static CustomFormResult
    convertResult(std::optional<std::string> const& result, nlohmann::ordered_json const& formData);

    static ll::form::Form::RawFormCallback convertCallback(Callback&& callback, std::vector<int> resultIndices);

    inline CustomFormWrapper& sendTo(Player& player, Callback&& callback = {}) {
        ll::form::Form::sendRawTo(player, form.getFormData(), convertCallback(std::move(callback), resultIndices));
        return *this;
    };

    inline CustomFormWrapper& sendUpdate(Player& player, Callback&& callback = {}) {
        ll::form::Form::sendRawUpdate(player, form.getFormData(), convertCallback(std::move(callback), resultIndices));
        return *this;
    };
};
} // namespace lse::form