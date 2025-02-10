#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/GamingStatus.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/entity/components_json_legacy/NpcComponent.h"
#include "mc/entity/components_json_legacy/ProjectileComponent.h"
#include "mc/entity/components_json_legacy/TransformationComponent.h"
#include "mc/world/actor/ActorDamageCause.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/ActorDefinitionIdentifier.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/VanillaActorRendererId.h"
#include "mc/world/actor/boss/WitherBoss.h"
#include "mc/world/actor/npc/ActionContainer.h"
#include "mc/world/actor/npc/CommandAction.h"
#include "mc/world/actor/npc/StoredCommand.h"
#include "mc/world/actor/npc/UrlAction.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/effect/EffectDuration.h"
#include "mc/world/effect/MobEffectInstance.h"
#include "mc/world/item/CrossbowItem.h"
#include "mc/world/item/ItemInstance.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/TridentItem.h"
#include "mc/world/level/BedrockSpawner.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/phys/AABB.h"
#include "mc/world/phys/HitResult.h"

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
    MobHurtHook,
    HookPriority::Normal,
    Mob,
    &Mob::$_hurt,
    bool,
    ::ActorDamageSource const& source,
    float                      damage,
    bool                       knock,
    bool                       ignite
) {
    IF_LISTENED(EVENT_TYPES::onMobHurt) {
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
                Number::newNumber((int)source.getCause())
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onMobHurt)
    return origin(source, damage, knock, ignite);
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
        // Mob is still hurt after hook Mob::$hurtEffects, and all hurt events are handled by this function, but we just
        // need magic damage.
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
            auto&       commands = std::get<npc::CommandAction>(*action).commands;
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

LL_TYPE_INSTANCE_HOOK(
    EffectUpdateHook,
    HookPriority::Normal,
    MobEffectInstance,
    &MobEffectInstance::updateEffects,
    void,
    ::Actor* mob
) {
    IF_LISTENED(EVENT_TYPES::onEffectUpdated) {
        if (mob->isPlayer()) {
            if (!CallEvent(
                    EVENT_TYPES::onEffectUpdated,
                    PlayerClass::newPlayer(static_cast<Player*>(mob)),
                    String::newString(getComponentName().getString()),
                    Number::newNumber(getAmplifier()),
                    Number::newNumber(getDuration().getValueForSerialization())
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onEffectUpdated);
    origin(mob);
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
        CallEvent(
            EVENT_TYPES::onEntityTransformation,
            String::newString(std::to_string(originalActor.getOrCreateUniqueID().rawID)),
            EntityClass::newEntity(&transformed)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onEntityTransformation);

    origin(originalActor, transformed, transformation, ownerID, level);
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
void MobHurtEvent() {
    MobHurtHook::hook();
    MobHurtEffectHook::hook();
}
void NpcCommandEvent() { NpcCommandHook::hook(); }
void EffectUpdateEvent() { EffectUpdateHook::hook(); }
void TransformationEvent() { TransformationHook::hook(); }
} // namespace lse::events::entity