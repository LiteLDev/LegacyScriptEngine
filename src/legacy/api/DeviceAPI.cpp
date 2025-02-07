#include "api/DeviceAPI.h"

#include "api/APIHelp.h"
#include "ll/api/service/Bedrock.h"
#include "magic_enum.hpp"
#include "mc/certificates/WebToken.h"
#include "mc/common/ActorRuntimeID.h"
#include "mc/deps/input/InputMode.h"
#include "mc/deps/json/Value.h"
#include "mc/network/ConnectionRequest.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

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
Local<Object> DeviceClass::newDevice(Player* player) {
    auto newp = new DeviceClass(player);
    return newp->getScriptObject();
}

// 成员函数
void DeviceClass::setPlayer(Player* player) {
    try {
        if (player) {
            mWeakEntity = player->getWeakEntity();
            mValid      = true;
        }
    } catch (...) {
        mValid = false;
    }
}

Player* DeviceClass::getPlayer() {
    if (mValid) {
        return mWeakEntity.tryUnwrap<Player>().as_ptr();
    } else {
        return nullptr;
    }
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
        if (!player) return Local<Value>();

        return Number::newNumber(player->getNetworkStatus()->mCurrentPing);
    }
    CATCH("Fail in getLastPing!")
}

Local<Value> DeviceClass::getLastPacketLoss() {
    try {
        Player* player = getPlayer();
        if (!player) return Local<Value>();

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
        if (!player) return Local<Value>();

        if (player->isSimulatedPlayer()) String::newString("unknown");
        Json::Value& requestJson = player->getConnectionRequest()->mRawToken->mDataInfo;
        return String::newString(requestJson["ServerAddress"].asString("unknown"));
    }
    CATCH("Fail in getServerAddress!")
}

Local<Value> DeviceClass::getClientId() {
    try {
        Player* player = getPlayer();
        if (!player) return Local<Value>();

        return String::newString(player->getConnectionRequest()->getDeviceId());
    }
    CATCH("Fail in getClientId!")
}

Local<Value> DeviceClass::getInputMode() {
    try {
        Player* player = getPlayer();
        if (!player) return Local<Value>();

        Json::Value& requestJson = player->getConnectionRequest()->mRawToken->mDataInfo;
        return Number::newNumber(requestJson["CurrentInputMode"].asInt(0));
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
