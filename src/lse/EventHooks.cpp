#include "EventHooks.h"

#include "api/BlockAPI.h"
#include "api/EventAPI.h"
#include "api/ItemAPI.h"
#include "api/PlayerAPI.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include "mc/entity/WeakEntityRef.h"
#include "mc/entity/utilities/ActorType.h"
#include "mc/server/module/VanillaServerGameplayEventListener.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/events/EventResult.h"
#include "mc/world/level/BlockEventCoordinator.h"
#include "mc/world/level/block/actor/BarrelBlockActor.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/block/actor/ChestBlockActor.h"

namespace lse {
namespace EventHooks {
LL_AUTO_TYPE_INSTANCE_HOOK(
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

LL_AUTO_TYPE_INSTANCE_HOOK(
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

LL_AUTO_TYPE_INSTANCE_HOOK(
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

LL_AUTO_TYPE_INSTANCE_HOOK(
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
}

LL_AUTO_TYPE_INSTANCE_HOOK(
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
}

void PlayerStartDestroyBlock() { PlayerStartDestroyHook::hook(); }
void PlayerDropItem() { PlayerDropItemHook::hook(); }
void PlayerOpenContainerEvent() { PlayerOpenContainerHook::hook(); }
void PlayerCloseContainerEvent() {
    PlayerCloseContainerHook1::hook();
    PlayerCloseContainerHook2::hook();
}
} // namespace EventHooks
} // namespace lse
