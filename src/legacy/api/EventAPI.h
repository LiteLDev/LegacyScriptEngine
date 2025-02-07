#pragma once
#include "api/APIHelp.h"
#include "main/EconomicSystem.h"

//////////////////// Funcs ////////////////////

void InitBasicEventListeners();
void EnableEventListener(int eventId);

bool LLSEAddEventListener(ScriptEngine* engine, const std::string& eventName, const Local<Function>& func);
bool LLSERemoveAllEventListeners(ScriptEngine* engine);
bool LLSECallEventsOnHotLoad(ScriptEngine* engine);
bool LLSECallEventsOnHotUnload(ScriptEngine* engine);

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

struct ListenerListType {
    ScriptEngine*            engine;
    script::Global<Function> func;
};

// 监听器表
extern std::list<ListenerListType> listenerList[int(EVENT_TYPES::EVENT_COUNT)];

// 监听器历史
extern bool hasListened[int(EVENT_TYPES::EVENT_COUNT)];

// 监听器异常拦截
inline std::string EventTypeToString(EVENT_TYPES e) { return std::string(magic_enum::enum_name(e)); }

#define CallEvent(type, ...)                                                                                           \
    [&]() {                                                                                                            \
        std::list<ListenerListType>& nowList     = listenerList[(int)type];                                            \
        bool                         returnValue = true;                                                               \
        for (auto& listener : nowList) {                                                                               \
            EngineScope enter(listener.engine);                                                                        \
            CallEventImpl(listener, returnValue, type, __VA_ARGS__);                                                   \
        }                                                                                                              \
        return returnValue;                                                                                            \
    }()

template <typename... T>
void CallEventImpl(ListenerListType& listener, bool& returnValue, EVENT_TYPES type, T&&... args) {
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
    std::list<ListenerListType>& nowList = listenerList[(int)type];                                                    \
    for (auto& listener : nowList) {                                                                                   \
        EngineScope enter(listener.engine);                                                                            \
        FakeCallEventImpl(listener, engine, type, __VA_ARGS__);                                                        \
    }

template <typename... T>
void FakeCallEventImpl(ListenerListType& listener, ScriptEngine* engine, EVENT_TYPES type, T&&... args) {
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
