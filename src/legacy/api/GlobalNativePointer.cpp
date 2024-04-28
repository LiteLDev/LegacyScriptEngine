#include "api/NativeAPI.h"
#include "ll/api/service/Bedrock.h"
#include "mc/network/NetworkSystem.h"
#include "mc/server/common/commands/AllowListCommand.h"
#include "mc/world/Minecraft.h"

Local<Value> GlobalNativePointer::getLevelPtr(const Arguments& args) {
    return NativePointer::newNativePointer(ll::service::getLevel());
}

Local<Value> GlobalNativePointer::getMinecraftPtr(const Arguments& args) {
    return NativePointer::newNativePointer(ll::service::getMinecraft());
}

Local<Value> GlobalNativePointer::getServerNetworkHandlerPtr(const Arguments& args) {
    return NativePointer::newNativePointer(ll::service::getServerNetworkHandler());
}

Local<Value> GlobalNativePointer::getMinecraftCommandsPtr(const Arguments& args) {
    return NativePointer::newNativePointer(&ll::service::getMinecraft()->getCommands());
}

Local<Value> GlobalNativePointer::getLevelStoragePtr(const Arguments& args) {
    return NativePointer::newNativePointer(&ll::service::getLevel()->getLevelStorage());
}

Local<Value> GlobalNativePointer::getDBStoragePtr(const Arguments& args) {
    return NativePointer::newNativePointer(&ll::service::getLevel()->getLevelStorage());
}

Local<Value> GlobalNativePointer::getRakNetServerLocatorPtr(const Arguments& args) {
    return NativePointer::newNativePointer(&ll::service::getNetworkSystem()->getServerLocator());
}

Local<Value> GlobalNativePointer::getRakNetRakPeerPtr(const Arguments& args) {
    return NativePointer::newNativePointer(ll::service::getRakPeer());
}

Local<Value> GlobalNativePointer::getScoreboardPtr(const Arguments& args) {
    return NativePointer::newNativePointer(&ll::service::getLevel()->getScoreboard());
}

Local<Value> GlobalNativePointer::getAllowListFilePtr(const Arguments& args) {
    return NativePointer::newNativePointer(AllowListCommand::mAllowListFile);
}

Local<Value> GlobalNativePointer::getPropertiesSettingsPtr(const Arguments& args) {
    return NativePointer::newNativePointer(ll::service::getPropertiesSettings());
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
