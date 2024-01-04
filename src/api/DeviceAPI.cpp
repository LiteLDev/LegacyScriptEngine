#include "api/DeviceAPI.h"
#include "api/APIHelp.h"
#include "ll/api/service/Bedrock.h"
#include "magic_enum.hpp"
#include "mc/certificates/WebToken.h"
#include "mc/network/ConnectionRequest.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"
#include <string>

using namespace std;

//////////////////// Class Definition ////////////////////

ClassDefine<DeviceClass> DeviceClassBuilder =
    defineClass<DeviceClass>("LLSE_Device")
        .constructor(nullptr)
        .instanceProperty("ip", &DeviceClass::getIP)
        .instanceProperty("avgPing", &DeviceClass::getAvgPing)
        .instanceProperty("avgPacketLoss", &DeviceClass::getAvgPacketLoss)
        .instanceProperty("lastPing", &DeviceClass::getLastPing)
        .instanceProperty("lastPacketLoss", &DeviceClass::getLastPacketLoss)
        .instanceProperty("os", &DeviceClass::getOs)
        //.instanceProperty("inputMode", &DeviceClass::getInputMode)
        //.instanceProperty("playMode", &DeviceClass::getPlayMode)
        .instanceProperty("serverAddress", &DeviceClass::getServerAddress)
        .instanceProperty("clientId", &DeviceClass::getClientId)
        .build();

//////////////////// Classes ////////////////////

// 生成函数
Local<Object> DeviceClass::newDevice(Player *p) {
  auto newp = new DeviceClass(p);
  return newp->getScriptObject();
}

// 成员函数
void DeviceClass::setPlayer(Player *player) {
  __try {
    id = player->getOrCreateUniqueID();
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    isValid = false;
  }
}

Player *DeviceClass::getPlayer() {
  if (!isValid)
    return nullptr;
  else
    return ll::service::getLevel()->getPlayer(id);
}

Local<Value> DeviceClass::getIP() {
  try {
    Player *player = getPlayer();
    if (!player)
      return Local<Value>();

    return String::newString(player->getNetworkIdentifier().getAddress());
  }
  CATCH("Fail in GetIP!")
}

Local<Value> DeviceClass::getAvgPing() {
  try {
    Player *player = getPlayer();
    if (!player)
      return Local<Value>();

    return Number::newNumber(player->getNetworkStatus()->mAveragePing);
  }
  CATCH("Fail in getAvgPing!")
}

Local<Value> DeviceClass::getAvgPacketLoss() {
  try {
    Player *player = getPlayer();
    if (!player)
      return Local<Value>();

    return Number::newNumber(player->getNetworkStatus()->mAveragePacketLoss);
  }
  CATCH("Fail in getAvgPacketLoss!")
}

Local<Value> DeviceClass::getLastPing() {
  try {
    Player *player = getPlayer();
    if (!player) {
      return Local<Value>();
    }

    return Number::newNumber(player->getNetworkStatus()->mCurrentPing);
  }
  CATCH("Fail in getLastPing!")
}

Local<Value> DeviceClass::getLastPacketLoss() {
  try {
    Player *player = getPlayer();
    if (!player) {
      return Local<Value>();
    }

    return Number::newNumber(player->getNetworkStatus()->mCurrentPacketLoss);
  }
  CATCH("Fail in getLastPacketLoss!")
}

Local<Value> DeviceClass::getOs() {
  try {
    Player *player = getPlayer();
    if (!player)
      return Local<Value>();

    return String::newString(magic_enum::enum_name(player->getPlatform()));
  }
  CATCH("Fail in getOs!")
}

Local<Value> DeviceClass::getServerAddress() {
  try {
    Player *player = getPlayer();
    if (!player) {
      return Local<Value>();
    }
    if (player->isSimulatedPlayer())
      String::newString("unknown");
    auto map = ll::service::getServerNetworkHandler()
                   ->fetchConnectionRequest(player->getNetworkIdentifier())
                   .mRawToken.get()
                   ->mDataInfo.value_.map_;
    for (auto iter = map->begin(); iter != map->end(); ++iter) {
      string s(iter->first.c_str());
      if (s.find("ServerAddress") != s.npos) {
        auto ServerAddress = iter->second.value_.string_;
        return String::newString(ServerAddress);
      }
    }
    return String::newString("unknown");
  }
  CATCH("Fail in getServerAddress!")
}

Local<Value> DeviceClass::getClientId() {
  try {
    Player *player = getPlayer();
    if (!player)
      return Local<Value>();

    return String::newString(player->getDeviceId()); //=============???
  }
  CATCH("Fail in getClientId!")
}

// Local<Value> DeviceClass::getInputMode() {
//     try {
//         Player* player = getPlayer();
//         if (!player)
//             return Local<Value>();
//
//         return Number::newNumber((int)player->getInputMode());
//     }
//     CATCH("Fail in getInputMode!")
// }
//
// Local<Value> DeviceClass::getPlayMode() {
//     try {
//         Player* player = getPlayer();
//         if (!player)
//             return Local<Value>();
//
//         return Number::newNumber((int)player->getPlayMode());
//     }
//     CATCH("Fail in getPlayMode!")
// }