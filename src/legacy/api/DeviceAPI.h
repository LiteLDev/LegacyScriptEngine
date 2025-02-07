#pragma once
#include "api/APIHelp.h"
#include "mc/common/ActorRuntimeID.h"
#include "mc/deps/ecs/WeakEntityRef.h"

//////////////////// Classes ////////////////////
class Player;
class DeviceClass : public ScriptClass {
private:
    WeakRef<EntityContext> mWeakEntity;
    bool                   mValid;

public:
    explicit DeviceClass(Player* player) : ScriptClass(ScriptClass::ConstructFromCpp<DeviceClass>{}) {
        setPlayer(player);
    }

    void    setPlayer(Player* player);
    Player* getPlayer();

    static Local<Object> newDevice(Player* player);

    Local<Value> getIP();
    Local<Value> getAvgPing();
    Local<Value> getAvgPacketLoss();
    Local<Value> getLastPing();
    Local<Value> getLastPacketLoss();
    Local<Value> getOs();
    Local<Value> getInputMode();
    // Local<Value> getPlayMode();
    Local<Value> getServerAddress();
    Local<Value> getClientId();
};
extern ClassDefine<DeviceClass> DeviceClassBuilder;
