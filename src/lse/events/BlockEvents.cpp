#include "lse/events/BlockEvents.h"

#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/world/BlockChangedEvent.h"
#include "ll/api/event/world/FireSpreadEvent.h"
#include "lse/api/Thread.h"
#include "mc/world/level/Explosion.h"

#include <ila/event/world/ExplosionEvent.h>
#include <ila/event/world/PistonPushEvent.h>
#include <ila/event/world/RedstoneUpdateEvent.h>
#include <ila/event/world/level/block/FarmDecayEvent.h>
#include <ila/event/world/level/block/LiquidFlowEvent.h>

namespace lse::events::block {
using namespace ll::event;
using api::thread::isServerThread;
#define bus EventBus::getInstance()

void onBlockExplode() {
    bus.emplaceListener<ila::mc::ExplosionBeforeEvent>([](ila::mc::ExplosionBeforeEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onBlockExplode) {
            if (isServerThread()) {
                if (!CallEvent(
                        EVENT_TYPES::onBlockExplode,
                        BlockClass::newBlock(*ev.explosion().mPos, ev.blockSource().getDimensionId()),
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
        IF_LISTENED_END(EVENT_TYPES::onBlockExplode);
    });
}

void onRedStoneUpdate() {
    bus.emplaceListener<ila::mc::RedstoneUpdateBeforeEvent>([](ila::mc::RedstoneUpdateBeforeEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onRedStoneUpdate) {
            if (isServerThread()) {
                if (!CallEvent(
                        EVENT_TYPES::onRedStoneUpdate,
                        BlockClass::newBlock(ev.pos(), ev.blockSource().getDimensionId()),
                        Number::newNumber(ev.strength()),
                        Boolean::newBoolean(ev.isFirstTime())
                    )) {
                    ev.cancel();
                }
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onRedStoneUpdate);
    });
}

void onLiquidFlow() {
    bus.emplaceListener<ila::mc::LiquidFlowBeforeEvent>([](ila::mc::LiquidFlowBeforeEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onLiquidFlow) {
            if (!CallEvent(
                    EVENT_TYPES::onLiquidFlow,
                    ev.blockSource().isInstaticking(ev.pos())
                        ? Local<Value>()
                        : BlockClass::newBlock(ev.pos(), ev.blockSource().getDimensionId()),
                    IntPos::newPos(ev.pos(), ev.blockSource().getDimensionId())
                )) {
                ev.cancel();
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onLiquidFlow);
    });
}

void onFarmLandDecay() {
    bus.emplaceListener<ila::mc::FarmDecayBeforeEvent>([](ila::mc::FarmDecayBeforeEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onFarmLandDecay) {
            if (isServerThread()) {
                if (!CallEvent(
                        EVENT_TYPES::onFarmLandDecay,
                        IntPos::newPos(ev.pos(), ev.blockSource().getDimensionId()),
                        EntityClass::newEntity(ev.actor())
                    )) {
                    ev.cancel();
                }
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onFarmLandDecay);
    });
}

void onPistonTryPush() {
    bus.emplaceListener<ila::mc::PistonPushBeforeEvent>([](ila::mc::PistonPushBeforeEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onPistonTryPush) {
            if (isServerThread()) {
                if (ev.blockSource().getBlock(ev.pushPos()).isAir()) {
                    return;
                }
                if (!CallEvent(
                        EVENT_TYPES::onPistonTryPush,
                        IntPos::newPos(ev.pistonPos(), ev.blockSource().getDimensionId()),
                        BlockClass::newBlock(ev.pushPos(), ev.blockSource().getDimensionId())
                    )) {
                    ev.cancel();
                }
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onPistonTryPush);
    });
}

void onPistonPush() {
    bus.emplaceListener<ila::mc::PistonPushBeforeEvent>([](ila::mc::PistonPushBeforeEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onPistonPush) {
            if (isServerThread()) {
                CallEvent(
                    EVENT_TYPES::onPistonPush,
                    IntPos::newPos(ev.pistonPos(), ev.blockSource().getDimensionId()),
                    BlockClass::newBlock(ev.pushPos(), ev.blockSource().getDimensionId())
                );
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onPistonPush);
    });
}

void onFireSpread() {
    bus.emplaceListener<FireSpreadEvent>([](FireSpreadEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onFireSpread) {
            if (isServerThread()) {
                if (!CallEvent(
                        EVENT_TYPES::onFireSpread,
                        IntPos::newPos(ev.pos(), ev.blockSource().getDimensionId())
                    )) {
                    ev.cancel();
                }
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onFireSpread);
    });
}

void onBlockChanged() {
    bus.emplaceListener<BlockChangedEvent>([](BlockChangedEvent const& ev) {
        IF_LISTENED(EVENT_TYPES::onBlockChanged) {
            if (isServerThread()) {
                CallEvent(
                    EVENT_TYPES::onBlockChanged,
                    BlockClass::newBlock(ev.previousBlock(), ev.pos(), ev.blockSource()),
                    BlockClass::newBlock(ev.newBlock(), ev.pos(), ev.blockSource())
                ); // Not cancellable
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onBlockChanged);
    });
}
} // namespace lse::events::block
