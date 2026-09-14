#pragma once

namespace lse::events::block {
void ContainerChangeEvent();
void ArmorStandSwapItemEvent();
void PressurePlateTriggerEvent();
void FarmDecayEvent();
void PistonPushEvent();
void ExplodeEvent();
void RespawnAnchorExplodeEvent();
void PortalSpawnEvent();
void BlockExplodedEvent();
void RedstoneUpdateEvent();
void DispenseItemEvent();
void LiquidFlowEvent();
void CommandBlockExecuteEvent();
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
