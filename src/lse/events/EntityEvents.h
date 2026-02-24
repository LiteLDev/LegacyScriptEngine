#pragma once

namespace lse::events::entity {
void ProjectileSpawnEvent();
void ProjectileCreatedEvent();
void ActorRideEvent();
void WitherDestroyEvent();
void ProjectileHitEntityEvent();
void ProjectileHitBlockEvent();
void MobHurtEvent();
void NpcCommandEvent();
void EndermanTakeBlockEvent();
void EffectUpdateEvent();
void TransformationEvent();
void PortalTrySpawnPigZombieEvent();
} // namespace lse::events::entity
