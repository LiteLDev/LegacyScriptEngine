#include "EventHooks.h"

#include "api/BlockAPI.h"
#include "api/EventAPI.h"
#include "api/PlayerAPI.h"
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

void PlayerStartDestroyBlock() { PlayerStartDestroyHook::hook(); }
} // namespace EventHooks
} // namespace lse
