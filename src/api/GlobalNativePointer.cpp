#include "api/NativeAPI.h"
#include "ll/api/service/GlobalService.h"
#include "mc/world/Minecraft.h"
#include "mc/server/common/commands/AllowListCommand.h"
#include "mc/network/NetworkSystem.h"


Local<Value> GlobalNativePointer::getLevelPtr(const Arguments& args) {
    return NativePointer::newNativePointer(ll::Global<Level>);
}

Local<Value> GlobalNativePointer::getMinecraftPtr(const Arguments& args) {
    return NativePointer::newNativePointer(ll::Global<Minecraft>);
}

Local<Value> GlobalNativePointer::getServerNetworkHandlerPtr(const Arguments& args) {
    return NativePointer::newNativePointer(ll::Global<ServerNetworkHandler>);
}

Local<Value> GlobalNativePointer::getMinecraftCommandsPtr(const Arguments& args) {
    return NativePointer::newNativePointer(&ll::Global<Minecraft>->getCommands());
}

Local<Value> GlobalNativePointer::getLevelStoragePtr(const Arguments& args) {
    return NativePointer::newNativePointer(&ll::Global<Level>->getLevelStorage());
}

Local<Value> GlobalNativePointer::getDBStoragePtr(const Arguments& args) {
    return NativePointer::newNativePointer(&ll::Global<Level>->getLevelStorage());
}

Local<Value> GlobalNativePointer::getRakNetServerLocatorPtr(const Arguments& args) {
    return NativePointer::newNativePointer(&ll::Global<NetworkSystem>->getServerLocator());
}

Local<Value> GlobalNativePointer::getRakNetRakPeerPtr(const Arguments& args) {
    return NativePointer::newNativePointer(ll::Global<RakNet::RakPeer>);
}

Local<Value> GlobalNativePointer::getScoreboardPtr(const Arguments& args) {
    return NativePointer::newNativePointer(&ll::Global<Level>->getScoreboard());
}

Local<Value> GlobalNativePointer::getAllowListFilePtr(const Arguments& args) {
    return NativePointer::newNativePointer(AllowListCommand::$mAllowListFile());
}

Local<Value> GlobalNativePointer::getPropertiesSettingsPtr(const Arguments& args) {
    return NativePointer::newNativePointer(ll::Global<PropertiesSettings>);
}

ClassDefine<GlobalNativePointer> GlobalNativePointerBuilder =
    defineClass<GlobalNativePointer>("GlobalPointer")
        .constructor()
        .function("Level", &GlobalNativePointer::getLevelPtr)
        .function("Minecraft", &GlobalNativePointer::getMinecraftPtr)
        .function("ServerNetworkHandler", &GlobalNativePointer::getServerNetworkHandlerPtr)
        .function("MinecraftCommands", &GlobalNativePointer::getMinecraftCommandsPtr)
        .function("LevelStorage", &GlobalNativePointer::getLevelStoragePtr)
        .function("DBStorage", &GlobalNativePointer::getDBStoragePtr)
        .function("RakNetServerLocator", &GlobalNativePointer::getRakNetServerLocatorPtr)
        .function("RakNetRakPeer", &GlobalNativePointer::getRakNetRakPeerPtr)
        .function("Scoreboard", &GlobalNativePointer::getScoreboardPtr)
        .function("AllowListFile", &GlobalNativePointer::getAllowListFilePtr)
        .function("PropertiesSettings", &GlobalNativePointer::getPropertiesSettingsPtr)
        .build();
