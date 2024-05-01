#include "EventHooks.h"

#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerPlayer.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/ActorUniqueID.h"
#include "mc/world/containers/ContainerID.h"
#include "mc/world/inventory/transaction/InventorySource.h"
#include "mc/world/scores/ScoreInfo.h"

#include <ll/api/memory/Hook.h>
#include <ll/api/memory/Memory.h>
#include <mc/common/wrapper/InteractionResult.h>
#include <mc/entity/WeakEntityRef.h>
#include <mc/entity/components/ProjectileComponent.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/server/module/VanillaServerGameplayEventListener.h>
#include <mc/world/actor/ActorDefinitionIdentifier.h>
#include <mc/world/actor/ArmorStand.h>
#include <mc/world/actor/FishingHook.h>
#include <mc/world/actor/boss/WitherBoss.h>
#include <mc/world/actor/item/ItemActor.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/containers/models/LevelContainerModel.h>
#include <mc/world/events/EventResult.h>
#include <mc/world/inventory/transaction/ComplexInventoryTransaction.h>
#include <mc/world/item/BucketItem.h>
#include <mc/world/item/CrossbowItem.h>
#include <mc/world/item/ItemInstance.h>
#include <mc/world/item/registry/ItemStack.h>
#include <mc/world/level/BlockEventCoordinator.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/ChangeDimensionRequest.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/Spawner.h>
#include <mc/world/level/block/BasePressurePlateBlock.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/ComparatorBlock.h>
#include <mc/world/level/block/DiodeBlock.h>
#include <mc/world/level/block/FarmBlock.h>
#include <mc/world/level/block/ItemFrameBlock.h>
#include <mc/world/level/block/LiquidBlockDynamic.h>
#include <mc/world/level/block/RedStoneWireBlock.h>
#include <mc/world/level/block/RedstoneTorchBlock.h>
#include <mc/world/level/block/RespawnAnchorBlock.h>
#include <mc/world/level/block/actor/BarrelBlockActor.h>
#include <mc/world/level/block/actor/BaseCommandBlock.h>
#include <mc/world/level/block/actor/BlockActor.h>
#include <mc/world/level/block/actor/ChestBlockActor.h>
#include <mc/world/level/block/actor/PistonBlockActor.h>
#include <mc/world/phys/AABB.h>
#include <mc/world/scores/ServerScoreboard.h>

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
    Block const&    block,
    uchar           unk_char
) {
    IF_LISTENED(EVENT_TYPES::onStartDestroyBlock) {
        CallEventVoid(
            EVENT_TYPES::onStartDestroyBlock,
            PlayerClass::newPlayer(&player),
            BlockClass::newBlock(&block, &blockPos, player.getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onStartDestroyBlock)
    origin(player, blockPos, block, unk_char);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerDropItemHook1,
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
            ItemClass::newItem(&const_cast<ItemStack&>(item), false)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onDropItem);
    return origin(item, randomly);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerDropItemHook2,
    HookPriority::Normal,
    ComplexInventoryTransaction,
    "?handle@ComplexInventoryTransaction@@UEBA?AW4InventoryTransactionError@@AEAVPlayer@@_N@Z",
    InventoryTransactionError,
    Player& player,
    bool    isSenderAuthority
) {
    if (type == ComplexInventoryTransaction::Type::NormalTransaction) {
        IF_LISTENED(EVENT_TYPES::onDropItem) {
            InventorySource source(InventorySourceType::ContainerInventory, ContainerID::Inventory);
            auto&           actions = data.getActions(source);
            if (actions.size() == 1) {
                CallEventRtnValue(
                    EVENT_TYPES::onDropItem,
                    InventoryTransactionError::NoError,
                    PlayerClass::newPlayer(&player),
                    ItemClass::newItem(&const_cast<ItemStack&>(player.getInventory().getItem(actions[0].mSlot)), false)
                );
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onDropItem);
    }
    return origin(player, isSenderAuthority);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerOpenContainerHook,
    HookPriority::Normal,
    VanillaServerGameplayEventListener,
    &VanillaServerGameplayEventListener::onEvent,
    EventResult,
    struct PlayerOpenContainerEvent const& playerOpenContainerEvent
) {
    IF_LISTENED(EVENT_TYPES::onOpenContainer) {
        Actor* actor = static_cast<WeakEntityRef*>((void*)&playerOpenContainerEvent)->tryUnwrap<Actor>();
        if (actor->isType(ActorType::Player)) {
            CallEventRtnValue(
                EVENT_TYPES::onOpenContainer,
                EventResult::StopProcessing,
                PlayerClass::newPlayer(static_cast<Player*>(actor)),
                BlockClass::newBlock(
                    ll::memory::dAccess<BlockPos>(&playerOpenContainerEvent, 28),
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
    &ChestBlockActor::stopOpen,
    void,
    Player& player
) {
    IF_LISTENED(EVENT_TYPES::onCloseContainer) {
        CallEventVoid(
            EVENT_TYPES::onCloseContainer,
            PlayerClass::newPlayer(&const_cast<Player&>(player)),
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
    &BarrelBlockActor::stopOpen,
    void,
    Player& player
) {
    IF_LISTENED(EVENT_TYPES::onCloseContainer) {
        CallEventVoid(
            EVENT_TYPES::onCloseContainer,
            PlayerClass::newPlayer(&const_cast<Player&>(player)),
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
            ItemClass::newItem(&const_cast<ItemStack&>(oldItem), false),
            ItemClass::newItem(&const_cast<ItemStack&>(newItem), false)
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
                ItemClass::newItem(&const_cast<ItemStack&>(oldItem), false),
                ItemClass::newItem(&const_cast<ItemStack&>(newItem), false)
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
            !item.isNull() ? ItemClass::newItem(&const_cast<ItemStack&>(item), false) : Local<Value>()
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
    return shouldPush;
}

LL_TYPE_INSTANCE_HOOK(PlayerEatHook, HookPriority::Normal, Player, &Player::eat, void, ItemStack const& instance) {
    IF_LISTENED(EVENT_TYPES::onAte) {
        CallEventVoid(
            EVENT_TYPES::onAte,
            PlayerClass::newPlayer(this),
            ItemClass::newItem(&const_cast<ItemStack&>(instance), false)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onAte);
    origin(instance);
}

LL_TYPE_INSTANCE_HOOK(
    ExplodeHook,
    HookPriority::Normal,
    Level,
    &Level::explode,
    bool,
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
            CallEventRtnValue(
                EVENT_TYPES::onEntityExplode,
                false,
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

    IF_LISTENED(EVENT_TYPES::onBlockExplode) {
        CallEventRtnValue(
            EVENT_TYPES::onBlockExplode,
            false,
            BlockClass::newBlock(pos, region.getDimensionId()),
            IntPos::newPos(pos, region.getDimensionId()),
            Number::newNumber(explosionRadius),
            Number::newNumber(maxResistance),
            Boolean::newBoolean(breaksBlocks),
            Boolean::newBoolean(fire)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onBlockExplode);
    return origin(region, source, pos, explosionRadius, fire, breaksBlocks, maxResistance, allowUnderwater);
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

#define RedstoneUpdateHookMacro(NAME, TYPE, SYMBOL)                                                                    \
    LL_TYPE_INSTANCE_HOOK(                                                                                             \
        NAME,                                                                                                          \
        HookPriority::Normal,                                                                                          \
        TYPE,                                                                                                          \
        SYMBOL,                                                                                                        \
        void,                                                                                                          \
        BlockSource&    region,                                                                                        \
        BlockPos const& pos,                                                                                           \
        int             strength,                                                                                      \
        bool            isFirstTime                                                                                    \
    ) {                                                                                                                \
        IF_LISTENED(EVENT_TYPES::onRedStoneUpdate) {                                                                   \
            CallEventVoid(                                                                                             \
                EVENT_TYPES::onRedStoneUpdate,                                                                         \
                BlockClass::newBlock(pos, region.getDimensionId()),                                                    \
                Number::newNumber(strength),                                                                           \
                Boolean::newBoolean(isFirstTime)                                                                       \
            );                                                                                                         \
        }                                                                                                              \
        IF_LISTENED_END(EVENT_TYPES::onRedStoneUpdate);                                                                \
        origin(region, pos, strength, isFirstTime);                                                                    \
    }

RedstoneUpdateHookMacro(
    RedstoneUpdateHook1,
    RedStoneWireBlock,
    "?onRedstoneUpdate@RedStoneWireBlock@@UEBAXAEAVBlockSource@@AEBVBlockPos@@H_N@Z"
);

RedstoneUpdateHookMacro(
    RedstoneUpdateHook2,
    DiodeBlock,
    "?onRedstoneUpdate@DiodeBlock@@UEBAXAEAVBlockSource@@AEBVBlockPos@@H_N@Z"
);

RedstoneUpdateHookMacro(
    RedstoneUpdateHook3,
    RedstoneTorchBlock,
    "?onRedstoneUpdate@RedstoneTorchBlock@@UEBAXAEAVBlockSource@@AEBVBlockPos@@H_N@Z"
);

RedstoneUpdateHookMacro(
    RedstoneUpdateHook4,
    ComparatorBlock,
    "?onRedstoneUpdate@ComparatorBlock@@UEBAXAEAVBlockSource@@AEBVBlockPos@@H_N@Z"
);

LL_TYPE_INSTANCE_HOOK(
    LiquidFlowHook,
    HookPriority::Normal,
    LiquidBlockDynamic,
    &LiquidBlockDynamic::_canSpreadTo,
    bool,
    BlockSource&    region,
    BlockPos const& pos,
    BlockPos const& flowFromPos,
    uchar           flowFromDirection
) {
    IF_LISTENED(EVENT_TYPES::onLiquidFlow) {
        CallEventRtnValue(
            EVENT_TYPES::onLiquidFlow,
            false,
            BlockClass::newBlock(pos, region.getDimensionId()),
            IntPos::newPos(pos, region.getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onLiquidFlow);
    return origin(region, pos, flowFromPos, flowFromDirection);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerChangeDimensionHook,
    HookPriority::Normal,
    Level,
    &Level::requestPlayerChangeDimension,
    void,
    Player&                  player,
    ChangeDimensionRequest&& changeRequest
) {
    IF_LISTENED(EVENT_TYPES::onChangeDim) {
        CallEventVoid(
            EVENT_TYPES::onChangeDim,
            PlayerClass::newPlayer(&player),
            Number::newNumber(changeRequest.mToDimensionId)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onChangeDim);
    origin(player, std::move(changeRequest));
}

LL_TYPE_INSTANCE_HOOK(
    PlayerOpenContainerScreenHook,
    HookPriority::Normal,
    Player,
    &Player::canOpenContainerScreen,
    bool
) {
    IF_LISTENED(EVENT_TYPES::onOpenContainerScreen) {
        CallEventRtnValue(EVENT_TYPES::onOpenContainerScreen, false, PlayerClass::newPlayer(this));
    }
    IF_LISTENED_END(EVENT_TYPES::onOpenContainerScreen);
    return origin();
}

LL_TYPE_INSTANCE_HOOK(
    CommandBlockExecuteHook,
    HookPriority::Normal,
    BaseCommandBlock,
    &BaseCommandBlock::_performCommand,
    bool,
    BlockSource&         region,
    CommandOrigin const& commandOrigin,
    bool&                markForSaving
) {
    IF_LISTENED(EVENT_TYPES::onCmdBlockExecute) {
        if (commandOrigin.getOriginType() == CommandOriginType::MinecartCommandBlock) {
            CallEventRtnValue(
                EVENT_TYPES::onCmdBlockExecute,
                false,
                String::newString(this->getCommand()),
                FloatPos::newPos(commandOrigin.getEntity()->getPosition(), region.getDimensionId()),
                Boolean::newBoolean(true)
            );
        } else {
            CallEventRtnValue(
                EVENT_TYPES::onCmdBlockExecute,
                false,
                String::newString(this->getCommand()),
                FloatPos::newPos(commandOrigin.getBlockPosition(), region.getDimensionId()),
                Boolean::newBoolean(false)
            );
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onCmdBlockExecute);
    return origin(region, commandOrigin, markForSaving);
}

LL_TYPE_STATIC_HOOK(
    PlayerUseRespawnAnchorHook,
    HookPriority::Normal,
    RespawnAnchorBlock,
    &RespawnAnchorBlock::_trySetSpawn,
    bool,
    Player&         player,
    BlockPos const& pos,
    BlockSource&    region,
    class Level&    level
) {
    IF_LISTENED(EVENT_TYPES::onUseRespawnAnchor) {
        CallEventRtnValue(
            EVENT_TYPES::onUseRespawnAnchor,
            false,
            PlayerClass::newPlayer(&player),
            IntPos::newPos(pos, region.getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onUseRespawnAnchor);
    return origin(player, pos, region, level);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerSleepHook,
    HookPriority::Normal,
    Player,
    "?startSleepInBed@Player@@UEAA?AW4BedSleepingResult@@AEBVBlockPos@@@Z",
    BedSleepingResult,
    BlockPos const& pos
) {
    IF_LISTENED(EVENT_TYPES::onBedEnter) {
        CallEventRtnValue(
            EVENT_TYPES::onBedEnter,
            (BedSleepingResult)0,
            PlayerClass::newPlayer(this),
            IntPos::newPos(pos, this->getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onBedEnter);
    return origin(pos);
}
LL_TYPE_INSTANCE_HOOK(
    PlayerOpenInventoryHook,
    HookPriority::Normal,
    ServerPlayer,
    "?openInventory@ServerPlayer@@UEAAXXZ",
    void,
) {
    IF_LISTENED(EVENT_TYPES::onOpenInventory) {
        CallEventVoid(EVENT_TYPES::onOpenInventory, PlayerClass::newPlayer(this));
    }
    IF_LISTENED_END(EVENT_TYPES::onOpenInventory);
    origin();
}

LL_TYPE_INSTANCE_HOOK(
    PlayerPullFishingHook,
    HookPriority::Normal,
    FishingHook,
    &FishingHook::_pullCloser,
    void,
    Actor& inEntity,
    float  inSpeed
) {
    IF_LISTENED(EVENT_TYPES::onPlayerPullFishingHook) {
        CallEventVoid(
            EVENT_TYPES::onPlayerPullFishingHook,
            PlayerClass::newPlayer(this->getPlayerOwner()),
            EntityClass::newEntity(&inEntity),
            inEntity.isType(ActorType::ItemEntity)
                ? ItemClass::newItem(&static_cast<ItemActor&>(inEntity).item(), false)
                : Local<Value>()
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onPlayerPullFishingHook);
    origin(inEntity, inSpeed);
}

LL_TYPE_INSTANCE_HOOK(
    ScoreChangedHook,
    HookPriority::Normal,
    ServerScoreboard,
    "?onScoreChanged@ServerScoreboard@@UEAAXAEBUScoreboardId@@AEBVObjective@@@Z",
    void,
    ScoreboardId const& id,
    Objective const&    obj
) {
    IF_LISTENED(EVENT_TYPES::onScoreChanged) {
        if (id.getIdentityDef().isPlayerType()) {
            CallEventVoid(
                EVENT_TYPES::onScoreChanged,
                PlayerClass::newPlayer(
                    ll::service::getLevel()->getPlayer(ActorUniqueID(id.getIdentityDef().getPlayerId().mActorUniqueId))
                ),
                Number::newNumber(obj.getPlayerScore(id).mScore),
                String::newString(obj.getName()),
                String::newString(obj.getDisplayName())
            );
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onScoreChanged);
    origin(id, obj);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerUseBucketPlaceHook,
    HookPriority::Normal,
    BucketItem,
    &BucketItem::_emptyBucket,
    bool,
    BlockSource&     region,
    Block const&     contents,
    BlockPos const&  pos,
    Actor*           placer,
    ItemStack const& instance,
    uchar            face
) {
    IF_LISTENED(EVENT_TYPES::onUseBucketPlace) {
        CallEventRtnValue(
            EVENT_TYPES::onUseBucketPlace,
            false,
            PlayerClass::newPlayer(static_cast<Player*>(placer)),
            ItemClass::newItem(&const_cast<ItemStack&>(instance), false),
            BlockClass::newBlock(&contents, &pos, &region),
            Number::newNumber(face),
            FloatPos::newPos(pos, region.getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onUseBucketPlace);
    return origin(region, contents, pos, placer, instance, face);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerUseBucketTakeHook1,
    HookPriority::Normal,
    BucketItem,
    &BucketItem::_takeLiquid,
    bool,
    ItemStack&      item,
    Actor&          entity,
    BlockPos const& pos
) {
    IF_LISTENED(EVENT_TYPES::onUseBucketTake) {
        CallEventRtnValue(
            EVENT_TYPES::onUseBucketTake,
            false,
            PlayerClass::newPlayer(&static_cast<Player&>(entity)),
            ItemClass::newItem(&item, false),
            BlockClass::newBlock(&pos, entity.getDimensionId()),
            Number::newNumber(-1),
            FloatPos::newPos(pos, entity.getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onUseBucketTake);
    return origin(item, entity, pos);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerUseBucketTakeHook2,
    HookPriority::Normal,
    BucketItem,
    &BucketItem::_takePowderSnow,
    bool,
    ItemStack&      item,
    Actor&          entity,
    BlockPos const& pos
) {
    IF_LISTENED(EVENT_TYPES::onUseBucketTake) {
        CallEventRtnValue(
            EVENT_TYPES::onUseBucketTake,
            false,
            PlayerClass::newPlayer(&static_cast<Player&>(entity)),
            ItemClass::newItem(&item, false),
            BlockClass::newBlock(&pos, entity.getDimensionId()),
            Number::newNumber(-1),
            FloatPos::newPos(pos, entity.getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onUseBucketTake);
    return origin(item, entity, pos);
}

// LL_TYPE_INSTANCE_HOOK(
//     PlayerUseBucketTakeHook3,
//     HookPriority::Normal,
//     BucketItem,
//     &BucketItem::_useOn,
//     InteractionResult,
//     ItemStack&  instance,
//     Actor&      entity,
//     BlockPos    pos,
//     uchar       face,
//     Vec3 const& clickPos
// ) {
//     IF_LISTENED(EVENT_TYPES::onUseBucketTake) {
//         CallEventRtnValue(
//             EVENT_TYPES::onUseBucketTake,
//             InteractionResult{InteractionResult::Result::Fail},
//             PlayerClass::newPlayer(),
//             ItemClass::newItem(&instance, false),
//             EntityClass::newEntity(&entity),
//             Number::newNumber(face),
//             FloatPos::newPos(pos, entity.getDimensionId())
//         );
//     }
//     IF_LISTENED_END(EVENT_TYPES::onUseBucketTake);
//     return origin(instance, entity, pos, face, clickPos);
// }

LL_TYPE_INSTANCE_HOOK(PlayerConsumeTotemHook, HookPriority::Normal, Player, "?consumeTotem@Player@@UEAA_NXZ", bool) {
    IF_LISTENED(EVENT_TYPES::onConsumeTotem) {
        CallEventRtnValue(EVENT_TYPES::onConsumeTotem, false, PlayerClass::newPlayer(this));
    }
    IF_LISTENED_END(EVENT_TYPES::onConsumeTotem);
    return origin();
}

LL_TYPE_INSTANCE_HOOK(
    PlayerSetArmorHook,
    HookPriority::Normal,
    ServerPlayer,
    "?setArmor@ServerPlayer@@UEAAXW4ArmorSlot@@AEBVItemStack@@@Z",
    void,
    ArmorSlot        armorSlot,
    ItemStack const& item
) {
    IF_LISTENED(EVENT_TYPES::onSetArmor) {
        CallEventVoid(
            EVENT_TYPES::onSetArmor,
            PlayerClass::newPlayer(this),
            Number::newNumber((int)armorSlot),
            ItemClass::newItem(&const_cast<ItemStack&>(item), false)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onSetArmor);
    origin(std::move(armorSlot), item);
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
        CallEventVoid(
            EVENT_TYPES::onProjectileHitEntity,
            EntityClass::newEntity(res.getEntity()),
            EntityClass::newEntity(&owner)
        );
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
        CallEventVoid(
            EVENT_TYPES::onProjectileHitBlock,
            BlockClass::newBlock(this, &pos, &region),
            EntityClass::newEntity(&const_cast<Actor&>(projectile))
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onProjectileHitBlock);
    origin(region, pos, projectile);
}

void PlayerStartDestroyBlock() { PlayerStartDestroyHook::hook(); }
void PlayerDropItem() {
    PlayerDropItemHook1::hook();
    PlayerDropItemHook2::hook();
}
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
void RedstoneupdateEvent() {
    RedstoneUpdateHook1::hook();
    RedstoneUpdateHook2::hook();
    RedstoneUpdateHook3::hook();
    RedstoneUpdateHook4::hook();
}
void LiquidFlowEvent() { LiquidFlowHook::hook(); }
void PlayerChangeDimensionEvent() { PlayerChangeDimensionHook::hook(); };
void PlayerOpenContainerScreenEvent() { PlayerOpenContainerScreenHook::hook(); }
void CommandBlockExecuteEvent() { CommandBlockExecuteHook::hook(); }
void PlayerUseRespawnAnchorEvent() { PlayerUseRespawnAnchorHook::hook(); }
void PlayerSleepEvent() { PlayerSleepHook::hook(); }
void PlayerOpenInventoryEvent() { PlayerOpenInventoryHook::hook(); }
void PlayerPullFishingHookEvent() { PlayerPullFishingHook::hook(); }
void ScoreChangedEvent() { ScoreChangedHook::hook(); }
void PlayerUseBucketPlaceEvent() { PlayerUseBucketPlaceHook::hook(); }
void PlayerUseBucketTakeEvent() {
    PlayerUseBucketTakeHook1::hook();
    PlayerUseBucketTakeHook2::hook();
}
void PlayerConsumeTotemEvent() { PlayerConsumeTotemHook::hook(); }
void PlayerSetArmorEvent() { PlayerSetArmorHook::hook(); }
void ProjectileHitEntityEvent() { ProjectileHitEntityHook::hook(); }
void ProjectileHitBlockEvent() { ProjectileHitBlockHook::hook(); }

// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
// NOLINTEND(modernize-use-trailing-return-type)

} // namespace lse::events
