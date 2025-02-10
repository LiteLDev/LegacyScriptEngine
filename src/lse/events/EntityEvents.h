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
void EffectUpdateEvent();
void TransformationEvent();
} // namespace lse::events::entity