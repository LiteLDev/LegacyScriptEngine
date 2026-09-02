#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "lse/api/Thread.h"
#include "mc/common/Globals.h"
#include "mc/deps/shared_types/legacy/actor/ActorDamageCause.h"
#include "mc/entity/components_json_legacy/NpcComponent.h"
#include "mc/entity/components_json_legacy/ProjectileComponent.h"
#include "mc/entity/components_json_legacy/TransformationComponent.h"
#include "mc/events/MinecraftEventing.h"
#include "mc/legacy/ActorUniqueID.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/ActorDefinitionIdentifier.h"
#include "mc/world/actor/ActorFactory.h"
#include "mc/world/actor/ActorHurtResult.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/SpawnChecks.h"
#include "mc/world/actor/VanillaActorRendererId.h"
#include "mc/world/actor/boss/WitherBoss.h"
#include "mc/world/actor/item/FireworksRocketActor.h"
#include "mc/world/actor/npc/CommandAction.h"
#include "mc/world/actor/npc/StoredCommand.h"
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
#include "mc/world/level/PortalShape.h"
#include "mc/world/level/block/PortalBlock.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/phys/AABB.h"
#include "mc/world/phys/HitResult.h"

#include <optional>
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

namespace FireworksRocketSpawn {
std::pair<Vec3, std::pair<Actor*, HashedString>> currentFirerocketSpawner;
LL_TYPE_INSTANCE_HOOK(
    Hook1,
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
    Hook2,
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
} // namespace FireworksRocketSpawn

namespace PortalTrySpawnPigZombie {
std::pair<BlockPos, PortalAxis> currentPortal;
LL_TYPE_INSTANCE_HOOK(
    PortalShapeEvalueateHook,
    HookPriority::Normal,
    PortalShape,
    &PortalShape::evaluate,
    void,
    ::BlockPos const&    originalPosition,
    ::BlockSource const& source
) {
    currentPortal = {originalPosition, mAxis};
    origin(originalPosition, source);
}

LL_TYPE_STATIC_HOOK(
    PortalCanSpawnPigZombieHook,
    HookPriority::Normal,
    SpawnChecks,
    &SpawnChecks::canSpawnPigZombieFromPortal,
    bool,
    ::Dimension const& dimension,
    ::IRandom&         random
) {
    IF_LISTENED(EVENT_TYPES::onPortalTrySpawnPigZombie) {
        if (isServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onPortalTrySpawnPigZombie,
                    IntPos::newPos(currentPortal.first, dimension.getDimensionId()),
                    Number::newNumber(static_cast<int>(currentPortal.second))
                )) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onPortalTrySpawnPigZombie);
    return origin(dimension, random);
}
} // namespace PortalTrySpawnPigZombie

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

LL_TYPE_STATIC_HOOK(
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

namespace Transformation {
std::pair<EntityId, Actor*> currentActor = {{}, nullptr};
LL_TYPE_INSTANCE_HOOK(
    TransformedActorHook,
    HookPriority::Normal,
    ActorFactory,
    &ActorFactory::createTransformedActor,
    ::OwnerPtr<::EntityContext>,
    ::ActorDefinitionIdentifier const& identifier,
    ::Actor*                           from
) {
    auto context = origin(identifier, from);
    currentActor = {context->mEntity, from};
    return context;
}

LL_TYPE_INSTANCE_HOOK(
    addEntityHook,
    HookPriority::Normal,
    Level,
    &Level::$addEntity,
    ::Actor*,
    ::BlockSource&              region,
    ::OwnerPtr<::EntityContext> entity
) {
    auto newActor = origin(region, entity);
    IF_LISTENED(EVENT_TYPES::onEntityTransformation) {
        if (isServerThread() && currentActor.first) {
            if (currentActor.first == entity->mEntity && currentActor.second) {
                CallEvent(
                    EVENT_TYPES::onEntityTransformation,
                    String::newString(std::to_string(currentActor.second->getOrCreateUniqueID().rawID)),
                    EntityClass::newEntity(newActor)
                );
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onEntityTransformation);
    return newActor;
}
} // namespace Transformation

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
    static ll::memory::HookRegistrar<
        ProjectileSpawnHook1,
        ProjectileSpawnHook2,
        FireworksRocketSpawn::Hook1,
        FireworksRocketSpawn::Hook2>
        reg;
};
void PortalTrySpawnPigZombieEvent() {
    static ll::memory::HookRegistrar<
        PortalTrySpawnPigZombie::PortalShapeEvalueateHook,
        PortalTrySpawnPigZombie::PortalCanSpawnPigZombieHook>
        reg;
}
void ProjectileCreatedEvent() { static ll::memory::HookRegistrar<ProjectileSpawnHook1> reg; };
void ActorRideEvent() { static ll::memory::HookRegistrar<ActorRideHook> reg; }
void WitherDestroyEvent() { static ll::memory::HookRegistrar<WitherDestroyHook> reg; }
void ProjectileHitEntityEvent() { static ll::memory::HookRegistrar<ProjectileHitEntityHook> reg; }
void ProjectileHitBlockEvent() { static ll::memory::HookRegistrar<ProjectileHitBlockHook> reg; }
void MobHurtEvent() { static ll::memory::HookRegistrar<MobHurtHook> reg; }
void NpcCommandEvent() { static ll::memory::HookRegistrar<NpcCommandHook> reg; }
void EndermanTakeBlockEvent() { static ll::memory::HookRegistrar<EndermanTakeBlockHook> reg; }
void TransformationEvent() {
    static ll::memory::HookRegistrar<Transformation::addEntityHook, Transformation::TransformedActorHook> reg;
}
} // namespace lse::events::entity
