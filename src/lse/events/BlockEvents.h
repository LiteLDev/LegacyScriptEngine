#pragma once

namespace lse::events::block {
void ContainerChangeEvent();
void RespawnAnchorExplodeEvent();
void PortalSpawnEvent();
void BlockExplodedEvent();
void CommandBlockExecuteEvent();
void DispenseItemEvent();
void HopperEvent(bool pullIn);

void onBlockExplode();
void onRedStoneUpdate();
void onLiquidFlow();
void onFarmLandDecay();
void onPistonTryPush();
void onPistonPush();
void onFireSpread();
void onBlockChanged();
} // namespace lse::events::block
