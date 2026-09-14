#pragma once

namespace lse::events::entity {
void ProjectileSpawnEvent();
void PortalTrySpawnPigZombieEvent();
void ProjectileCreatedEvent();
void ProjectileHitEntityEvent();
void ProjectileHitBlockEvent();
void MobHurtEvent();
void NpcCommandEvent();
void TransformationEvent();

void onEntityExplode();
void onStepOnPressurePlate();
void onMobDie();
void onMobSpawn();
void onMobTrySpawn();
void onMobSpawned();
void onEndermanTakeBlock();
void onWitherBossDestroy();
} // namespace lse::events::entity
