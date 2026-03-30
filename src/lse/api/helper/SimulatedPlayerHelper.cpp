#include "SimulatedPlayerHelper.h"

#include "mc/entity/components_json_legacy/NavigationComponent.h"
#include "mc/world/gamemode/GameMode.h"

namespace lse::api {

bool SimulatedPlayerHelper::simulateUseItem(SimulatedPlayer const& player, ItemStack& item) {
    if (player.isAlive() && item.mValid_DeprecatedSeeComment && item.mItem && !item.isNull() && item.mCount) {
        return player.mGameMode->useItem(item);
    }
    return false;
}

bool SimulatedPlayerHelper::simulateUseItemInSlotOnBlock(
    SimulatedPlayer&                    player,
    int                                 slot,
    BlockPos const&                     pos,
    ScriptModuleMinecraft::ScriptFacing face,
    Vec3 const&                         facePos
) {
    auto& itemStack = player.setSelectedSlot(slot);
    return player.simulateUseItemOnBlock(const_cast<ItemStack&>(itemStack), pos, face, facePos);
}

} // namespace lse::api
