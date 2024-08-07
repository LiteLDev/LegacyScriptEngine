#pragma once
#include "api/APIHelp.h"
#include "mc/deps/core/mce/UUID.h"
#include "mc/world/ActorUniqueID.h"

//////////////////// Classes ////////////////////
class Player;
class DeviceClass : public ScriptClass {
private:
    ActorUniqueID id;
    bool          isValid = true;

public:
    explicit DeviceClass(ActorUniqueID& uid) : ScriptClass(ScriptClass::ConstructFromCpp<DeviceClass>{}) {
        setPlayer(uid);
    }

    void    setPlayer(ActorUniqueID& uid);
    Player* getPlayer();

    static Local<Object> newDevice(ActorUniqueID& uid);

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
