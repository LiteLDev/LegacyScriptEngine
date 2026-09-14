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

void onEntityExplode();
void onStepOnPressurePlate();
void onMobDie();
void onMobSpawn();
void onMobTrySpawn();
void onMobSpawned();
void onEndermanTakeBlock();
} // namespace lse::events::entity
