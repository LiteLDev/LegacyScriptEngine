#include "SimulatedPlayerHelper.h"

#include "mc/deps/ecs/gamerefs_entity/EntityContext.h"
#include "mc/entity/components_json_legacy/NavigationComponent.h"
#include "mc/server/sim/LookAtIntent.h"
#include "mc/server/sim/MoveInDirectionIntent.h"
#include "mc/server/sim/MoveToPositionIntent.h"
#include "mc/server/sim/MovementIntent.h"
#include "mc/server/sim/NavigateToEntityIntent.h"
#include "mc/server/sim/NavigateToPositionsIntent.h"
#include "mc/server/sim/VoidMoveIntent.h"
#include "mc/server/sim/sim.h"
#include "mc/world/actor/ai/navigation/PathNavigation.h"
#include "mc/world/actor/provider/MobMovement.h"
#include "mc/world/gamemode/GameMode.h"
#include "mc/world/level/BlockPos.h"

namespace lse::api {
bool SimulatedPlayerHelper::simulateRespawn(SimulatedPlayer& player) {
    auto& spawnPos = player.mPlayerRespawnPoint->mPlayerPosition;
    Vec3  respawnPos;
    respawnPos.x                     = (float)spawnPos->x + 0.5f;
    respawnPos.y                     = (float)spawnPos->y + 1.62001f;
    respawnPos.z                     = (float)spawnPos->z + 0.5f;
    player.mRespawnPositionCandidate = respawnPos;
    player.mRespawnReady             = true;
    player.mRespawningFromTheEnd     = false;
    if (player.isAlive() || !player.mRespawnReady) {
        return false;
    }
    player.respawn();
    return true;
}

void SimulatedPlayerHelper::simulateLookAt(SimulatedPlayer& player, Actor& actor, sim::LookDuration lookType) {
    sim::lookAt(player, actor.getEntityContext(), lookType);
}

void SimulatedPlayerHelper::simulateLookAt(
    SimulatedPlayer&  player,
    BlockPos const&   blockPos,
    sim::LookDuration lookType
) {
    glm::vec3 vec3;
    vec3.x = (float)blockPos.x + 0.5f;
    vec3.y = (float)blockPos.y + 0.5f;
    vec3.z = (float)blockPos.z + 0.5f;
    sim::lookAt(player, vec3, lookType);
}

void SimulatedPlayerHelper::simulateLookAt(SimulatedPlayer& player, Vec3 const& pos, sim::LookDuration lookType) {
    glm::vec3 vec3(pos.x, pos.y, pos.z);
    sim::lookAt(player, vec3, lookType);
}

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

void SimulatedPlayerHelper::simulateStopUsingItem(SimulatedPlayer& player) {
    if (player.isAlive()) {
        player.releaseUsingItem();
    }
}

void SimulatedPlayerHelper::simulateStopMoving(SimulatedPlayer& player) {
    auto& type = player.mSimulatedMovement->mType.get();
    if (std::holds_alternative<sim::MoveInDirectionIntent>(type)
        || std::holds_alternative<sim::MoveToPositionIntent>(type)) {
        MobMovement::setLocalMoveVelocity(player.getEntityContext(), Vec3::ZERO());
    } else if (std::holds_alternative<sim::NavigateToPositionsIntent>(type)
               || std::holds_alternative<sim::NavigateToEntityIntent>(type)) {
        MobMovement::setLocalMoveVelocity(player.getEntityContext(), Vec3::ZERO());
        auto component = player.getEntityContext().tryGetComponent<NavigationComponent>();
        if (component) {
            component->mNavigation->stop(component, player);
        }
    }
}
} // namespace lse::api