#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include "ll/api/service/Bedrock.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/actor/ArmorStand.h"
#include "mc/world/actor/Hopper.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/containers/models/LevelContainerModel.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Explosion.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/ActivatorRailBlock.h"
#include "mc/world/level/block/BasePressurePlateBlock.h"
#include "mc/world/level/block/BaseRailBlock.h"
#include "mc/world/level/block/BigDripleafBlock.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/CommandBlock.h"
#include "mc/world/level/block/ComparatorBlock.h"
#include "mc/world/level/block/CopperBulbBlock.h"
#include "mc/world/level/block/CrafterBlock.h"
#include "mc/world/level/block/DiodeBlock.h"
#include "mc/world/level/block/DispenserBlock.h"
#include "mc/world/level/block/DoorBlock.h"
#include "mc/world/level/block/FarmBlock.h"
#include "mc/world/level/block/FenceGateBlock.h"
#include "mc/world/level/block/HopperBlock.h"
#include "mc/world/level/block/LiquidBlockDynamic.h"
#include "mc/world/level/block/NoteBlock.h"
#include "mc/world/level/block/PoweredRailBlock.h"
#include "mc/world/level/block/RedStoneWireBlock.h"
#include "mc/world/level/block/RedstoneLampBlock.h"
#include "mc/world/level/block/RedstoneTorchBlock.h"
#include "mc/world/level/block/RespawnAnchorBlock.h"
#include "mc/world/level/block/StructureBlock.h"
#include "mc/world/level/block/TntBlock.h"
#include "mc/world/level/block/TrapDoorBlock.h"
#include "mc/world/level/block/actor/BaseCommandBlock.h"
#include "mc/world/level/block/actor/PistonBlockActor.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/material/Material.h"

namespace lse::events::block {
LL_TYPE_INSTANCE_HOOK(
    ContainerChangeHook,
    HookPriority::Normal,
    LevelContainerModel,
    &LevelContainerModel::$_onItemChanged,
    void,
    int              slotNumber,
    ItemStack const& oldItem,
    ItemStack const& newItem
) {
    IF_LISTENED(EVENT_TYPES::onContainerChange) {
        if (*reinterpret_cast<void***>(this) != LevelContainerModel::$vftable())
            return origin(slotNumber, oldItem, newItem);

        Player& player = mUnk84d147.as<Player&>();
        if (player.hasOpenContainer()) {
            if (!CallEvent(
                    EVENT_TYPES::onContainerChange,
                    PlayerClass::newPlayer(&player),
                    BlockClass::newBlock(mUnk74419a.as<BlockPos>(), player.getDimensionId()),
                    Number::newNumber(slotNumber + this->_getContainerOffset()),
                    ItemClass::newItem(&const_cast<ItemStack&>(oldItem)),
                    ItemClass::newItem(&const_cast<ItemStack&>(newItem))
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onContainerChange);
    origin(slotNumber, oldItem, newItem);
}

LL_TYPE_INSTANCE_HOOK(
    ArmorStandSwapItemHook,
    HookPriority::Normal,
    ArmorStand,
    &ArmorStand::_trySwapItem,
    bool,
    Player&                              player,
    ::SharedTypes::Legacy::EquipmentSlot slot
) {
    IF_LISTENED(EVENT_TYPES::onChangeArmorStand) {
        if (!CallEvent(
                EVENT_TYPES::onChangeArmorStand,
                EntityClass::newEntity(this),
                PlayerClass::newPlayer(&player),
                Number::newNumber((int)slot)
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onChangeArmorStand);
    return origin(player, slot);
}

LL_TYPE_INSTANCE_HOOK(
    PressurePlateTriggerHook,
    HookPriority::Normal,
    BasePressurePlateBlock,
    &BasePressurePlateBlock::$shouldTriggerEntityInside,
    bool,
    BlockSource&    region,
    BlockPos const& pos,
    Actor&          entity
) {
    IF_LISTENED(EVENT_TYPES::onStepOnPressurePlate) {
        if (!CallEvent(
                EVENT_TYPES::onStepOnPressurePlate,
                EntityClass::newEntity(&entity),
                BlockClass::newBlock(pos, region.getDimensionId())
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onStepOnPressurePlate);
    return origin(region, pos, entity);
}

LL_TYPE_INSTANCE_HOOK(
    FarmDecayHook,
    HookPriority::Normal,
    FarmBlock,
    &FarmBlock::$transformOnFall,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    Actor*          actor,
    float           fallDistance
) {
    IF_LISTENED(EVENT_TYPES::onFarmLandDecay) {
        if (!CallEvent(
                EVENT_TYPES::onFarmLandDecay,
                IntPos::newPos(pos, region.getDimensionId()),
                EntityClass::newEntity(actor)
            )) {
            return;
        }
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
        if (!CallEvent(
                EVENT_TYPES::onPistonTryPush,
                IntPos::newPos(this->getPosition(), region.getDimensionId()),
                BlockClass::newBlock(curPos, region.getDimensionId())
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onPistonTryPush);
    bool shouldPush = origin(region, curPos, curBranchFacing, pistonMoveFacing);
    IF_LISTENED(EVENT_TYPES::onPistonPush) {
        if (shouldPush) {
            CallEvent( // Not cancellable
                EVENT_TYPES::onPistonPush,
                IntPos::newPos(this->getPosition(), region.getDimensionId()),
                BlockClass::newBlock(curPos, region.getDimensionId())
            );
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onPistonPush);
    return shouldPush;
}

LL_TYPE_INSTANCE_HOOK(ExplodeHook, HookPriority::Normal, Explosion, &Explosion::explode, bool) {
    IF_LISTENED(EVENT_TYPES::onEntityExplode) {
        if (mSourceID->rawID != ActorUniqueID::INVALID_ID().rawID) {
            if (!CallEvent(
                    EVENT_TYPES::onEntityExplode,
                    EntityClass::newEntity(ll::service::getLevel()->fetchEntity(mSourceID, false)),
                    FloatPos::newPos(mPos, mRegion.getDimensionId()),
                    Number::newNumber(mRadius),
                    Number::newNumber(mMaxResistance),
                    Boolean::newBoolean(mBreaking),
                    Boolean::newBoolean(mFire)
                )) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onEntityExplode);

    IF_LISTENED(EVENT_TYPES::onBlockExplode) {
        if (!CallEvent(
                EVENT_TYPES::onBlockExplode,
                BlockClass::newBlock(*mPos, mRegion.getDimensionId()),
                FloatPos::newPos(mPos, mRegion.getDimensionId()),
                Number::newNumber(mRadius),
                Number::newNumber(mMaxResistance),
                Boolean::newBoolean(mBreaking),
                Boolean::newBoolean(mFire)
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onBlockExplode);
    return origin();
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
        if (!CallEvent(
                EVENT_TYPES::onRespawnAnchorExplode,
                IntPos::newPos(pos, region.getDimensionId()),
                PlayerClass::newPlayer(&player)
            )) {
            return;
        }
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
        CallEvent(
            EVENT_TYPES::onBlockExploded,
            BlockClass::newBlock(pos, region.getDimensionId()),
            EntityClass::newEntity(entitySource)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onBlockExploded);
    origin(region, pos, entitySource);
}

namespace redstone {
inline bool RedstoneUpdateEvent(BlockSource& region, BlockPos const& pos, int& strength, bool& isFirstTime) {
    if (!CallEvent(
            EVENT_TYPES::onRedStoneUpdate,
            BlockClass::newBlock(pos, region.getDimensionId()),
            Number::newNumber(strength),
            Boolean::newBoolean(isFirstTime)
        )) {
        return false;
    }
    return true;
}

#define REDSTONEHOOK(BLOCK)                                                                                            \
    LL_TYPE_INSTANCE_HOOK(                                                                                             \
        BLOCK##Hook,                                                                                                   \
        HookPriority::Normal,                                                                                          \
        BLOCK,                                                                                                         \
        &BLOCK::$onRedstoneUpdate,                                                                                     \
        void,                                                                                                          \
        BlockSource&    region,                                                                                        \
        BlockPos const& pos,                                                                                           \
        int             strength,                                                                                      \
        bool            isFirstTime                                                                                    \
    ) {                                                                                                                \
        IF_LISTENED(EVENT_TYPES::onRedStoneUpdate) {                                                                   \
            if (!RedstoneUpdateEvent(region, pos, strength, isFirstTime)) {                                            \
                return;                                                                                                \
            }                                                                                                          \
        }                                                                                                              \
        IF_LISTENED_END(EVENT_TYPES::onRedStoneUpdate);                                                                \
        origin(region, pos, strength, isFirstTime);                                                                    \
    }

REDSTONEHOOK(RedStoneWireBlock)
REDSTONEHOOK(DiodeBlock)
REDSTONEHOOK(RedstoneTorchBlock)
REDSTONEHOOK(ComparatorBlock)
REDSTONEHOOK(HopperBlock)
REDSTONEHOOK(CrafterBlock)
REDSTONEHOOK(CommandBlock)
REDSTONEHOOK(BaseRailBlock)
REDSTONEHOOK(PoweredRailBlock)
REDSTONEHOOK(BigDripleafBlock)
REDSTONEHOOK(CopperBulbBlock)
REDSTONEHOOK(DoorBlock)
REDSTONEHOOK(FenceGateBlock)
REDSTONEHOOK(DispenserBlock)
REDSTONEHOOK(StructureBlock)
REDSTONEHOOK(TrapDoorBlock)
REDSTONEHOOK(NoteBlock)
REDSTONEHOOK(ActivatorRailBlock)
REDSTONEHOOK(RedstoneLampBlock)
REDSTONEHOOK(TntBlock)

#undef REDSTONEHOOK

} // namespace redstone

LL_TYPE_INSTANCE_HOOK(
    LiquidFlowHook,
    HookPriority::Normal,
    LiquidBlockDynamic,
    &LiquidBlockDynamic::_trySpreadTo,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    int               neighbor,
    ::BlockPos const& flowFromPos,
    uchar             flowFromDirection
) {
    IF_LISTENED(EVENT_TYPES::onLiquidFlow) {
        if (!CallEvent(
                EVENT_TYPES::onLiquidFlow,
                region.isInstaticking(pos) ? Local<Value>() : BlockClass::newBlock(pos, region.getDimensionId()),
                IntPos::newPos(pos, region.getDimensionId())
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onLiquidFlow);
    return origin(region, pos, neighbor, flowFromPos, flowFromDirection);
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
            if (!CallEvent(
                    EVENT_TYPES::onCmdBlockExecute,
                    String::newString(this->getCommand()),
                    FloatPos::newPos(commandOrigin.getEntity()->getPosition(), region.getDimensionId()),
                    Boolean::newBoolean(true)
                )) {
                return false;
            }
        } else {
            if (!CallEvent(
                    EVENT_TYPES::onCmdBlockExecute,
                    String::newString(this->getCommand()),
                    FloatPos::newPos(commandOrigin.getBlockPosition(), region.getDimensionId()),
                    Boolean::newBoolean(false)
                )) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onCmdBlockExecute);
    return origin(region, commandOrigin, markForSaving);
}

namespace hopper {
enum class HopperStatus { None, PullIn, PullOut } hopperStatus = HopperStatus::None;
Vec3 hopperPos;

LL_TYPE_INSTANCE_HOOK(
    HopperPullInHook,
    HookPriority::Normal,
    Hopper,
    &Hopper::_tryPullInItemsFromAboveContainer,
    bool,
    BlockSource& region,
    Container&   toContainer,
    Vec3 const&  pos
) {
    hopperStatus = HopperStatus::PullIn;
    hopperPos    = pos;
    return origin(region, toContainer, pos);
}

LL_TYPE_INSTANCE_HOOK(
    HopperPushOutHook,
    HookPriority::Normal,
    Hopper,
    &Hopper::_pushOutItems,
    bool,
    BlockSource& region,
    Container&   fromContainer,
    Vec3 const&  position,
    int          attachedFace
) {
    hopperStatus = HopperStatus::PullOut;
    hopperPos    = position;
    return origin(region, fromContainer, position, attachedFace);
}

LL_TYPE_INSTANCE_HOOK(
    HopperAddItemHook,
    HookPriority::Normal,
    Hopper,
    &Hopper::_tryMoveInItem,
    bool,
    ::BlockSource& region,
    ::Container&   container,
    ::ItemStack&   item,
    int            slot,
    int            face,
    int            itemCount
) {
    IF_LISTENED(EVENT_TYPES::onHopperSearchItem) {
        if (hopperStatus == HopperStatus::PullIn) {
            if (!CallEvent(
                    EVENT_TYPES::onHopperSearchItem,
                    FloatPos::newPos(hopperPos, region.getDimensionId()),
                    Boolean::newBoolean(this->mIsEntity),
                    ItemClass::newItem(&item)
                )) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onHopperSearchItem);
    IF_LISTENED(EVENT_TYPES::onHopperPushOut) {
        if (hopperStatus == HopperStatus::PullOut) {
            if (!CallEvent(
                    EVENT_TYPES::onHopperPushOut,
                    FloatPos::newPos(hopperPos, region.getDimensionId()),
                    Boolean::newBoolean(this->mIsEntity),
                    ItemClass::newItem(&item)
                )) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onHopperPushOut);
    hopperStatus = HopperStatus::None;
    return origin(region, container, item, slot, face, itemCount);
}
} // namespace hopper

void ContainerChangeEvent() { ContainerChangeHook::hook(); }
void ArmorStandSwapItemEvent() { ArmorStandSwapItemHook::hook(); }
void PressurePlateTriggerEvent() { PressurePlateTriggerHook::hook(); }
void FarmDecayEvent() { FarmDecayHook::hook(); }
void PistonPushEvent() { PistonPushHook::hook(); }
void ExplodeEvent() { ExplodeHook::hook(); }
void RespawnAnchorExplodeEvent() { RespawnAnchorExplodeHook::hook(); }
void BlockExplodedEvent() { BlockExplodedHook ::hook(); }
void RedstoneUpdateEvent() {
    redstone::RedstoneTorchBlockHook::hook();
    redstone::RedStoneWireBlockHook::hook();
    redstone::DiodeBlockHook::hook();
    redstone::ComparatorBlockHook::hook();
    redstone::HopperBlockHook::hook();
    redstone::CrafterBlockHook::hook();
    redstone::CommandBlockHook::hook();
    redstone::BaseRailBlockHook::hook();
    redstone::PoweredRailBlockHook::hook();
    redstone::BigDripleafBlockHook::hook();
    redstone::CopperBulbBlockHook::hook();
    redstone::DoorBlockHook::hook();
    redstone::FenceGateBlockHook::hook();
    redstone::DispenserBlockHook::hook();
    redstone::StructureBlockHook::hook();
    redstone::TrapDoorBlockHook::hook();
    redstone::NoteBlockHook::hook();
    redstone::ActivatorRailBlockHook::hook();
    redstone::RedstoneLampBlockHook::hook();
    redstone::TntBlockHook::hook();
}
void LiquidFlowEvent() { LiquidFlowHook::hook(); }
void CommandBlockExecuteEvent() { CommandBlockExecuteHook::hook(); }
void HopperEvent(bool pullIn) {
    hopper::HopperAddItemHook::hook();
    if (pullIn) {
        hopper::HopperPullInHook::hook();
    } else {
        hopper::HopperPushOutHook::hook();
    }
}
} // namespace lse::events::block