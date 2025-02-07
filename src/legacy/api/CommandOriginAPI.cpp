#include "api/CommandOriginAPI.h"

#include "api/APIHelp.h"
#include "api/BaseAPI.h"
#include "api/EntityAPI.h"
#include "api/NbtAPI.h"
#include "api/PlayerAPI.h"
#include "magic_enum.hpp"
#include "mc/nbt/CompoundTag.h"
#include "mc/server/commands/Command.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/Dimension.h"

#include <magic_enum.hpp>

//////////////////// Class Definition ////////////////////
ClassDefine<void> OriginTypeStaticBuilder = EnumDefineBuilder<CommandOriginType>::build("OriginType");

ClassDefine<CommandOriginClass> CommandOriginClassBuilder =
    defineClass<CommandOriginClass>("LLSE_CommandOrigin")
        .constructor(nullptr)
        .instanceProperty("type", &CommandOriginClass::getOriginType)
        .instanceProperty("typeName", &CommandOriginClass::getOriginTypeName)
        .instanceProperty("name", &CommandOriginClass::getOriginName)
        .instanceProperty("pos", &CommandOriginClass::getPosition)
        .instanceProperty("blockPos", &CommandOriginClass::getBlockPosition)
        .instanceProperty("entity", &CommandOriginClass::getEntity)
        .instanceProperty("player", &CommandOriginClass::getPlayer)

        .instanceFunction("getNbt", &CommandOriginClass::getNbt)
        .instanceFunction("toString", &CommandOriginClass::toString)

        .build();

//////////////////// APIs ////////////////////

CommandOriginClass::CommandOriginClass(CommandOrigin const* p)
: ScriptClass(ScriptClass::ConstructFromCpp<CommandOriginClass>{}),
  ptr(p) {};

Local<Object> CommandOriginClass::newCommandOrigin(CommandOrigin const* p) {
    auto newp = new CommandOriginClass(p);
    return newp->getScriptObject();
}

Local<Value> CommandOriginClass::getOriginType() {
    try {
        return Number::newNumber((int)get()->getOriginType());
    }
    CATCH("Fail in getOriginType!");
}

Local<Value> CommandOriginClass::getOriginTypeName() {
    try {
        return String::newString(magic_enum::enum_name(get()->getOriginType()));
    }
    CATCH("Fail in getOriginTypeName!");
}

Local<Value> CommandOriginClass::getOriginName() {
    try {
        return String::newString(get()->getName());
    }
    CATCH("Fail in getOriginName!");
}

Local<Value> CommandOriginClass::getBlockPosition() {
    try {
        auto dim = get()->getDimension();
        return IntPos::newPos(get()->getBlockPosition(), dim ? (int)dim->getDimensionId() : 0);
    }
    CATCH("Fail in getBlockPosition!");
}

Local<Value> CommandOriginClass::getPosition() {
    try {
        auto dim = get()->getDimension();
        return FloatPos::newPos(get()->getWorldPosition(), dim ? (int)dim->getDimensionId() : 0);
    }
    CATCH("Fail in getPosition!");
}

Local<Value> CommandOriginClass::getEntity() {
    try {
        auto entity = get()->getEntity();
        if (!entity) return Local<Value>();
        return EntityClass::newEntity(entity);
    }
    CATCH("Fail in getEntity!");
}

Local<Value> CommandOriginClass::getPlayer() {
    try {
        Actor* player = get()->getEntity();
        if (!player) return Local<Value>();
        return PlayerClass::newPlayer(static_cast<Player*>(player));
    }
    CATCH("Fail in getPlayer!");
}

Local<Value> CommandOriginClass::getNbt(const Arguments&) {
    try {
        return NbtCompoundClass::pack(std::make_unique<CompoundTag>(get()->serialize()));
    }
    CATCH("Fail in getNbt!");
}

Local<Value> CommandOriginClass::toString() {
    try {
        return String::newString("<CommandOrigin>");
    }
    CATCH("Fail in toString!");
}
