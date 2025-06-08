#pragma once

#include "ll/api/form/CustomForm.h"
#include "ll/api/form/FormBase.h"

#include <string>

namespace lse::form {

using CustomFormResult = std::optional<std::string>;

class RawCustomForm : public ::ll::form::CustomForm {

public:
    using RawCallback = ll::form::Form::RawFormCallback;

    inline RawCustomForm& sendRawTo(Player& player, RawCallback&& callback = {}) {
        ll::form::Form::sendRawTo(player, getFormData(), std::move(callback));
        return *this;
    };

    inline RawCustomForm& sendRawUpdate(Player& player, RawCallback&& callback = {}) {
        ll::form::Form::sendRawUpdate(player, getFormData(), std::move(callback));
        return *this;
    };
};
} // namespace lse::form