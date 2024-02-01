#include "EventHooks.h"

#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/PlayerAPI.h"

#include <ll/api/memory/Hook.h>
#include <ll/api/memory/Memory.h>
#include <mc/entity/WeakEntityRef.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/server/module/VanillaServerGameplayEventListener.h>
#include <mc/world/actor/ActorDefinitionIdentifier.h>
#include <mc/world/actor/ArmorStand.h>
#include <mc/world/actor/boss/WitherBoss.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/containers/models/LevelContainerModel.h>
#include <mc/world/events/EventResult.h>
#include <mc/world/item/CrossbowItem.h>
#include <mc/world/item/ItemInstance.h>
#include <mc/world/item/registry/ItemStack.h>
#include <mc/world/level/BlockEventCoordinator.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/Spawner.h>
#include <mc/world/level/block/BasePressurePlateBlock.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/FarmBlock.h>
#include <mc/world/level/block/ItemFrameBlock.h>
#include <mc/world/level/block/RespawnAnchorBlock.h>
#include <mc/world/level/block/actor/BarrelBlockActor.h>
#include <mc/world/level/block/actor/BlockActor.h>
#include <mc/world/level/block/actor/ChestBlockActor.h>
#include <mc/world/level/block/actor/PistonBlockActor.h>
#include <mc/world/phys/AABB.h>

namespace lse::events {

// NOLINTBEGIN(modernize-use-trailing-return-type)
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)

LL_TYPE_INSTANCE_HOOK(
    PlayerStartDestroyHook,
    HookPriority::Normal,
    BlockEventCoordinator,
    &BlockEventCoordinator::sendBlockDestructionStarted,
    void,
    Player&         player,
    BlockPos const& blockPos,
    uchar           unk_char
) {
    IF_LISTENED(EVENT_TYPES::onStartDestroyBlock) {
        CallEventVoid(
            EVENT_TYPES::onStartDestroyBlock,
            PlayerClass::newPlayer(&player),
            BlockClass::newBlock(blockPos, player.getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onStartDestroyBlock)
    origin(player, blockPos, unk_char);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerDropItemHook,
    HookPriority::Normal,
    Player,
    "?drop@Player@@UEAA_NAEBVItemStack@@_N@Z",
    bool,
    ItemStack const& item,
    bool             randomly
) {
    IF_LISTENED(EVENT_TYPES::onDropItem) {
        CallEventRtnValue(
            EVENT_TYPES::onDropItem,
            false,
            PlayerClass::newPlayer(this),
            ItemClass::newItem(const_cast<ItemStack*>(&item))
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onDropItem);
    return origin(item, randomly);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerOpenContainerHook,
    HookPriority::Normal,
    VanillaServerGameplayEventListener,
    "?onEvent@VanillaServerGameplayEventListener@@UEAA?AW4EventResult@@AEBUPlayerOpenContainerEvent@@@Z",
    EventResult,
    void* playerOpenContainerEvent
) {
    IF_LISTENED(EVENT_TYPES::onOpenContainer) {
        Actor* actor = static_cast<WeakEntityRef*>(playerOpenContainerEvent)->tryUnwrap<Actor>();
        if (actor->isType(ActorType::Player)) {
            CallEventRtnValue(
                EVENT_TYPES::onOpenContainer,
                EventResult::StopProcessing,
                PlayerClass::newPlayer(static_cast<Player*>(actor)),
                BlockClass::newBlock(
                    ll::memory::dAccess<BlockPos>(playerOpenContainerEvent, 28),
                    actor->getDimensionId()
                )
            );
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onOpenContainer);
    return origin(playerOpenContainerEvent);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerCloseContainerHook1,
    HookPriority::Normal,
    ChestBlockActor,
    "?stopOpen@ChestBlockActor@@UEAAXAEAVPlayer@@@Z",
    void,
    Player const& player
) {
    IF_LISTENED(EVENT_TYPES::onCloseContainer) {
        CallEventVoid(
            EVENT_TYPES::onCloseContainer,
            PlayerClass::newPlayer(const_cast<Player*>(&player)),
            BlockClass::newBlock(
                ((BlockActor*)((char*)this - 240))->getPosition(),
                player.getDimensionId()
            ) // IDA ChestBlockActor::stopOpen
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onCloseContainer);
    origin(player);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerCloseContainerHook2,
    HookPriority::Normal,
    BarrelBlockActor,
    "?stopOpen@BarrelBlockActor@@UEAAXAEAVPlayer@@@Z",
    void,
    Player const& player
) {
    IF_LISTENED(EVENT_TYPES::onCloseContainer) {
        CallEventVoid(
            EVENT_TYPES::onCloseContainer,
            PlayerClass::newPlayer(const_cast<Player*>(&player)),
            BlockClass::newBlock(
                ((BlockActor*)((char*)this - 240))->getPosition(),
                player.getDimensionId()
            ) // IDA ChestBlockActor::stopOpen
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onCloseContainer);
    origin(player);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerChangeSlotHook,
    HookPriority::Normal,
    Player,
    &Player::inventoryChanged,
    void,
    Container&       container,
    int              slot,
    ItemStack const& oldItem,
    ItemStack const& newItem,
    bool             idk
) {
    IF_LISTENED(EVENT_TYPES::onInventoryChange) {
        CallEventVoid(
            EVENT_TYPES::onInventoryChange,
            PlayerClass::newPlayer(this),
            slot,
            ItemClass::newItem(const_cast<ItemStack*>(&oldItem)),
            ItemClass::newItem(const_cast<ItemStack*>(&newItem))
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onInventoryChange);
    origin(container, slot, oldItem, newItem, idk);
}

LL_TYPE_INSTANCE_HOOK(
    ContainerChangeHook,
    HookPriority::Normal,
    LevelContainerModel,
    "?_onItemChanged@LevelContainerModel@@MEAAXHAEBVItemStack@@0@Z",
    void,
    int              slotNumber,
    ItemStack const& oldItem,
    ItemStack const& newItem
) {
    IF_LISTENED(EVENT_TYPES::onContainerChange) {
        Player* player = ll::memory::dAccess<Player*>(this, 208); // IDA LevelContainerModel::LevelContainerModel
        if (player->hasOpenContainer()) {
            CallEventVoid(
                EVENT_TYPES::onContainerChange,
                PlayerClass::newPlayer(player),
                BlockClass::newBlock((BlockPos*)((char*)this + 216), player->getDimensionId()),
                Number::newNumber(slotNumber + this->_getContainerOffset()),
                ItemClass::newItem(const_cast<ItemStack*>(&oldItem)),
                ItemClass::newItem(const_cast<ItemStack*>(&newItem))
            );
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onContainerChange);
    origin(slotNumber, oldItem, newItem);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerAttackBlockHook,
    HookPriority::Normal,
    Block,
    &Block::attack,
    bool,
    Player*         player,
    BlockPos const& pos
) {
    IF_LISTENED(EVENT_TYPES::onAttackBlock) {
        ItemStack const& item = player->getSelectedItem();
        CallEventRtnValue(
            EVENT_TYPES::onAttackBlock,
            false,
            PlayerClass::newPlayer(player),
            BlockClass::newBlock(pos, player->getDimensionId()),
            !item.isNull() ? ItemClass::newItem(const_cast<ItemStack*>(&item)) : Local<Value>()
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onAttackBlock);
    return origin(player, pos);
}

LL_TYPE_INSTANCE_HOOK(
    ArmorStandSwapItemHook,
    HookPriority::Normal,
    ArmorStand,
    &ArmorStand::_trySwapItem,
    bool,
    Player&                    player,
    Puv::Legacy::EquipmentSlot slot
) {
    IF_LISTENED(EVENT_TYPES::onChangeArmorStand) {
        CallEventRtnValue(
            EVENT_TYPES::onChangeArmorStand,
            false,
            EntityClass::newEntity(this),
            PlayerClass::newPlayer(&player),
            Number::newNumber((int)slot)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onChangeArmorStand);
    return origin(player, slot);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerUseFrameHook1,
    HookPriority::Normal,
    ItemFrameBlock,
    "?use@ItemFrameBlock@@UEBA_NAEAVPlayer@@AEBVBlockPos@@E@Z",
    bool,
    Player&         player,
    BlockPos const& pos,
    uchar           face
) {
    IF_LISTENED(EVENT_TYPES::onUseFrameBlock) {
        CallEventRtnValue(
            EVENT_TYPES::onUseFrameBlock,
            false,
            PlayerClass::newPlayer(&player),
            BlockClass::newBlock(pos, player.getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onUseFrameBlock);
    return origin(player, pos, face);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerUseFrameHook2,
    HookPriority::Normal,
    ItemFrameBlock,
    "?attack@ItemFrameBlock@@UEBA_NPEAVPlayer@@AEBVBlockPos@@@Z",
    bool,
    Player*         player,
    BlockPos const& pos
) {
    IF_LISTENED(EVENT_TYPES::onUseFrameBlock) {
        CallEventRtnValue(
            EVENT_TYPES::onUseFrameBlock,
            false,
            PlayerClass::newPlayer(player),
            BlockClass::newBlock(pos, player->getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onUseFrameBlock);
    return origin(player, pos);
}

LL_TYPE_INSTANCE_HOOK(
    ProjectileSpawnHook1,
    HookPriority::Normal,
    Spawner,
    &Spawner::spawnProjectile,
    Actor*,
    BlockSource&                     region,
    ActorDefinitionIdentifier const& id,
    Actor*                           spawner,
    Vec3 const&                      position,
    Vec3 const&                      direction
) {
    IF_LISTENED(EVENT_TYPES::onSpawnProjectile) {
        CallEventRtnValue(
            EVENT_TYPES::onSpawnProjectile,
            nullptr,
            EntityClass::newEntity(spawner),
            String::newString(id.getCanonicalName())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onSpawnProjectile);
    Actor* projectile = origin(region, id, spawner, position, direction);
    IF_LISTENED(EVENT_TYPES::onProjectileCreated) {
        CallEventUncancelable(
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
        CallEventVoid(
            EVENT_TYPES::onSpawnProjectile,
            EntityClass::newEntity(&player),
            String::newString(projectileInstance.getTypeName())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onSpawnProjectile);
    origin(projectileInstance, player);
}

LL_TYPE_INSTANCE_HOOK(
    PressurePlateTriggerHook,
    HookPriority::Normal,
    BasePressurePlateBlock,
    "?shouldTriggerEntityInside@BasePressurePlateBlock@@UEBA_NAEAVBlockSource@@AEBVBlockPos@@AEAVActor@@@Z",
    bool,
    BlockSource&    region,
    BlockPos const& pos,
    Actor&          entity
) {
    IF_LISTENED(EVENT_TYPES::onStepOnPressurePlate) {
        CallEventRtnValue(
            EVENT_TYPES::onStepOnPressurePlate,
            false,
            EntityClass::newEntity(&entity),
            BlockClass::newBlock(pos, region.getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onStepOnPressurePlate);
    return origin(region, pos, entity);
}

LL_TYPE_INSTANCE_HOOK(
    ActorRideHook,
    HookPriority::Normal,
    Actor,
    "?canAddPassenger@Actor@@UEBA_NAEAV1@@Z",
    bool,
    Actor& passenger
) {
    IF_LISTENED(EVENT_TYPES::onRide) {
        CallEventRtnValue(EVENT_TYPES::onRide, false, EntityClass::newEntity(&passenger), EntityClass::newEntity(this));
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
        CallEventVoid(
            EVENT_TYPES::onWitherBossDestroy,
            EntityClass::newEntity(this),
            IntPos::newPos(bb.min, region.getDimensionId()),
            IntPos::newPos(bb.max, region.getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onWitherBossDestroy);
    origin(level, bb, region, range, type);
}

LL_TYPE_INSTANCE_HOOK(
    FarmDecayHook,
    HookPriority::Normal,
    FarmBlock,
    "?transformOnFall@FarmBlock@@UEBAXAEAVBlockSource@@AEBVBlockPos@@PEAVActor@@M@Z",
    void,
    BlockSource&    region,
    BlockPos const& pos,
    Actor*          actor,
    float           fallDistance
) {
    IF_LISTENED(EVENT_TYPES::onFarmLandDecay) {
        CallEventVoid(
            EVENT_TYPES::onFarmLandDecay,
            IntPos::newPos(pos, region.getDimensionId()),
            EntityClass::newEntity(actor)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onFarmLandDecay);
    origin(region, pos, actor, fallDistance);
}

LL_TYPE_INSTANCE_HOOK(
    PistonPushHook,
    HookPriority::Normal,
    PistonBlockActor,
    &PistonBlockActor::_attachedBlockWalker,
    bool,
    BlockSource&    region,
    BlockPos const& curPos,
    uchar           curBranchFacing,
    uchar           pistonMoveFacing
) {
    IF_LISTENED(EVENT_TYPES::onPistonTryPush) {
        if (region.getBlock(curPos).isAir()) {
            return origin(region, curPos, curBranchFacing, pistonMoveFacing);
        }
        CallEventRtnValue(
            EVENT_TYPES::onPistonTryPush,
            false,
            IntPos::newPos(curPos, region.getDimensionId()),
            BlockClass::newBlock(curPos, region.getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onPistonTryPush);
    bool shouldPush = origin(region, curPos, curBranchFacing, pistonMoveFacing);
    IF_LISTENED(EVENT_TYPES::onPistonPush) {
        if (shouldPush) {
            CallEventUncancelable(
                EVENT_TYPES::onPistonPush,
                IntPos::newPos(curPos, region.getDimensionId()),
                BlockClass::newBlock(curPos, region.getDimensionId())
            );
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onPistonPush);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerEatHook,
    HookPriority::Normal,
    Player,
    "?eat@Player@@QEAAXAEBVItemStack@@@Z",
    void,
    ItemStack const& instance
) {
    IF_LISTENED(EVENT_TYPES::onAte) {
        CallEventVoid(
            EVENT_TYPES::onAte,
            PlayerClass::newPlayer(this),
            ItemClass::newItem(const_cast<ItemStack*>(&instance))
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onAte);
    origin(instance);
}

LL_TYPE_INSTANCE_HOOK(
    ExplodeHook,
    HookPriority::Normal,
    Level,
    "?explode@Level@@UEAAXAEAVBlockSource@@PEAVActor@@AEBVVec3@@M_N3M3@Z",
    void,
    BlockSource& region,
    Actor*       source,
    Vec3 const&  pos,
    float        explosionRadius,
    bool         fire,
    bool         breaksBlocks,
    float        maxResistance,
    bool         allowUnderwater
) {
    IF_LISTENED(EVENT_TYPES::onEntityExplode) {
        if (source) {
            CallEventVoid(
                EVENT_TYPES::onEntityExplode,
                EntityClass::newEntity(source),
                FloatPos::newPos(pos, region.getDimensionId()),
                Number::newNumber(explosionRadius),
                Number::newNumber(maxResistance),
                Boolean::newBoolean(breaksBlocks),
                Boolean::newBoolean(fire)
            );
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onEntityExplode);

    IF_LISTENED(EVENT_TYPES::onBedExplode) {
        if (region.getBlock(pos).getTypeName() == "minecraft:bed") {
            CallEventVoid(EVENT_TYPES::onBedExplode, IntPos::newPos(pos, region.getDimensionId()));
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onBedExplode);

    IF_LISTENED(EVENT_TYPES::onExplode) {
        if (source) {
            CallEventVoid(
                EVENT_TYPES::onExplode,
                EntityClass::newEntity(source),
                FloatPos::newPos(pos, region.getDimensionId()),
                Number::newNumber(explosionRadius),
                Number::newNumber(maxResistance),
                Boolean::newBoolean(breaksBlocks),
                Boolean::newBoolean(fire)
            );
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onExplode);

    IF_LISTENED(EVENT_TYPES::onBlockExplode) {
        CallEventVoid(
            EVENT_TYPES::onBlockExplode,
            BlockClass::newBlock(pos, region.getDimensionId()),
            IntPos::newPos(pos, region.getDimensionId()),
            Number::newNumber(explosionRadius),
            Number::newNumber(maxResistance),
            Boolean::newBoolean(breaksBlocks),
            Boolean::newBoolean(fire)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onBlockExplode);
    origin(region, source, pos, explosionRadius, fire, breaksBlocks, maxResistance, allowUnderwater);
}

LL_TYPE_STATIC_HOOK(
    RespawnAnchorExplodeHook,
    HookPriority::Normal,
    RespawnAnchorBlock,
    &RespawnAnchorBlock::_explode,
    void,
    Player&         player,
    BlockPos const& pos,
    BlockSource&    region,
    Level&          level
) {
    IF_LISTENED(EVENT_TYPES::onRespawnAnchorExplode) {
        CallEventVoid(
            EVENT_TYPES::onRespawnAnchorExplode,
            IntPos::newPos(pos, region.getDimensionId()),
            PlayerClass::newPlayer(&player)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onRespawnAnchorExplode);
    origin(player, pos, region, level);
}

LL_TYPE_INSTANCE_HOOK(
    BlockExplodedHook,
    HookPriority::Normal,
    Block,
    &Block::onExploded,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    Actor*          entitySource
) {
    IF_LISTENED(EVENT_TYPES::onBlockExploded) {
        CallEventVoid(
            EVENT_TYPES::onBlockExploded,
            BlockClass::newBlock(pos, region.getDimensionId()),
            EntityClass::newEntity(entitySource)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onBlockExploded);
    origin(region, pos, entitySource);
}

void PlayerStartDestroyBlock() { PlayerStartDestroyHook::hook(); }
void PlayerDropItem() { PlayerDropItemHook::hook(); }
void PlayerOpenContainerEvent() { PlayerOpenContainerHook::hook(); }
void PlayerCloseContainerEvent() {
    PlayerCloseContainerHook1::hook();
    PlayerCloseContainerHook2::hook();
}
void PlayerChangeSlotEvent() { PlayerChangeSlotHook::hook(); }
void ContainerChangeEvent() { ContainerChangeHook::hook(); }
void PlayerAttackBlockEvent() { PlayerAttackBlockHook::hook(); }
void ArmorStandSwapItemEvent() { ArmorStandSwapItemHook::hook(); }
void PlayerUseFrameEvent() {
    PlayerUseFrameHook1::hook();
    PlayerUseFrameHook2::hook();
}
void ProjectileSpawnEvent() {
    ProjectileSpawnHook1::hook();
    ProjectileSpawnHook2 ::hook();
};
void ProjectileCreatedEvent() { ProjectileSpawnHook1::hook(); };
void PressurePlateTriggerEvent() { PressurePlateTriggerHook::hook(); }
void ActorRideEvent() { ActorRideHook::hook(); }
void WitherDestroyEvent() { WitherDestroyHook::hook(); }
void FarmDecayEvent() { FarmDecayHook::hook(); }
void PistonPushEvent() { PistonPushHook::hook(); }
void PlayerEatEvent() { PlayerEatHook::hook(); }
void ExplodeEvent() { ExplodeHook::hook(); }
void RespawnAnchorExplodeEvent() { RespawnAnchorExplodeHook::hook(); }
void BlockExplodedEvent() { BlockExplodedHook ::hook(); }

// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
// NOLINTEND(modernize-use-trailing-return-type)

} // namespace lse::events
