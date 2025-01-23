#include "FormPacketHelper.h"

#include "FormUI.h"
#include "ll/api/form/FormBase.h"
#include "ll/api/form/FormIdManager.h"
#include "ll/api/memory/Hook.h"
#include "mc/network/PacketHandlerDispatcherInstance.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/packet/ModalFormResponsePacket.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/player/Player.h"

#include <nlohmann/json.hpp>

namespace lse::form {

//////////////////////////////// Data ////////////////////////////////

enum class FormType {
    SimpleFormBuilder,
    ModalFormBuilder,
    CustomFormBuilder,
    SimpleFormPacket,
    ModalFormPacket,
    CustomFormPacket
};

std::unordered_map<unsigned, FormType> formTypes;

std::unordered_map<unsigned, std::function<void(Player*, int, FormCancelReason reason)>>  simpleFormPacketCallbacks;
std::unordered_map<unsigned, std::function<void(Player*, bool, FormCancelReason reason)>> modalFormPacketCallbacks;
std::unordered_map<unsigned, std::function<void(Player*, std::string, FormCancelReason reason)>>
    customFormPacketCallbacks;

std::unordered_map<unsigned, std::shared_ptr<SimpleForm>> simpleFormBuilders;
std::unordered_map<unsigned, std::shared_ptr<ModalForm>>  modalFormBuilders;
std::unordered_map<unsigned, std::shared_ptr<CustomForm>> customFormBuilders;

//////////////////////////////// Functions ////////////////////////////////

unsigned NewFormId() {
    unsigned formId;
    do {
        formId = ll::form::FormIdManager::genFormId();
    } while (formTypes.find(formId) != formTypes.end());
    return formId;
}

void SetSimpleFormPacketCallback(unsigned formId, std::function<void(Player*, int, FormCancelReason reason)> callback) {
    formTypes[formId]                 = FormType::SimpleFormPacket;
    simpleFormPacketCallbacks[formId] = callback;
}

void SetModalFormPacketCallback(unsigned formId, std::function<void(Player*, bool, FormCancelReason reason)> callback) {
    formTypes[formId]                = FormType::ModalFormPacket;
    modalFormPacketCallbacks[formId] = callback;
}

void SetCustomFormPacketCallback(
    unsigned                                                           formId,
    std::function<void(Player*, std::string, FormCancelReason reason)> callback
) {
    formTypes[formId]                 = FormType::CustomFormPacket;
    customFormPacketCallbacks[formId] = callback;
}

void SetSimpleFormBuilderData(unsigned formId, std::shared_ptr<SimpleForm> data) {
    formTypes[formId]          = FormType::SimpleFormBuilder;
    simpleFormBuilders[formId] = data;
}

void SetModalFormBuilderData(unsigned formId, std::shared_ptr<ModalForm> data) {
    formTypes[formId]         = FormType::ModalFormBuilder;
    modalFormBuilders[formId] = data;
}

void SetCustomFormBuilderData(unsigned formId, std::shared_ptr<CustomForm> data) {
    formTypes[formId]          = FormType::CustomFormBuilder;
    customFormBuilders[formId] = data;
}

void HandleFormPacket(Player* player, unsigned formId, const std::string& data, FormCancelReason reason) {
    if (formTypes.find(formId) == formTypes.end()) return;

    if (formTypes[formId] == FormType::SimpleFormBuilder) {
        int chosen = data != "null" ? stoi(data) : -1;

        // Simple Form Builder
        auto form = simpleFormBuilders[formId];
        if (form->callback) form->callback(player, chosen, reason);
        // Button Callback
        if (chosen >= 0) {
            if (chosen >= form->elements.size()) return;
            auto button = dynamic_pointer_cast<Button>(form->elements[chosen]);
            if (button->callback) button->callback(player, reason);
        }
        simpleFormBuilders.erase(formId);
    } else if (formTypes[formId] == FormType::ModalFormBuilder) {
        int chosen = data == "true" ? 1 : 0;

        // Modal Form Builder
        auto form = modalFormBuilders[formId];
        if (form->callback) form->callback(player, chosen, reason);
        modalFormBuilders.erase(formId);
    } else if (formTypes[formId] == FormType::CustomFormBuilder) {
        // Custom Form Builder
        auto form = customFormBuilders[formId];

        if (data == "null") {
            customFormBuilders.erase(formId);
            if (form->callback) form->callback(player, {}, reason);
            return;
        }

        nlohmann::json res      = nlohmann::json::parse(data);
        int            nowIndex = 0;
        for (nlohmann::json& j : res) {
            switch (form->getType(nowIndex)) {
            case CustomFormElement::Type::Label: // label's data is null
                break;
            case CustomFormElement::Type::Input:
                form->setValue(nowIndex, j.get<std::string>());
                break;
            case CustomFormElement::Type::Toggle:
                form->setValue(nowIndex, j.get<bool>());
                break;
            case CustomFormElement::Type::Slider:
                form->setValue(nowIndex, j.get<double>());
                break;
            case CustomFormElement::Type::Dropdown: {
                auto& options = dynamic_pointer_cast<Dropdown>(form->elements[nowIndex].second)->options;
                form->setValue(nowIndex, options[j.get<int>()]);
                break;
            }
            case CustomFormElement::Type::StepSlider: {
                auto& options = dynamic_pointer_cast<StepSlider>(form->elements[nowIndex].second)->options;
                form->setValue(nowIndex, options[j.get<int>()]);
                break;
            }
            default:
                break;
            }
            ++nowIndex;
        }

        if (form->callback) {
            std::map<std::string, std::shared_ptr<CustomFormElement>> callbackData;
            for (auto& [k, v] : form->elements) callbackData[k] = v;

            form->callback(player, callbackData, reason);
        }

        customFormBuilders.erase(formId);
    } else if (formTypes[formId] == FormType::SimpleFormPacket) {
        int chosen = data != "null" ? stoi(data) : -1;
        simpleFormPacketCallbacks[formId](player, chosen, reason);
        simpleFormPacketCallbacks.erase(formId);
    } else if (formTypes[formId] == FormType::CustomFormPacket) {
        customFormPacketCallbacks[formId](player, data, reason);
        customFormPacketCallbacks.erase(formId);
    } else if (formTypes[formId] == FormType::ModalFormPacket) {
        int chosen = data == "true" ? 1 : 0;
        modalFormPacketCallbacks[formId](player, chosen, reason);
        modalFormPacketCallbacks.erase(formId);
    }
    formTypes.erase(formId);
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    FormResponseHandler,
    HookPriority::Highest,
    PacketHandlerDispatcherInstance<ModalFormResponsePacket>,
    &PacketHandlerDispatcherInstance<ModalFormResponsePacket>::handle,
    void,
    NetworkIdentifier const& source,
    NetEventCallback&        callback,
    std::shared_ptr<Packet>& packet
) {
    auto& handle = ll::memory::dAccess<ServerNetworkHandler>(&callback, -16);
    if (auto player = handle._getServerPlayer(source, SubClientId::PrimaryClient); player) {
        auto& modalPacket = (ModalFormResponsePacket&)*packet;

        auto data = std::string{"null"};

        if (!modalPacket.mFormCancelReason && modalPacket.mJSONResponse) {
            data = modalPacket.mJSONResponse->toStyledString();
            if (data.ends_with('\n')) {
                data.pop_back();
                if (data.ends_with('\r')) {
                    data.pop_back();
                }
            }
        }

        HandleFormPacket(player, modalPacket.mFormId, data, modalPacket.mFormCancelReason);
    }
    origin(source, callback, packet);
}
} // namespace lse::form
