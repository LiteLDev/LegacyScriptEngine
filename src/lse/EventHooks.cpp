#include "EventHooks.h"

#include "api/BlockAPI.h"
#include "api/EntityAPI.h"
#include "api/EventAPI.h"
#include "api/ItemAPI.h"
#include "api/PlayerAPI.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include "mc/entity/WeakEntityRef.h"
#include "mc/entity/utilities/ActorType.h"
#include "mc/server/module/VanillaServerGameplayEventListener.h"
#include "mc/world/actor/ArmorStand.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/containers/models/LevelContainerModel.h"
#include "mc/world/events/EventResult.h"
#include "mc/world/item/registry/ItemStack.h"
#include "mc/world/level/BlockEventCoordinator.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BarrelBlockActor.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/block/actor/ChestBlockActor.h"

namespace lse {
namespace EventHooks {
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
} // namespace EventHooks
} // namespace lse
