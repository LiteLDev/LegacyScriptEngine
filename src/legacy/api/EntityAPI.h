#pragma once
#include "api/APIHelp.h"
#include "mc/deps/ecs/WeakEntityRef.h"

//////////////////// Classes ////////////////////
class Actor;
class EntityClass : public ScriptClass {
private:
    WeakRef<EntityContext> mWeakEntity;
    bool                   mValid = false;

public:
    explicit EntityClass(Actor const* actor);

    Actor* get() const;

    static Local<Object> newEntity(Actor const* actor);
    static Actor*        extract(Local<Value> const& v);
    static Actor*        tryExtractActor(Local<Value> const& v);

    Local<Value> getName() const;
    Local<Value> getType() const;
    Local<Value> getId() const;
    Local<Value> getPos() const;
    Local<Value> getPosDelta() const;
    Local<Value> getFeetPos() const;
    Local<Value> getBlockPos() const;
    Local<Value> getMaxHealth() const;
    Local<Value> getHealth() const;
    Local<Value> getCanFly() const;
    Local<Value> getCanFreeze() const;
    Local<Value> getCanSeeDaylight() const;
    Local<Value> getCanPickupItems() const;
    Local<Value> getInAir() const;
    Local<Value> getInWater() const;
    Local<Value> getInClouds() const;
    Local<Value> getInLava() const;
    Local<Value> getInRain() const;
    Local<Value> getInSnow() const;
    Local<Value> getInWall() const;
    Local<Value> getInWaterOrRain() const;
    Local<Value> getInWorld() const;
    Local<Value> getSpeed() const;
    Local<Value> getBiomeName() const;
    Local<Value> getBiomeId() const;
    Local<Value> getDirection() const;
    Local<Value> getUniqueID() const;
    Local<Value> getRuntimeID() const;
    Local<Value> isInvisible() const;
    Local<Value> isInsidePortal() const;
    Local<Value> isTrusting() const;
    Local<Value> isTouchingDamageBlock() const;
    Local<Value> isOnFire() const;
    Local<Value> isOnGround() const;
    Local<Value> isOnHotBlock() const;
    Local<Value> isTrading() const;
    Local<Value> isRiding() const;
    Local<Value> isDancing() const;
    Local<Value> isSleeping() const;
    Local<Value> isAngry() const;
    Local<Value> isBaby() const;
    Local<Value> isMoving() const;

    Local<Value> setPosDelta(Arguments const& args) const;
    Local<Value> teleport(Arguments const& args) const;
    Local<Value> kill(Arguments const& args) const;
    Local<Value> despawn(Arguments const& args) const;
    Local<Value> remove(Arguments const& args) const;
    Local<Value> hurt(Arguments const& args) const;
    Local<Value> heal(Arguments const& args) const;
    Local<Value> setHealth(Arguments const& args) const;
    Local<Value> setAbsorption(Arguments const& args) const;
    Local<Value> setAttackDamage(Arguments const& args) const;
    Local<Value> setMaxAttackDamage(Arguments const& args) const;
    Local<Value> setFollowRange(Arguments const& args) const;
    Local<Value> setKnockbackResistance(Arguments const& args) const;
    Local<Value> setLuck(Arguments const& args) const;
    Local<Value> setMovementSpeed(Arguments const& args) const;
    Local<Value> setUnderwaterMovementSpeed(Arguments const& args) const;
    Local<Value> setLavaMovementSpeed(Arguments const& args) const;
    Local<Value> setMaxHealth(Arguments const& args) const;
    Local<Value> setOnFire(Arguments const& args) const;
    Local<Value> setFire(Arguments const& args) const;
    Local<Value> stopFire(Arguments const& args) const;
    Local<Value> setScale(Arguments const& args) const;

    Local<Value> distanceTo(Arguments const& args) const;
    Local<Value> distanceToSqr(Arguments const& args) const;

    Local<Value> isPlayer(Arguments const& args) const;
    Local<Value> toPlayer(Arguments const& args) const;
    Local<Value> isItemEntity(Arguments const& args) const;
    Local<Value> toItem(Arguments const& args) const;
    Local<Value> getBlockStandingOn(Arguments const& args) const;
    Local<Value> getArmor(Arguments const& args) const;
    Local<Value> hasContainer(Arguments const& args) const;
    Local<Value> getContainer(Arguments const& args) const;
    Local<Value> refreshItems(Arguments const& args) const;

    Local<Value> getNbt(Arguments const& args) const;
    Local<Value> setNbt(Arguments const& args) const;
    Local<Value> addTag(Arguments const& args) const;
    Local<Value> removeTag(Arguments const& args) const;
    Local<Value> hasTag(Arguments const& args) const;
    Local<Value> getAllTags(Arguments const& args) const;
    Local<Value> getEntityFromViewVector(Arguments const& args) const;
    Local<Value> getBlockFromViewVector(Arguments const& args) const;

    Local<Value> quickEvalMolangScript(Arguments const& args) const;

    Local<Value> getAllEffects() const;
    Local<Value> addEffect(Arguments const& args) const;
    Local<Value> removeEffect(Arguments const& args) const;
};
extern ClassDefine<EntityClass> EntityClassBuilder;
