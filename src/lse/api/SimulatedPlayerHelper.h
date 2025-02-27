#include "mc/server/SimulatedPlayer.h"
#include "mc/server/sim/LookDuration.h"

namespace lse::api {
class SimulatedPlayerHelper {
public:
    static bool simulateRespawn(SimulatedPlayer& player);
    static void simulateLookAt(SimulatedPlayer& player, Actor& actor, sim::LookDuration lookType);
    static void simulateLookAt(SimulatedPlayer& player, BlockPos const& blockPos, sim::LookDuration lookType);
    static void simulateLookAt(SimulatedPlayer& player, Vec3 const& pos, sim::LookDuration lookType);
    static bool simulateUseItem(SimulatedPlayer& player, ItemStack& item);
    static bool simulateUseItemInSlotOnBlock(
        SimulatedPlayer&                    player,
        int                                 slot,
        BlockPos const&                     pos,
        ScriptModuleMinecraft::ScriptFacing face,
        Vec3 const&                         facePos
    );
    static void simulateStopUsingItem(SimulatedPlayer& player);
    static void simulateStopMoving(SimulatedPlayer& player);
};
} // namespace lse::api