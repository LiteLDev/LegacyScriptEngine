#pragma once

namespace lse::events::block {
void ContainerChangeEvent();
void ArmorStandSwapItemEvent();
void PressurePlateTriggerEvent();
void FarmDecayEvent();
void PistonPushEvent();
void ExplodeEvent();
void RespawnAnchorExplodeEvent();
void BlockExplodedEvent();
void RedstoneUpdateEvent();
void LiquidFlowEvent();
void CommandBlockExecuteEvent();
void HopperEvent(bool pullIn);
}