#include "ll/api/memory/Hook.h"

namespace lse {
namespace EventHooks {
void PlayerStartDestroyBlock();
void PlayerDropItem();
void PlayerOpenContainerEvent();
void PlayerCloseContainerEvent();
void PlayerChangeSlotEvent();
void ContainerChangeEvent();
void PlayerAttackBlockEvent();
void ArmorStandSwapItemEvent();
void PlayerUseFrameEvent();
void ProjectileSpawnEvent();
void ProjectileCreatedEvent();
void PressurePlateTriggerEvent();
void ActorRideEvent();
void WitherDestroyEvent();
void FarmDecayEvent();
} // namespace EventHooks
} // namespace lse
