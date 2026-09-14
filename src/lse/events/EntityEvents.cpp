#include "lse/events/EntityEvents.h"

#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/entity/MobDieEvent.h"
#include "ll/api/event/world/SpawnMobEvent.h"
#include "ll/api/service/Bedrock.h"
#include "lse/api/Thread.h"
#include "mc/world/level/Explosion.h"

#include <ila/event/world/ExplosionEvent.h>
#include <ila/event/world/WitherDestroyEvent.h>
#include <ila/event/world/actor/ActorTriggerPressurePlateEvent.h>
#include <ila/event/world/actor/MobTakeBlockEvent.h>

namespace lse::events::entity {
using namespace ll::event;
using api::thread::isServerThread;
#define bus EventBus::getInstance()

void onEntityExplode() {
    bus.emplaceListener<ila::mc::ExplosionBeforeEvent>([](ila::mc::ExplosionBeforeEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onEntityExplode) {
            if (isServerThread()) {
                if (ev.explosion().mSourceID->rawID != ActorUniqueID::INVALID_ID().rawID) {
                    if (!CallEvent(
                            EVENT_TYPES::onEntityExplode,
                            EntityClass::newEntity(
                                ll::service::getLevel()->fetchEntity(ev.explosion().mSourceID, false)
                            ),
                            FloatPos::newPos(ev.explosion().mPos, ev.blockSource().getDimensionId()),
                            Number::newNumber(ev.explosion().mRadius),
                            Number::newNumber(ev.explosion().mMaxResistance),
                            Boolean::newBoolean(ev.explosion().mBreaking),
                            Boolean::newBoolean(ev.explosion().mFire)
                        )) {
                        ev.cancel();
                    }
                }
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onEntityExplode);
    });
}

void onStepOnPressurePlate() {
    bus.emplaceListener<ila::mc::ActorTriggerPressurePlateBeforeEvent>(
        [](ila::mc::ActorTriggerPressurePlateBeforeEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onStepOnPressurePlate) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onStepOnPressurePlate,
                            EntityClass::newEntity(&ev.self()),
                            BlockClass::newBlock(ev.pos(), ev.self().getDimensionId())
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onStepOnPressurePlate);
        }
    );
}

void onMobDie() {
    bus.emplaceListener<MobDieEvent>([](MobDieEvent const& ev) {
        IF_LISTENED(EVENT_TYPES::onMobDie) {
            if (isServerThread()) {
                Actor* source = nullptr;
                if (ev.source().isEntitySource()) {
                    source = ll::service::getLevel()->fetchEntity(ev.source().getDamagingEntityUniqueID(), false);
                    if (source) {
                        if (ev.source().isChildEntitySource()) source = source->getOwner();
                    }
                }

                CallEvent(
                    EVENT_TYPES::onMobDie,
                    EntityClass::newEntity(&ev.self()),
                    (source ? EntityClass::newEntity(source) : Local<Value>()),
                    Number::newNumber(static_cast<int>(ev.source().mCause))
                ); // Not cancellable
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onMobDie);
    });
}

void onMobSpawn() {
    lse::LegacyScriptEngine::getLogger().warn("Event 'onMobSpawn' is outdated, please use 'onMobTrySpawn' instead.");
    bus.emplaceListener<SpawningMobEvent>([](SpawningMobEvent const& ev) {
        IF_LISTENED(EVENT_TYPES::onMobSpawn) {
            if (isServerThread()) {
                CallEvent(
                    EVENT_TYPES::onMobSpawn,
                    String::newString(ev.identifier().mFullName),
                    FloatPos::newPos(ev.pos(), ev.blockSource().getDimensionId())
                ); // Not cancellable
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onMobSpawn);
    });
}

void onMobTrySpawn() {
    bus.emplaceListener<SpawningMobEvent>([](SpawningMobEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onMobTrySpawn) {
            if (isServerThread()) {
                if (!CallEvent(
                        EVENT_TYPES::onMobTrySpawn,
                        String::newString(ev.identifier().mFullName),
                        FloatPos::newPos(ev.pos(), ev.blockSource().getDimensionId())
                    )) {
                    ev.cancel();
                }
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onMobTrySpawn);
    });
}

void onMobSpawned() {
    bus.emplaceListener<SpawnedMobEvent>([](SpawnedMobEvent const& ev) {
        IF_LISTENED(EVENT_TYPES::onMobSpawned) {
            if (isServerThread()) {
                CallEvent(
                    EVENT_TYPES::onMobSpawned,
                    EntityClass::newEntity(ev.mob().has_value() ? ev.mob().as_ptr() : nullptr),
                    FloatPos::newPos(ev.pos(), ev.blockSource().getDimensionId())
                ); // Not cancellable
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onMobSpawned);
    });
}

void onEndermanTakeBlock() {
    bus.emplaceListener<ila::mc::MobTakeBlockBeforeEvent>([](ila::mc::MobTakeBlockBeforeEvent& ev) {
        if (ev.self().isType(ActorType::EnderMan)) {
            auto& mob = ev.self();
            int   dim = mob.getDimensionId();
            if (!CallEvent(
                    EVENT_TYPES::onEndermanTakeBlock,
                    EntityClass::newEntity(&mob),
                    BlockClass::newBlock(mob.getDimensionBlockSource().getBlock(ev.pos()), ev.pos(), dim),
                    IntPos::newPos(ev.pos(), dim)
                )) {
                ev.cancel();
            }
        }
    });
}

void onWitherBossDestroy() {
    bus.emplaceListener<ila::mc::WitherDestroyBeforeEvent>([](ila::mc::WitherDestroyBeforeEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onWitherBossDestroy) {
            if (isServerThread()) {
                if (!CallEvent(
                        EVENT_TYPES::onWitherBossDestroy,
                        EntityClass::newEntity(&ev.wither()),
                        IntPos::newPos(ev.box().min, ev.blockSource().getDimensionId()),
                        IntPos::newPos(ev.box().max, ev.blockSource().getDimensionId())
                    )) {
                    ev.cancel();
                }
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onWitherBossDestroy);
    });
}
} // namespace lse::events::entity
