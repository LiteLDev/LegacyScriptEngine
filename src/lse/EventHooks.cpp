#include "EventHooks.h"

#include "api/BlockAPI.h"
#include "api/EventAPI.h"
#include "api/ItemAPI.h"
#include "api/PlayerAPI.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockEventCoordinator.h"

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

void PlayerStartDestroyBlock() { PlayerStartDestroyHook::hook(); }
void PlayerDropItem() { PlayerDropItemHook::hook(); }
} // namespace EventHooks
} // namespace lse
