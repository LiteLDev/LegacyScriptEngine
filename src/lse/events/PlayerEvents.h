#pragma once

namespace lse::events::player {
void AttackBlockEvent();
void ChangeSlotEvent();
void CloseContainerEvent();
void DropItem();
void OpenContainerEvent();
void StartDestroyBlock();
void UseFrameEvent();
void EatEvent();
void ChangeDimensionEvent();
void OpenContainerScreenEvent();
void UseRespawnAnchorEvent();
void SleepEvent();
void OpenInventoryEvent();
void PullFishingHookEvent();
void UseBucketPlaceEvent();
void UseBucketTakeEvent();
void ConsumeTotemEvent();
void SetArmorEvent();
void InteractEntityEvent();
void AddEffectEvent();
void RemoveEffectEvent();
} // namespace lse::events::player