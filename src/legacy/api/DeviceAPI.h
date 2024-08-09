#pragma once
#include "api/APIHelp.h"
#include "mc/deps/core/mce/UUID.h"
#include "mc/world/ActorRuntimeID.h"
#include "mc/world/ActorUniqueID.h"

//////////////////// Classes ////////////////////
class Player;
class DeviceClass : public ScriptClass {
private:
    ActorRuntimeID runtimeId;

public:
    explicit DeviceClass(ActorRuntimeID& id) : ScriptClass(ScriptClass::ConstructFromCpp<DeviceClass>{}) {
        setPlayer(id);
    }

    void    setPlayer(ActorRuntimeID& id);
    Player* getPlayer();

    static Local<Object> newDevice(ActorRuntimeID& id);

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
