#include "mc/server/SimulatedPlayer.h"
#include "mc/server/sim/LookDuration.h"

namespace lse::api {
class SimulatedPlayerHelper {
public:
    static bool simulateUseItem(SimulatedPlayer& player, ItemStack& item);
    static bool simulateUseItemInSlotOnBlock(
        SimulatedPlayer&                    player,
        int                                 slot,
        BlockPos const&                     pos,
        ScriptModuleMinecraft::ScriptFacing face,
        Vec3 const&                         facePos
    );
};
} // namespace lse::api