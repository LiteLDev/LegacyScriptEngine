#pragma once
#include "legacy/api/APIHelp.h"
#include "mc/deps/ecs/WeakEntityRef.h"

//////////////////// Classes ////////////////////
class Player;
class DeviceClass : public ScriptClass {
private:
    WeakRef<EntityContext> mWeakEntity;
    bool                   mValid;

public:
    explicit DeviceClass(Player const* player);

    Player* getPlayer() const;

    static Local<Object> newDevice(Player const* player);

    Local<Value> getIP() const;
    Local<Value> getAvgPing() const;
    Local<Value> getAvgPacketLoss() const;
    Local<Value> getLastPing() const;
    Local<Value> getLastPacketLoss() const;
    Local<Value> getOs() const;
    Local<Value> getInputMode() const;
    // Local<Value> getPlayMode();
    Local<Value> getServerAddress() const;
    Local<Value> getClientId() const;
};
extern ClassDefine<DeviceClass> DeviceClassBuilder;
