#include "api/DeviceAPI.h"

#include "api/APIHelp.h"
#include "ll/api/service/Bedrock.h"
#include "magic_enum.hpp"
#include "mc/certificates/WebToken.h"
#include "mc/deps/input/InputMode.h"
#include "mc/deps/json/Value.h"
#include "mc/legacy/ActorRuntimeID.h"
#include "mc/network/ConnectionRequest.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/world/actor/player/Player.h"

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
DeviceClass::DeviceClass(Player const* player) : ScriptClass(ScriptClass::ConstructFromCpp<DeviceClass>{}) {
    try {
        if (player) {
            mWeakEntity = player->getWeakEntity();
            mValid      = true;
        }
    } catch (...) {}
}

Local<Object> DeviceClass::newDevice(Player const* player) {
    auto newp = new DeviceClass(player);
    return newp->getScriptObject();
}

// 成员函数
Player* DeviceClass::getPlayer() const {
    if (mValid) {
        return mWeakEntity.tryUnwrap<Player>().as_ptr();
    } else {
        return nullptr;
    }
}

Local<Value> DeviceClass::getIP() const {
    try {
        Player* player = getPlayer();
        if (!player) return {};

        return String::newString(player->getNetworkIdentifier().getIPAndPort());
    }
    CATCH_AND_THROW
}

Local<Value> DeviceClass::getAvgPing() const {
    try {
        Player* player = getPlayer();
        if (!player) return {};

        return Number::newNumber(player->getNetworkStatus()->mAveragePing);
    }
    CATCH_AND_THROW
}

Local<Value> DeviceClass::getAvgPacketLoss() const {
    try {
        Player* player = getPlayer();
        if (!player) return {};

        return Number::newNumber(player->getNetworkStatus()->mAveragePacketLoss);
    }
    CATCH_AND_THROW
}

Local<Value> DeviceClass::getLastPing() const {
    try {
        Player* player = getPlayer();
        if (!player) return {};

        return Number::newNumber(player->getNetworkStatus()->mCurrentPing);
    }
    CATCH_AND_THROW
}

Local<Value> DeviceClass::getLastPacketLoss() const {
    try {
        Player* player = getPlayer();
        if (!player) return {};

        return Number::newNumber(player->getNetworkStatus()->mCurrentPacketLoss);
    }
    CATCH_AND_THROW
}

Local<Value> DeviceClass::getOs() const {
    try {
        Player* player = getPlayer();
        if (!player) return {};

        return String::newString(magic_enum::enum_name(player->mBuildPlatform));
    }
    CATCH_AND_THROW
}

Local<Value> DeviceClass::getServerAddress() const {
    try {
        Player* player = getPlayer();
        if (!player) return {};

        if (player->isSimulatedPlayer()) String::newString("unknown");
        Json::Value& requestJson = player->getConnectionRequest()->mRawToken->mDataInfo;
        return String::newString(requestJson["ServerAddress"].asString("unknown"));
    }
    CATCH_AND_THROW
}

Local<Value> DeviceClass::getClientId() const {
    try {
        Player* player = getPlayer();
        if (!player) return {};

        return String::newString(player->getConnectionRequest()->getDeviceId());
    }
    CATCH_AND_THROW
}

Local<Value> DeviceClass::getInputMode() const {
    try {
        Player* player = getPlayer();
        if (!player) return {};

        Json::Value& requestJson = player->getConnectionRequest()->mRawToken->mDataInfo;
        return Number::newNumber(requestJson["CurrentInputMode"].asInt(0));
    }
    CATCH_AND_THROW
}

// Local<Value> DeviceClass::getPlayMode() {
//     try {
//         Player* player = getPlayer();
//         if (!player) return {};

//         return Number::newNumber(0);
//     }
//     CATCH_AND_THROW
// }
