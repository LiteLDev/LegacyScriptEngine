#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include "ll/api/service/Bedrock.h"
#include "lse/api/Thread.h"
#include "mc/legacy/ActorUniqueID.h"
#include "mc/scripting/modules/minecraft/events/ScriptBlockGlobalEventListener.h"
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
#include "mc/world/level/block/DispenserBlock.h"
#include "mc/world/level/block/DoorBlock.h"
#include "mc/world/level/block/FarmBlock.h"
#include "mc/world/level/block/FenceGateBlock.h"
#include "mc/world/level/block/HopperBlock.h"
#include "mc/world/level/block/LiquidBlock.h"
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
#include "mc/world/level/block/block_events/BlockRedstoneUpdateEvent.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/material/Material.h"

namespace lse::events::block {
using api::thread::checkClientIsServerThread;

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
        if (checkClientIsServerThread()) {
            if (*reinterpret_cast<void***>(this) != LevelContainerModel::$vftable())
                return origin(slotNumber, oldItem, newItem);

            // Player::hasOpenContainer()
            if (mPlayer.mContainerManager) {
                if (!CallEvent(
                        EVENT_TYPES::onContainerChange,
                        PlayerClass::newPlayer(&mPlayer),
                        BlockClass::newBlock(mBlockPos, mPlayer.getDimensionId().id),
                        Number::newNumber(slotNumber + this->_getContainerOffset()),
                        ItemClass::newItem(&const_cast<ItemStack&>(oldItem)),
                        ItemClass::newItem(&const_cast<ItemStack&>(newItem))
                    )) {
                    return;
                }
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
        if (checkClientIsServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onChangeArmorStand,
                    EntityClass::newEntity(this),
                    PlayerClass::newPlayer(&player),
                    Number::newNumber((int)slot)
                )) {
                return false;
            }
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
        if (checkClientIsServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onStepOnPressurePlate,
                    EntityClass::newEntity(&entity),
                    BlockClass::newBlock(pos, region.getDimensionId())
                )) {
                return false;
            }
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
        if (checkClientIsServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onFarmLandDecay,
                    IntPos::newPos(pos, region.getDimensionId()),
                    EntityClass::newEntity(actor)
                )) {
                return;
            }
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
        if (checkClientIsServerThread()) {
            if (region.getBlock(curPos).isAir()) {
                return origin(region, curPos, curBranchFacing, pistonMoveFacing);
            }
            if (!CallEvent(
                    EVENT_TYPES::onPistonTryPush,
                    IntPos::newPos(this->mPosition, region.getDimensionId()),
                    BlockClass::newBlock(curPos, region.getDimensionId())
                )) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onPistonTryPush);
    bool shouldPush = origin(region, curPos, curBranchFacing, pistonMoveFacing);
    IF_LISTENED(EVENT_TYPES::onPistonPush) {
        if (checkClientIsServerThread() && shouldPush) {
            CallEvent( // Not cancellable
                EVENT_TYPES::onPistonPush,
                IntPos::newPos(this->mPosition, region.getDimensionId()),
                BlockClass::newBlock(curPos, region.getDimensionId())
            );
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onPistonPush);
    return shouldPush;
}

LL_TYPE_INSTANCE_HOOK(ExplodeHook, HookPriority::Normal, Explosion, &Explosion::explode, bool, ::IRandom& random) {
    IF_LISTENED(EVENT_TYPES::onEntityExplode) {
        if (checkClientIsServerThread()) {
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
    }
    IF_LISTENED_END(EVENT_TYPES::onEntityExplode);

    IF_LISTENED(EVENT_TYPES::onBlockExplode) {
        if (checkClientIsServerThread()) {
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
    }
    IF_LISTENED_END(EVENT_TYPES::onBlockExplode);
    return origin(random);
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
        if (checkClientIsServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onRespawnAnchorExplode,
                    IntPos::newPos(pos, region.getDimensionId()),
                    PlayerClass::newPlayer(&player)
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onRespawnAnchorExplode);
    origin(player, pos, region, level);
}

LL_TYPE_INSTANCE_HOOK(
    BlockExplodedHook,
    HookPriority::Normal,
    ScriptModuleMinecraft::ScriptBlockGlobalEventListener,
    &ScriptBlockGlobalEventListener::$onBlockExploded,
    EventResult,
    Dimension&      dimension,
    BlockPos const& blockPos,
    Block const&    destroyedBlock,
    Actor*          source
) {
    IF_LISTENED(EVENT_TYPES::onBlockExploded) {
        if (checkClientIsServerThread()) {
            if (destroyedBlock.isAir()) {
                return origin(dimension, blockPos, destroyedBlock, source);
            }
            CallEvent(
                EVENT_TYPES::onBlockExploded,
                BlockClass::newBlock(destroyedBlock, blockPos, dimension.getDimensionId()),
                EntityClass::newEntity(source)
            );
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onBlockExploded);
    return origin(dimension, blockPos, destroyedBlock, source);
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

#define REDSTONE_EVNET_HOOK_OLD(BLOCK)                                                                                 \
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
            if (checkClientIsServerThread()) {                                                                         \
                if (!RedstoneUpdateEvent(region, pos, strength, isFirstTime)) {                                        \
                    return;                                                                                            \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
        IF_LISTENED_END(EVENT_TYPES::onRedStoneUpdate);                                                                \
        origin(region, pos, strength, isFirstTime);                                                                    \
    }

#define REDSTONE_EVNET_HOOK_1(BLOCK)                                                                                   \
    LL_TYPE_INSTANCE_HOOK(                                                                                             \
        BLOCK##Hook,                                                                                                   \
        HookPriority::Normal,                                                                                          \
        BLOCK,                                                                                                         \
        &BLOCK::$_onRedstoneUpdate,                                                                                    \
        void,                                                                                                          \
        BlockEvents::BlockRedstoneUpdateEvent& blockEvent                                                              \
    ) {                                                                                                                \
        IF_LISTENED(EVENT_TYPES::onRedStoneUpdate) {                                                                   \
            if (checkClientIsServerThread()) {                                                                         \
                if (!RedstoneUpdateEvent(                                                                              \
                        blockEvent.mRegion,                                                                            \
                        blockEvent.mPos,                                                                               \
                        blockEvent.mSignalLevel,                                                                       \
                        blockEvent.mIsFirstTime                                                                        \
                    )) {                                                                                               \
                    return;                                                                                            \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
        IF_LISTENED_END(EVENT_TYPES::onRedStoneUpdate);                                                                \
        origin(blockEvent);                                                                                            \
    }

#define REDSTONE_EVNET_HOOK_2(BLOCK)                                                                                   \
    LL_TYPE_INSTANCE_HOOK(                                                                                             \
        BLOCK##Hook,                                                                                                   \
        HookPriority::Normal,                                                                                          \
        BLOCK,                                                                                                         \
        &BLOCK::_onRedstoneUpdate,                                                                                     \
        void,                                                                                                          \
        BlockEvents::BlockRedstoneUpdateEvent& blockEvent                                                              \
    ) {                                                                                                                \
        IF_LISTENED(EVENT_TYPES::onRedStoneUpdate) {                                                                   \
            if (checkClientIsServerThread()) {                                                                         \
                if (!RedstoneUpdateEvent(                                                                              \
                        blockEvent.mRegion,                                                                            \
                        blockEvent.mPos,                                                                               \
                        blockEvent.mSignalLevel,                                                                       \
                        blockEvent.mIsFirstTime                                                                        \
                    )) {                                                                                               \
                    return;                                                                                            \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
        IF_LISTENED_END(EVENT_TYPES::onRedStoneUpdate);                                                                \
        origin(blockEvent);                                                                                            \
    }

REDSTONE_EVNET_HOOK_OLD(HopperBlock)
REDSTONE_EVNET_HOOK_OLD(CrafterBlock)
REDSTONE_EVNET_HOOK_OLD(CommandBlock)
REDSTONE_EVNET_HOOK_OLD(BigDripleafBlock)
REDSTONE_EVNET_HOOK_OLD(CopperBulbBlock)
REDSTONE_EVNET_HOOK_OLD(DoorBlock)
REDSTONE_EVNET_HOOK_OLD(FenceGateBlock)
REDSTONE_EVNET_HOOK_OLD(DispenserBlock)
REDSTONE_EVNET_HOOK_OLD(StructureBlock)
REDSTONE_EVNET_HOOK_OLD(TrapDoorBlock)
REDSTONE_EVNET_HOOK_OLD(NoteBlock)
REDSTONE_EVNET_HOOK_OLD(RedstoneLampBlock)
REDSTONE_EVNET_HOOK_OLD(TntBlock)

REDSTONE_EVNET_HOOK_1(BaseRailBlock)
REDSTONE_EVNET_HOOK_1(PoweredRailBlock)
REDSTONE_EVNET_HOOK_1(ActivatorRailBlock)

REDSTONE_EVNET_HOOK_2(RedStoneWireBlock)
REDSTONE_EVNET_HOOK_2(RedstoneTorchBlock)
REDSTONE_EVNET_HOOK_2(ComparatorBlock)

#undef REDSTONE_EVNET_HOOK_OLD
#undef REDSTONE_EVNET_HOOK

} // namespace redstone

bool materialsAreEqual(Material const& a, Material const& b) {
    return a.mType == b.mType && a.mNeverBuildable == b.mNeverBuildable && a.mLiquid == b.mLiquid
        && a.mBlocksMotion == b.mBlocksMotion && a.mBlocksPrecipitation == b.mBlocksPrecipitation
        && a.mSolid == b.mSolid && a.mSuperHot == b.mSuperHot;
}

bool liquidBlockCanSpreadTo(
    LiquidBlock&    liquidBlock,
    BlockSource&    region,
    BlockPos const& pos,
    BlockPos const& flowFromPos,
    uchar           flowFromDirection
) {
    if (pos.y < region.getMinHeight()) {
        return false;
    }
    if (const auto& block = region.getLiquidBlock(pos);
        materialsAreEqual(block.getBlockType().mMaterial, liquidBlock.mMaterial)
        || block.getBlockType().mMaterial.mType == MaterialType::Lava
        || liquidBlock._isLiquidBlocking(region, pos, flowFromPos, flowFromDirection)) {
        return false;
    }
    return true;
}

LL_TYPE_INSTANCE_HOOK(
    LiquidFlowHook,
    HookPriority::Normal,
    LiquidBlock,
    &LiquidBlock::_trySpreadTo,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    int               neighbor,
    ::BlockPos const& flowFromPos,
    uchar             flowFromDirection
) {
    IF_LISTENED(EVENT_TYPES::onLiquidFlow) {
        if (checkClientIsServerThread() && liquidBlockCanSpreadTo(*this, region, pos, flowFromPos, flowFromDirection)) {
            if (!CallEvent(
                    EVENT_TYPES::onLiquidFlow,
                    region.isInstaticking(pos) ? Local<Value>() : BlockClass::newBlock(pos, region.getDimensionId()),
                    IntPos::newPos(pos, region.getDimensionId())
                )) {
                return;
            }
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
        if (checkClientIsServerThread()) {
            if (commandOrigin.getOriginType() == CommandOriginType::MinecartCommandBlock) {
                if (!CallEvent(
                        EVENT_TYPES::onCmdBlockExecute,
                        String::newString(this->mCommand),
                        FloatPos::newPos(commandOrigin.getEntity()->getPosition(), region.getDimensionId()),
                        Boolean::newBoolean(true)
                    )) {
                    return false;
                }
            } else {
                if (!CallEvent(
                        EVENT_TYPES::onCmdBlockExecute,
                        String::newString(this->mCommand),
                        FloatPos::newPos(commandOrigin.getBlockPosition(), region.getDimensionId()),
                        Boolean::newBoolean(false)
                    )) {
                    return false;
                }
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
    if (checkClientIsServerThread()) {
        hopperStatus = HopperStatus::PullIn;
        hopperPos    = pos;
    }
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
    if (checkClientIsServerThread()) {
        hopperStatus = HopperStatus::PullOut;
        hopperPos    = position;
    }
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
        if (checkClientIsServerThread() && hopperStatus == HopperStatus::PullIn) {
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
        if (checkClientIsServerThread() && hopperStatus == HopperStatus::PullOut) {
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
