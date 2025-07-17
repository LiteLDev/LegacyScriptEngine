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
#include "mc/deps/vanilla_components/StateVectorComponent.h"
#include "mc/entity/components/ActorRotationComponent.h"
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
#include "mc/world/actor/provider/ActorAttribute.h"
#include "mc/world/actor/provider/ActorEquipment.h"
#include "mc/world/actor/provider/SynchedActorDataAccess.h"
#include "mc/world/attribute/MutableAttributeWithContext.h"
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

EntityClass::EntityClass(Actor* actor) : ScriptClass(ScriptClass::ConstructFromCpp<EntityClass>{}) {
    try {
        if (actor) {
            mWeakEntity = actor->getWeakEntity();
            mValid      = true;
        }
    } catch (...) {}
}

// 生成函数
Local<Object> EntityClass::newEntity(Actor* actor) {
    auto newp = new EntityClass(actor);
    return newp->getScriptObject();
}

Actor* EntityClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<EntityClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<EntityClass>(v)->get();
    else return nullptr;
}

Actor* EntityClass::tryExtractActor(Local<Value> v) {
    if (IsInstanceOf<EntityClass>(v)) return EntityClass::extract(v);
    if (IsInstanceOf<PlayerClass>(v)) return PlayerClass::extract(v);
    return nullptr;
}

// 成员函数
Actor* EntityClass::get() {
    if (mValid) {
        return mWeakEntity.tryUnwrap<Actor>().as_ptr();
    } else {
        return nullptr;
    }
}

Local<Value> EntityClass::getUniqueID() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();
        else return String::newString(std::to_string(entity->getOrCreateUniqueID().rawID));
    }
    CATCH("Fail in getUniqueID!")
}

Local<Value> EntityClass::getRuntimeID() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();
        else return String::newString(std::to_string(entity->getRuntimeID().rawID));
    }
    CATCH("Fail in getUniqueID!")
}

Local<Value> EntityClass::isInvisible() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isInvisible());
    }
    CATCH("Fail in isInvisible!")
}

Local<Value> EntityClass::isInsidePortal() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        auto component = entity->getEntityContext().tryGetComponent<InsideBlockComponent>();
        if (component) {
            auto& fullName = component->mInsideBlock->getLegacyBlock().mNameInfo->mFullName;
            return Boolean::newBoolean(
                *fullName == VanillaBlockTypeIds::Portal() || *fullName == VanillaBlockTypeIds::EndPortal()
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in isInsidePortal!")
}

Local<Value> EntityClass::isTrusting() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(
            SynchedActorDataAccess::getActorFlag(entity->getEntityContext(), ActorFlags::Trusting)
        );
    }
    CATCH("Fail in isTrusting!")
}

Local<Value> EntityClass::isTouchingDamageBlock() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isTouchingDamageBlock());
    }
    CATCH("Fail in isTouchingDamageBlock!")
}

Local<Value> EntityClass::isOnFire() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isOnFire());
    }
    CATCH("Fail in isOnFire!")
}

Local<Value> EntityClass::isOnGround() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isOnGround());
    }
    CATCH("Fail in isOnGround!")
}

Local<Value> EntityClass::isOnHotBlock() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->getEntityContext().hasComponent<IsOnHotBlockFlagComponent>());
    }
    CATCH("Fail in isOnHotBlock!")
}

Local<Value> EntityClass::isTrading() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isTrading());
    }
    CATCH("Fail in isTrading!")
}

Local<Value> EntityClass::isRiding() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isRiding());
    }
    CATCH("Fail in isRiding!")
}

Local<Value> EntityClass::isDancing() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isDancing());
    }
    CATCH("Fail in isDancing!")
}

Local<Value> EntityClass::isSleeping() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isSleeping());
    }
    CATCH("Fail in isSleeping!")
}

Local<Value> EntityClass::isAngry() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(SynchedActorDataAccess::getActorFlag(entity->getEntityContext(), ActorFlags::Angry));
    }
    CATCH("Fail in isAngry!")
}

Local<Value> EntityClass::isBaby() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isBaby());
    }
    CATCH("Fail in isBaby!")
}

Local<Value> EntityClass::isMoving() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(SynchedActorDataAccess::getActorFlag(entity->getEntityContext(), ActorFlags::Moving)
        );
    }
    CATCH("Fail in isMoving!")
}

Local<Value> EntityClass::getName() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return String::newString(CommandUtils::getActorName(*entity));
    }
    CATCH("Fail in getEntityName!")
}

Local<Value> EntityClass::getType() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return String::newString(entity->getTypeName());
    }
    CATCH("Fail in getEntityType!")
}

Local<Value> EntityClass::getId() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Number::newNumber(enum_integer(entity->getEntityTypeId()));
    }
    CATCH("Fail in getEntityId!")
}

Local<Value> EntityClass::getPos() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return FloatPos::newPos(entity->getPosition(), entity->getDimensionId());
    }
    CATCH("Fail in GetEntityPos!")
}

Local<Value> EntityClass::getPosDelta() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return FloatPos::newPos(entity->getPosDelta(), entity->getDimensionId());
    }
    CATCH("Fail in GetEntityPosDelta!")
}

Local<Value> EntityClass::setPosDelta(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        Actor* entity = get();
        if (!entity) return Boolean::newBoolean(false);
        Vec3 delta;
        if (args.size() == 1) {
            if (!IsInstanceOf<FloatPos>(args[0])) {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
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
    CATCH("Fail in GetEntityPos!")
}

Local<Value> EntityClass::getFeetPos() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return FloatPos::newPos(entity->getFeetPos(), entity->getDimensionId());
    }
    CATCH("Fail in GetEntityFeetPos!")
}

Local<Value> EntityClass::getBlockPos() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return IntPos::newPos(entity->getFeetBlockPos(), entity->getDimensionId());
    }
    CATCH("Fail in GetEntityBlockPos!")
}

Local<Value> EntityClass::getMaxHealth() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Number::newNumber(entity->getMaxHealth());
    }
    CATCH("Fail in GetMaxHealth!")
}

Local<Value> EntityClass::getHealth() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Number::newNumber(ActorAttribute::getHealth(entity->getEntityContext()));
    }
    CATCH("Fail in GetHealth!")
}

Local<Value> EntityClass::getCanFly() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->canFly());
    }
    CATCH("Fail in getCanFly!")
}

Local<Value> EntityClass::getCanFreeze() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->canFreeze());
    }
    CATCH("Fail in getCanFreeze!")
}

Local<Value> EntityClass::getCanSeeDaylight() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->canSeeDaylight());
    }
    CATCH("Fail in getCanSeeDaylight!")
}

Local<Value> EntityClass::getCanPickupItems() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->mCanPickupItems);
    }
    CATCH("Fail in getCanPickupItems!")
}

Local<Value> EntityClass::getInAir() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(!entity->isOnGround() && !entity->isInWater());
    }
    CATCH("Fail in getInAir!")
}

Local<Value> EntityClass::getInWater() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isInWater());
    }
    CATCH("Fail in getInWater!")
}

Local<Value> EntityClass::getInClouds() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        float cloudHeight = entity->getDimension().getCloudHeight();
        float y           = entity->getPosition().y;
        return Boolean::newBoolean(y > cloudHeight && y < cloudHeight + 4.0f);
    }
    CATCH("Fail in getInClouds!")
}

Local<Value> EntityClass::getInLava() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(
            ActorMobilityUtils::shouldApplyLava(entity->getDimensionBlockSourceConst(), entity->getEntityContext())
        );
    }
    CATCH("Fail in getInLava!")
}

Local<Value> EntityClass::getInRain() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isInRain());
    }
    CATCH("Fail in getInRain!")
}

Local<Value> EntityClass::getInSnow() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isInSnow());
    }
    CATCH("Fail in getInSnow!")
}

Local<Value> EntityClass::getInWall() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        // The original Actor::isInWall() was moved to MobSuffocationSystemImpl::isInWall() in 1.21.60.10, but the later
        // needs too many parameters.
        return Boolean::newBoolean(entity->getDimensionBlockSource().isInWall(
            entity->getAttachPos(SharedTypes::Legacy::ActorLocation::BreathingPoint)
        ));
    }
    CATCH("Fail in getInWall!")
}

Local<Value> EntityClass::getInWaterOrRain() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isInWaterOrRain());
    }
    CATCH("Fail in getInWaterOrRain!")
}

Local<Value> EntityClass::getInWorld() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isInWorld());
    }
    CATCH("Fail in getInWorld!")
}

Local<Value> EntityClass::getSpeed() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Number::newNumber(entity->getPosDeltaPerSecLength());
    }
    CATCH("Fail in getSpeed!")
}

Local<Value> EntityClass::getDirection() {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        // getRotation()
        Vec2 vec = entity->mBuiltInComponents->mActorRotationComponent->mRotationDegree;
        return DirectionAngle::newAngle(vec.x, vec.y);
    }
    CATCH("Fail in getDirection!")
}

Local<Value> EntityClass::teleport(const Arguments& args) {
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
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = *posObj;
                }
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Boolean::newBoolean(false);
            }
            if (args.size() == 2 && IsInstanceOf<DirectionAngle>(args[1])) {
                auto angle      = DirectionAngle::extract(args[1]);
                ang.x           = angle->pitch;
                ang.y           = angle->yaw;
                rotationIsValid = true;
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
                auto angle      = DirectionAngle::extract(args[4]);
                ang.x           = angle->pitch;
                ang.y           = angle->yaw;
                rotationIsValid = true;
            }
        } else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Boolean::newBoolean(false);
        }
        if (!rotationIsValid) {
            // getRotation()
            ang = entity->mBuiltInComponents->mActorRotationComponent->mRotationDegree;
        }
        entity->teleport(pos.getVec3(), pos.dim, ang);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in TeleportEntity!")
}

Local<Value> EntityClass::distanceTo(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        FloatVec4 pos{};

        Actor* actor = get();
        if (!actor) return Local<Value>();

        if (args.size() == 1) { // pos | player | entity
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Local<Value>();
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Local<Value>();
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<PlayerClass>(args[0]) || IsInstanceOf<EntityClass>(args[0])) {
                // Player or Entity

                Actor* targetActor = EntityClass::tryExtractActor(args[0]);
                if (!targetActor) return Local<Value>();

                Vec3 targetActorPos = targetActor->getPosition();

                pos.x   = targetActorPos.x;
                pos.y   = targetActorPos.y;
                pos.z   = targetActorPos.z;
                pos.dim = targetActor->getDimensionId();
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
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
            LOG_WRONG_ARGS_COUNT(__FUNCTION__);
            return Local<Value>();
        }

        if (actor->getDimensionId().id != pos.dim) return Number::newNumber(INT_MAX);

        return Number::newNumber(actor->distanceTo(pos.getVec3()));
    }
    CATCH("Fail in distanceTo!")
}

Local<Value> EntityClass::distanceToSqr(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        FloatVec4 pos;

        Actor* actor = get();
        if (!actor) return Local<Value>();

        if (args.size() == 1) {
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Local<Value>();
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Local<Value>();
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<PlayerClass>(args[0]) || IsInstanceOf<EntityClass>(args[0])) {
                // Player or Entity

                Actor* targetActor = EntityClass::tryExtractActor(args[0]);
                if (!targetActor) return Local<Value>();

                Vec3 targetActorPos = targetActor->getPosition();

                pos.x   = targetActorPos.x;
                pos.y   = targetActorPos.y;
                pos.z   = targetActorPos.z;
                pos.dim = targetActor->getDimensionId();
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
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
            LOG_WRONG_ARGS_COUNT(__FUNCTION__);
            return Local<Value>();
        }

        if (actor->getDimensionId().id != pos.dim) return Number::newNumber(INT_MAX);

        return Number::newNumber(actor->distanceToSqr(pos.getVec3()));
    }
    CATCH("Fail in distanceToSqr!")
}

Local<Value> EntityClass::kill(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        entity->kill();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in killEntity!")
}

Local<Value> EntityClass::despawn(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        entity->despawn();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in despawnEntity!")
}

Local<Value> EntityClass::remove(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        entity->remove();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in removeEntity!")
}

Local<Value> EntityClass::isPlayer(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->isType(ActorType::Player));
    }
    CATCH("Fail in isPlayer!")
}

Local<Value> EntityClass::toPlayer(const Arguments&) {
    try {
        if (auto player = mWeakEntity.tryUnwrap<Player>()) {
            return PlayerClass::newPlayer(player);
        }
        return Local<Value>();
    }
    CATCH("Fail in toPlayer!");
}

Local<Value> EntityClass::isItemEntity(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->hasCategory(ActorCategory::Item));
    }
    CATCH("Fail in isItemEntity!")
}

Local<Value> EntityClass::toItem(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity || !entity->hasCategory(ActorCategory::Item)) {
            return Local<Value>();
        } else {
            return ItemClass::newItem(&static_cast<ItemActor*>(entity)->item());
        }
    }
    CATCH("Fail in toItem!");
}

Local<Value> EntityClass::getBlockStandingOn(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return BlockClass::newBlock(entity->getBlockPosCurrentlyStandingOn(nullptr), entity->getDimensionId());
    }
    CATCH("Fail in getBlockStandingOn!");
}

Local<Value> EntityClass::getArmor(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return ContainerClass::newContainer(&ActorEquipment::getArmorContainer(entity->getEntityContext()));
    }
    CATCH("Fail in getArmor!");
}

Local<Value> EntityClass::refreshItems(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        static_cast<Mob*>(entity)->refreshInventory();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in refreshItems!");
}

Local<Value> EntityClass::hasContainer(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        Vec3 pos = entity->getPosition();
        return Boolean::newBoolean(entity->getDimensionBlockSource().tryGetContainer(BlockPos(pos)) ? true : false);
    }
    CATCH("Fail in hasContainer!");
}

Local<Value> EntityClass::getContainer(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        Vec3       pos       = entity->getPosition();
        Container* container = entity->getDimensionBlockSource().tryGetContainer(BlockPos(pos));
        return container ? ContainerClass::newContainer(container) : Local<Value>();
    }
    CATCH("Fail in getContainer!");
}

Local<Value> EntityClass::hurt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        Actor* entity = get();
        if (!entity) {
            return Boolean::newBoolean(false);
        }
        float damage = args[0].asNumber().toFloat();
        int   type   = 0;
        if (args.size() >= 2) {
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            type = args[1].asNumber().toInt32();
        }
        if (args.size() == 3) {
            std::optional<Actor*> source = EntityClass::tryExtractActor(args[2]);
            if (!source) {
                return Boolean::newBoolean(false);
            }
            ActorDamageByActorSource damageBySource =
                ActorDamageByActorSource(*source.value(), (SharedTypes::Legacy::ActorDamageCause)type);
            return Boolean::newBoolean(entity->_hurt(damageBySource, damage, true, false));
        }
        ActorDamageSource damageSource;
        damageSource.mCause = (SharedTypes::Legacy::ActorDamageCause)type;
        return Boolean::newBoolean(entity->_hurt(damageSource, damage, true, false));
    }
    CATCH("Fail in hurt!");
}

Local<Value> EntityClass::heal(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        entity->heal(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in heal!");
}

Local<Value> EntityClass::setHealth(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        MutableAttributeWithContext attribute = entity->getMutableAttribute(SharedAttributes::HEALTH());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setHealth!");
}

Local<Value> EntityClass::setAbsorption(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        MutableAttributeWithContext attribute = entity->getMutableAttribute(SharedAttributes::ABSORPTION());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setAbsorptionAttribute!");
}

Local<Value> EntityClass::setAttackDamage(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        MutableAttributeWithContext attribute = entity->getMutableAttribute(SharedAttributes::ATTACK_DAMAGE());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setAttackDamage!");
}

Local<Value> EntityClass::setMaxAttackDamage(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        MutableAttributeWithContext attribute = entity->getMutableAttribute(SharedAttributes::ATTACK_DAMAGE());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setMaxAttackDamage!");
}

Local<Value> EntityClass::setFollowRange(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        MutableAttributeWithContext attribute = entity->getMutableAttribute(SharedAttributes::FOLLOW_RANGE());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setFollowRange!");
}

Local<Value> EntityClass::setKnockbackResistance(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        MutableAttributeWithContext attribute = entity->getMutableAttribute(SharedAttributes::KNOCKBACK_RESISTANCE());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setKnockbackResistance!");
}

Local<Value> EntityClass::setLuck(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        MutableAttributeWithContext attribute = entity->getMutableAttribute(SharedAttributes::LUCK());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setLuck!");
}

Local<Value> EntityClass::setMovementSpeed(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        MutableAttributeWithContext attribute = entity->getMutableAttribute(SharedAttributes::MOVEMENT_SPEED());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(false);
    }
    CATCH("Fail in setMovementSpeed!");
}

Local<Value> EntityClass::setUnderwaterMovementSpeed(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        MutableAttributeWithContext attribute =
            entity->getMutableAttribute(SharedAttributes::UNDERWATER_MOVEMENT_SPEED());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setUnderwaterMovementSpeed!");
}

Local<Value> EntityClass::setLavaMovementSpeed(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        MutableAttributeWithContext attribute = entity->getMutableAttribute(SharedAttributes::LAVA_MOVEMENT_SPEED());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setLavaMovementSpeed!");
}

Local<Value> EntityClass::setMaxHealth(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        MutableAttributeWithContext attribute = entity->getMutableAttribute(SharedAttributes::HEALTH());
        AttributeHelper::setMaxValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setMaxHealth!");
}

// For Compatibility
Local<Value> EntityClass::setOnFire(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        int time = args[0].asNumber().toInt32();
        entity->setOnFire(time, true);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setOnFire!")
}

Local<Value> EntityClass::setFire(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kBoolean);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        int  time     = args[0].asNumber().toInt32();
        bool isEffect = args[1].asBoolean().value();

        entity->setOnFire(time, isEffect);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setFire!")
}

Local<Value> EntityClass::stopFire(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        entity->stopFire();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in stopFire!")
}

Local<Value> EntityClass::setScale(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        SynchedActorDataAccess::setBoundingBoxScale(entity->getEntityContext(), args[0].asNumber().toFloat());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setScale!")
}

Local<Value> EntityClass::getNbt(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        std::unique_ptr<CompoundTag> tag = std::make_unique<CompoundTag>();
        entity->save(*tag);
        return NbtCompoundClass::pack(std::move(tag));
    }
    CATCH("Fail in getNbt!")
}

Local<Value> EntityClass::setNbt(const Arguments& args) {
    using namespace lse::api;
    CHECK_ARGS_COUNT(args, 1);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        auto nbt = NbtCompoundClass::extract(args[0]);
        if (!nbt) {
            return Local<Value>();
        }

        return Boolean::newBoolean(entity->load(*nbt, MoreGlobal::defaultDataLoadHelper()));
    }
    CATCH("Fail in setNbt!")
}

Local<Value> EntityClass::addTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->addTag(args[0].asString().toString()));
    }
    CATCH("Fail in addTag!");
}

Local<Value> EntityClass::removeTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->removeTag(args[0].asString().toString()));
    }
    CATCH("Fail in removeTag!");
}

Local<Value> EntityClass::hasTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        return Boolean::newBoolean(entity->hasTag(args[0].asString().toString()));
    }
    CATCH("Fail in hasTag!");
}

Local<Value> EntityClass::getAllTags(const Arguments&) {
    try {
        Actor* entity = get();
        if (!entity) return Local<Value>();

        Local<Array> arr       = Array::newArray();
        auto         component = entity->getEntityContext().tryGetComponent<TagsComponent<IDType<LevelTagSetIDType>>>();
        if (component) {
            for (auto& tag : get()->getLevel().getTagRegistry().getTagsInSet(component->mTagSetID)) {
                arr.add(String::newString(tag));
            }
            return arr;
        }
        return Local<Value>();
    }
    CATCH("Fail in getAllTags!");
}

Local<Value> EntityClass::getEntityFromViewVector(const Arguments& args) {

    try {
        Actor* actor = get();
        if (!actor) return Local<Value>();
        float maxDistance = 5.25f;
        if (args.size() > 0) {
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            maxDistance = args[0].asNumber().toFloat();
        }
        HitResult result = actor->traceRay(maxDistance, true, false);
        Actor*    entity = result.getEntity();
        if (result.mType != HitResultType::NoHit && entity) {
            return EntityClass::newEntity(entity);
        }
        return Local<Value>();
    }
    CATCH("Fail in getEntityFromViewVector!");
}

Local<Value> EntityClass::getBlockFromViewVector(const Arguments& args) {
    try {
        Actor* actor = get();
        if (!actor) return Local<Value>();
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
        HitResult res = actor->traceRay(
            maxDistance,
            false,
            true,
            [&solidOnly, &fullOnly, &includeLiquid](BlockSource const&, Block const& block, bool) {
                if (solidOnly && !block.mCachedComponentData->mUnkd6c5eb.as<bool>()) {
                    return false;
                }
                if (fullOnly && !block.isSlabBlock()) {
                    return false;
                }
                if (!includeLiquid && BlockUtils::isLiquidSource(block)) {
                    return false;
                }
                return true;
            }
        );
        if (res.mType == HitResultType::NoHit) {
            return Local<Value>();
        }
        BlockPos bp;
        if (includeLiquid && res.mIsHitLiquid) {
            bp = res.mLiquidPos;
        } else {
            bp = res.mBlock;
        }
        Block const&       bl     = actor->getDimensionBlockSource().getBlock(bp);
        BlockLegacy const& legacy = bl.getLegacyBlock();
        if (bl.isAir() || (legacy.mProperties == BlockProperty::None && legacy.mMaterial.mType == MaterialType::Any)) {
            return Local<Value>();
        }
        return BlockClass::newBlock(bl, bp, actor->getDimensionId());
    }
    CATCH("Fail in getBlockFromViewVector!");
}

Local<Value> EntityClass::quickEvalMolangScript(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        Actor* actor = get();
        if (!actor) return Local<Value>();
        return Number::newNumber(actor->evalMolang(args[0].asString().toString()));
    }
    CATCH("Fail in quickEvalMolangScript!");
}

Local<Value> EntityClass::getBiomeId() {
    try {
        Actor* actor = get();
        if (!actor) return Local<Value>();
        auto& bio = actor->getDimensionBlockSource().getBiome(actor->getFeetBlockPos());
        return Number::newNumber(bio.mId);
    }
    CATCH("Fail in getBiomeId!");
}

Local<Value> EntityClass::getBiomeName() {
    try {
        Actor* actor = get();
        if (!actor) return Local<Value>();
        auto& bio = actor->getDimensionBlockSource().getBiome(actor->getFeetBlockPos());
        return String::newString(bio.mHash->getString());
    }
    CATCH("Fail in getBiomeName!");
}

Local<Value> EntityClass::getAllEffects() {
    try {
        Actor* actor = get();
        if (!actor) {
            return Local<Value>();
        }
        Local<Array> effectList = Array::newArray();
        for (auto& effect : actor->_getAllEffectsNonConst()) {
            effectList.add(Number::newNumber((long long)effect.mId));
        }
        return effectList;
    }
    CATCH("Fail in getAllEffects!")
}

Local<Value> EntityClass::addEffect(const Arguments& args) {
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
        unsigned int      id = args[0].asNumber().toInt32();
        EffectDuration    duration{args[1].asNumber().toInt32()};
        int               level         = args[2].asNumber().toInt32();
        bool              showParticles = args[3].asBoolean().value();
        MobEffectInstance effect(id);
        effect.mDuration      = duration;
        effect.mAmplifier     = level;
        effect.mEffectVisible = showParticles;
        actor->addEffect(effect);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in addEffect!");
}

Local<Value> EntityClass::removeEffect(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        Actor* actor = get();
        if (!actor) {
            return Boolean::newBoolean(false);
        }
        int id = args[0].asNumber().toInt32();
        actor->removeEffect(id);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in removeEffect!");
}

Local<Value> McClass::getAllEntities(const Arguments&) {
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
    CATCH("Fail in GetAllEntities");
}

Local<Value> McClass::getEntities(const Arguments& args) {
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
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
            }
            if (args.size() > 1) {
                if (IsInstanceOf<IntPos>(args[1])) {
                    // IntPos
                    IntPos* posObj = IntPos::extractPos(args[1]);
                    if (dim != posObj->dim) {
                        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Wrong Dimension!");
                        return Local<Value>();
                    }
                    aabb.max = Vec3(posObj->x, posObj->y, posObj->z) + 1;
                    dim      = posObj->dim;

                } else if (IsInstanceOf<FloatPos>(args[1])) {
                    // FloatPos
                    FloatPos* posObj = FloatPos::extractPos(args[1]);
                    if (dim != posObj->dim) {
                        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Wrong Dimension!");
                        return Local<Value>();
                    }
                    aabb.max = posObj->getVec3();
                    dim      = posObj->dim;
                } else if (args[1].getKind() == ValueKind::kNumber) {
                    aabb.max = aabb.min + 1;
                    dis      = args[1].asNumber().toFloat();
                } else {
                    LOG_WRONG_ARG_TYPE(__FUNCTION__);
                    return Local<Value>();
                }
                if (args.size() > 2) {
                    if (args[2].getKind() == ValueKind::kNumber) {
                        dis = args[2].asNumber().toFloat();
                    } else {
                        LOG_WRONG_ARG_TYPE(__FUNCTION__);
                        return Local<Value>();
                    }
                } else {
                    aabb.max = aabb.min + 1;
                }
            } else {
                aabb.max = aabb.min + 1;
            }
        } else {
            LOG_TOO_FEW_ARGS(__FUNCTION__);
            return Local<Value>();
        }

        auto arr       = Array::newArray();
        auto dimension = ll::service::getLevel()->getDimension(dim);
        if (!dimension.lock()) {
            LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "Wrong Dimension!");
            return Local<Value>();
        }
        BlockSource& bs         = dimension.lock()->getBlockSourceFromMainChunkSource();
        auto         entityList = bs.getEntities(aabb, dis);
        for (auto i : entityList) {
            arr.add(EntityClass::newEntity(i));
        }
        return arr;
    }
    CATCH("Fail in getEntities");
}

Local<Value> McClass::getEntity(const Arguments& args) {
    try {
        CHECK_ARGS_COUNT(args, 1)
        CHECK_ARG_TYPE(args[0], ValueKind::kNumber)

        auto level = ll::service::getLevel();

        if (auto* entity = level->fetchEntity(ActorUniqueID(args[0].asNumber().toInt64()), false)) {
            return EntityClass::newEntity(entity);
        }
        if (auto* entity = level->getRuntimeEntity(ActorRuntimeID(args[0].asNumber().toInt64()), false)) {
            return EntityClass::newEntity(entity);
        }
        return Local<Value>();
    }
    CATCH("Fail in getEntity");
}

Local<Value> McClass::cloneMob(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);

    try {
        Actor* ac = EntityClass::extract(args[0]);
        if (!ac) {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>(); // Null
        }

        FloatVec4 pos;

        if (args.size() == 2) {
            if (IsInstanceOf<IntPos>(args[1])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[1]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<FloatPos>(args[1])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[1]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = *posObj;
                }
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
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
            LOG_WRONG_ARGS_COUNT(__FUNCTION__);
            return Local<Value>();
        }
        ActorDefinitionIdentifier id(ac->getTypeName());
        Mob*                      entity = ll::service::getLevel()->getSpawner().spawnMob(
            ll::service::getLevel()->getDimension(pos.dim).lock()->getBlockSourceFromMainChunkSource(),
            id,
            nullptr,
            pos.getVec3(),
            false,
            true,
            false
        );
        if (!entity) return Local<Value>(); // Null
        else return EntityClass::newEntity(entity);
    }
    CATCH("Fail in CloneMob!");
}

Local<Value> McClass::spawnMob(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        std::string name = args[0].asString().toString();
        FloatVec4   pos;

        if (args.size() == 2) {
            if (IsInstanceOf<IntPos>(args[1])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[1]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<FloatPos>(args[1])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[1]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = *posObj;
                }
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
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
            LOG_WRONG_ARGS_COUNT(__FUNCTION__);
            return Local<Value>();
        }

        ActorDefinitionIdentifier id(name);
        Mob*                      entity = ll::service::getLevel()->getSpawner().spawnMob(
            ll::service::getLevel()->getDimension(pos.dim).lock()->getBlockSourceFromMainChunkSource(),
            id,
            nullptr,
            pos.getVec3(),
            false,
            true,
            false
        );
        if (!entity) return Local<Value>(); // Null
        else return EntityClass::newEntity(entity);
    }
    CATCH("Fail in SpawnMob!");
}

Local<Value> McClass::explode(const Arguments& args) {
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
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = *posObj;
                }
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
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
            LOG_WRONG_ARGS_COUNT(__FUNCTION__);
            return Local<Value>();
            break;
        }
        std::optional<Actor*> source = EntityClass::tryExtractActor(args[beginIndex]); // Can be nullptr

        if (args.size() == 5 || args.size() == 8) {
            CHECK_ARG_TYPE(args[beginIndex + 1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[beginIndex + 2], ValueKind::kBoolean);
            CHECK_ARG_TYPE(args[beginIndex + 3], ValueKind::kBoolean);

            float radius    = args[beginIndex + 1].asNumber().toFloat();
            bool  isDestroy = args[beginIndex + 2].asBoolean().value();
            bool  isFire    = args[beginIndex + 3].asBoolean().value();

            return Boolean::newBoolean(ll::service::getLevel()->explode(
                ll::service::getLevel()->getDimension(pos.dim).lock()->getBlockSourceFromMainChunkSource(),
                source.value_or(nullptr),
                pos.getVec3(),
                radius,
                isFire,
                isDestroy,
                FLT_MAX,
                false
            ));
        } else {
            CHECK_ARG_TYPE(args[beginIndex + 1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[beginIndex + 2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[beginIndex + 3], ValueKind::kBoolean);
            CHECK_ARG_TYPE(args[beginIndex + 4], ValueKind::kBoolean);

            float maxResistance = args[beginIndex + 1].asNumber().toFloat();
            float radius        = args[beginIndex + 2].asNumber().toFloat();
            bool  isDestroy     = args[beginIndex + 3].asBoolean().value();
            bool  isFire        = args[beginIndex + 4].asBoolean().value();

            return Boolean::newBoolean(ll::service::getLevel()->explode(
                ll::service::getLevel()->getDimension(pos.dim).lock()->getBlockSourceFromMainChunkSource(),
                source.value_or(nullptr),
                pos.getVec3(),
                radius,
                isFire,
                isDestroy,
                maxResistance,
                false
            ));
        }
    }
    CATCH("Fail in Explode!");
}
