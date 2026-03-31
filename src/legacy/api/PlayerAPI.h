#pragma once
#include "legacy/api/APIHelp.h"
#include "mc/deps/ecs/WeakEntityRef.h"

class SimulatedPlayer;

//////////////////// Classes ////////////////////
class Player;
class PlayerClass : public ScriptClass {
    WeakRef<EntityContext> mWeakEntity;
    bool                   mValid;

public:
    explicit PlayerClass(Player const* player);

    Player*          get() const;
    SimulatedPlayer* asSimulatedPlayer() const;

    static Local<Object> newPlayer(Player const* p);
    static Player*       extract(Local<Value> const& v);

    Local<Value> getName() const;
    Local<Value> getPos() const;
    Local<Value> getFeetPos() const;
    Local<Value> getBlockPos() const;
    Local<Value> getLastDeathPos() const;
    Local<Value> getRealName() const;
    Local<Value> getXuid() const;
    Local<Value> getUuid() const;
    Local<Value> getPermLevel() const;
    Local<Value> getGameMode() const;
    Local<Value> getCanSleep() const;
    Local<Value> getCanFly() const;
    Local<Value> getCanBeSeenOnMap() const;
    Local<Value> getCanFreeze() const;
    Local<Value> getCanSeeDaylight() const;
    Local<Value> getCanShowNameTag() const;
    Local<Value> getCanStartSleepInBed() const;
    Local<Value> getCanPickupItems() const;
    Local<Value> getMaxHealth() const;
    Local<Value> getHealth() const;
    Local<Value> getInAir() const;
    Local<Value> getInWater() const;
    Local<Value> getInLava() const;
    Local<Value> getInRain() const;
    Local<Value> getInSnow() const;
    Local<Value> getInWall() const;
    Local<Value> getInWaterOrRain() const;
    Local<Value> getInWorld() const;
    Local<Value> getInClouds() const;
    Local<Value> getSpeed() const;
    Local<Value> getDirection() const;
    Local<Value> getUniqueID() const;
    Local<Value> getRuntimeID() const;
    Local<Value> getLangCode() const;
    Local<Value> getBiomeName() const;
    Local<Value> getBiomeId() const;
    Local<Value> isLoading() const;
    Local<Value> isInvisible() const;
    Local<Value> isInsidePortal() const;
    Local<Value> isHurt() const;
    Local<Value> isTrusting() const;
    Local<Value> isTouchingDamageBlock() const;
    Local<Value> isHungry() const;
    Local<Value> isOnFire() const;
    Local<Value> isOnGround() const;
    Local<Value> isOnHotBlock() const;
    Local<Value> isTrading() const;
    Local<Value> isAdventure() const;
    Local<Value> isGliding() const;
    Local<Value> isSurvival() const;
    Local<Value> isSpectator() const;
    Local<Value> isRiding() const;
    Local<Value> isDancing() const;
    Local<Value> isCreative() const;
    Local<Value> isFlying() const;
    Local<Value> isSleeping() const;
    Local<Value> isMoving() const;
    Local<Value> isSneaking() const;

    Local<Value> isOP(Arguments const& args) const;
    Local<Value> setPermLevel(Arguments const& args) const;
    Local<Value> setGameMode(Arguments const& args) const;

    Local<Value> runcmd(Arguments const& args) const;
    Local<Value> teleport(Arguments const& args) const;
    Local<Value> kill(Arguments const& args) const;
    Local<Value> kick(Arguments const& args) const;
    Local<Value> tell(Arguments const& args) const;
    Local<Value> setTitle(Arguments const& args) const;
    Local<Value> talkAs(Arguments const& args) const;
    Local<Value> talkTo(Arguments const& args) const;
    Local<Value> rename(Arguments const& args) const;
    Local<Value> transServer(Arguments const& args) const;
    Local<Value> crash(Arguments const& args) const;
    Local<Value> hurt(Arguments const& args) const;
    Local<Value> heal(Arguments const& args) const;
    Local<Value> setHealth(Arguments const& args) const;
    Local<Value> setMaxHealth(Arguments const& args) const;
    Local<Value> setAbsorption(Arguments const& args) const;
    Local<Value> setAttackDamage(Arguments const& args) const;
    Local<Value> setMaxAttackDamage(Arguments const& args) const;
    Local<Value> setFollowRange(Arguments const& args) const;
    Local<Value> setKnockbackResistance(Arguments const& args) const;
    Local<Value> setLuck(Arguments const& args) const;
    Local<Value> setMovementSpeed(Arguments const& args) const;
    Local<Value> setUnderwaterMovementSpeed(Arguments const& args) const;
    Local<Value> setLavaMovementSpeed(Arguments const& args) const;
    Local<Value> setHungry(Arguments const& args) const;
    Local<Value> setOnFire(Arguments const& args) const;
    Local<Value> setFire(Arguments const& args) const;
    Local<Value> stopFire(Arguments const& args) const;
    Local<Value> refreshChunks(Arguments const& args) const;
    Local<Value> giveItem(Arguments const& args) const;
    Local<Value> clearItem(Arguments const& args) const;
    Local<Value> isSprinting(Arguments const& args) const;
    Local<Value> setSprinting(Arguments const& args) const;
    Local<Value> sendToast(Arguments const& args) const;
    Local<Value> distanceTo(Arguments const& args) const;
    Local<Value> distanceToSqr(Arguments const& args) const;

    Local<Value> getBlockStandingOn(Arguments const& args) const;
    Local<Value> getDevice(Arguments const& args) const;
    Local<Value> getHand(Arguments const& args) const;
    Local<Value> getOffHand(Arguments const& args) const;
    Local<Value> getInventory(Arguments const& args) const;
    Local<Value> getArmor(Arguments const& args) const;
    Local<Value> getEnderChest(Arguments const& args) const;
    Local<Value> getRespawnPosition(Arguments const& args) const;
    Local<Value> setRespawnPosition(Arguments const& args) const;
    Local<Value> refreshItems(Arguments const& args) const;

    Local<Value> getScore(Arguments const& args) const;
    Local<Value> setScore(Arguments const& args) const;
    Local<Value> addScore(Arguments const& args) const;
    Local<Value> reduceScore(Arguments const& args) const;
    Local<Value> deleteScore(Arguments const& args) const;
    Local<Value> setSidebar(Arguments const& args) const;
    Local<Value> removeSidebar(Arguments const& args) const;
    Local<Value> setBossBar(Arguments const& args) const;
    Local<Value> removeBossBar(Arguments const& args) const;
    Local<Value> addLevel(Arguments const& args) const;
    Local<Value> reduceLevel(Arguments const& args) const;
    Local<Value> getLevel(Arguments const& arg) const;
    Local<Value> setLevel(Arguments const& arg) const;
    Local<Value> resetLevel(Arguments const& arg) const;
    Local<Value> setScale(Arguments const& arg) const;
    Local<Value> addExperience(Arguments const& args) const;
    Local<Value> reduceExperience(Arguments const& args) const;
    Local<Value> getCurrentExperience(Arguments const& arg) const;
    Local<Value> setCurrentExperience(Arguments const& arg) const;
    Local<Value> getTotalExperience(Arguments const& arg) const;
    Local<Value> setTotalExperience(Arguments const& arg) const;
    Local<Value> getXpNeededForNextLevel(Arguments const& arg) const;

    Local<Value> sendSimpleForm(Arguments const& args) const;
    Local<Value> sendModalForm(Arguments const& args) const;
    Local<Value> sendCustomForm(Arguments const& args) const;
    Local<Value> sendForm(Arguments const& args) const;
    Local<Value> closeForm(Arguments const& args) const;
    Local<Value> sendPacket(Arguments const& args) const;

    Local<Value> setExtraData(Arguments const& args) const;
    Local<Value> getExtraData(Arguments const& args) const;
    Local<Value> delExtraData(Arguments const& args) const;

    Local<Value> getNbt(Arguments const& args) const;
    Local<Value> setNbt(Arguments const& args) const;
    Local<Value> addTag(Arguments const& args) const;
    Local<Value> hasTag(Arguments const& args) const;
    Local<Value> removeTag(Arguments const& args) const;
    Local<Value> getAllTags(Arguments const& args) const;
    Local<Value> getAbilities(Arguments const& args) const;
    Local<Value> getAttributes(Arguments const& args) const;
    Local<Value> getEntityFromViewVector(Arguments const& args) const;
    Local<Value> getBlockFromViewVector(Arguments const& args) const;

    Local<Value> isSimulatedPlayer(Arguments const& args) const;
    Local<Value> quickEvalMolangScript(Arguments const& args) const;

    Local<Value> getAllEffects() const;
    Local<Value> addEffect(Arguments const& args) const;
    Local<Value> removeEffect(Arguments const& args) const;

    // LLMoney

    Local<Value> getMoney(Arguments const& args) const;
    Local<Value> setMoney(Arguments const& args) const;
    Local<Value> reduceMoney(Arguments const& args) const;
    Local<Value> addMoney(Arguments const& args) const;
    Local<Value> transMoney(Arguments const& args) const;
    Local<Value> getMoneyHistory(Arguments const& args) const;

    // SimulatedPlayer API (API/SimulatedPlayerAPI.cpp)

    Local<Value> simulateRespawn(Arguments const& args) const;
    Local<Value> simulateSneak(Arguments const& args) const;
    Local<Value> simulateAttack(Arguments const& args) const;
    Local<Value> simulateDestroy(Arguments const& args);
    Local<Value> simulateDisconnect(Arguments const& args) const;
    Local<Value> simulateInteract(Arguments const& args);
    Local<Value> simulateJump(Arguments const& args) const;
    Local<Value> simulateLocalMove(Arguments const& args);
    Local<Value> simulateWorldMove(Arguments const& args);
    Local<Value> simulateMoveTo(Arguments const& args);
    Local<Value> simulateLookAt(Arguments const& args) const;
    Local<Value> simulateSetBodyRotation(Arguments const& args) const;
    Local<Value> simulateNavigateTo(Arguments const& args);
    Local<Value> simulateUseItem(Arguments const& args) const;
    Local<Value> simulateStopDestroyingBlock(Arguments const& args) const;
    Local<Value> simulateStopInteracting(Arguments const& args) const;
    Local<Value> simulateStopMoving(Arguments const& args) const;
    Local<Value> simulateStopUsingItem(Arguments const& args) const;
    Local<Value> simulateStopSneaking(Arguments const& args) const;

    // bool simulateSetItem(class ItemStack&, bool, int);
    // bool simulateGiveItem(class ItemStack&, bool);

    // For Compatibility
    Local<Value> getIP() const;
    Local<Value> getAllItems(Arguments const& args) const;
    Local<Value> removeItem(Arguments const& args) const;

    Local<Value> setAbility(Arguments const& args) const;

    Local<Value> toEntity(Arguments const& args) const;
};
extern ClassDefine<PlayerClass> PlayerClassBuilder;
