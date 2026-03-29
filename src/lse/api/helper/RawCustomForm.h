#pragma once

#include "ll/api/form/CustomForm.h"
#include "ll/api/form/FormBase.h"

#include <string>

namespace lse::form {

using CustomFormResult = std::optional<std::string>;

class RawCustomForm : public ll::form::CustomForm {

public:
    using RawCallback = RawFormCallback;

    RawCustomForm& sendRawTo(Player& player, RawCallback&& callback = {}) {
        Form::sendRawTo(player, getFormData(), std::move(callback));
        return *this;
    }

    RawCustomForm& sendRawUpdate(Player& player, RawCallback&& callback = {}) {
        Form::sendRawUpdate(player, getFormData(), std::move(callback));
        return *this;
    }
};
} // namespace lse::form
