#pragma once

#include <span> // For fixing compile
#include "FormUI.h"
#include "mc/world/actor/player/Player.h"

#include <string>

namespace lse::form {
unsigned NewFormId();

void SetSimpleFormPacketCallback(unsigned formId, std::function<void(Player*, int)> callback);

void SetModalFormPacketCallback(unsigned formId, std::function<void(Player*, bool)> callback);

void SetCustomFormPacketCallback(unsigned formId, std::function<void(Player*, std::string)> callback);

void SetSimpleFormBuilderData(unsigned formId, std::shared_ptr<SimpleForm> data);

void SetModalFormBuilderData(unsigned formId, std::shared_ptr<ModalForm> data);

void SetCustomFormBuilderData(unsigned formId, std::shared_ptr<CustomForm> data);

void HandleFormPacket(Player* player, unsigned formId, const std::string& data);
} // namespace lse::form
