#include "api/CommandOriginAPI.h"

#include "api/APIHelp.h"
#include "api/BaseAPI.h"
#include "api/EntityAPI.h"
#include "api/NbtAPI.h"
#include "api/PlayerAPI.h"
#include "magic_enum.hpp"
#include "mc/nbt/CompoundTag.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/dimension/Dimension.h"

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

CommandOriginClass::CommandOriginClass(std::shared_ptr<CommandOrigin const> const& ori)
: ScriptClass(ScriptClass::ConstructFromCpp<CommandOriginClass>{}),
  origin(ori) {};

Local<Value> CommandOriginClass::getOriginType() {
    try {
        return Number::newNumber(static_cast<int>(get()->getOriginType()));
    }
    CATCH_AND_THROW
}

Local<Value> CommandOriginClass::getOriginTypeName() {
    try {
        return String::newString(magic_enum::enum_name(get()->getOriginType()));
    }
    CATCH_AND_THROW
}

Local<Value> CommandOriginClass::getOriginName() {
    try {
        return String::newString(get()->getName());
    }
    CATCH_AND_THROW
}

Local<Value> CommandOriginClass::getBlockPosition() {
    try {
        auto dim = get()->getDimension();
        return IntPos::newPos(get()->getBlockPosition(), dim ? static_cast<int>(dim->getDimensionId()) : 0);
    }
    CATCH_AND_THROW
}

Local<Value> CommandOriginClass::getPosition() {
    try {
        auto dim = get()->getDimension();
        return FloatPos::newPos(get()->getWorldPosition(), dim ? static_cast<int>(dim->getDimensionId()) : 0);
    }
    CATCH_AND_THROW
}

Local<Value> CommandOriginClass::getEntity() {
    try {
        auto entity = get()->getEntity();
        if (!entity) return {};
        return EntityClass::newEntity(entity);
    }
    CATCH_AND_THROW
}

Local<Value> CommandOriginClass::getPlayer() {
    try {
        Actor* player = get()->getEntity();
        if (!player) return {};
        return PlayerClass::newPlayer(static_cast<Player*>(player));
    }
    CATCH_AND_THROW
}

Local<Value> CommandOriginClass::getNbt(Arguments const&) {
    try {
        return NbtCompoundClass::pack(std::make_unique<CompoundTag>(get()->serialize()));
    }
    CATCH_AND_THROW
}

Local<Value> CommandOriginClass::toString() {
    try {
        return String::newString("<CommandOrigin>");
    }
    CATCH_AND_THROW
}
