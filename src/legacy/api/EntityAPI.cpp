#include "api/EntityAPI.h"

#include "api/APIHelp.h"
#include "api/BaseAPI.h"
#include "api/BlockAPI.h"
#include "api/ContainerAPI.h"
#include "api/ItemAPI.h"
#include "api/McAPI.h"
#include "api/NbtAPI.h"
#include "api/PlayerAPI.h"
#include "ll/api/memory/Memory.h"
#include "ll/api/service/Bedrock.h"
#include "lse/api/MoreGlobal.h"
#include "lse/api/helper/AttributeHelper.h"
#include "mc/deps/core/math/Vec2.h"
#include "mc/deps/shared_types/legacy/actor/ActorDamageCause.h"
#include "mc/deps/vanilla_components/ActorDataFlagComponent.h"
#include "mc/deps/vanilla_components/StateVectorComponent.h"
#include "mc/entity/components/AttributesComponent.h"
#include "mc/entity/components/InsideBlockComponent.h"
#include "mc/entity/components/IsOnHotBlockFlagComponent.h"
#include "mc/entity/components/TagsComponent.h"
#include "mc/entity/utilities/ActorMobilityUtils.h"
#include "mc/legacy/ActorRuntimeID.h"
#include "mc/legacy/ActorUniqueID.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/server/commands/CommandUtils.h"
#include "mc/util/BlockUtils.h"
#include "mc/world/SimpleContainer.h"
#include "mc/world/actor/ActorDamageByActorSource.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/ActorDefinitionIdentifier.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/item/ItemActor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/provider/ActorEquipment.h"
#include "mc/world/actor/provider/SynchedActorDataAccess.h"
#include "mc/world/attribute/Attribute.h"
#include "mc/world/attribute/AttributeInstanceHandle.h" // IWYU pragma: keep
#include "mc/world/attribute/SharedAttributes.h"
#include "mc/world/effect/EffectDuration.h"
#include "mc/world/effect/MobEffectInstance.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Spawner.h"
#include "mc/world/level/biome/Biome.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/VanillaBlockTypeIds.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/material/Material.h"
#include "mc/world/phys/AABB.h"
#include "mc/world/phys/HitResult.h"

#include <climits>
#include <memory>

using lse::api::AttributeHelper;
using magic_enum::enum_integer;

//////////////////// Class Definition ////////////////////

ClassDefine<EntityClass> EntityClassBuilder =
    defineClass<EntityClass>("LLSE_Entity")
        .constructor(nullptr)
        .instanceProperty("name", &EntityClass::getName)
        .instanceProperty("type", &EntityClass::getType)
        .instanceProperty("id", &EntityClass::getId)
        .instanceProperty("pos", &EntityClass::getPos)
        .instanceProperty("posDelta", &EntityClass::getPosDelta)
        .instanceProperty("feetPos", &EntityClass::getFeetPos)
        .instanceProperty("blockPos", &EntityClass::getBlockPos)
        .instanceProperty("maxHealth", &EntityClass::getMaxHealth)
        .instanceProperty("health", &EntityClass::getHealth)
        .instanceProperty("canFly", &EntityClass::getCanFly)
        .instanceProperty("canFreeze", &EntityClass::getCanFreeze)
        .instanceProperty("canSeeDaylight", &EntityClass::getCanSeeDaylight)
        .instanceProperty("canPickupItems", &EntityClass::getCanPickupItems)
        .instanceProperty("inAir", &EntityClass::getInAir)
        .instanceProperty("inWater", &EntityClass::getInWater)
        .instanceProperty("inLava", &EntityClass::getInLava)
        .instanceProperty("inRain", &EntityClass::getInRain)
        .instanceProperty("inSnow", &EntityClass::getInSnow)
        .instanceProperty("inWall", &EntityClass::getInWall)
        .instanceProperty("inWaterOrRain", &EntityClass::getInWaterOrRain)
        .instanceProperty("inWorld", &EntityClass::getInWorld)
        .instanceProperty("speed", &EntityClass::getSpeed)
        .instanceProperty("direction", &EntityClass::getDirection)
        .instanceProperty("uniqueId", &EntityClass::getUniqueID)
        .instanceProperty("runtimeId", &EntityClass::getRuntimeID)
        .instanceProperty("isInvisible", &EntityClass::isInvisible)
        .instanceProperty("isInsidePortal", &EntityClass::isInsidePortal)
        .instanceProperty("isTrusting", &EntityClass::isTrusting)
        .instanceProperty("isTouchingDamageBlock", &EntityClass::isTouchingDamageBlock)
        .instanceProperty("isOnFire", &EntityClass::isOnFire)
        .instanceProperty("isOnGround", &EntityClass::isOnGround)
        .instanceProperty("isOnHotBlock", &EntityClass::isOnHotBlock)
        .instanceProperty("isTrading", &EntityClass::isTrading)
        .instanceProperty("isRiding", &EntityClass::isRiding)
        .instanceProperty("isDancing", &EntityClass::isDancing)
        .instanceProperty("isSleeping", &EntityClass::isSleeping)
        .instanceProperty("isAngry", &EntityClass::isAngry)
        .instanceProperty("isBaby", &EntityClass::isBaby)
        .instanceProperty("isMoving", &EntityClass::isMoving)

        .instanceFunction("teleport", &EntityClass::teleport)
        .instanceFunction("kill", &EntityClass::kill)
        .instanceFunction("despawn", &EntityClass::despawn)
        .instanceFunction("remove", &EntityClass::remove)
        .instanceFunction("hurt", &EntityClass::hurt)
        .instanceFunction("heal", &EntityClass::heal)
        .instanceFunction("setPosDelta", &EntityClass::setPosDelta)
        .instanceFunction("setHealth", &EntityClass::setHealth)
        .instanceFunction("setAbsorption", &EntityClass::setAbsorption)
        .instanceFunction("setAttackDamage", &EntityClass::setAttackDamage)
        .instanceFunction("setMaxAttackDamage", &EntityClass::setMaxAttackDamage)
        .instanceFunction("setFollowRange", &EntityClass::setFollowRange)
        .instanceFunction("setKnockbackResistance", &EntityClass::setKnockbackResistance)
        .instanceFunction("setLuck", &EntityClass::setLuck)
        .instanceFunction("setMovementSpeed", &EntityClass::setMovementSpeed)
        .instanceFunction("setUnderwaterMovementSpeed", &EntityClass::setUnderwaterMovementSpeed)
        .instanceFunction("setLavaMovementSpeed", &EntityClass::setLavaMovementSpeed)
        .instanceFunction("setMaxHealth", &EntityClass::setMaxHealth)
        .instanceFunction("setFire", &EntityClass::setFire)
        .instanceFunction("stopFire", &EntityClass::stopFire)
        .instanceFunction("isPlayer", &EntityClass::isPlayer)
        .instanceFunction("toPlayer", &EntityClass::toPlayer)
        .instanceFunction("isItemEntity", &EntityClass::isItemEntity)
        .instanceFunction("toItem", &EntityClass::toItem)
        .instanceFunction("getBlockStandingOn", &EntityClass::getBlockStandingOn)
        .instanceFunction("getArmor", &EntityClass::getArmor)
        .instanceFunction("distanceTo", &EntityClass::distanceTo)
        .instanceFunction("distanceToSqr", &EntityClass::distanceToSqr)
        .instanceFunction("hasContainer", &EntityClass::hasContainer)
        .instanceFunction("getContainer", &EntityClass::getContainer)
        .instanceFunction("refreshItems", &EntityClass::refreshItems)
        .instanceFunction("setScale", &EntityClass::setScale)
        .instanceFunction("setNbt", &EntityClass::setNbt)
        .instanceFunction("getNbt", &EntityClass::getNbt)
        .instanceFunction("addTag", &EntityClass::addTag)
        .instanceFunction("removeTag", &EntityClass::removeTag)
        .instanceFunction("hasTag", &EntityClass::hasTag)
        .instanceFunction("getAllTags", &EntityClass::getAllTags)
        .instanceFunction("getEntityFromViewVector", &EntityClass::getEntityFromViewVector)
        .instanceFunction("getBlockFromViewVector", &EntityClass::getBlockFromViewVector)
        .instanceFunction("getBiomeName", &EntityClass::getBiomeName)
        .instanceFunction("getBiomeId", &EntityClass::getBiomeId)
        .instanceFunction("quickEvalMolangScript", &EntityClass::quickEvalMolangScript)

        .instanceFunction("getAllEffects", &EntityClass::getAllEffects)
        .instanceFunction("addEffect", &EntityClass::addEffect)
        .instanceFunction("removeEffect", &EntityClass::removeEffect)

        // For Compatibility
        .instanceFunction("setTag", &EntityClass::setNbt)
        .instanceFunction("setOnFire", &EntityClass::setOnFire)
        .instanceFunction("getTag", &EntityClass::getNbt)
        .instanceFunction("distanceToPos", &EntityClass::distanceTo)
        .build();

//////////////////// Classes ////////////////////

EntityClass::EntityClass(Actor const* actor) : ScriptClass(ScriptClass::ConstructFromCpp<EntityClass>{}) {
    try {
        if (actor) {
            mWeakEntity = actor->getWeakEntity();
            mValid      = true;
        }
    } catch (...) {}
}

// 生成函数
Local<Object> EntityClass::newEntity(Actor const* actor) {
    auto const newp = new EntityClass(actor);
    return newp->getScriptObject();
}

Actor* EntityClass::extract(Local<Value> const& v) {
    if (EngineScope::currentEngine()->isInstanceOf<EntityClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<EntityClass>(v)->get();
    return nullptr;
}

Actor* EntityClass::tryExtractActor(Local<Value> const& v) {
    if (IsInstanceOf<EntityClass>(v)) return EntityClass::extract(v);
    if (IsInstanceOf<PlayerClass>(v)) return PlayerClass::extract(v);
    return nullptr;
}

// 成员函数
Actor* EntityClass::get() const {
    if (mValid) {
        return mWeakEntity.tryUnwrap<Actor>().as_ptr();
    }
    return nullptr;
}

Local<Value> EntityClass::getUniqueID() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};
        return String::newString(std::to_string(entity->getOrCreateUniqueID().rawID));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getRuntimeID() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};
        return String::newString(std::to_string(entity->getRuntimeID().rawID));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isInvisible() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isInvisible());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isInsidePortal() const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        if (auto const component = entity->getEntityContext().tryGetComponent<InsideBlockComponent>()) {
            auto& fullName = component->mInsideBlock->getBlockType().mNameInfo->mFullName;
            return Boolean::newBoolean(
                *fullName == VanillaBlockTypeIds::Portal() || *fullName == VanillaBlockTypeIds::EndPortal()
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isTrusting() const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(
            SynchedActorDataAccess::getActorFlag(entity->getEntityContext(), ActorFlags::Trusting)
        );
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isTouchingDamageBlock() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isTouchingDamageBlock());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isOnFire() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isOnFire());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isOnGround() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isOnGround());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isOnHotBlock() const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->getEntityContext().hasComponent<IsOnHotBlockFlagComponent>());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isTrading() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isTrading());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isRiding() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isRiding());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isDancing() const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(
            SynchedActorDataAccess::getActorFlag(entity->getEntityContext(), ActorFlags::Dancing)
        );
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isSleeping() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isSleeping());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isAngry() const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(SynchedActorDataAccess::getActorFlag(entity->getEntityContext(), ActorFlags::Angry));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isBaby() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isBaby());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isMoving() const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(
            SynchedActorDataAccess::getActorFlag(entity->getEntityContext(), ActorFlags::Moving)
        );
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getName() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return String::newString(CommandUtils::getActorName(*entity));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getType() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return String::newString(entity->getTypeName());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getId() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Number::newNumber(enum_integer(entity->getEntityTypeId()));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getPos() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return FloatPos::newPos(entity->getPosition(), entity->getDimensionId().id);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getPosDelta() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return FloatPos::newPos(entity->getPosDelta(), entity->getDimensionId().id);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setPosDelta(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);

    try {
        Actor* entity = get();
        if (!entity) return Boolean::newBoolean(false);
        Vec3 delta;
        if (args.size() == 1) {
            if (!IsInstanceOf<FloatPos>(args[0])) {
                throw WrongArgTypeException(__FUNCTION__);
            }
            delta = EngineScope::currentEngine()->getNativeInstance<FloatPos>(args[0])->getVec3();
        } else if (args.size() == 3) {
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);

            delta.x = args[0].asNumber().toFloat();
            delta.y = args[1].asNumber().toFloat();
            delta.z = args[2].asNumber().toFloat();
        }
        entity->mBuiltInComponents->mStateVectorComponent->mPosDelta = delta;

        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getFeetPos() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return FloatPos::newPos(entity->getFeetPos(), entity->getDimensionId().id);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getBlockPos() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return IntPos::newPos(entity->getFeetBlockPos(), entity->getDimensionId().id);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getMaxHealth() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Number::newNumber(entity->getMaxHealth());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getHealth() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Number::newNumber(entity->getHealth());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getCanFly() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->canFly());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getCanFreeze() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->canFreeze());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getCanSeeDaylight() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->canSeeDaylight());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getCanPickupItems() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->mCanPickupItems);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getInAir() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(!entity->isOnGround() && !entity->isInWater());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getInWater() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isInWater());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getInClouds() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        float const cloudHeight = entity->getDimension().getCloudHeight();
        float const y           = entity->getPosition().y;
        return Boolean::newBoolean(y > cloudHeight && y < cloudHeight + 4.0f);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getInLava() const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(
            ActorMobilityUtils::shouldApplyLava(entity->getDimensionBlockSourceConst(), entity->getEntityContext())
        );
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getInRain() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isInRain());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getInSnow() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isInSnow());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getInWall() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        // The original Actor::isInWall() was moved to MobSuffocationSystemImpl::isInWall() in 1.21.60.10, but the later
        // needs too many parameters.
        return Boolean::newBoolean(entity->getDimensionBlockSource().isInWall(
            entity->getAttachPos(SharedTypes::Legacy::ActorLocation::BreathingPoint)
        ));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getInWaterOrRain() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isInWaterOrRain());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getInWorld() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isInWorld());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getSpeed() const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Number::newNumber(entity->getPosDeltaPerSecLength());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getDirection() const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        // getRotation()
        Vec2 const vec = entity->mBuiltInComponents->mActorRotationComponent->mRotationDegree;
        return DirectionAngle::newAngle(vec.x, vec.y);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::teleport(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1)

    try {
        Actor* entity = get();
        if (!entity) return Boolean::newBoolean(false);
        FloatVec4 pos;
        bool      rotationIsValid = false;
        Vec2      ang;

        if (args.size() <= 2) {
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos const* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = *posObj;
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos const* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = static_cast<FloatVec4>(*posObj);
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
            if (args.size() == 2 && IsInstanceOf<DirectionAngle>(args[1])) {
                auto const angle = DirectionAngle::extract(args[1]);
                ang.x            = angle->pitch;
                ang.y            = angle->yaw;
                rotationIsValid  = true;
            }
        } else if (args.size() <= 5) { // teleport(x,y,z,dimid[,rot])
            // number pos
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);

            pos.x   = args[0].asNumber().toFloat();
            pos.y   = args[1].asNumber().toFloat();
            pos.z   = args[2].asNumber().toFloat();
            pos.dim = args[3].asNumber().toInt32();
            if (args.size() == 5 && IsInstanceOf<DirectionAngle>(args[4])) {
                auto const angle = DirectionAngle::extract(args[4]);
                ang.x            = angle->pitch;
                ang.y            = angle->yaw;
                rotationIsValid  = true;
            }
        } else {
            throw WrongArgTypeException(__FUNCTION__);
        }
        if (!rotationIsValid) {
            // getRotation()
            ang = entity->mBuiltInComponents->mActorRotationComponent->mRotationDegree;
        }
        entity->teleport(pos.getVec3(), pos.dim, ang);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::distanceTo(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);

    try {
        FloatVec4 pos{};

        Actor const* actor = get();
        if (!actor) return {};

        if (args.size() == 1) { // pos | player | entity
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos const* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return {};
                pos = *posObj;
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos const* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return {};
                pos = static_cast<FloatVec4>(*posObj);
            } else if (IsInstanceOf<PlayerClass>(args[0]) || IsInstanceOf<EntityClass>(args[0])) {
                // Player or Entity

                Actor const* targetActor = EntityClass::tryExtractActor(args[0]);
                if (!targetActor) return {};

                Vec3 const targetActorPos = targetActor->getPosition();

                pos.x   = targetActorPos.x;
                pos.y   = targetActorPos.y;
                pos.z   = targetActorPos.z;
                pos.dim = targetActor->getDimensionId().id;
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
        } else if (args.size() == 4) { // x, y, z, dimId
            // number pos
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);

            pos.x   = args[0].asNumber().toFloat();
            pos.y   = args[1].asNumber().toFloat();
            pos.z   = args[2].asNumber().toFloat();
            pos.dim = args[3].asNumber().toInt32();
        } else {
            throw WrongArgsCountException(__FUNCTION__);
        }

        if (actor->getDimensionId().id != pos.dim) return Number::newNumber(INT_MAX);

        return Number::newNumber(actor->getPosition().distanceTo(pos.getVec3()));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::distanceToSqr(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);

    try {
        FloatVec4 pos;

        Actor const* actor = get();
        if (!actor) return {};

        if (args.size() == 1) {
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos const* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return {};
                pos = *posObj;
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos const* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return {};
                pos = static_cast<FloatVec4>(*posObj);
            } else if (IsInstanceOf<PlayerClass>(args[0]) || IsInstanceOf<EntityClass>(args[0])) {
                // Player or Entity

                Actor const* targetActor = EntityClass::tryExtractActor(args[0]);
                if (!targetActor) return {};

                Vec3 const targetActorPos = targetActor->getPosition();

                pos.x   = targetActorPos.x;
                pos.y   = targetActorPos.y;
                pos.z   = targetActorPos.z;
                pos.dim = targetActor->getDimensionId().id;
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
        } else if (args.size() == 4) {
            // number pos
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);

            pos.x   = args[0].asNumber().toFloat();
            pos.y   = args[1].asNumber().toFloat();
            pos.z   = args[2].asNumber().toFloat();
            pos.dim = args[3].asNumber().toInt32();
        } else {
            throw WrongArgsCountException(__FUNCTION__);
        }

        if (actor->getDimensionId().id != pos.dim) return Number::newNumber(INT_MAX);

        return Number::newNumber(actor->getPosition().distanceToSqr(pos.getVec3()));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::kill(Arguments const&) const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        entity->kill();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::despawn(Arguments const&) const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        entity->despawn();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::remove(Arguments const&) const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        entity->remove();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isPlayer(Arguments const&) const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->isType(ActorType::Player));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::toPlayer(Arguments const&) const {
    try {
        if (auto const player = mWeakEntity.tryUnwrap<Player>()) {
            return PlayerClass::newPlayer(player);
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::isItemEntity(Arguments const&) const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->hasCategory(ActorCategory::Item));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::toItem(Arguments const&) const {
    try {
        Actor* entity = get();
        if (!entity || !entity->hasCategory(ActorCategory::Item)) {
            return {};
        }
        return ItemClass::newItem(&static_cast<ItemActor*>(entity)->item());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getBlockStandingOn(Arguments const&) const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        return BlockClass::newBlock(entity->getBlockPosCurrentlyStandingOn(nullptr), entity->getDimensionId().id);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getArmor(Arguments const&) const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        return ContainerClass::newContainer(&ActorEquipment::getArmorContainer(entity->getEntityContext()));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::refreshItems(Arguments const&) const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        static_cast<Mob*>(entity)->refreshInventory();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::hasContainer(Arguments const&) const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        Vec3 const pos = entity->getPosition();
        return Boolean::newBoolean(entity->getDimensionBlockSource().tryGetContainer(BlockPos(pos)) ? true : false);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getContainer(Arguments const&) const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        Vec3 const pos       = entity->getPosition();
        Container* container = entity->getDimensionBlockSource().tryGetContainer(BlockPos(pos));
        return container ? ContainerClass::newContainer(container) : Local<Value>();
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::hurt(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        Actor* entity = get();
        if (!entity) {
            return Boolean::newBoolean(false);
        }
        float const damage = args[0].asNumber().toFloat();
        int         type   = 0;
        if (args.size() >= 2) {
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            type = args[1].asNumber().toInt32();
        }
        if (args.size() == 3) {
            Actor const* source = EntityClass::tryExtractActor(args[2]);
            if (!source) {
                return Boolean::newBoolean(false);
            }
            ActorDamageByActorSource const damageBySource =
                ActorDamageByActorSource(*source, static_cast<SharedTypes::Legacy::ActorDamageCause>(type));
            return Boolean::newBoolean(entity->_hurt(damageBySource, damage, true, false));
        }
        ActorDamageSource damageSource;
        damageSource.mCause = static_cast<SharedTypes::Legacy::ActorDamageCause>(type);
        return Boolean::newBoolean(entity->_hurt(damageSource, damage, true, false));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::heal(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        Actor* entity = get();
        if (!entity) return {};

        entity->heal(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setHealth(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return {};

        if (auto const component = entity->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::HEALTH(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setAbsorption(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return {};

        if (auto const component = entity->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::ABSORPTION(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setAttackDamage(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return {};

        if (auto const component = entity->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::ATTACK_DAMAGE(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setMaxAttackDamage(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return {};

        if (auto const component = entity->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setMaxValue(
                    component->mAttributes,
                    SharedAttributes::ATTACK_DAMAGE(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setFollowRange(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return {};

        if (auto const component = entity->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::FOLLOW_RANGE(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setKnockbackResistance(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return {};

        if (auto const component = entity->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::KNOCKBACK_RESISTANCE(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setLuck(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return {};

        if (auto const component = entity->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::LUCK(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setMovementSpeed(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return {};

        if (auto const component = entity->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::MOVEMENT_SPEED(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setUnderwaterMovementSpeed(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return {};

        if (auto const component = entity->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::UNDERWATER_MOVEMENT_SPEED(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setLavaMovementSpeed(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return {};

        if (auto const component = entity->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::LAVA_MOVEMENT_SPEED(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setMaxHealth(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return {};

        if (auto const component = entity->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setMaxValue(
                    component->mAttributes,
                    SharedAttributes::HEALTH(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

// For Compatibility
Local<Value> EntityClass::setOnFire(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return {};

        int const time = args[0].asNumber().toInt32();
        entity->setOnFire(time, true);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setFire(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kBoolean);

    try {
        Actor* entity = get();
        if (!entity) return {};

        int const  time     = args[0].asNumber().toInt32();
        bool const isEffect = args[1].asBoolean().value();

        entity->setOnFire(time, isEffect);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::stopFire(Arguments const&) const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        entity->stopFire();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setScale(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return {};

        SynchedActorDataAccess::setBoundingBoxScale(entity->getEntityContext(), args[0].asNumber().toFloat());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getNbt(Arguments const&) const {
    try {
        Actor const* entity = get();
        if (!entity) return {};

        std::unique_ptr<CompoundTag> tag = std::make_unique<CompoundTag>();
        entity->save(*tag);
        return NbtCompoundClass::pack(std::move(tag));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::setNbt(Arguments const& args) const {
    using namespace lse::api;
    CHECK_ARGS_COUNT(args, 1);

    try {
        Actor* entity = get();
        if (!entity) return {};

        auto const nbt = NbtCompoundClass::extract(args[0]);
        if (!nbt) {
            return {};
        }

        return Boolean::newBoolean(entity->load(*nbt, MoreGlobal::defaultDataLoadHelper()));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::addTag(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Actor* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->addTag(args[0].asString().toString()));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::removeTag(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Actor* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->removeTag(args[0].asString().toString()));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::hasTag(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Actor const* entity = get();
        if (!entity) return {};

        return Boolean::newBoolean(entity->hasTag(args[0].asString().toString()));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getAllTags(Arguments const&) const {
    try {
        Actor* entity = get();
        if (!entity) return {};

        Local<Array> arr = Array::newArray();
        if (auto const component =
                entity->getEntityContext().tryGetComponent<TagsComponent<IDType<LevelTagSetIDType>>>()) {
            for (auto& tag : get()->getLevel().getTagRegistry().getTagsInSet(component->mTagSetID)) {
                arr.add(String::newString(tag));
            }
            return arr;
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getEntityFromViewVector(Arguments const& args) const {

    try {
        Actor const* actor = get();
        if (!actor) return {};
        float maxDistance = 5.25f;
        if (args.size() > 0) {
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            maxDistance = args[0].asNumber().toFloat();
        }
        HitResult const result = actor->traceRay(maxDistance, true, false);
        Actor const*    entity = result.getEntity();
        if (result.mType != HitResultType::NoHit && entity) {
            return EntityClass::newEntity(entity);
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getBlockFromViewVector(Arguments const& args) const {
    try {
        Actor const* actor = get();
        if (!actor) return {};
        bool  includeLiquid = false;
        bool  solidOnly     = false;
        float maxDistance   = 5.25f;
        bool  fullOnly      = false;
        if (args.size() > 0) {
            CHECK_ARG_TYPE(args[0], ValueKind::kBoolean);
            includeLiquid = args[0].asBoolean().value();
        }
        if (args.size() > 1) {
            CHECK_ARG_TYPE(args[1], ValueKind::kBoolean);
            solidOnly = args[1].asBoolean().value();
        }
        if (args.size() > 2) {
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            maxDistance = args[2].asNumber().toFloat();
        }
        if (args.size() > 3) {
            CHECK_ARG_TYPE(args[3], ValueKind::kBoolean);
            fullOnly = args[3].asBoolean().value();
        }
        HitResult const res = actor->traceRay(
            maxDistance,
            false,
            true,
            [&solidOnly, &fullOnly, &includeLiquid](BlockSource const&, Block const& block, bool) {
                if (solidOnly && !block.mCachedComponentData->mIsSolid) {
                    return false;
                }
                if (fullOnly && !block.getBlockType().isSlabBlock()) {
                    return false;
                }
                if (!includeLiquid && BlockUtils::isLiquidSource(block)) {
                    return false;
                }
                return true;
            }
        );
        if (res.mType == HitResultType::NoHit) {
            return {};
        }
        BlockPos bp;
        if (includeLiquid && res.mIsHitLiquid) {
            bp = res.mLiquidPos;
        } else {
            bp = res.mBlock;
        }
        Block const&     bl     = actor->getDimensionBlockSource().getBlock(bp);
        BlockType const& legacy = bl.getBlockType();
        if (bl.isAir() || (legacy.mProperties == BlockProperty::None && legacy.mMaterial.mType == MaterialType::Any)) {
            return {};
        }
        return BlockClass::newBlock(bl, bp, actor->getDimensionId());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::quickEvalMolangScript(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        Actor* actor = get();
        if (!actor) return {};
        return Number::newNumber(actor->evalMolang(args[0].asString().toString()));
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getBiomeId() const {
    try {
        Actor const* actor = get();
        if (!actor) return {};
        auto& bio = actor->getDimensionBlockSource().getBiome(actor->getFeetBlockPos());
        return Number::newNumber(bio.mId->mValue);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getBiomeName() const {
    try {
        Actor const* actor = get();
        if (!actor) return {};
        auto& bio = actor->getDimensionBlockSource().getBiome(actor->getFeetBlockPos());
        return String::newString(bio.mHash->getString());
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::getAllEffects() const {
    try {
        Actor* actor = get();
        if (!actor) {
            return {};
        }
        Local<Array> effectList = Array::newArray();
        for (auto const& effect : actor->_getAllEffectsNonConst()) {
            effectList.add(Number::newNumber(static_cast<long long>(effect.mId)));
        }
        return effectList;
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::addEffect(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 4);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[3], ValueKind::kBoolean);
    try {
        Actor* actor = get();
        if (!actor) {
            return Boolean::newBoolean(false);
        }
        unsigned int const id = args[0].asNumber().toInt32();
        EffectDuration     duration{args[1].asNumber().toInt32()};
        int const          level         = args[2].asNumber().toInt32();
        bool const         showParticles = args[3].asBoolean().value();
        MobEffectInstance  effect(id);
        effect.mDuration      = duration;
        effect.mAmplifier     = level;
        effect.mEffectVisible = showParticles;
        actor->addEffect(effect);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> EntityClass::removeEffect(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        Actor* actor = get();
        if (!actor) {
            return Boolean::newBoolean(false);
        }
        int const id = args[0].asNumber().toInt32();
        actor->removeEffect(id);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::getAllEntities(Arguments const&) {
    try {
        auto& entityList = ll::service::getLevel()->getEntities();
        auto  arr        = Array::newArray();
        for (auto& i : entityList) {
            if (i.has_value() && i.tryUnwrap().has_value()) {
                arr.add(EntityClass::newEntity(&i.tryUnwrap().get()));
            }
        }
        return arr;
    }
    CATCH_AND_THROW
}

Local<Value> McClass::getEntities(Arguments const& args) {
    try {
        int   dim;
        float dis = 2.0f;
        AABB  aabb;
        if (args.size() > 0) {
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);

                aabb.min = Vec3(posObj->x, posObj->y, posObj->z);
                dim      = posObj->dim;

            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                aabb.min         = posObj->getVec3();
                dim              = posObj->dim;
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
            if (args.size() > 1) {
                if (IsInstanceOf<IntPos>(args[1])) {
                    // IntPos
                    IntPos* posObj = IntPos::extractPos(args[1]);
                    if (dim != posObj->dim) {
                        throw CreateExceptionWithInfo(__FUNCTION__, "Wrong Dimension!");
                    }
                    aabb.max = Vec3(posObj->x, posObj->y, posObj->z) + 1;
                    dim      = posObj->dim;

                } else if (IsInstanceOf<FloatPos>(args[1])) {
                    // FloatPos
                    FloatPos* posObj = FloatPos::extractPos(args[1]);
                    if (dim != posObj->dim) {
                        throw CreateExceptionWithInfo(__FUNCTION__, "Wrong Dimension!");
                    }
                    aabb.max = posObj->getVec3();
                    dim      = posObj->dim;
                } else if (args[1].getKind() == ValueKind::kNumber) {
                    aabb.max = aabb.min + 1;
                    dis      = args[1].asNumber().toFloat();
                } else {
                    throw WrongArgTypeException(__FUNCTION__);
                }
                if (args.size() > 2) {
                    if (args[2].getKind() == ValueKind::kNumber) {
                        dis = args[2].asNumber().toFloat();
                    } else {
                        throw WrongArgTypeException(__FUNCTION__);
                    }
                } else {
                    aabb.max = aabb.min + 1;
                }
            } else {
                aabb.max = aabb.min + 1;
            }
        } else {
            throw TooFewArgsException(__FUNCTION__);
        }
        aabb.max += dis;
        aabb.min -= dis;

        auto arr       = Array::newArray();
        auto dimension = ll::service::getLevel()->getDimension(dim);
        if (!dimension.lock()) {
            throw CreateExceptionWithInfo(__FUNCTION__, "Wrong Dimension!");
        }
        BlockSource& bs         = dimension.lock()->getBlockSourceFromMainChunkSource();
        auto         entityList = bs.getEntities(aabb);
        for (auto i : entityList) {
            arr.add(EntityClass::newEntity(i));
        }
        return arr;
    }
    CATCH_AND_THROW
}

Local<Value> McClass::getEntity(Arguments const& args) {
    try {
        CHECK_ARGS_COUNT(args, 1)
        CHECK_ARG_TYPE(args[0], ValueKind::kNumber)

        auto const level = ll::service::getLevel();

        if (auto const* entity = level->fetchEntity(ActorUniqueID(args[0].asNumber().toInt64()), false)) {
            return EntityClass::newEntity(entity);
        }
        if (auto const* entity = level->getRuntimeEntity(ActorRuntimeID(args[0].asNumber().toInt64()), false)) {
            return EntityClass::newEntity(entity);
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> McClass::cloneMob(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);

    try {
        Actor const* ac = EntityClass::extract(args[0]);
        if (!ac) {
            throw WrongArgTypeException(__FUNCTION__);
        }

        FloatVec4 pos;

        if (args.size() == 2) {
            if (IsInstanceOf<IntPos>(args[1])) {
                // IntPos
                IntPos const* posObj = IntPos::extractPos(args[1]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = *posObj;
            } else if (IsInstanceOf<FloatPos>(args[1])) {
                // FloatPos
                FloatPos const* posObj = FloatPos::extractPos(args[1]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = static_cast<FloatVec4>(*posObj);
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
        } else if (args.size() == 5) {
            // Number Pos
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[4], ValueKind::kNumber);
            pos = {
                args[1].asNumber().toFloat(),
                args[2].asNumber().toFloat(),
                args[3].asNumber().toFloat(),
                args[4].asNumber().toInt32()
            };
        } else {
            throw WrongArgsCountException(__FUNCTION__);
        }
        ActorDefinitionIdentifier const id(ac->getTypeName());
        Mob const*                      entity = ll::service::getLevel()->getSpawner().spawnMob(
            ll::service::getLevel()->getDimension(pos.dim).lock()->getBlockSourceFromMainChunkSource(),
            id,
            nullptr,
            pos.getVec3(),
            false,
            true,
            false
        );
        if (!entity) return {}; // Null
        return EntityClass::newEntity(entity);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::spawnMob(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        std::string name = args[0].asString().toString();
        FloatVec4   pos;

        if (args.size() == 2) {
            if (IsInstanceOf<IntPos>(args[1])) {
                // IntPos
                IntPos const* posObj = IntPos::extractPos(args[1]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = *posObj;
            } else if (IsInstanceOf<FloatPos>(args[1])) {
                // FloatPos
                FloatPos const* posObj = FloatPos::extractPos(args[1]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = static_cast<FloatVec4>(*posObj);
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
        } else if (args.size() == 5) {
            // Number Pos
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[4], ValueKind::kNumber);
            pos = {
                args[1].asNumber().toFloat(),
                args[2].asNumber().toFloat(),
                args[3].asNumber().toFloat(),
                args[4].asNumber().toInt32()
            };
        } else {
            throw WrongArgsCountException(__FUNCTION__);
        }

        ActorDefinitionIdentifier const id(name);
        Mob const*                      entity = ll::service::getLevel()->getSpawner().spawnMob(
            ll::service::getLevel()->getDimension(pos.dim).lock()->getBlockSourceFromMainChunkSource(),
            id,
            nullptr,
            pos.getVec3(),
            false,
            true,
            false
        );
        if (!entity) return {}; // Null
        return EntityClass::newEntity(entity);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::explode(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 5);

    try {
        FloatVec4 pos;
        int       beginIndex;
        switch (args.size()) {
        case 5:
        case 6:
            // PosObj
            beginIndex = 1;

            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos const* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = *posObj;
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos const* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = static_cast<FloatVec4>(*posObj);
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
            break;
        case 8:
        case 9:
            // Number Pos
            beginIndex = 4;
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
            pos = {
                args[0].asNumber().toFloat(),
                args[1].asNumber().toFloat(),
                args[2].asNumber().toFloat(),
                args[3].asNumber().toInt32()
            };
            break;
        default:
            throw WrongArgsCountException(__FUNCTION__);
        }
        std::optional<Actor*> source = EntityClass::tryExtractActor(args[beginIndex]); // Can be nullptr

        if (args.size() == 5 || args.size() == 8) {
            CHECK_ARG_TYPE(args[beginIndex + 1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[beginIndex + 2], ValueKind::kBoolean);
            CHECK_ARG_TYPE(args[beginIndex + 3], ValueKind::kBoolean);

            float const radius    = args[beginIndex + 1].asNumber().toFloat();
            bool const  isDestroy = args[beginIndex + 2].asBoolean().value();
            bool const  isFire    = args[beginIndex + 3].asBoolean().value();

            return Boolean::newBoolean(
                ll::service::getLevel()->explode(
                    ll::service::getLevel()->getDimension(pos.dim).lock()->getBlockSourceFromMainChunkSource(),
                    source.value_or(nullptr),
                    pos.getVec3(),
                    radius,
                    isFire,
                    isDestroy,
                    FLT_MAX,
                    false
                )
            );
        }
        CHECK_ARG_TYPE(args[beginIndex + 1], ValueKind::kNumber);
        CHECK_ARG_TYPE(args[beginIndex + 2], ValueKind::kNumber);
        CHECK_ARG_TYPE(args[beginIndex + 3], ValueKind::kBoolean);
        CHECK_ARG_TYPE(args[beginIndex + 4], ValueKind::kBoolean);

        float const maxResistance = args[beginIndex + 1].asNumber().toFloat();
        float const radius        = args[beginIndex + 2].asNumber().toFloat();
        bool const  isDestroy     = args[beginIndex + 3].asBoolean().value();
        bool const  isFire        = args[beginIndex + 4].asBoolean().value();

        return Boolean::newBoolean(
            ll::service::getLevel()->explode(
                ll::service::getLevel()->getDimension(pos.dim).lock()->getBlockSourceFromMainChunkSource(),
                source.value_or(nullptr),
                pos.getVec3(),
                radius,
                isFire,
                isDestroy,
                maxResistance,
                false
            )
        );
    }
    CATCH_AND_THROW
}
