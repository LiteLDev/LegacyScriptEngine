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

template <typename... T>
bool CallEvent(EVENT_TYPES type, T const&... args) {
    std::list<ListenerListType>& nowList     = listenerList[(int)type];
    bool                         returnValue = true;
    for (auto& listener : nowList) {
        EngineScope enter(listener.engine);
        try {
            auto result = listener.func.get().call({}, args...);
            if (result.isBoolean() && result.asBoolean().value() == false) {
                returnValue = false;
            }
        } catch (const Exception& e) {
            lse::getSelfPluginInstance().getLogger().error("Event Callback Failed!");
            lse::getSelfPluginInstance().getLogger().error(e.what());

            lse::getSelfPluginInstance().getLogger().error("In Event: " + EventTypeToString(type));
            lse::getSelfPluginInstance().getLogger().error("In Plugin: " + getEngineOwnData()->pluginName);
        } catch (const std::exception& e) {
            lse::getSelfPluginInstance().getLogger().error("Event Callback Failed!");
            lse::getSelfPluginInstance().getLogger().error("C++ Uncaught Exception Detected!");
            lse::getSelfPluginInstance().getLogger().error(ll::string_utils::tou8str(e.what()));
            PrintScriptStackTrace();
            lse::getSelfPluginInstance().getLogger().error("In Event: " + EventTypeToString(type));
            lse::getSelfPluginInstance().getLogger().error("In Plugin: " + getEngineOwnData()->pluginName);
        }
    }
    return returnValue;
}

template <typename... T>
void FakeCallEvent(ScriptEngine* engine, EVENT_TYPES type, T&&... args) {
    std::list<ListenerListType>& nowList = listenerList[int(type)];
    for (auto& listener : nowList) {
        if (listener.engine == engine) {
            EngineScope enter(listener.engine);
            try {
                listener.func.get().call({}, args...);
            } catch (const Exception& e) {
                lse::getSelfPluginInstance().getLogger().error("Event Callback Failed!");
                lse::getSelfPluginInstance().getLogger().error(e.what());

                lse::getSelfPluginInstance().getLogger().error("In Event: " + EventTypeToString(type));
                lse::getSelfPluginInstance().getLogger().error("In Plugin: " + getEngineOwnData()->pluginName);
            } catch (const std::exception& e) {
                lse::getSelfPluginInstance().getLogger().error("Event Callback Failed!");
                lse::getSelfPluginInstance().getLogger().error("C++ Uncaught Exception Detected!");
                lse::getSelfPluginInstance().getLogger().error(ll::string_utils::tou8str(e.what()));
                PrintScriptStackTrace();
                lse::getSelfPluginInstance().getLogger().error("In Event: " + EventTypeToString(type));
                lse::getSelfPluginInstance().getLogger().error("In Plugin: " + getEngineOwnData()->pluginName);
            }
        }
    }
}

// 异常检查
#define IF_LISTENED(TYPE)                                                                                              \
    if (!listenerList[int(TYPE)].empty()) {                                                                            \
        EngineScope enter(listenerList[int(TYPE)].front().engine);                                                     \
        try
#define IF_LISTENED_END(TYPE)                                                                                          \
    catch (...) {                                                                                                      \
        lse::getSelfPluginInstance().getLogger().error("Event Callback Failed!");                                      \
        lse::getSelfPluginInstance().getLogger().error("Uncaught Exception Detected!");                                \
        lse::getSelfPluginInstance().getLogger().error("In Event: " + EventTypeToString(TYPE));                        \
    }                                                                                                                  \
    }
