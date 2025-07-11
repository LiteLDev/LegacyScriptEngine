#pragma once
#include "api/APIHelp.h"
#include "main/EconomicSystem.h"

//////////////////// Funcs ////////////////////

struct EventListener;

void InitBasicEventListeners();
void EnableEventListener(int eventId);
optional_ref<EventListener>
     LLSEAddEventListener(ScriptEngine* engine, const std::string& eventName, const Local<Function>& func);
bool LLSERemoveAllEventListeners(ScriptEngine* engine);
bool LLSECallEventsOnHotLoad(ScriptEngine* engine);
bool LLSECallEventsOnUnload(ScriptEngine* engine);

//////////////////// Callback ////////////////////

bool MoneyBeforeEventCallback(LLMoneyEvent type, std::string from, std::string to, long long value);
bool MoneyEventCallback(LLMoneyEvent type, std::string from, std::string to, long long value);

enum class EVENT_TYPES : int {
    /* Player Events */
    onPreJoin = 0,
    onJoin,
    onLeft,
    onRespawn,
    onPlayerDie,
    onPlayerCmd,
    onChat,
    onChangeDim,
    onJump,
    onSneak,
    onPlayerSwing,
    onAttackEntity,
    onAttackBlock,
    onUseItem,
    onUseItemOn,
    onUseBucketPlace,
    onUseBucketTake,
    onTakeItem,
    onDropItem,
    onEat,
    onAte,
    onConsumeTotem,
    onEffectAdded,
    onEffectUpdated,
    onEffectRemoved,
    onStartDestroyBlock,
    onDestroyBlock,
    onPlaceBlock,
    afterPlaceBlock,
    onOpenContainer,
    onCloseContainer,
    onInventoryChange,
    onPlayerPullFishingHook,
    onPlayerInteractEntity,
    onChangeSprinting,
    onSetArmor,
    onUseRespawnAnchor,
    onOpenContainerScreen,
    onExperienceAdd,
    onBedEnter,
    onOpenInventory,
    /* Entity Events */
    onMobDie,
    onMobHurt,
    onEntityExplode,
    onProjectileHitEntity,
    onWitherBossDestroy,
    onRide,
    onSpawnProjectile,
    onProjectileCreated,
    onEntityTransformation,
    onMobTrySpawn,
    onMobSpawned,
    onNpcCmd,
    onEndermanTakeBlock,
    /* Block Events */
    onBlockInteracted,
    onBlockChanged,
    onBlockExplode,
    onRespawnAnchorExplode,
    onBlockExploded,
    onFireSpread,
    onCmdBlockExecute,
    onContainerChange,
    onProjectileHitBlock,
    onRedStoneUpdate,
    onHopperSearchItem,
    onHopperPushOut,
    onPistonTryPush,
    onPistonPush,
    onFarmLandDecay,
    onUseFrameBlock,
    onLiquidFlow,
    onStepOnPressurePlate,
    onChangeArmorStand,
    /* Other Events */
    onScoreChanged,
    onTick,
    onServerStarted,
    onConsoleCmd,
    /* Economic Events */
    onMoneyAdd,
    onMoneyReduce,
    onMoneyTrans,
    onMoneySet,
    beforeMoneyAdd,
    beforeMoneyReduce,
    beforeMoneyTrans,
    beforeMoneySet,
    /* Outdated Events */
    onAttack,
    onExplode,
    onBedExplode,
    onMobSpawn,
    EVENT_COUNT
};

//////////////////// Listeners ////////////////////

inline std::set<EVENT_TYPES> dirtyEventTypes{};

struct EventListener {
    ScriptEngine*            engine;
    script::Global<Function> func;
    // mark as removed and remove at next tick
    using RemovedRef = std::shared_ptr<std::reference_wrapper<bool>>;
    EVENT_TYPES type;
    bool        removed = false;
    RemovedRef  removedRef{std::make_shared<RemovedRef::element_type>(std::ref(removed))};

    [[nodiscard]] inline auto remover() const {
        return [ref{RemovedRef::weak_type{removedRef}}, type{type}]() -> bool {
            auto removed = ref.lock();
            if (removed) {
                removed->get() = true;
                dirtyEventTypes.emplace(type);
            }
            return !!removed;
        };
    }
    EventListener(ScriptEngine* engine, script::Global<Function> func, EVENT_TYPES type)
    : engine(engine),
      func(std::move(func)),
      type(type) {};
    EventListener(const EventListener&) = delete;
};

// 监听器表
extern std::list<EventListener> listenerList[int(EVENT_TYPES::EVENT_COUNT)];

// 监听器历史
extern bool hasListened[int(EVENT_TYPES::EVENT_COUNT)];

// 监听器异常拦截
inline std::string EventTypeToString(EVENT_TYPES e) { return std::string(magic_enum::enum_name(e)); }

#define CallEvent(type, ...)                                                                                           \
    [&]() {                                                                                                            \
        std::list<EventListener>& nowList     = listenerList[(int)type];                                               \
        bool                      returnValue = true;                                                                  \
        for (auto& listener : nowList | std::views::filter([](auto& l) { return !l.removed; })) {                      \
            EngineScope enter(listener.engine);                                                                        \
            CallEventImpl(listener, returnValue, type, __VA_ARGS__);                                                   \
        }                                                                                                              \
        return returnValue;                                                                                            \
    }()

template <typename... T>
void CallEventImpl(EventListener& listener, bool& returnValue, EVENT_TYPES type, T&&... args) {
    try {
        auto result = listener.func.get().call({}, args...);
        if (result.isBoolean() && result.asBoolean().value() == false) {
            returnValue = false;
        }
    } catch (const Exception& e) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("CallEvent Callback Failed!");
        ll::error_utils::printException(e, lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("In Event: " + EventTypeToString(type));
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "In Plugin: " + getEngineOwnData()->pluginName
        );
    } catch (...) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("CallEvent Callback Failed!");
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("In Event: " + EventTypeToString(type));
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "In Plugin: " + getEngineOwnData()->pluginName
        );
    }
}

#define FakeCallEvent(engine, type, ...)                                                                               \
    std::list<EventListener>& nowList = listenerList[(int)type];                                                       \
    for (auto& listener : nowList | std::views::filter([](auto& l) { return !l.removed; })) {                          \
        EngineScope enter(listener.engine);                                                                            \
        FakeCallEventImpl(listener, engine, type, __VA_ARGS__);                                                        \
    }

template <typename... T>
void FakeCallEventImpl(EventListener& listener, ScriptEngine* engine, EVENT_TYPES type, T&&... args) {
    if (listener.engine == engine) {
        try {
            listener.func.get().call({}, args...);
        } catch (const Exception& e) {
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("FakeCallEvent Callback Failed!");
            ll::error_utils::printException(e, lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("In Event: " + EventTypeToString(type));
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                "In Plugin: " + getEngineOwnData()->pluginName
            );
        } catch (...) {
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("FakeCallEvent Callback Failed!");
            ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("In Event: " + EventTypeToString(type));
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                "In Plugin: " + getEngineOwnData()->pluginName
            );
        }
    }
}

// 异常检查
#define IF_LISTENED(TYPE)                                                                                              \
    if (!listenerList[int(TYPE)].empty()) {                                                                            \
        try
#define IF_LISTENED_END(TYPE)                                                                                          \
    catch (...) {                                                                                                      \
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("Event Callback Failed!");                  \
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());          \
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("In Event: " + EventTypeToString(TYPE));    \
    }                                                                                                                  \
    }
