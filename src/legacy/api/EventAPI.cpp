#include "legacy/api/EventAPI.h"

#include "legacy/api/APIHelp.h"
#include "legacy/api/LegacyCommandAPI.h"
#include "legacy/api/McAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "legacy/engine/EngineManager.h" // IWYU pragma: keep
#include "legacy/engine/EngineOwnData.h"
#include "legacy/main/BuiltinCommands.h" // IWYU pragma: keep
#include "legacy/main/Global.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/command/ExecuteCommandEvent.h"
#include "ll/api/event/server/ServerStartedEvent.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "lse/Entry.h"
#include "lse/api/Thread.h"
#include "lse/events/BlockEvents.h"
#include "lse/events/EntityEvents.h"
#include "lse/events/OtherEvents.h"
#include "lse/events/PlayerEvents.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/Item.h"
#include "mc/world/level/dimension/Dimension.h"

#ifdef LSE_BACKEND_NODEJS
#include "legacy/main/NodeJsHelper.h"
#endif

#ifdef LSE_BACKEND_PYTHON
#include "legacy/main/PythonHelper.h"
#endif

#include <list>
#include <string>

using lse::api::thread::isServerThread;

//////////////////// Listeners ////////////////////

// 监听器表
std::list<EventListener> listenerList[static_cast<int>(EVENT_TYPES::EVENT_COUNT)];

// 监听器历史
bool hasListened[static_cast<int>(EVENT_TYPES::EVENT_COUNT)] = {false};

//////////////////// APIs ////////////////////

Local<Value> McClass::listen(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        auto eventName = args[0].asString().toString();
        auto listener  = LLSEAddEventListener(EngineScope::currentEngine(), eventName, args[1].asFunction());
        return Boolean::newBoolean(listener.has_value());
    }
    CATCH_AND_THROW
}

//////////////////// Funcs ////////////////////

optional_ref<EventListener>
LLSEAddEventListener(ScriptEngine* engine, std::string const& eventName, Local<Function> const& func) {
    try {
        auto  event_enum = magic_enum::enum_cast<EVENT_TYPES>(eventName);
        auto  eventId    = static_cast<int>(event_enum.value());
        auto& listener   = listenerList[eventId].emplace_back(engine, script::Global<Function>(func), *event_enum);
        if (!hasListened[eventId]) {
            hasListened[eventId] = true;
            EnableEventListener(eventId);
        }
        return {listener};
    } catch (...) {
        lse::LegacyScriptEngine::getLogger().error("Event {} not found!"_tr(eventName));
        lse::LegacyScriptEngine::getLogger().error("In plugin: {}", getEngineData(engine)->pluginName);
        return std::nullopt;
    }
}

bool LLSERemoveAllEventListeners(std::shared_ptr<ScriptEngine> engine) {
    for (auto& listeners : listenerList) {
        listeners.remove_if([engine](auto& listener) { return listener.engine == engine.get(); });
    }
    return true;
}

bool LLSECallEventsOnHotLoad(std::shared_ptr<ScriptEngine> const& engine) {
    FakeCallEvent(engine.get(), EVENT_TYPES::onServerStarted);

    ll::service::getLevel()->forEachPlayer([&](Player const& pl) -> bool {
        FakeCallEvent(engine.get(), EVENT_TYPES::onPreJoin, PlayerClass::newPlayer(&pl));
        return true;
    });
    ll::service::getLevel()->forEachPlayer([&](Player const& pl) -> bool {
        FakeCallEvent(engine.get(), EVENT_TYPES::onJoin, PlayerClass::newPlayer(&pl));
        return true;
    });

    return true;
}

bool LLSECallEventsOnUnload(std::shared_ptr<ScriptEngine> const& engine) {
    // Players may be online when the server is stopping
    ll::service::getLevel()->forEachPlayer([&](Player const& pl) -> bool {
        FakeCallEvent(engine.get(), EVENT_TYPES::onLeft, PlayerClass::newPlayer(&pl));
        return true;
    });
    EngineScope scope(engine.get());
    for (auto& cb : getEngineData(engine)->unloadCallbacks | std::views::values) {
        try {
            cb(engine);
        }
        CATCH_IN_CALLBACK("onUnload")
    }
    getEngineData(engine)->unloadCallbacks.clear();
    return true;
}

//////////////////// Events ////////////////////

void EnableEventListener(int eventId) {
    using namespace ll::event;
    EventBus& bus = EventBus::getInstance();
    switch (static_cast<EVENT_TYPES>(eventId)) {
    case EVENT_TYPES::onJoin:
        lse::events::player::onJoin();
        break;
    case EVENT_TYPES::onPreJoin:
        lse::events::player::onPreJoin();
        break;
    case EVENT_TYPES::onLeft:
        lse::events::player::onLeft();
        break;
    case EVENT_TYPES::onChat:
        lse::events::player::onChat();
        break;
    case EVENT_TYPES::onChangeDim:
        lse::events::player::onChangeDim();
        break;
    case EVENT_TYPES::onPlayerSwing:
        lse::events::player::onPlayerSwing();
        break;
    case EVENT_TYPES::onAttackEntity:
        lse::events::player::onAttackEntity();
        break;
    case EVENT_TYPES::onAttackBlock:
        lse::events::player::onAttackBlock();
        break;
    case EVENT_TYPES::onPlayerDie:
        lse::events::player::onPlayerDie();
        break;
    case EVENT_TYPES::onRespawn:
        lse::events::player::onRespawn();
        break;
    case EVENT_TYPES::onStartDestroyBlock:
        lse::events::player::onStartDestroyBlock();
        break;
    case EVENT_TYPES::onDestroyBlock:
        lse::events::player::onDestroyBlock();
        break;
    case EVENT_TYPES::onPlaceBlock:
        lse::events::player::onPlaceBlock();
        break;
    case EVENT_TYPES::afterPlaceBlock:
        lse::events::player::afterPlaceBlock();
        break;
    case EVENT_TYPES::onJump:
        lse::events::player::onJump();
        break;
    case EVENT_TYPES::onDropItem:
        lse::events::player::onDropItem();
        break;
    case EVENT_TYPES::onTakeItem:
        lse::events::player::onTakeItem();
        break;
    case EVENT_TYPES::onOpenContainer:
        lse::events::player::onOpenContainer();
        break;
    case EVENT_TYPES::onCloseContainer:
        lse::events::player::CloseContainerEvent();
        break;
    case EVENT_TYPES::onInventoryChange:
        lse::events::player::onInventoryChange();
        break;
    case EVENT_TYPES::onUseItem:
        lse::events::player::onUseItem();
        break;
    case EVENT_TYPES::onUseItemOn:
        lse::events::player::onUseItemOn();
        break;
    case EVENT_TYPES::onUseBucketPlace:
        lse::events::player::UseBucketPlaceEvent();
        break;
    case EVENT_TYPES::onUseBucketTake:
        lse::events::player::UseBucketTakeEvent();
        break;
    case EVENT_TYPES::onContainerChange:
        lse::events::block::ContainerChangeEvent();
        break;
    case EVENT_TYPES::onDispenseItem:
        lse::events::block::DispenseItemEvent();
        break;
    case EVENT_TYPES::onChangeArmorStand:
        lse::events::player::onChangeArmorStand();
        break;
    case EVENT_TYPES::onChangeSprinting:
        lse::events::player::onChangeSprinting();
        break;
    case EVENT_TYPES::onSneak:
        lse::events::player::onSneak();
        break;
    case EVENT_TYPES::onOpenContainerScreen:
        lse::events::player::OpenContainerScreenEvent();
        break;
    case EVENT_TYPES::onSetArmor:
        lse::events::player::SetArmorEvent();
        break;
    case EVENT_TYPES::onEat:
        lse::events::player::onEat();
        break;
    case EVENT_TYPES::onAte:
        lse::events::player::onAte();
        break;
    case EVENT_TYPES::onConsumeTotem:
        lse::events::player::ConsumeTotemEvent();
        break;
    case EVENT_TYPES::onEffectAdded:
        lse::events::player::onEffectAdded();
        break;
    case EVENT_TYPES::onEffectRemoved:
        lse::events::player::RemoveEffectEvent();
        break;
    case EVENT_TYPES::onEffectUpdated:
        lse::events::player::onEffectUpdated();
        break;
    case EVENT_TYPES::onUseRespawnAnchor:
        lse::events::player::UseRespawnAnchorEvent();
        break;
    case EVENT_TYPES::onRide:
        lse::events::player::onRide();
        break;
    case EVENT_TYPES::onEntityExplode:
        lse::events::entity::onEntityExplode();
        break;
    case EVENT_TYPES::onBlockExplode:
        lse::events::block::onBlockExplode();
        break;
    case EVENT_TYPES::onRespawnAnchorExplode:
        lse::events::block::RespawnAnchorExplodeEvent();
        break;
    case EVENT_TYPES::onPortalTrySpawn:
        lse::events::block::PortalSpawnEvent();
        break;
    case EVENT_TYPES::onBlockExploded:
        lse::events::block::BlockExplodedEvent();
        break;
    case EVENT_TYPES::onCmdBlockExecute:
        lse::events::block::CommandBlockExecuteEvent();
        break;
    case EVENT_TYPES::onRedStoneUpdate:
        lse::events::block::onRedStoneUpdate();
        break;
    case EVENT_TYPES::onWitherBossDestroy:
        lse::events::entity::onWitherBossDestroy();
        break;
    case EVENT_TYPES::onMobHurt:
        lse::events::entity::MobHurtEvent();
        break;
    case EVENT_TYPES::onStepOnPressurePlate:
        lse::events::entity::onStepOnPressurePlate();
        break;
    case EVENT_TYPES::onMobDie:
        lse::events::entity::onMobDie();
        break;
    case EVENT_TYPES::onSpawnProjectile:
        lse::events::entity::ProjectileSpawnEvent();
        break;
    case EVENT_TYPES::onProjectileCreated:
        lse::events::entity::ProjectileCreatedEvent();
        break;
    case EVENT_TYPES::onProjectileHitEntity:
        lse::events::entity::ProjectileHitEntityEvent();
        break;
    case EVENT_TYPES::onEntityTransformation:
        lse::events::entity::TransformationEvent();
        break;
    case EVENT_TYPES::onProjectileHitBlock:
        lse::events::entity::ProjectileHitBlockEvent();
        break;
    case EVENT_TYPES::onLiquidFlow:
        lse::events::block::onLiquidFlow();
        break;
    case EVENT_TYPES::onUseFrameBlock:
        lse::events::player::onUseFrameBlock();
        break;
    case EVENT_TYPES::onBlockInteracted:
        lse::events::player::BlockInteractedEvent();
        break;
    case EVENT_TYPES::onFarmLandDecay:
        lse::events::block::onFarmLandDecay();
        break;
    case EVENT_TYPES::onPistonTryPush:
        lse::events::block::onPistonTryPush();
        break;
    case EVENT_TYPES::onPistonPush:
        lse::events::block::onPistonPush();
        break;
    case EVENT_TYPES::onHopperSearchItem:
        lse::events::block::HopperEvent(true);
        break;
    case EVENT_TYPES::onHopperPushOut:
        lse::events::block::HopperEvent(false);
        break;
    case EVENT_TYPES::onFireSpread:
        lse::events::block::onFireSpread();
        break;
    case EVENT_TYPES::onBlockChanged:
        lse::events::block::onBlockChanged();
        break;
    case EVENT_TYPES::onScoreChanged:
        lse::events::other::ScoreChangedEvent();
        break;
    case EVENT_TYPES::onMobSpawn:
        lse::events::entity::onMobSpawn();
        break;
    case EVENT_TYPES::onMobTrySpawn:
        lse::events::entity::onMobTrySpawn();
        break;
    case EVENT_TYPES::onMobSpawned:
        lse::events::entity::onMobSpawned();
        break;
    case EVENT_TYPES::onPortalTrySpawnPigZombie:
        lse::events::entity::PortalTrySpawnPigZombieEvent();
        break;
    case EVENT_TYPES::onExperienceAdd:
        lse::events::player::onExperienceAdd();
        break;
    case EVENT_TYPES::onBedEnter:
        lse::events::player::onBedEnter();
        break;
    case EVENT_TYPES::onOpenInventory:
        lse::events::player::OpenInventoryEvent();
        break;
    case EVENT_TYPES::onPlayerPullFishingHook:
        lse::events::player::PullFishingHookEvent();
        break;
    case EVENT_TYPES::onPlayerInteractEntity:
        lse::events::player::onPlayerInteractEntity();
        break;
    case EVENT_TYPES::onNpcCmd:
        lse::events::entity::NpcCommandEvent();
        break;
    case EVENT_TYPES::onEndermanTakeBlock:
        lse::events::entity::onEndermanTakeBlock();
        break;
    default:
        break;
    }
}

void InitBasicEventListeners() {
    using namespace ll::event;
    EventBus& bus = EventBus::getInstance();

    bus.emplaceListener<ExecutingCommandEvent>([](ExecutingCommandEvent& ev) {
        auto originType = ev.commandContext().mOrigin->getOriginType();
        if (originType == CommandOriginType::DedicatedServer) {
            std::string cmd = ev.commandContext().mCommand;
            if (cmd.starts_with("/")) {
                cmd.erase(0, 1);
            }
#ifndef LSE_BACKEND_NODEJS
            if (!ProcessDebugEngine(cmd)) {
                ev.cancel();
                return;
            }
#endif
#ifdef LSE_BACKEND_NODEJS
            if (!NodeJsHelper::processConsoleNpmCmd(cmd)) {
                ev.cancel();
                return;
            }
#elif defined(LSE_BACKEND_PYTHON)
            if (!PythonHelper::processConsolePipCmd(cmd)) {
                ev.cancel();
                return;
            }
#endif
            IF_LISTENED(EVENT_TYPES::onConsoleCmd) {
                if (!CallEvent(EVENT_TYPES::onConsoleCmd, String::newString(cmd))) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onConsoleCmd);
        } else if (originType == CommandOriginType::Player) {
            std::string cmd = ev.commandContext().mCommand;
            if (cmd.starts_with("/")) {
                cmd.erase(0, 1);
            }
            Player* player = static_cast<Player*>(ev.commandContext().mOrigin->getEntity());
            IF_LISTENED(EVENT_TYPES::onPlayerCmd) {
                if (!CallEvent(EVENT_TYPES::onPlayerCmd, PlayerClass::newPlayer(player), String::newString(cmd))) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onPlayerCmd);
        }
    });

    using namespace ll::chrono_literals;
    // ===== onServerStarted =====
    bus.emplaceListener<ServerStartedEvent>([](ServerStartedEvent&) {
        ll::coro::keepThis([]() -> ll::coro::CoroTask<> {
            co_await 1_tick;
            IF_LISTENED(EVENT_TYPES::onServerStarted) {
                CallEvent(EVENT_TYPES::onServerStarted); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onServerStarted);

            lse::legacy_command::registerLegacyCommands();
        }).launch(ll::thread::ServerThreadExecutor::getDefault());
    });

    // 植入tick
    ll::coro::keepThis([]() -> ll::coro::CoroTask<> {
        while (true) {
            co_await 1_tick;
            for (auto& type : dirtyEventTypes) {
                auto& list = listenerList[static_cast<int>(type)];
                for (auto iter = list.begin(); iter != list.end();) {
                    if (iter->removed) {
                        EngineScope scope(iter->engine);
                        iter = list.erase(iter);
                    } else {
                        ++iter;
                    }
                }
            }
#ifndef LSE_BACKEND_NODEJS
            try {
                auto snapshot = EngineManager::getGlobalEngines();
                for (auto& engine : snapshot) {
                    if (EngineManager::isValid(engine.get())
                        && EngineManager::getEngineType(engine) == LLSE_BACKEND_TYPE) {
                        EngineScope enter(engine.get());
                        engine->messageQueue()->loopQueue(script::utils::MessageQueue::LoopType::kLoopOnce);
                    }
                }
            } catch (...) {
                lse::LegacyScriptEngine::getLogger().error("Error occurred in Engine Message Loop!");
                ::legacy::script_error::printCurrentException(lse::LegacyScriptEngine::getLogger());
            }
#endif

            // Call tick event
            IF_LISTENED(EVENT_TYPES::onTick) {
                CallEvent(EVENT_TYPES::onTick); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onTick);
        }
    }).launch(ll::thread::ServerThreadExecutor::getDefault());
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
bool MoneyBeforeEventCallback(LLMoneyEvent type, std::string from, std::string to, long long value) {
    switch (type) {
    case Add: {
        IF_LISTENED(EVENT_TYPES::beforeMoneyAdd) {
            if (!CallEvent(EVENT_TYPES::beforeMoneyAdd, String::newString(to), Number::newNumber(value))) {
                return false;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::beforeMoneyAdd);
        break;
    }
    case Reduce: {
        IF_LISTENED(EVENT_TYPES::beforeMoneyReduce) {
            if (!CallEvent(EVENT_TYPES::beforeMoneyReduce, String::newString(to), Number::newNumber(value))) {
                return false;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::beforeMoneyReduce);
        break;
    }
    case Trans: {
        IF_LISTENED(EVENT_TYPES::beforeMoneyTrans) {
            if (!CallEvent(
                    EVENT_TYPES::beforeMoneyTrans,
                    String::newString(from),
                    String::newString(to),
                    Number::newNumber(value)
                )) {
                return false;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::beforeMoneyTrans);
        break;
    }
    case Set: {
        IF_LISTENED(EVENT_TYPES::beforeMoneySet) {
            if (!CallEvent(EVENT_TYPES::beforeMoneySet, String::newString(to), Number::newNumber(value))) {
                return false;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::beforeMoneySet);
        break;
    }
    default:
        break;
    }
    return true;
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
bool MoneyEventCallback(LLMoneyEvent type, std::string from, std::string to, long long value) {
    switch (type) {
    case Add: {
        IF_LISTENED(EVENT_TYPES::onMoneyAdd) {
            if (!CallEvent(EVENT_TYPES::onMoneyAdd, String::newString(to), Number::newNumber(value))) {
                return false;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onMoneyAdd);
        break;
    }
    case Reduce: {
        IF_LISTENED(EVENT_TYPES::onMoneyReduce) {
            if (!CallEvent(EVENT_TYPES::onMoneyReduce, String::newString(to), Number::newNumber(value))) {
                return false;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onMoneyReduce);
        break;
    }
    case Trans: {
        IF_LISTENED(EVENT_TYPES::onMoneyTrans) {
            if (!CallEvent(
                    EVENT_TYPES::onMoneyTrans,
                    String::newString(from),
                    String::newString(to),
                    Number::newNumber(value)
                )) {
                return false;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onMoneyTrans);
        break;
    }
    case Set: {
        IF_LISTENED(EVENT_TYPES::onMoneySet) {
            if (!CallEvent(EVENT_TYPES::onMoneySet, String::newString(to), Number::newNumber(value))) {
                return false;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onMoneySet);
        break;
    }
    default:
        break;
    }
    return true;
}
