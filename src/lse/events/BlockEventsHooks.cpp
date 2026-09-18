#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/ContainerAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/memory/Hook.h"
#include "lse/api/Thread.h"
#include "mc/scripting/modules/minecraft/events/ScriptBlockGlobalEventListener.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/actor/ArmorStand.h"
#include "mc/world/actor/Hopper.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/containers/models/LevelContainerModel.h"
#include "mc/world/item/Item.h"
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
#include "mc/world/level/block/PortalBlock.h"
#include "mc/world/level/block/PoweredRailBlock.h"
#include "mc/world/level/block/RedStoneWireBlock.h"
#include "mc/world/level/block/RedstoneLampBlock.h"
#include "mc/world/level/block/RedstoneTorchBlock.h"
#include "mc/world/level/block/RespawnAnchorBlock.h"
#include "mc/world/level/block/StructureBlock.h"
#include "mc/world/level/block/TntBlock.h"
#include "mc/world/level/block/TrapDoorBlock.h"
#include "mc/world/level/block/VanillaBlockTypeIds.h"
#include "mc/world/level/block/VanillaStates.h"
#include "mc/world/level/block/actor/BaseCommandBlock.h"
#include "mc/world/level/block/actor/PistonBlockActor.h"
#include "mc/world/level/block/block_events/BlockPlayerInteractEvent.h"
#include "mc/world/level/block/block_events/BlockRedstoneUpdateEvent.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/material/Material.h"
namespace lse::events::block {

using api::thread::isServerThread;

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
        if (isServerThread()) {
            if (*reinterpret_cast<void***>(this) != LevelContainerModel::$vftable())
                return origin(slotNumber, oldItem, newItem);

            // Player::hasOpenContainer()
            if (mPlayer.mContainerManager) {
                if (!CallEvent(
                        EVENT_TYPES::onContainerChange,
                        PlayerClass::newPlayer(&mPlayer),
                        BlockClass::newBlock(mBlockPos, mPlayer.getDimensionId()),
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
    UseRespawnAnchorHook,
    HookPriority::Normal,
    RespawnAnchorBlock,
    &RespawnAnchorBlock::use,
    void,
    ::BlockEvents::BlockPlayerInteractEvent& eventData
) {
    IF_LISTENED(EVENT_TYPES::onRespawnAnchorExplode) {
        if (isServerThread()) {
            // 原版逻辑：手持萤石且未满时 _tryCharge 会优先充能并短路，不会爆炸；
            // 充能大于 0 且不在下界（维度 1）时 _trySetSpawn 才会调用 _explode 引爆
            auto& block         = eventData.mPlayer.getDimensionBlockSource().getBlock(eventData.mPos);
            int   charge        = block.getState<int>(VanillaStates::RespawnAnchorCharge().mID).value_or(0);
            auto& item          = eventData.mPlayer.getSelectedItem();
            auto* itemBlockType = item.mItem ? item.mItem->mBlockType : nullptr;
            bool  isCharging = itemBlockType && *itemBlockType->mNameInfo->mFullName == VanillaBlockTypeIds::Glowstone()
                            && charge < static_cast<int>(VanillaStates::RespawnAnchorCharge().mVariationCount) - 1;
            if (charge > 0 && !isCharging && eventData.mPlayer.getDimensionId() != 1) {
                if (!CallEvent(
                        EVENT_TYPES::onRespawnAnchorExplode,
                        IntPos::newPos(eventData.mPos, eventData.mPlayer.getDimensionId()),
                        PlayerClass::newPlayer(&eventData.mPlayer)
                    )) {
                    return;
                }
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onRespawnAnchorExplode);
    origin(eventData);
}

LL_TYPE_STATIC_HOOK(
    PortalSpawnHook,
    HookPriority::Normal,
    PortalBlock,
    &PortalBlock::trySpawnPortal,
    bool,
    BlockSource&    region,
    BlockPos const& pos
) {
    IF_LISTENED(EVENT_TYPES::onPortalTrySpawn) {
        if (isServerThread()) {
            if (!CallEvent(EVENT_TYPES::onPortalTrySpawn, IntPos::newPos(pos, region.getDimensionId()))) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onPortalTrySpawn);
    return origin(region, pos);
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
        if (isServerThread()) {
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

bool materialsAreEqual(Material const& a, Material const& b) { return a.mType == b.mType; }

bool liquidBlockCanSpreadTo(
    LiquidBlock const& liquidBlock,
    BlockSource&       region,
    BlockPos const&    pos,
    BlockPos const&    flowFromPos,
    uchar              flowFromDirection
) {
    if (pos.y < region.getMinHeight() || !region.hasBlock(pos)) {
        return false;
    }
    if (auto const& block = region.getLiquidBlock(pos);
        materialsAreEqual(block.getBlockType().mMaterial, liquidBlock.mMaterial)
        || block.getBlockType().mMaterial.mType == SharedTypes::v1_26_20::MaterialType::Lava
        || LiquidBlock::_isLiquidBlocking(region, pos, flowFromPos, flowFromDirection)) {
        return false;
    }
    return true;
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
        if (isServerThread()) {
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

namespace dispenser {
LL_TYPE_STATIC_HOOK(
    DispenserEjectItemHook,
    HookPriority::Normal,
    DispenserBlock,
    &DispenserBlock::ejectItem,
    void,
    BlockSource&     region,
    Vec3 const&      pos,
    uchar            face,
    ItemStack const& item,
    Container&       container,
    int              slot,
    int              countLimit
) {
    IF_LISTENED(EVENT_TYPES::onDispenseItem) {
        if (isServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onDispenseItem,
                    FloatPos::newPos(pos, region.getDimensionId()),
                    ItemClass::newItem(&const_cast<ItemStack&>(item)),
                    Number::newNumber(slot),
                    Number::newNumber((int)face),
                    ContainerClass::newContainer(&container)
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onDispenseItem);
    origin(region, pos, face, item, container, slot, countLimit);
}
} // namespace dispenser

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
    if (isServerThread()) {
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
    if (isServerThread()) {
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
    ::BlockSource&                                                region,
    ::Container&                                                  container,
    ::std::optional<::std::reference_wrapper<::ItemActor>> const& itemActor,
    ::ItemStack&                                                  item,
    int                                                           slot,
    int                                                           face,
    int                                                           itemCount
) {
    IF_LISTENED(EVENT_TYPES::onHopperSearchItem) {
        if (isServerThread() && hopperStatus == HopperStatus::PullIn) {
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
        if (isServerThread() && hopperStatus == HopperStatus::PullOut) {
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
    return origin(region, container, itemActor, item, slot, face, itemCount);
}
} // namespace hopper

void ContainerChangeEvent() { static ll::memory::HookRegistrar<ContainerChangeHook> reg; }
void RespawnAnchorExplodeEvent() { static ll::memory::HookRegistrar<UseRespawnAnchorHook> reg; }
void PortalSpawnEvent() { static ll::memory::HookRegistrar<PortalSpawnHook> reg; }
void BlockExplodedEvent() { static ll::memory::HookRegistrar<BlockExplodedHook> reg; }
void CommandBlockExecuteEvent() { static ll::memory::HookRegistrar<CommandBlockExecuteHook> reg; }
void DispenseItemEvent() { static ll::memory::HookRegistrar<dispenser::DispenserEjectItemHook> reg; }
void HopperEvent(bool pullIn) {
    static ll::memory::HookRegistrar<hopper::HopperAddItemHook> reg;
    if (pullIn) {
        static ll::memory::HookRegistrar<hopper::HopperPullInHook> reg;
    } else {
        static ll::memory::HookRegistrar<hopper::HopperPushOutHook> reg;
    }
}
} // namespace lse::events::block
