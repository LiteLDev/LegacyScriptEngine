#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/service/Bedrock.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/world/actor/ActorDamageCause.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/inventory/transaction/InventoryAction.h"
#include "mc/world/inventory/transaction/InventorySource.h"
#include "mc/world/inventory/transaction/InventoryTransaction.h"
#include "mc/world/phys/HitResult.h"
#include "mc/world/scores/IdentityDefinition.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include "mc/entity/components_json_legacy/ProjectileComponent.h"
#include "mc/server/module/VanillaServerGameplayEventListener.h"
#include "mc/world/actor/ActorDefinitionIdentifier.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/FishingHook.h"
#include "mc/world/actor/VanillaActorRendererId.h"
#include "mc/world/actor/boss/WitherBoss.h"
#include "mc/world/actor/item/ItemActor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/containers/models/LevelContainerModel.h"
#include "mc/world/events/BlockEventCoordinator.h"
#include "mc/world/gamemode/InteractionResult.h"
#include "mc/world/inventory/transaction/ComplexInventoryTransaction.h"
#include "mc/world/item/CrossbowItem.h"
#include "mc/world/item/ItemInstance.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/TridentItem.h"
#include "mc/world/level/BedrockSpawner.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChangeDimensionRequest.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/ChestBlockActor.h"
#include "mc/world/level/block/actor/PistonBlockActor.h"
#include "mc/world/phys/AABB.h"
#include "mc/world/scores/ServerScoreboard.h"
#include "mc/world/level/Explosion.h"
#include "mc/world/level/material/Material.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/actor/player/PlayerItemInUse.h"
#include "mc/entity/components_json_legacy/NpcComponent.h"
#include "mc/world/actor/npc/ActionContainer.h"
#include "mc/world/actor/npc/CommandAction.h"
#include "mc/world/actor/npc/UrlAction.h"
#include "mc/world/actor/npc/StoredCommand.h"

namespace lse::events::entity {
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
        if (id._getLegacyActorType() != ActorType::Trident) {
            if (!CallEvent(
                    EVENT_TYPES::onSpawnProjectile,
                    EntityClass::newEntity(spawner),
                    String::newString(id.getCanonicalName())
                )) {
                return nullptr;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onSpawnProjectile);
    Actor* projectile = origin(region, id, spawner, position, direction);
    IF_LISTENED(EVENT_TYPES::onProjectileCreated) {
        CallEvent( // Not nancellable
            EVENT_TYPES::onProjectileCreated,
            EntityClass::newEntity(spawner),
            EntityClass::newEntity(projectile)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onProjectileCreated);
    return projectile;
}

LL_TYPE_INSTANCE_HOOK(
    ProjectileSpawnHook2,
    HookPriority::Normal,
    CrossbowItem,
    &CrossbowItem::_shootFirework,
    void,
    ItemInstance const& projectileInstance,
    Player&             player
) {
    IF_LISTENED(EVENT_TYPES::onSpawnProjectile) {
        if (!CallEvent(
                EVENT_TYPES::onSpawnProjectile,
                EntityClass::newEntity(&player),
                String::newString(projectileInstance.getTypeName())
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onSpawnProjectile);
    origin(projectileInstance, player);
}

LL_TYPE_INSTANCE_HOOK(
    ProjectileSpawnHook3,
    HookPriority::Normal,
    TridentItem,
    &TridentItem::$releaseUsing,
    void,
    ItemStack& item,
    Player*    player,
    int        durationLeft
) {
    IF_LISTENED(EVENT_TYPES::onSpawnProjectile) {
        if (!CallEvent(
                EVENT_TYPES::onSpawnProjectile,
                EntityClass::newEntity(player),
                String::newString(VanillaActorRendererId::trident().getString())
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onSpawnProjectile);
    origin(item, player, durationLeft);
}

LL_TYPE_INSTANCE_HOOK(ActorRideHook, HookPriority::Normal, Actor, &Actor::$canAddPassenger, bool, Actor& passenger) {
    IF_LISTENED(EVENT_TYPES::onRide) {
        if (!CallEvent(EVENT_TYPES::onRide, EntityClass::newEntity(&passenger), EntityClass::newEntity(this))) {
            return false;
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
        if (!CallEvent(
                EVENT_TYPES::onWitherBossDestroy,
                EntityClass::newEntity(this),
                IntPos::newPos(bb.min, region.getDimensionId()),
                IntPos::newPos(bb.max, region.getDimensionId())
            )) {
            return;
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
        if (res.getEntity()) {
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
    Block,
    &Block::onProjectileHit,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    Actor const&    projectile
) {
    IF_LISTENED(EVENT_TYPES::onProjectileHitBlock) {
        if (pos != BlockPos::ZERO() && !this->isAir()) {
            if (!CallEvent(
                    EVENT_TYPES::onProjectileHitBlock,
                    BlockClass::newBlock(*this, pos, region),
                    EntityClass::newEntity(&const_cast<Actor&>(projectile))
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onProjectileHitBlock);
    origin(region, pos, projectile);
}

LL_TYPE_INSTANCE_HOOK(
    MobHurtEffectHook,
    HookPriority::Normal,
    Mob,
    &Mob::getDamageAfterResistanceEffect,
    float,
    ActorDamageSource const& source,
    float                    damage
) {
    IF_LISTENED(EVENT_TYPES::onMobHurt) {
        if (source.getCause() == ActorDamageCause::Magic || source.getCause() == ActorDamageCause::Wither) {
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
                    Number::newNumber((int)source.getCause())
                )) {
                return 0.0f;
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
        auto action = this->getActionsContainer().at(actionIndex);
        if (std::holds_alternative<npc::CommandAction>(*action)) {
            auto& commands   = std::get<npc::CommandAction>(*action).commands;
            std::string command;
            for (auto& cmd : commands.get()) {
                command += cmd.mUnk879303.as<std::string>() + ";";
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
    IF_LISTENED_END(EVENT_TYPES::onNpcCmd);
    origin(owner, sourcePlayer, actionIndex, sceneName);
}

void ProjectileSpawnEvent() {
    ProjectileSpawnHook1::hook();
    ProjectileSpawnHook2::hook();
    ProjectileSpawnHook3::hook();
};
void ProjectileCreatedEvent() { ProjectileSpawnHook1::hook(); };
void ActorRideEvent() { ActorRideHook::hook(); }
void WitherDestroyEvent() { WitherDestroyHook::hook(); }
void ProjectileHitEntityEvent() { ProjectileHitEntityHook::hook(); }
void ProjectileHitBlockEvent() { ProjectileHitBlockHook::hook(); }
void MobHurtEvent() { MobHurtEffectHook::hook(); }
void NpcCommandEvent() { NpcCommandHook::hook(); }
}