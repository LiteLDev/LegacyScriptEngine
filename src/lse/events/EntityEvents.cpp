#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include "ll/api/service/Bedrock.h"
#include "lse/api/Thread.h"
#include "lse/api/helper/BlockHelper.h"
#include "mc/common/Globals.h"
#include "mc/deps/shared_types/legacy/actor/ActorDamageCause.h"
#include "mc/entity/components_json_legacy/NpcComponent.h"
#include "mc/entity/components_json_legacy/ProjectileComponent.h"
#include "mc/entity/components_json_legacy/TransformationComponent.h"
#include "mc/legacy/ActorUniqueID.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/ActorDefinitionIdentifier.h"
#include "mc/world/actor/ActorFactory.h"
#include "mc/world/actor/ActorHurtResult.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/VanillaActorRendererId.h"
#include "mc/world/actor/boss/WitherBoss.h"
#include "mc/world/actor/item/FireworksRocketActor.h"
#include "mc/world/actor/npc/CommandAction.h"
#include "mc/world/actor/npc/StoredCommand.h"
#include "mc/world/actor/npc/UrlAction.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/effect/MobEffectInstance.h"
#include "mc/world/events/ActorEventCoordinator.h"
#include "mc/world/item/CrossbowItem.h"
#include "mc/world/item/ItemInstance.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/TridentItem.h"
#include "mc/world/level/BedrockSpawner.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/PortalBlock.h"
#include "mc/world/phys/AABB.h"
#include "mc/world/phys/HitResult.h"

#include <utility>

namespace lse::events::entity {

using api::thread::isServerThread;

LL_TYPE_INSTANCE_HOOK(
    ProjectileSpawnHook1,
    HookPriority::Normal,
    BedrockSpawner,
    &BedrockSpawner::$spawnProjectile,
    Actor*,
    BlockSource&                     region,
    ActorDefinitionIdentifier const& id,
    Actor*                           spawner,
    Vec3 const&                      position,
    Vec3 const&                      direction
) {
    IF_LISTENED(EVENT_TYPES::onSpawnProjectile) {
        if (isServerThread()) {
            static auto& tridentName = EntityCanonicalName(ActorType::Trident);
            if (*id.mCanonicalName != tridentName) {
                if (!CallEvent(
                        EVENT_TYPES::onSpawnProjectile,
                        EntityClass::newEntity(spawner),
                        String::newString(id.mCanonicalName->getString())
                    )) {
                    return nullptr;
                }
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onSpawnProjectile);
    Actor* projectile = origin(region, id, spawner, position, direction);
    IF_LISTENED(EVENT_TYPES::onProjectileCreated) {
        if (isServerThread()) {
            CallEvent( // Not nancellable
            EVENT_TYPES::onProjectileCreated,
            EntityClass::newEntity(spawner),
            EntityClass::newEntity(projectile)
        );
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onProjectileCreated);
    return projectile;
}

LL_TYPE_INSTANCE_HOOK(
    ProjectileSpawnHook2,
    HookPriority::Normal,
    TridentItem,
    &TridentItem::$releaseUsing,
    void,
    ItemStack& item,
    Player*    player,
    int        durationLeft
) {
    IF_LISTENED(EVENT_TYPES::onSpawnProjectile) {
        if (isServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onSpawnProjectile,
                    EntityClass::newEntity(player),
                    String::newString(item.getTypeName())
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onSpawnProjectile);
    origin(item, player, durationLeft);
}

std::pair<Vec3, std::pair<Actor*, HashedString>> currentFirerocketSpawner;

LL_TYPE_INSTANCE_HOOK(
    FireworksRocketSpawnHook1,
    HookPriority::Normal,
    ActorFactory,
    &ActorFactory::createSpawnedActor,
    ::OwnerPtr<::EntityContext>,
    ::ActorDefinitionIdentifier const& identifier,
    ::Actor*                           spawner,
    ::Vec3 const&                      position,
    ::Vec2 const&                      rotation
) {
    currentFirerocketSpawner = {
        position,
        {spawner, identifier.mCanonicalName}
    };
    return origin(identifier, spawner, position, rotation);
}

LL_TYPE_INSTANCE_HOOK(
    FireworksRocketSpawnHook2,
    HookPriority::Normal,
    FireworksRocketActor,
    &FireworksRocketActor::init,
    void,
    ::Level&             level,
    ::Vec3 const&        playerPos,
    ::CompoundTag const& rocketUserData,
    ::Vec3 const&        dir,
    ::ActorUniqueID      attachedEntity,
    bool                 isProjectile
) {
    IF_LISTENED(EVENT_TYPES::onSpawnProjectile) {
        if (isServerThread() && isProjectile) {
            if (playerPos == currentFirerocketSpawner.first) {
                if (!CallEvent(
                        EVENT_TYPES::onSpawnProjectile,
                        EntityClass::newEntity(currentFirerocketSpawner.second.first),
                        String::newString(currentFirerocketSpawner.second.second.getString())
                    )) {
                    return;
                }
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onSpawnProjectile);
    origin(level, playerPos, rocketUserData, dir, attachedEntity, isProjectile);
}

LL_TYPE_STATIC_HOOK(
    PortalTrySpawnPigZombieHook,
    HookPriority::Normal,
    PortalBlock,
    &PortalBlock::trySpawnPigZombie,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    PortalAxis      axis
) {
    IF_LISTENED(EVENT_TYPES::onPortalTrySpawnPigZombie) {
        if (isServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onPortalTrySpawnPigZombie,
                    IntPos::newPos(pos, region.getDimensionId()),
                    Number::newNumber(static_cast<int>(axis))
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onPortalTrySpawnPigZombie);
    origin(region, pos, axis);
}

LL_TYPE_INSTANCE_HOOK(ActorRideHook, HookPriority::Normal, Actor, &Actor::$canAddPassenger, bool, Actor& passenger) {
    IF_LISTENED(EVENT_TYPES::onRide) {
        if (isServerThread()) {
            if (!CallEvent(EVENT_TYPES::onRide, EntityClass::newEntity(&passenger), EntityClass::newEntity(this))) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onRide);
    return origin(passenger);
}

LL_TYPE_INSTANCE_HOOK(
    WitherDestroyHook,
    HookPriority::Normal,
    WitherBoss,
    &WitherBoss::_destroyBlocks,
    void,
    Level&                       level,
    AABB const&                  bb,
    BlockSource&                 region,
    int                          range,
    WitherBoss::WitherAttackType type
) {
    IF_LISTENED(EVENT_TYPES::onWitherBossDestroy) {
        if (isServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onWitherBossDestroy,
                    EntityClass::newEntity(this),
                    IntPos::newPos(bb.min, region.getDimensionId()),
                    IntPos::newPos(bb.max, region.getDimensionId())
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onWitherBossDestroy);
    origin(level, bb, region, range, type);
}

LL_TYPE_INSTANCE_HOOK(
    ProjectileHitEntityHook,
    HookPriority::Normal,
    ProjectileComponent,
    &ProjectileComponent::onHit,
    void,
    Actor&           owner,
    HitResult const& res
) {
    IF_LISTENED(EVENT_TYPES::onProjectileHitEntity) {
        if (isServerThread() && res.getEntity()) {
            if (!CallEvent(
                    EVENT_TYPES::onProjectileHitEntity,
                    EntityClass::newEntity(res.getEntity()),
                    EntityClass::newEntity(&owner)
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onProjectileHitEntity);
    origin(owner, res);
}

LL_TYPE_INSTANCE_HOOK(
    ProjectileHitBlockHook,
    HookPriority::Normal,
    ProjectileComponent,
    &ProjectileComponent::onHit,
    void,
    ::Actor&           owner,
    ::HitResult const& res
) {
    IF_LISTENED(EVENT_TYPES::onProjectileHitBlock) {
        if (isServerThread()) {
            auto& region = owner.getDimensionBlockSource();
            auto& block  = region.getBlock(res.mBlock);
            if (res.mType == HitResultType::Tile && res.mBlock != BlockPos::ZERO() && !block.isAir()) {
                if (!CallEvent(
                        EVENT_TYPES::onProjectileHitBlock,
                        BlockClass::newBlock(block, res.mBlock, region),
                        EntityClass::newEntity(&owner)
                    )) {
                    return;
                }
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onProjectileHitBlock);
    return origin(owner, res);
}

LL_TYPE_INSTANCE_HOOK(
    MobHurtHook,
    HookPriority::Normal,
    Mob,
    &Mob::$_hurt,
    ActorHurtResult,
    ::ActorDamageSource const& source,
    float                      damage,
    ::HurtParameters const&    hurtParameters
) {
    IF_LISTENED(EVENT_TYPES::onMobHurt) {
        if (isServerThread()) {
            // LeviLamina's ActorHurtEvent can't handle fire hurt, so we just hook Mob::$_hurt.
            Actor* damageSource = nullptr;
            if (source.isEntitySource()) {
                if (source.isChildEntitySource()) {
                    damageSource = ll::service::getLevel()->fetchEntity(source.getEntityUniqueID(), false);
                } else {
                    damageSource = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
                }
            }

            if (!CallEvent(
                    EVENT_TYPES::onMobHurt,
                    EntityClass::newEntity(this),
                    damageSource ? EntityClass::newEntity(damageSource) : Local<Value>(),
                    Number::newNumber(damage < 0.0f ? -damage : damage),
                    Number::newNumber(static_cast<int>(source.mCause))
                )) {
                return {false, false};
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onMobHurt)
    return origin(source, damage, hurtParameters);
}

LL_TYPE_INSTANCE_HOOK(
    MobHurtEffectHook,
    HookPriority::Normal,
    Mob,
    &Mob::getDamageAfterResistanceEffect,
    float,
    ::ActorDamageSource const& source,
    float                      damage
) {
    IF_LISTENED(EVENT_TYPES::onMobHurt) {
        if (isServerThread()) {
            // Mob is still hurt after hook Mob::$hurtEffects, and all hurt events are handled by this function, but we
            // just need magic damage.
            if (source.mCause == SharedTypes::Legacy::ActorDamageCause::Magic
                || source.mCause == SharedTypes::Legacy::ActorDamageCause::Wither) {
                Actor* damageSource = nullptr;
                if (source.isEntitySource()) {
                    if (source.isChildEntitySource()) {
                        damageSource = ll::service::getLevel()->fetchEntity(source.getEntityUniqueID(), false);
                    } else {
                        damageSource = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
                    }
                }

                if (!CallEvent(
                        EVENT_TYPES::onMobHurt,
                        EntityClass::newEntity(this),
                        damageSource ? EntityClass::newEntity(damageSource) : Local<Value>(),
                        Number::newNumber(damage < 0.0f ? -damage : damage),
                        Number::newNumber(static_cast<int>(source.mCause))
                    )) {
                    return 0.0f;
                }
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onMobHurt)
    return origin(source, damage);
}

LL_TYPE_INSTANCE_HOOK(
    NpcCommandHook,
    HookPriority::Normal,
    NpcComponent,
    &NpcComponent::executeCommandAction,
    void,
    ::Actor&             owner,
    ::Player&            sourcePlayer,
    int                  actionIndex,
    ::std::string const& sceneName
) {
    IF_LISTENED(EVENT_TYPES::onNpcCmd) {
        if (isServerThread()) {
            auto& action =
                owner.getEntityContext().tryGetComponent<NpcComponent>()->mActionsContainer->mActions->at(actionIndex);
            if (std::holds_alternative<npc::CommandAction>(action)) {
                auto&       commands = std::get<npc::CommandAction>(action).commands;
                std::string command;
                for (auto& cmd : commands.get()) {
                    command += cmd.rawCommand.get() + ";";
                }
                if (!commands->empty()) {
                    command.pop_back();
                }
                if (!CallEvent(
                        EVENT_TYPES::onNpcCmd,
                        EntityClass::newEntity(&owner),
                        PlayerClass::newPlayer(&sourcePlayer),
                        String::newString(command)
                    )) {
                    return;
                }
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onNpcCmd);
    origin(owner, sourcePlayer, actionIndex, sceneName);
}

LL_TYPE_INSTANCE_HOOK(
    EffectUpdateHook,
    HookPriority::Normal,
    Actor,
    &Actor::onEffectUpdated,
    void,
    MobEffectInstance& effect
) {
    IF_LISTENED(EVENT_TYPES::onEffectUpdated) {
        if (isServerThread() && isPlayer()) {
            if (!CallEvent(
                    EVENT_TYPES::onEffectUpdated,
                    PlayerClass::newPlayer(reinterpret_cast<Player*>(this)),
                    String::newString(MobEffect::mMobEffects()[effect.mId]->mComponentName->getString()),
                    Number::newNumber(effect.mAmplifier),
                    Number::newNumber(effect.mDuration->mValue)
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onEffectUpdated);
    origin(effect);
}

LL_TYPE_INSTANCE_HOOK(
    TransformationHook,
    HookPriority::Normal,
    TransformationComponent,
    &TransformationComponent::maintainOldData,
    void,
    ::Actor&                           originalActor,
    ::Actor&                           transformed,
    ::TransformationDescription const& transformation,
    ::ActorUniqueID const&             ownerID,
    ::Level const&                     level
) {
    IF_LISTENED(EVENT_TYPES::onEntityTransformation) {
        if (isServerThread()) {
            CallEvent(
                EVENT_TYPES::onEntityTransformation,
                String::newString(std::to_string(originalActor.getOrCreateUniqueID().rawID)),
                EntityClass::newEntity(&transformed)
            );
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onEntityTransformation);

    origin(originalActor, transformed, transformation, ownerID, level);
}

LL_TYPE_INSTANCE_HOOK(
    EndermanTakeBlockHook,
    HookPriority::Normal,
    ActorEventCoordinator,
    &ActorEventCoordinator::sendEvent,
    void,
    EventRef<ActorGameplayEvent<void>> const& event
) {

    IF_LISTENED(EVENT_TYPES::onEndermanTakeBlock) {
        if (isServerThread()) {
            bool canceled = event.get().visit([&]<typename T0>(T0&& arg) {
                if constexpr (std::is_same_v<std::decay_t<T0>, Details::ValueOrRef<ActorGriefingBlockEvent const>>) {
                    auto& griefingEvent = arg.value();
                    auto  entity        = griefingEvent.mActorContext->tryUnwrap();
                    if (entity && entity->isType(ActorType::EnderMan)) {
                        if (!CallEvent(
                                EVENT_TYPES::onEndermanTakeBlock,
                                EntityClass::newEntity(entity.as_ptr()),
                                BlockClass::newBlock(
                                    *griefingEvent.mBlock,
                                    BlockPos(griefingEvent.mPos),
                                    entity->getDimensionId()
                                ),
                                IntPos::newPos(BlockPos(griefingEvent.mPos), entity->getDimensionId())
                            )) {
                            return true;
                        }
                    }
                }
                return false;
            });
            if (canceled) return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onEndermanTakeBlock);

    return origin(event);
}

void ProjectileSpawnEvent() {
    static ll::memory::
        HookRegistrar<ProjectileSpawnHook1, ProjectileSpawnHook2, FireworksRocketSpawnHook1, FireworksRocketSpawnHook2>
            reg;
};
void PortalTrySpawnPigZombieEvent() { static ll::memory::HookRegistrar<PortalTrySpawnPigZombieHook> reg; }
void ProjectileCreatedEvent() { static ll::memory::HookRegistrar<ProjectileSpawnHook1> reg; };
void ActorRideEvent() { static ll::memory::HookRegistrar<ActorRideHook> reg; }
void WitherDestroyEvent() { static ll::memory::HookRegistrar<WitherDestroyHook> reg; }
void ProjectileHitEntityEvent() { static ll::memory::HookRegistrar<ProjectileHitEntityHook> reg; }
void ProjectileHitBlockEvent() { static ll::memory::HookRegistrar<ProjectileHitBlockHook> reg; }
void MobHurtEvent() { static ll::memory::HookRegistrar<MobHurtHook, MobHurtEffectHook> reg; }
void NpcCommandEvent() { static ll::memory::HookRegistrar<NpcCommandHook> reg; }
void EndermanTakeBlockEvent() { static ll::memory::HookRegistrar<EndermanTakeBlockHook> reg; }
void EffectUpdateEvent() { static ll::memory::HookRegistrar<EffectUpdateHook> reg; }
void TransformationEvent() { static ll::memory::HookRegistrar<TransformationHook> reg; }
} // namespace lse::events::entity
