#include "api/DeviceAPI.h"

#include "api/APIHelp.h"
#include "ll/api/service/Bedrock.h"
#include "magic_enum.hpp"
#include "mc/certificates/WebToken.h"
#include "mc/enums/InputMode.h"
#include "mc/network/ConnectionRequest.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

#include <mc/deps/json/Value.h>
#include <string>

//////////////////// Class Definition ////////////////////
ClassDefine<void> InputModeStaticBuilder = EnumDefineBuilder<InputMode>::build("InputMode");

ClassDefine<DeviceClass> DeviceClassBuilder = defineClass<DeviceClass>("LLSE_Device")
                                                  .constructor(nullptr)
                                                  .instanceProperty("ip", &DeviceClass::getIP)
                                                  .instanceProperty("avgPing", &DeviceClass::getAvgPing)
                                                  .instanceProperty("avgPacketLoss", &DeviceClass::getAvgPacketLoss)
                                                  .instanceProperty("lastPing", &DeviceClass::getLastPing)
                                                  .instanceProperty("lastPacketLoss", &DeviceClass::getLastPacketLoss)
                                                  .instanceProperty("os", &DeviceClass::getOs)
                                                  .instanceProperty("inputMode", &DeviceClass::getInputMode)
                                                  //   .instanceProperty("playMode", &DeviceClass::getPlayMode)
                                                  .instanceProperty("serverAddress", &DeviceClass::getServerAddress)
                                                  .instanceProperty("clientId", &DeviceClass::getClientId)
                                                  .build();

//////////////////// Classes ////////////////////

// 生成函数
Local<Object> DeviceClass::newDevice(Player* p) {
    auto newp = new DeviceClass(p);
    return newp->getScriptObject();
}

// 成员函数
void DeviceClass::setPlayer(Player* player) {
    try {
        id = player->getOrCreateUniqueID();
    } catch (...) {
        isValid = false;
    }
}

Player* DeviceClass::getPlayer() {
    if (!isValid) return nullptr;
    else return ll::service::getLevel()->getPlayer(id);
}

Local<Value> DeviceClass::getIP() {
    try {
        Player* player = getPlayer();
        if (!player) return Local<Value>();

        return String::newString(player->getNetworkIdentifier().getIPAndPort());
    }
    CATCH("Fail in GetIP!")
}

Local<Value> DeviceClass::getAvgPing() {
    try {
        Player* player = getPlayer();
        if (!player) return Local<Value>();

        return Number::newNumber(player->getNetworkStatus()->mAveragePing);
    }
    CATCH("Fail in getAvgPing!")
}

Local<Value> DeviceClass::getAvgPacketLoss() {
    try {
        Player* player = getPlayer();
        if (!player) return Local<Value>();

        return Number::newNumber(player->getNetworkStatus()->mAveragePacketLoss);
    }
    CATCH("Fail in getAvgPacketLoss!")
}

Local<Value> DeviceClass::getLastPing() {
    try {
        Player* player = getPlayer();
        if (!player) {
            return Local<Value>();
        }

        return Number::newNumber(player->getNetworkStatus()->mCurrentPing);
    }
    CATCH("Fail in getLastPing!")
}

Local<Value> DeviceClass::getLastPacketLoss() {
    try {
        Player* player = getPlayer();
        if (!player) {
            return Local<Value>();
        }

        return Number::newNumber(player->getNetworkStatus()->mCurrentPacketLoss);
    }
    CATCH("Fail in getLastPacketLoss!")
}

Local<Value> DeviceClass::getOs() {
    try {
        Player* player = getPlayer();
        if (!player) return Local<Value>();

        return String::newString(magic_enum::enum_name(player->getPlatform()));
    }
    CATCH("Fail in getOs!")
}

Local<Value> DeviceClass::getServerAddress() {
    try {
        Player* player = getPlayer();
        if (!player) {
            return Local<Value>();
        }
        if (player->isSimulatedPlayer()) String::newString("unknown");
        Json::Value& requestJson = ll::service::getServerNetworkHandler()
                                       ->fetchConnectionRequest(player->getNetworkIdentifier())
                                       .mRawToken->mDataInfo;
        return String::newString(requestJson.get("ServerAddress", "unknown").asString("unknown"));
    }
    CATCH("Fail in getServerAddress!")
}

Local<Value> DeviceClass::getClientId() {
    try {
        Player* player = getPlayer();
        if (!player) return Local<Value>();

        return String::newString(
            ll::service::getServerNetworkHandler()->fetchConnectionRequest(player->getNetworkIdentifier()).getDeviceId()
        );
    }
    CATCH("Fail in getClientId!")
}

Local<Value> DeviceClass::getInputMode() {
    try {
        Player* player = getPlayer();
        if (!player) return Local<Value>();

        Json::Value& requestJson = ll::service::getServerNetworkHandler()
                                       ->fetchConnectionRequest(player->getNetworkIdentifier())
                                       .mRawToken->mDataInfo;
        return Number::newNumber(requestJson.get("CurrentInputMode", 0).asInt(0));
    }
    CATCH("Fail in getInputMode!")
}

// Local<Value> DeviceClass::getPlayMode() {
//     try {
//         Player* player = getPlayer();
//         if (!player) return Local<Value>();

//         return Number::newNumber(0);
//     }
//     CATCH("Fail in getPlayMode!")
// }
