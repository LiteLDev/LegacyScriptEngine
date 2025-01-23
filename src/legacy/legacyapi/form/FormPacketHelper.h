#pragma once

#include "FormUI.h"

#include <string>

class Player;

namespace lse::form {
unsigned NewFormId();

void SetSimpleFormPacketCallback(unsigned formId, std::function<void(Player*, int, FormCancelReason reason)> callback);

void SetModalFormPacketCallback(unsigned formId, std::function<void(Player*, bool, FormCancelReason reason)> callback);

void SetCustomFormPacketCallback(
    unsigned                                                           formId,
    std::function<void(Player*, std::string, FormCancelReason reason)> callback
);

void SetSimpleFormBuilderData(unsigned formId, std::shared_ptr<SimpleForm> data);

void SetModalFormBuilderData(unsigned formId, std::shared_ptr<ModalForm> data);

void SetCustomFormBuilderData(unsigned formId, std::shared_ptr<CustomForm> data);

void HandleFormPacket(Player* player, unsigned formId, const std::string& data, FormCancelReason reason);
} // namespace lse::form
