#include "SimulatedPlayerHelper.h"

#include "mc/entity/components_json_legacy/NavigationComponent.h"
#include "mc/server/sim/MoveInDirectionIntent.h"
#include "mc/server/sim/MoveToPositionIntent.h"
#include "mc/server/sim/NavigateToEntityIntent.h"
#include "mc/server/sim/NavigateToPositionsIntent.h"
#include "mc/server/sim/sim.h"
#include "mc/world/actor/ai/navigation/PathNavigation.h"
#include "mc/world/actor/provider/MobMovement.h"
#include "mc/world/gamemode/GameMode.h"
#include "mc/world/level/BlockPos.h"

namespace lse::api {

bool SimulatedPlayerHelper::simulateUseItem(SimulatedPlayer& player, ItemStack& item) {
    if (player.isAlive() && item.mValid_DeprecatedSeeComment && item.mItem && !item.isNull() && item.mCount) {
        return player.mGameMode->useItem(item);
    }
    return false;
}

bool SimulatedPlayerHelper::simulateUseItemInSlotOnBlock(
    SimulatedPlayer&                    player,
    int                                 slot,
    const BlockPos&                     pos,
    ScriptModuleMinecraft::ScriptFacing face,
    const Vec3&                         facePos
) {
    auto& itemStack = player.setSelectedSlot(slot);
    return player.simulateUseItemOnBlock(const_cast<ItemStack&>(itemStack), pos, face, facePos);
}

} // namespace lse::api