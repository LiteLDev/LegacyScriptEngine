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
    // onMove,
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
    onStepOnPressurePlate,
    onSpawnProjectile,
    onProjectileCreated,
    onChangeArmorStand,
    onEntityTransformation,
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
    /* Other Events */
    onScoreChanged,
    onTick,
    onServerStarted,
    onConsoleCmd,
    onConsoleOutput,
    /* Economic Events */
    onMoneyAdd,
    onMoneyReduce,
    onMoneyTrans,
    onMoneySet,
    beforeMoneyAdd,
    beforeMoneyReduce,
    beforeMoneyTrans,
    beforeMoneySet,
    onFormResponsePacket,
    /* Outdated Events */
    onAttack,
    onExplode,
    onBedExplode,
    onMobSpawn,
    onMobTrySpawn,
    onMobSpawned,
    onContainerChangeSlot,
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

#define LISTENER_CATCH(TYPE)                                                                                           \
    catch (const Exception& e) {                                                                                       \
        lse::getSelfPluginInstance().getLogger().error("Event Callback Failed!");                                      \
        lse::getSelfPluginInstance().getLogger().error(e.what());                                                      \
        lse::getSelfPluginInstance().getLogger().error("In Event: " + EventTypeToString(TYPE));                        \
        lse::getSelfPluginInstance().getLogger().error("In Plugin: " + ENGINE_OWN_DATA()->pluginName);                 \
    }                                                                                                                  \
    catch (const std::exception& e) {                                                                                  \
        lse::getSelfPluginInstance().getLogger().error("Event Callback Failed!");                                      \
        lse::getSelfPluginInstance().getLogger().error("C++ Uncaught Exception Detected!");                            \
        lse::getSelfPluginInstance().getLogger().error(ll::string_utils::tou8str(e.what()));                           \
        PrintScriptStackTrace();                                                                                       \
        lse::getSelfPluginInstance().getLogger().error("In Event: " + EventTypeToString(TYPE));                        \
        lse::getSelfPluginInstance().getLogger().error("In Plugin: " + ENGINE_OWN_DATA()->pluginName);                 \
    }

// 调用事件监听函数，取消事件
#define CallEvent(TYPE, ...)                                                                                           \
    std::list<ListenerListType>& nowList = listenerList[int(TYPE)];                                                    \
    for (auto& listener : nowList) {                                                                                   \
        EngineScope enter(listener.engine);                                                                            \
        try {                                                                                                          \
            auto result = listener.func.get().call({}, __VA_ARGS__);                                                   \
            if (result.isBoolean() && result.asBoolean().value() == false) {                                           \
                ev.cancel();                                                                                           \
            }                                                                                                          \
        }                                                                                                              \
        LISTENER_CATCH(TYPE)                                                                                           \
    }

// 调用事件监听函数，拦截返回RETURN_VALUE
#define CallEventRtnValue(TYPE, RETURN_VALUE, ...)                                                                     \
    std::list<ListenerListType>& nowList     = listenerList[int(TYPE)];                                                \
    bool                         isCancelled = false;                                                                  \
    for (auto& listener : nowList) {                                                                                   \
        EngineScope enter(listener.engine);                                                                            \
        try {                                                                                                          \
            auto result = listener.func.get().call({}, __VA_ARGS__);                                                   \
            if (result.isBoolean() && result.asBoolean().value() == false) {                                           \
                isCancelled = true;                                                                                    \
            }                                                                                                          \
        }                                                                                                              \
        LISTENER_CATCH(TYPE)                                                                                           \
    }                                                                                                                  \
    if (isCancelled) {                                                                                                 \
        return RETURN_VALUE;                                                                                           \
    }

// 调用事件监听函数，拦截返回
#define CallEventVoid(TYPE, ...)                                                                                       \
    std::list<ListenerListType>& nowList     = listenerList[int(TYPE)];                                                \
    bool                         isCancelled = false;                                                                  \
    for (auto& listener : nowList) {                                                                                   \
        EngineScope enter(listener.engine);                                                                            \
        try {                                                                                                          \
            auto result = listener.func.get().call({}, __VA_ARGS__);                                                   \
            if (result.isBoolean() && result.asBoolean().value() == false) {                                           \
                isCancelled = true;                                                                                    \
            }                                                                                                          \
        }                                                                                                              \
        LISTENER_CATCH(TYPE)                                                                                           \
    }                                                                                                                  \
    if (isCancelled) {                                                                                                 \
        return;                                                                                                        \
    }

// 调用事件监听函数，不可拦截
#define CallEventUncancelable(TYPE, ...)                                                                               \
    std::list<ListenerListType>& nowList = listenerList[int(TYPE)];                                                    \
    for (auto& listener : nowList) {                                                                                   \
        EngineScope enter(listener.engine);                                                                            \
        try {                                                                                                          \
            auto result = listener.func.get().call({}, __VA_ARGS__);                                                   \
        }                                                                                                              \
        LISTENER_CATCH(TYPE)                                                                                           \
    }

// 模拟事件调用监听
#define FakeCallEvent(ENGINE, TYPE, ...)                                                                               \
    {                                                                                                                  \
        std::list<ListenerListType>& nowList = listenerList[int(TYPE)];                                                \
        for (auto& listener : nowList) {                                                                               \
            if (listener.engine == ENGINE) {                                                                           \
                EngineScope enter(listener.engine);                                                                    \
                try {                                                                                                  \
                    listener.func.get().call({}, __VA_ARGS__);                                                         \
                }                                                                                                      \
                LISTENER_CATCH(TYPE)                                                                                   \
            }                                                                                                          \
        }                                                                                                              \
    }

// 异常检查
#define IF_LISTENED(TYPE)                                                                                              \
    if (!listenerList[int(TYPE)].empty()) {                                                                            \
        try
#define IF_LISTENED_END(TYPE)                                                                                          \
    catch (...) {                                                                                                      \
        lse::getSelfPluginInstance().getLogger().error("Event Callback Failed!");                                      \
        lse::getSelfPluginInstance().getLogger().error("Uncaught Exception Detected!");                                \
        lse::getSelfPluginInstance().getLogger().error("In Event: " + EventTypeToString(TYPE));                        \
    }                                                                                                                  \
    }
