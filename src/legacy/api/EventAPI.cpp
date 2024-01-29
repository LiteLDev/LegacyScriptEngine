#include "api/EventAPI.h"

#include "../engine/LocalShareData.h"
#include "../engine/TimeTaskSystem.h"
#include "../main/BuiltinCommands.h"
#include "BaseAPI.h"
#include "BlockAPI.h"
#include "CommandCompatibleAPI.h"
#include "EntityAPI.h"
#include "ItemAPI.h"
#include "api/APIHelp.h"
#include "api/McAPI.h"
#include "api/PlayerAPI.h"
#include "engine/EngineOwnData.h"
#include "engine/GlobalShareData.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/command/ExecuteCommandEvent.h"
#include "ll/api/event/player/PlayerAddExperienceEvent.h"
#include "ll/api/event/player/PlayerAttackEvent.h"
#include "ll/api/event/player/PlayerChatEvent.h"
#include "ll/api/event/player/PlayerConnectEvent.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/event/player/PlayerDieEvent.h"
#include "ll/api/event/player/PlayerInteractBlockEvent.h"
#include "ll/api/event/player/PlayerJoinEvent.h"
#include "ll/api/event/player/PlayerJumpEvent.h"
#include "ll/api/event/player/PlayerLeaveEvent.h"
#include "ll/api/event/player/PlayerPickUpItemEvent.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"
#include "ll/api/event/player/PlayerRespawnEvent.h"
#include "ll/api/event/player/PlayerSneakEvent.h"
#include "ll/api/event/player/PlayerSprintEvent.h"
#include "ll/api/event/player/PlayerSwingEvent.h"
#include "ll/api/event/player/PlayerUseItemEvent.h"
#include "ll/api/event/player/PlayerUseItemOnEvent.h"
#include "ll/api/event/server/ServerStartedEvent.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/schedule/Scheduler.h"
#include "ll/api/schedule/Task.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/utils/StringUtils.h"
#include "main/Global.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level./BlockSource.h"
#include "mc/world/level/BlockEventCoordinator.h"
#include "mc/world/level/dimension/Dimension.h"

#include <exception>
#include <list>
#include <shared_mutex>
#include <string>

//////////////////// Listeners ////////////////////

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

struct ListenerListType {
    ScriptEngine*            engine;
    script::Global<Function> func;
};

// 监听器表
static std::list<ListenerListType> listenerList[int(EVENT_TYPES::EVENT_COUNT)];

// 监听器历史
static bool hasListened[int(EVENT_TYPES::EVENT_COUNT)] = {false};

// 监听器异常拦截
string EventTypeToString(EVENT_TYPES e) { return string(magic_enum::enum_name(e)); }

#define LISTENER_CATCH(TYPE)                                                                                           \
    catch (const Exception& e) {                                                                                       \
        logger.error("Event Callback Failed!");                                                                        \
        logger.error(e.what());                                                                                        \
        logger.error("In Event: " + EventTypeToString(TYPE));                                                          \
        logger.error("In Plugin: " + ENGINE_OWN_DATA()->pluginName);                                                   \
    }                                                                                                                  \
    catch (const std::exception& e) {                                                                                  \
        logger.error("Event Callback Failed!");                                                                        \
        logger.error("C++ Uncaught Exception Detected!");                                                              \
        logger.error(ll::string_utils::tou8str(e.what()));                                                             \
        PrintScriptStackTrace();                                                                                       \
        logger.error("In Event: " + EventTypeToString(TYPE));                                                          \
        logger.error("In Plugin: " + ENGINE_OWN_DATA()->pluginName);                                                   \
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
                return;                                                                                                \
            }                                                                                                          \
        }                                                                                                              \
        LISTENER_CATCH(TYPE)                                                                                           \
    }

// 调用事件监听函数，拦截返回RETURN_VALUE
#define CallEventRtnValue(TYPE, RETURN_VALUE, ...)                                                                     \
    std::list<ListenerListType>& nowList = listenerList[int(TYPE)];                                                    \
    for (auto& listener : nowList) {                                                                                   \
        EngineScope enter(listener.engine);                                                                            \
        try {                                                                                                          \
            auto result = listener.func.get().call({}, __VA_ARGS__);                                                   \
            if (result.isBoolean() && result.asBoolean().value() == false) return RETURN_VALUE;                        \
        }                                                                                                              \
        LISTENER_CATCH(TYPE)                                                                                           \
    }

// 调用事件监听函数，拦截返回
#define CallEventVoid(TYPE, ...)                                                                                       \
    std::list<ListenerListType>& nowList = listenerList[int(TYPE)];                                                    \
    for (auto& listener : nowList) {                                                                                   \
        EngineScope enter(listener.engine);                                                                            \
        try {                                                                                                          \
            auto result = listener.func.get().call({}, __VA_ARGS__);                                                   \
            if (result.isBoolean() && result.asBoolean().value() == false) return;                                     \
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

// 延迟调用事件
#define CallEventDelayed(TYPE, ...)                                                                                    \
    std::list<ListenerListType>& nowList = listenerList[int(TYPE)];                                                    \
    for (auto& listener : nowList) {                                                                                   \
        EngineScope enter(listener.engine);                                                                            \
        try {                                                                                                          \
            NewTimeout(listener.func.get(), {__VA_ARGS__}, 5);                                                         \
        }                                                                                                              \
        LISTENER_CATCH(TYPE)                                                                                           \
    }

// 异常检查
#define IF_LISTENED(TYPE)                                                                                              \
    if (!listenerList[int(TYPE)].empty()) {                                                                            \
        try
#define IF_LISTENED_END(TYPE)                                                                                          \
    catch (...) {                                                                                                      \
        logger.error("Event Callback Failed!");                                                                        \
        logger.error("Uncaught Exception Detected!");                                                                  \
        logger.error("In Event: " + EventTypeToString(TYPE));                                                          \
    }                                                                                                                  \
    }

//////////////////// APIs ////////////////////

Local<Value> McClass::listen(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        return Boolean::newBoolean(
            LLSEAddEventListener(EngineScope::currentEngine(), args[0].toStr(), args[1].asFunction())
        );
    }
    CATCH("Fail to Bind Listener!");
}

//////////////////// Funcs ////////////////////

bool LLSEAddEventListener(ScriptEngine* engine, const string& eventName, const Local<Function>& func) {
    try {
        auto event_enum = magic_enum::enum_cast<EVENT_TYPES>(eventName);
        auto eventId    = int(event_enum.value());
        listenerList[eventId].push_back({engine, script::Global<Function>(func)});
        if (!hasListened[eventId]) {
            hasListened[eventId] = true;
            EnableEventListener(eventId);
        }
        return true;
    } catch (...) {
        logger.error("Event \"" + eventName + "\" No Found!\n");
        logger.error("In Plugin: " + ENGINE_GET_DATA(engine)->pluginName);
        return false;
    }
}

bool LLSERemoveAllEventListeners(ScriptEngine* engine) {
    for (auto& listeners : listenerList) {
        listeners.remove_if([engine](auto& listener) { return listener.engine == engine; });
    }
    return true;
}

bool LLSECallEventsOnHotLoad(ScriptEngine* engine) {
    FakeCallEvent(engine, EVENT_TYPES::onServerStarted);

    ll::service::getLevel()->forEachPlayer([&](Player& pl) -> bool {
        FakeCallEvent(engine, EVENT_TYPES::onPreJoin, PlayerClass::newPlayer(&pl));
        return true;
    });
    ll::service::getLevel()->forEachPlayer([&](Player& pl) -> bool {
        FakeCallEvent(engine, EVENT_TYPES::onJoin, PlayerClass::newPlayer(&pl));
        return true;
    });

    return true;
}

bool LLSECallEventsOnHotUnload(ScriptEngine* engine) {
    ll::service::getLevel()->forEachPlayer([&](Player& pl) -> bool {
        FakeCallEvent(engine, EVENT_TYPES::onLeft, PlayerClass::newPlayer(&pl));
        return true;
    });
    for (auto& [index, cb] : ENGINE_GET_DATA(engine)->unloadCallbacks) {
        cb(engine);
    }
    ENGINE_GET_DATA(engine)->unloadCallbacks.clear();
    return true;
}

//////////////////// Events ////////////////////

LL_AUTO_TYPE_INSTANCE_HOOK(
    PlayerStartDestroyHook,
    HookPriority::Normal,
    BlockEventCoordinator,
    &BlockEventCoordinator::sendBlockDestructionStarted,
    void,
    Player&         player,
    BlockPos const& blockPos,
    uchar           unk_char
) {
    IF_LISTENED(EVENT_TYPES::onStartDestroyBlock) {
        CallEventVoid(
            EVENT_TYPES::onStartDestroyBlock,
            PlayerClass::newPlayer(&player),
            BlockClass::newBlock(blockPos, player.getDimensionId())
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onStartDestroyBlock)
    origin(player, blockPos, unk_char);
}

// Todo
void EnableEventListener(int eventId) {
    using namespace ll::event;
    EventBus& bus = EventBus::getInstance();
    switch ((EVENT_TYPES)eventId) {
    case EVENT_TYPES::onJoin:
        bus.emplaceListener<PlayerJoinEvent>([](PlayerJoinEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onJoin) { CallEvent(EVENT_TYPES::onJoin, PlayerClass::newPlayer(&ev.self())); }
            IF_LISTENED_END(EVENT_TYPES::onJoin);
        });
        break;

    case EVENT_TYPES::onPreJoin:
        bus.emplaceListener<PlayerConnectEvent>([](PlayerConnectEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onPreJoin) {
                CallEvent(EVENT_TYPES::onPreJoin, PlayerClass::newPlayer(&ev.self()));
            }
            IF_LISTENED_END(EVENT_TYPES::onPreJoin);
        });
        break;

    case EVENT_TYPES::onLeft:
        bus.emplaceListener<PlayerLeaveEvent>([](PlayerLeaveEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onLeft) { CallEventVoid(EVENT_TYPES::onLeft, PlayerClass::newPlayer(&ev.self())); }
            IF_LISTENED_END(EVENT_TYPES::onLeft);
        });
        break;

    case EVENT_TYPES::onChat:
        bus.emplaceListener<PlayerChatEvent>([](PlayerChatEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onChat) {
                CallEvent(EVENT_TYPES::onChat, PlayerClass::newPlayer(&ev.self()), String::newString(ev.message()));
            }
            IF_LISTENED_END(EVENT_TYPES::onChat);
        });

        // case EVENT_TYPES::onChangeDim:
        //   Event::PlayerChangeDimEvent::subscribe([](const PlayerChangeDimEvent
        //   &ev) {
        //     IF_LISTENED(EVENT_TYPES::onChangeDim) {
        //       CallEvent(EVENT_TYPES::onChangeDim,
        //       PlayerClass::newPlayer(ev.mPlayer),
        //                 Number::newNumber(ev.mToDimensionId)); //======???
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onChangeDim);
        //   });
        //   break;

    case EVENT_TYPES::onPlayerSwing:
        bus.emplaceListener<PlayerSwingEvent>([](PlayerSwingEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onPlayerSwing) {
                CallEventVoid(EVENT_TYPES::onPlayerSwing, PlayerClass::newPlayer(&ev.self()));
            }
            IF_LISTENED_END(EVENT_TYPES::onPlayerSwing);
        });
        break;

    case EVENT_TYPES::onAttackEntity:
        bus.emplaceListener<PlayerAttackEvent>([](PlayerAttackEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onAttackEntity) {
                CallEvent(
                    EVENT_TYPES::onAttackEntity,
                    PlayerClass::newPlayer(&ev.self()),
                    EntityClass::newEntity(&ev.target())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onAttackEntity);
        });
        break;
        break;

        // case EVENT_TYPES::onAttackBlock:
        //   Event::PlayerAttackBlockEvent::subscribe(
        //       [](const PlayerAttackBlockEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onAttackBlock) {
        //           CallEvent(
        //               EVENT_TYPES::onAttackBlock,
        //               PlayerClass::newPlayer(ev.mPlayer),
        //               BlockClass::newBlock(ev.mBlockInstance),
        //               !ev.mItemStack->isNull() ?
        //               ItemClass::newItem(ev.mItemStack)
        //                                        : Local<Value>());
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onAttackBlock);
        //       });
        //   break;

    case EVENT_TYPES::onPlayerDie:
        bus.emplaceListener<ll::event::PlayerDieEvent>([](ll::event::PlayerDieEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onPlayerDie) {
                Actor* source = ll::service::getLevel()
                                    ->getDimension(ev.self().getDimensionId())
                                    ->fetchEntity(ev.source().getEntityUniqueID(), false);
                CallEventVoid(
                    EVENT_TYPES::onPlayerDie,
                    PlayerClass::newPlayer(&ev.self()),
                    (source ? EntityClass::newEntity(source) : Local<Value>())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onPlayerDie);
        });
        break;

    case EVENT_TYPES::onRespawn:
        bus.emplaceListener<ll::event::PlayerRespawnEvent>([](ll::event::PlayerRespawnEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onRespawn) {
                CallEventVoid(EVENT_TYPES::onRespawn, PlayerClass::newPlayer(&ev.self()));
            }
            IF_LISTENED_END(EVENT_TYPES::onRespawn)
        });
        break;

    case EVENT_TYPES::onStartDestroyBlock:

        break;

    case EVENT_TYPES::onDestroyBlock:
        bus.emplaceListener<PlayerDestroyBlockEvent>([](PlayerDestroyBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onStartDestroyBlock) {
                CallEvent(
                    EVENT_TYPES::onStartDestroyBlock,
                    PlayerClass::newPlayer(&ev.self()),
                    BlockClass::newBlock(ev.pos(), ev.self().getDimensionId())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onStartDestroyBlock);
        });
        break;

    case EVENT_TYPES::onPlaceBlock:
        bus.emplaceListener<PlayerPlacingBlockEvent>([](PlayerPlacingBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onPlaceBlock) {
                CallEvent(
                    EVENT_TYPES::onPlaceBlock,
                    PlayerClass::newPlayer(&ev.self()),
                    BlockClass::newBlock(ev.pos(), ev.self().getDimensionId())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onPlaceBlock);
        });
        break;

    case EVENT_TYPES::afterPlaceBlock:
        bus.emplaceListener<PlayerPlacedBlockEvent>([](PlayerPlacedBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::afterPlaceBlock) {
                CallEventVoid(
                    EVENT_TYPES::afterPlaceBlock,
                    PlayerClass::newPlayer(&ev.self()),
                    BlockClass::newBlock(ev.pos(), ev.self().getDimensionId())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::afterPlaceBlock);
        });
        break;

        /*case EVENT_TYPES::onMove:
            Event::PlayerMoveEvent::subscribe([](const PlayerMoveEvent& ev) {
                IF_LISTENED(EVENT_TYPES::onMove) {
                    CallEvent(EVENT_TYPES::onMove,
           PlayerClass::newPlayer(ev.mPlayer), FloatPos::newPos(ev.mPos,
           ev.mPlayer->getDimensionId()));
                }
                IF_LISTENED_END(EVENT_TYPES::onMove);
            });
            break;*/

    case EVENT_TYPES::onJump:
        bus.emplaceListener<PlayerJumpEvent>([](PlayerJumpEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onJump) { CallEventVoid(EVENT_TYPES::onJump, PlayerClass::newPlayer(&ev.self())); }
            IF_LISTENED_END(EVENT_TYPES::onJump);
        });
        break;

        // case EVENT_TYPES::onDropItem:
        //   Event::PlayerDropItemEvent::subscribe([](const PlayerDropItemEvent &ev)
        //   {
        //     IF_LISTENED(EVENT_TYPES::onDropItem) {
        //       CallEvent(
        //           EVENT_TYPES::onDropItem, PlayerClass::newPlayer(ev.mPlayer),
        //           ItemClass::newItem(ev.mItemStack)); // ###### Q lost items
        //           ######
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onDropItem);
        //   });
        //   break;

    case EVENT_TYPES::onTakeItem:
        bus.emplaceListener<PlayerPickUpItemEvent>([](PlayerPickUpItemEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onTakeItem) {
                CallEvent(
                    EVENT_TYPES::onTakeItem,
                    PlayerClass::newPlayer(&ev.self()),
                    EntityClass::newEntity(&ev.itemActor()),
                    ItemClass::newItem(&ev.itemActor().item())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onTakeItem);
        });
        break;

        // case EVENT_TYPES::onOpenContainer:
        //   Event::PlayerOpenContainerEvent::subscribe(
        //       [](const PlayerOpenContainerEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onOpenContainer) {
        //           CallEvent(EVENT_TYPES::onOpenContainer,
        //                     PlayerClass::newPlayer(ev.mPlayer),
        //                     BlockClass::newBlock(ev.mBlockInstance));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onOpenContainer);
        //       });
        //   break;

        // case EVENT_TYPES::onCloseContainer:
        //   Event::PlayerCloseContainerEvent::subscribe(
        //       [](const PlayerCloseContainerEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onCloseContainer) {
        //           CallEvent(EVENT_TYPES::onCloseContainer,
        //                     PlayerClass::newPlayer(ev.mPlayer),
        //                     BlockClass::newBlock(ev.mBlockInstance));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onCloseContainer);
        //       });
        //   break;

        // case EVENT_TYPES::onInventoryChange:
        //   Event::PlayerInventoryChangeEvent::subscribe(
        //       [](const PlayerInventoryChangeEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onInventoryChange) {
        //           CallEvent(EVENT_TYPES::onInventoryChange,
        //                     PlayerClass::newPlayer(ev.mPlayer), ev.mSlot,
        //                     ItemClass::newItem(ev.mPreviousItemStack),
        //                     ItemClass::newItem(ev.mNewItemStack));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onInventoryChange);
        //       });
        //   break;

    case EVENT_TYPES::onUseItem:
        bus.emplaceListener<PlayerUseItemEvent>([](PlayerUseItemEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onUseItem) {
                CallEvent(EVENT_TYPES::onUseItem, PlayerClass::newPlayer(&ev.self()), ItemClass::newItem(&ev.item()));
            }
            IF_LISTENED_END(EVENT_TYPES::onUseItem);
        });
        break;

    case EVENT_TYPES::onUseItemOn:
        bus.emplaceListener<PlayerUseItemOnEvent>([](PlayerUseItemOnEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onUseItemOn) {
                CallEvent(
                    EVENT_TYPES::onUseItemOn,
                    PlayerClass::newPlayer(&ev.self()),
                    ItemClass::newItem(&ev.item()),
                    BlockClass::newBlock(&ev.block().get(), &ev.blockPos(), ev.self().getDimensionId()),
                    Number::newNumber(ev.face()),
                    FloatPos::newPos(ev.clickPos(), ev.self().getDimensionId())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onUseItemOn);
        });
        break;

        // case EVENT_TYPES::onUseBucketPlace:
        // case EVENT_TYPES::onUseBucketTake:
        //   Event::PlayerUseBucketEvent::subscribe([](const PlayerUseBucketEvent
        //   &ev) {
        //     if (ev.mEventType == PlayerUseBucketEvent::EventType::Place) {
        //       IF_LISTENED(EVENT_TYPES::onUseBucketPlace) {
        //         CallEvent(
        //             EVENT_TYPES::onUseBucketPlace,
        //             PlayerClass::newPlayer((Player *)ev.mPlayer),
        //             ItemClass::newItem(ev.mBucket),
        //             BlockClass::newBlock(ev.mBlockInstance),
        //             Number::newNumber(ev.mFace),
        //             FloatPos::newPos(ev.mTargetPos,
        //             ev.mPlayer->getDimensionId()));
        //       }
        //       IF_LISTENED_END(EVENT_TYPES::onUseBucketPlace);
        //     } else if (ev.mEventType == PlayerUseBucketEvent::EventType::Take) {
        //       IF_LISTENED(EVENT_TYPES::onUseBucketTake) {
        //         if (ev.mTargetActor) {
        //           CallEvent(
        //               EVENT_TYPES::onUseBucketTake,
        //               PlayerClass::newPlayer((Player *)ev.mPlayer),
        //               ItemClass::newItem(ev.mBucket),
        //               EntityClass::newEntity(ev.mTargetActor),
        //               Number::newNumber(ev.mFace),
        //               FloatPos::newPos(ev.mTargetPos,
        //               ev.mPlayer->getDimensionId()));
        //         } else {
        //           CallEvent(
        //               EVENT_TYPES::onUseBucketTake,
        //               PlayerClass::newPlayer((Player *)ev.mPlayer),
        //               ItemClass::newItem(ev.mBucket),
        //               BlockClass::newBlock(ev.mBlockInstance),
        //               Number::newNumber(ev.mFace),
        //               FloatPos::newPos(ev.mTargetPos,
        //               ev.mPlayer->getDimensionId()));
        //         }
        //       }
        //       IF_LISTENED_END(EVENT_TYPES::onUseBucketTake);
        //     }
        //     return true;
        //   });
        //   break;

        // case EVENT_TYPES::onContainerChange:
        //   Event::ContainerChangeEvent::subscribe([](const ContainerChangeEvent
        //   &ev) {
        //     IF_LISTENED(EVENT_TYPES::onContainerChange) {
        //       CallEvent(EVENT_TYPES::onContainerChange,
        //                 PlayerClass::newPlayer(ev.mPlayer),
        //                 BlockClass::newBlock(ev.mBlockInstance),
        //                 Number::newNumber(ev.mSlot),
        //                 ItemClass::newItem(ev.mPreviousItemStack),
        //                 ItemClass::newItem(ev.mNewItemStack));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onContainerChange);
        //   });
        //   break;

        // case EVENT_TYPES::onChangeArmorStand:
        //   Event::ArmorStandChangeEvent::subscribe(
        //       [](const ArmorStandChangeEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onChangeArmorStand) {
        //           CallEvent(EVENT_TYPES::onChangeArmorStand,
        //                     EntityClass::newEntity((Actor *)ev.mArmorStand),
        //                     PlayerClass::newPlayer(ev.mPlayer),
        //                     Number::newNumber(ev.mSlot));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onChangeArmorStand);
        //       });
        //   break;

    case EVENT_TYPES::onChangeSprinting:
        bus.emplaceListener<PlayerSprintingEvent>([](PlayerSprintingEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onChangeSprinting) {
                CallEventVoid(
                    EVENT_TYPES::onChangeSprinting,
                    PlayerClass::newPlayer(&ev.self()),
                    Boolean::newBoolean(true)
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onChangeSprinting);
        });
        bus.emplaceListener<PlayerSprintedEvent>([](PlayerSprintedEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onChangeSprinting) {
                CallEventVoid(
                    EVENT_TYPES::onChangeSprinting,
                    PlayerClass::newPlayer(&ev.self()),
                    Boolean::newBoolean(false)
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onChangeSprinting);
        });
        break;

    case EVENT_TYPES::onSneak:
        bus.emplaceListener<PlayerSneakingEvent>([](PlayerSneakingEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onSneak) {
                CallEvent(EVENT_TYPES::onSneak, PlayerClass::newPlayer(&ev.self()), Boolean::newBoolean(true));
            }
            IF_LISTENED_END(EVENT_TYPES::onSneak);
        });
        bus.emplaceListener<PlayerSneakedEvent>([](PlayerSneakedEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onSneak) {
                CallEvent(EVENT_TYPES::onSneak, PlayerClass::newPlayer(&ev.self()), Boolean::newBoolean(false));
            }
            IF_LISTENED_END(EVENT_TYPES::onSneak);
        });
        break;

        // case EVENT_TYPES::onOpenContainerScreen:
        //   Event::PlayerOpenContainerScreenEvent::subscribe(
        //       [](const PlayerOpenContainerScreenEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onOpenContainerScreen) {
        //           CallEvent(EVENT_TYPES::onOpenContainerScreen,
        //                     PlayerClass::newPlayer(ev.mPlayer));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onOpenContainerScreen);
        //       });
        //   break;

        // case EVENT_TYPES::onSetArmor:
        //   Event::PlayerSetArmorEvent::subscribe([](const PlayerSetArmorEvent &ev)
        //   {
        //     IF_LISTENED(EVENT_TYPES::onSetArmor) {
        //       CallEvent(EVENT_TYPES::onSetArmor,
        //       PlayerClass::newPlayer(ev.mPlayer),
        //                 Number::newNumber(ev.mSlot),
        //                 ItemClass::newItem(ev.mArmorItem));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onSetArmor);
        //   });
        //   break;

        // case EVENT_TYPES::onEat:
        //   Event::PlayerEatEvent::subscribe([](const PlayerEatEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onEat) {
        //       CallEvent(EVENT_TYPES::onEat, PlayerClass::newPlayer(ev.mPlayer),
        //                 ItemClass::newItem(ev.mFoodItem));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onEat);
        //   });
        //   break;

        // case EVENT_TYPES::onAte:
        //   Event::PlayerAteEvent::subscribe([](const PlayerAteEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onAte) {
        //       CallEvent(EVENT_TYPES::onAte, PlayerClass::newPlayer(ev.mPlayer),
        //                 ItemClass::newItem(ev.mFoodItem));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onAte);
        //   });
        //   break;

        // case EVENT_TYPES::onConsumeTotem:
        //   Event::PlayerConsumeTotemEvent::subscribe(
        //       [](const PlayerConsumeTotemEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onConsumeTotem) {
        //           CallEvent(EVENT_TYPES::onConsumeTotem,
        //                     PlayerClass::newPlayer(ev.mPlayer));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onConsumeTotem);
        //       });
        //   break;

        // case EVENT_TYPES::onEffectAdded:
        // case EVENT_TYPES::onEffectRemoved:
        // case EVENT_TYPES::onEffectUpdated:
        //   Event::PlayerEffectChangedEvent::subscribe([](const
        //   PlayerEffectChangedEvent
        //                                                     &ev) {
        //     if (ev.mEventType == PlayerEffectChangedEvent::EventType::Add) {
        //       IF_LISTENED(EVENT_TYPES::onEffectAdded) {
        //         CallEvent(
        //             EVENT_TYPES::onEffectAdded,
        //             PlayerClass::newPlayer(ev.mPlayer),
        //             String::newString(ev.mEffect->getComponentName().getString()),
        //             Number::newNumber(ev.mEffect->getAmplifier()),
        //             Number::newNumber(ev.mEffect->getDuration()));
        //       }
        //       IF_LISTENED_END(EVENT_TYPES::onEffectAdded);
        //     } else if (ev.mEventType ==
        //     PlayerEffectChangedEvent::EventType::Remove) {
        //       IF_LISTENED(EVENT_TYPES::onEffectRemoved) {
        //         CallEvent(
        //             EVENT_TYPES::onEffectRemoved,
        //             PlayerClass::newPlayer(ev.mPlayer),
        //             String::newString(ev.mEffect->getComponentName().getString()));
        //       }
        //       IF_LISTENED_END(EVENT_TYPES::onEffectRemoved);
        //     } else if (ev.mEventType ==
        //     PlayerEffectChangedEvent::EventType::Update) {
        //       IF_LISTENED(EVENT_TYPES::onEffectUpdated) {
        //         CallEvent(
        //             EVENT_TYPES::onEffectUpdated,
        //             PlayerClass::newPlayer(ev.mPlayer),
        //             String::newString(ev.mEffect->getComponentName().getString()),
        //             Number::newNumber(ev.mEffect->getAmplifier()),
        //             Number::newNumber(ev.mEffect->getDuration()));
        //       }
        //       IF_LISTENED_END(EVENT_TYPES::onEffectUpdated);
        //     }
        //     return true;
        //   });
        //   break;

        // case EVENT_TYPES::onUseRespawnAnchor:
        //   Event::PlayerUseRespawnAnchorEvent::subscribe(
        //       [](const PlayerUseRespawnAnchorEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onUseRespawnAnchor) {
        //           BlockInstance bl(ev.mBlockInstance);
        //           CallEvent(EVENT_TYPES::onUseRespawnAnchor,
        //                     PlayerClass::newPlayer(ev.mPlayer),
        //                     IntPos::newPos(bl.getPosition(),
        //                     bl.getDimensionId()));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onUseRespawnAnchor);
        //       });
        //   break;

        // case EVENT_TYPES::onRide:
        //   Event::EntityRideEvent::subscribe([](const EntityRideEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onRide) {
        //       CallEvent(EVENT_TYPES::onRide, EntityClass::newEntity(ev.mRider),
        //                 EntityClass::newEntity(ev.mVehicle));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onRide);
        //   });
        //   break;

        // case EVENT_TYPES::onEntityExplode:
        //   Event::EntityExplodeEvent::subscribe([](const EntityExplodeEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onEntityExplode) {
        //       CallEvent(
        //           EVENT_TYPES::onEntityExplode,
        //           ev.mActor ? EntityClass::newEntity(ev.mActor) : Local<Value>(),
        //           FloatPos::newPos(ev.mPos, ev.mDimension->getDimensionId()),
        //           Number::newNumber(ev.mRadius),
        //           Number::newNumber(ev.mMaxResistance),
        //           Boolean::newBoolean(ev.mBreaking),
        //           Boolean::newBoolean(ev.mFire));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onEntityExplode);
        //   });
        //   break;

        // case EVENT_TYPES::onBlockExplode:
        //   Event::BlockExplodeEvent::subscribe([](const BlockExplodeEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onBlockExplode) {
        //       BlockInstance bl(ev.mBlockInstance);
        //       CallEvent(
        //           EVENT_TYPES::onBlockExplode, BlockClass::newBlock(bl),
        //           IntPos::newPos(bl.getPosition(), bl.getDimensionId()),
        //           Number::newNumber(ev.mRadius),
        //           Number::newNumber(ev.mMaxResistance),
        //           Boolean::newBoolean(ev.mBreaking),
        //           Boolean::newBoolean(ev.mFire));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onBlockExplode);
        //   });
        //   break;

        // case EVENT_TYPES::onRespawnAnchorExplode:
        //   Event::BlockExplodeEvent::subscribe([](const BlockExplodeEvent &ev) {
        //     BlockInstance bl(ev.mBlockInstance);
        //     if (bl.getBlock()->getName() == "minecraft:respawn_anchor") {
        //       IF_LISTENED(EVENT_TYPES::onRespawnAnchorExplode) {
        //         CallEvent(EVENT_TYPES::onRespawnAnchorExplode,
        //                   IntPos::newPos(bl.getPosition(), bl.getDimensionId()),
        //                   Local<Value>());
        //       }
        //       IF_LISTENED_END(EVENT_TYPES::onRespawnAnchorExplode);
        //     }
        //     return true;
        //   });
        //   break;

        // case EVENT_TYPES::onBlockExploded:
        //   Event::BlockExplodedEvent::subscribe([](const BlockExplodedEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onBlockExploded) {
        //       CallEvent(EVENT_TYPES::onBlockExploded,
        //                 BlockClass::newBlock(ev.mBlockInstance),
        //                 EntityClass::newEntity(ev.mExplodeSource));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onBlockExploded);
        //   });
        //   break;

        // case EVENT_TYPES::onCmdBlockExecute:
        //   Event::CmdBlockExecuteEvent::subscribe([](const CmdBlockExecuteEvent
        //   &ev) {
        //     IF_LISTENED(EVENT_TYPES::onCmdBlockExecute) {
        //       if (ev.mIsMinecart) {
        //         CallEvent(EVENT_TYPES::onCmdBlockExecute,
        //                   String::newString(ev.mCommand),
        //                   FloatPos::newPos(ev.mMinecart->getPosition(),
        //                                    ev.mMinecart->getDimensionId()),
        //                   Boolean::newBoolean(ev.mIsMinecart));
        //       } else {
        //         BlockInstance bl = ev.mBlockInstance;
        //         CallEvent(
        //             EVENT_TYPES::onCmdBlockExecute,
        //             String::newString(ev.mCommand),
        //             FloatPos::newPos(bl.getPosition().toVec3(),
        //             bl.getDimensionId()), Boolean::newBoolean(ev.mIsMinecart));
        //       }
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onCmdBlockExecute);
        //   });
        //   break;

        // case EVENT_TYPES::onRedStoneUpdate:
        //   Event::RedStoneUpdateEvent::subscribe([](const RedStoneUpdateEvent &ev)
        //   {
        //     IF_LISTENED(EVENT_TYPES::onRedStoneUpdate) {
        //       CallEvent(EVENT_TYPES::onRedStoneUpdate,
        //                 BlockClass::newBlock(ev.mBlockInstance),
        //                 Number::newNumber(ev.mRedStonePower),
        //                 Boolean::newBoolean(ev.mIsActivated));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onRedStoneUpdate);
        //   });
        //   break;

        // case EVENT_TYPES::onWitherBossDestroy:
        //   Event::WitherBossDestroyEvent::subscribe(
        //       [](const WitherBossDestroyEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onWitherBossDestroy) {
        //           AABB range = ev.mDestroyRange;
        //           int dimId = ((Actor *)ev.mWitherBoss)->getDimensionId();
        //           CallEvent(EVENT_TYPES::onWitherBossDestroy,
        //                     EntityClass::newEntity((Actor *)ev.mWitherBoss),
        //                     IntPos::newPos(range.min.toBlockPos(), dimId),
        //                     IntPos::newPos(range.max.toBlockPos(), dimId));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onWitherBossDestroy);
        //       });
        //   break;

        // case EVENT_TYPES::onMobHurt:
        //   Event::MobHurtEvent::subscribe([](const MobHurtEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onMobHurt) {
        //       Actor *source = nullptr;
        //       if (ev.mDamageSource->isEntitySource()) {
        //         if (ev.mDamageSource->isChildEntitySource())
        //           source =
        //           Level::getEntity(ev.mDamageSource->getEntityUniqueID());
        //         else
        //           source =
        //               Level::getEntity(ev.mDamageSource->getDamagingEntityUniqueID());
        //       }

        //       CallEvent(EVENT_TYPES::onMobHurt, EntityClass::newEntity(ev.mMob),
        //                 source ? EntityClass::newEntity(source) : Local<Value>(),
        //                 Number::newNumber(float(ev.mDamage)),
        //                 Number::newNumber((int)ev.mDamageSource->getCause()));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onMobHurt)
        //   });
        //   break;

        // case EVENT_TYPES::onStepOnPressurePlate:
        //   Event::EntityStepOnPressurePlateEvent::subscribe(
        //       [](const EntityStepOnPressurePlateEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onStepOnPressurePlate) {
        //           CallEvent(EVENT_TYPES::onStepOnPressurePlate,
        //                     EntityClass::newEntity(ev.mActor),
        //                     BlockClass::newBlock(ev.mBlockInstance));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onStepOnPressurePlate);
        //       });
        //   break;

        // case EVENT_TYPES::onMobDie:
        //   Event::MobDieEvent::subscribe([](const MobDieEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onMobDie) {
        //       Actor *source = nullptr;
        //       if (ev.mDamageSource->isEntitySource()) {
        //         source =
        //             Level::getEntity(ev.mDamageSource->getDamagingEntityUniqueID());
        //         if (ev.mDamageSource->isChildEntitySource())
        //           source = source->getOwner();
        //       }

        //       CallEvent(EVENT_TYPES::onMobDie,
        //                 EntityClass::newEntity((Actor *)ev.mMob),
        //                 (source ? EntityClass::newEntity(source) :
        //                 Local<Value>()),
        //                 Number::newNumber((int)ev.mDamageSource->getCause()));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onMobDie);
        //   });
        //   break;

        // case EVENT_TYPES::onSpawnProjectile:
        //   Event::ProjectileSpawnEvent::subscribe([](const ProjectileSpawnEvent
        //   &ev) {
        //     IF_LISTENED(EVENT_TYPES::onSpawnProjectile) {
        //       CallEvent(EVENT_TYPES::onSpawnProjectile,
        //                 EntityClass::newEntity(ev.mShooter),
        //                 String::newString(ev.mType));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onSpawnProjectile);
        //   });
        //   break;

        // case EVENT_TYPES::onProjectileCreated:
        //   Event::ProjectileCreatedEvent::subscribe(
        //       [](const ProjectileCreatedEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onProjectileCreated) {
        //           CallEvent(EVENT_TYPES::onProjectileCreated,
        //                     EntityClass::newEntity(ev.mShooter),
        //                     EntityClass::newEntity(ev.mProjectile));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onProjectileCreated);
        //       });
        //   break;

        // case EVENT_TYPES::onProjectileHitEntity:
        //   Event::ProjectileHitEntityEvent::subscribe(
        //       [](const ProjectileHitEntityEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onProjectileHitEntity) {
        //           CallEvent(EVENT_TYPES::onProjectileHitEntity,
        //                     EntityClass::newEntity(ev.mTarget),
        //                     EntityClass::newEntity(ev.mSource));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onProjectileHitEntity);
        //       });
        //   break;

        // case EVENT_TYPES::onEntityTransformation:
        //   Event::EntityTransformEvent::subscribe([](const EntityTransformEvent
        //   &ev) {
        //     IF_LISTENED(EVENT_TYPES::onEntityTransformation) {
        //       CallEvent(EVENT_TYPES::onEntityTransformation,
        //                 String::newString(to_string(ev.mBeforeEntityUniqueId->id)),
        //                 EntityClass::newEntity(ev.mAfterEntity));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onEntityTransformation);
        //   });
        //   break;

        // case EVENT_TYPES::onProjectileHitBlock:
        //   Event::ProjectileHitBlockEvent::subscribe(
        //       [](const ProjectileHitBlockEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onProjectileHitBlock) {
        //           CallEvent(EVENT_TYPES::onProjectileHitBlock,
        //                     BlockClass::newBlock(ev.mBlockInstance),
        //                     EntityClass::newEntity(ev.mSource));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onProjectileHitBlock);
        //       });
        //   break;

        // case EVENT_TYPES::onLiquidFlow:
        //   Event::LiquidSpreadEvent::subscribe([](const LiquidSpreadEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onLiquidFlow) {
        //       CallEvent(EVENT_TYPES::onLiquidFlow,
        //                 BlockClass::newBlock(ev.mBlockInstance),
        //                 IntPos::newPos(ev.mTarget, ev.mDimensionId));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onLiquidFlow);
        //   });
        //   break;

        // case EVENT_TYPES::onUseFrameBlock:
        //   Event::PlayerUseFrameBlockEvent::subscribe(
        //       [](const PlayerUseFrameBlockEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onUseFrameBlock) {
        //           CallEvent(EVENT_TYPES::onUseFrameBlock,
        //                     PlayerClass::newPlayer(ev.mPlayer),
        //                     BlockClass::newBlock(ev.mBlockInstance));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onUseFrameBlock);
        //       });
        //   break;

    case EVENT_TYPES::onBlockInteracted:

        bus.emplaceListener<PlayerInteractBlockEvent>([](PlayerInteractBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onBlockInteracted) {
                CallEvent(
                    EVENT_TYPES::onBlockInteracted,
                    PlayerClass::newPlayer(&ev.self()),
                    BlockClass::newBlock(ev.pos(), ev.self().getDimensionId())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onBlockInteracted);
        });
        // case EVENT_TYPES::onFarmLandDecay:
        //   Event::FarmLandDecayEvent::subscribe([](const FarmLandDecayEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onFarmLandDecay) {
        //       BlockInstance bl(ev.mBlockInstance);
        //       CallEvent(EVENT_TYPES::onFarmLandDecay,
        //                 IntPos::newPos(bl.getPosition(), bl.getDimensionId()),
        //                 EntityClass::newEntity(ev.mActor));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onFarmLandDecay);
        //   });
        //   break;

        // case EVENT_TYPES::onPistonTryPush:
        //   Event::PistonTryPushEvent::subscribe([](const PistonTryPushEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onPistonTryPush) {
        //       BlockInstance bl(ev.mPistonBlockInstance);
        //       CallEvent(EVENT_TYPES::onPistonTryPush,
        //                 IntPos::newPos(bl.getPosition(), bl.getDimensionId()),
        //                 BlockClass::newBlock(ev.mTargetBlockInstance));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onPistonTryPush);
        //   });
        //   break;

        // case EVENT_TYPES::onPistonPush:
        //   Event::PistonPushEvent::subscribe([](const PistonPushEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onPistonPush) {
        //       BlockInstance bl(ev.mPistonBlockInstance);
        //       CallEvent(EVENT_TYPES::onPistonPush,
        //                 IntPos::newPos(bl.getPosition(), bl.getDimensionId()),
        //                 BlockClass::newBlock(ev.mTargetBlockInstance));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onPistonPush);
        //   });
        //   break;

        // case EVENT_TYPES::onHopperSearchItem:
        //   Event::HopperSearchItemEvent::subscribe(
        //       [](const HopperSearchItemEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onHopperSearchItem) {
        //           CallEvent(EVENT_TYPES::onHopperSearchItem,
        //                     FloatPos::newPos(ev.mPos, ev.mDimensionId),
        //                     Boolean::newBoolean(ev.isMinecart),
        //                     ItemClass::newItem(ev.mItemStack));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onHopperSearchItem);
        //       });
        //   break;

        // case EVENT_TYPES::onHopperPushOut:
        //   Event::HopperPushOutEvent::subscribe([](const HopperPushOutEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onHopperPushOut) {
        //       CallEvent(EVENT_TYPES::onHopperPushOut,
        //                 FloatPos::newPos(ev.mPos, ev.mDimensionId),
        //                 Boolean::newBoolean(ev.isMinecart),
        //                 ItemClass::newItem(ev.mItemStack));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onHopperPushOut);
        //   });
        //   break;

        // case EVENT_TYPES::onFireSpread:
        //   Event::FireSpreadEvent::subscribe([](const FireSpreadEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onFireSpread) {
        //       CallEvent(EVENT_TYPES::onFireSpread,
        //                 IntPos::newPos(ev.mTarget, ev.mDimensionId));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onFireSpread);
        //   });
        //   break;

        // case EVENT_TYPES::onBlockChanged:
        //   Event::BlockChangedEvent::subscribe([](const BlockChangedEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onBlockChanged) {
        //       CallEvent(EVENT_TYPES::onBlockChanged,
        //                 BlockClass::newBlock(ev.mPreviousBlockInstance),
        //                 BlockClass::newBlock(ev.mNewBlockInstance));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onBlockChanged);
        //   });
        //   break;

        // case EVENT_TYPES::onScoreChanged:
        //   Event::PlayerScoreChangedEvent::subscribe(
        //       [](const PlayerScoreChangedEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onScoreChanged) {
        //           CallEvent(EVENT_TYPES::onScoreChanged,
        //                     PlayerClass::newPlayer(ev.mPlayer),
        //                     Number::newNumber(ev.mScore),
        //                     String::newString(ev.mObjective->getName()),
        //                     String::newString(ev.mObjective->getDisplayName()));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onScoreChanged);
        //       });
        //   break;

        // case EVENT_TYPES::onConsoleOutput:
        //   Event::ConsoleOutputEvent::subscribe([](const ConsoleOutputEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onConsoleOutput) {
        //       CallEvent(EVENT_TYPES::onConsoleOutput,
        //       String::newString(ev.mOutput));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onConsoleOutput);
        //   });
        //   break;

        // case EVENT_TYPES::onMobSpawn:
        //   logger.warn(
        //       "Event 'onMobSpawn' is outdated, please use 'onMobTrySpawn'
        //       instead.");
        //   Event::MobTrySpawnEvent::subscribe([](const MobTrySpawnEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onMobSpawn) {
        //       CallEvent(EVENT_TYPES::onMobSpawn, String::newString(ev.mTypeName),
        //                 FloatPos::newPos(ev.mPos, ev.mDimensionId));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onMobSpawn);
        //   });
        //   break;

        // case EVENT_TYPES::onMobTrySpawn:
        //   Event::MobTrySpawnEvent::subscribe([](const MobTrySpawnEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onMobTrySpawn) {
        //       CallEvent(EVENT_TYPES::onMobTrySpawn,
        //       String::newString(ev.mTypeName),
        //                 FloatPos::newPos(ev.mPos, ev.mDimensionId));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onMobTrySpawn);
        //   });
        //   break;

        // case EVENT_TYPES::onMobSpawned:
        //   Event::MobSpawnedEvent::subscribe([](const MobSpawnedEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onMobSpawned) {
        //       CallEvent(EVENT_TYPES::onMobSpawned,
        //                 EntityClass::newEntity((Actor *)(ev.mMob)),
        //                 FloatPos::newPos(ev.mPos, ev.mDimensionId));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onMobSpawned);
        //   });
        //   break;

    case EVENT_TYPES::onExperienceAdd:
        bus.emplaceListener<PlayerAddExperienceEvent>([](PlayerAddExperienceEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onExperienceAdd) {
                CallEvent(
                    EVENT_TYPES::onExperienceAdd,
                    PlayerClass::newPlayer(&ev.self()),
                    Number::newNumber(ev.experience())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onExperienceAdd);
        });
        break;

        // case EVENT_TYPES::onBedEnter:
        //   Event::PlayerBedEnterEvent::subscribe([](const PlayerBedEnterEvent &ev)
        //   {
        //     IF_LISTENED(EVENT_TYPES::onBedEnter) {
        //       BlockInstance bl(*ev.mBlockInstance);
        //       CallEvent(EVENT_TYPES::onBedEnter,
        //       PlayerClass::newPlayer(ev.mPlayer),
        //                 IntPos::newPos(bl.getPosition(), bl.getDimensionId()));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onBedEnter);
        //   });
        //   break;

        // case EVENT_TYPES::onOpenInventory:
        //   Event::PlayerOpenInventoryEvent::subscribe(
        //       [](const PlayerOpenInventoryEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onOpenInventory) {
        //           CallEvent(EVENT_TYPES::onOpenInventory,
        //                     PlayerClass::newPlayer(ev.mPlayer));
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onOpenInventory);
        //       });
        //   break;

        //   /* DEPRECATED AND RECENTLY REMOVED - START */

    case EVENT_TYPES::onAttack:
        bus.emplaceListener<PlayerAttackEvent>([](PlayerAttackEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onAttack) {
                CallEvent(
                    EVENT_TYPES::onAttack,
                    PlayerClass::newPlayer(&ev.self()),
                    EntityClass::newEntity(&ev.target())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onAttack);
        });
        break;

        // case EVENT_TYPES::onContainerChangeSlot:
        //   Event::ContainerChangeEvent::subscribe([](const ContainerChangeEvent
        //   &ev) {
        //     IF_LISTENED(EVENT_TYPES::onContainerChange) {
        //       CallEvent(EVENT_TYPES::onContainerChange,
        //                 PlayerClass::newPlayer(ev.mPlayer),
        //                 BlockClass::newBlock(ev.mBlockInstance),
        //                 Number::newNumber(ev.mSlot),
        //                 ItemClass::newItem(ev.mPreviousItemStack),
        //                 ItemClass::newItem(ev.mNewItemStack));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onContainerChange);
        //   });
        //   break;

        // case EVENT_TYPES::onExplode:
        //   Event::EntityExplodeEvent::subscribe([](const EntityExplodeEvent &ev) {
        //     IF_LISTENED(EVENT_TYPES::onExplode) {
        //       CallEvent(
        //           EVENT_TYPES::onExplode,
        //           ev.mActor ? EntityClass::newEntity(ev.mActor) : Local<Value>(),
        //           FloatPos::newPos(ev.mPos, ev.mDimension->getDimensionId()),
        //           Number::newNumber(ev.mRadius),
        //           Number::newNumber(ev.mMaxResistance),
        //           Boolean::newBoolean(ev.mBreaking),
        //           Boolean::newBoolean(ev.mFire));
        //     }
        //     IF_LISTENED_END(EVENT_TYPES::onExplode);
        //   });
        //   break;

        // case EVENT_TYPES::onBedExplode:
        //   Event::BlockExplodeEvent::subscribe([](const BlockExplodeEvent &ev) {
        //     BlockInstance bl(ev.mBlockInstance);
        //     if (bl.getBlock()->getName() == "minecraft:bed") {
        //       IF_LISTENED(EVENT_TYPES::onBedExplode) {
        //         CallEvent(EVENT_TYPES::onBedExplode,
        //                   IntPos::newPos(bl.getPosition(), bl.getDimensionId()));
        //       }
        //       IF_LISTENED_END(EVENT_TYPES::onBedExplode);
        //     }
        //     return true;
        //   });
        //   break;

        // case EVENT_TYPES::onPlayerPullFishingHook:
        //   Event::PlayerPullFishingHookEvent::subscribe(
        //       [](const PlayerPullFishingHookEvent &ev) {
        //         IF_LISTENED(EVENT_TYPES::onPlayerPullFishingHook) {
        //           CallEvent(EVENT_TYPES::onPlayerPullFishingHook,
        //                     PlayerClass::newPlayer(ev.mPlayer),
        //                     ev.mActor ? EntityClass::newEntity(ev.mActor)
        //                               : Local<Value>(),
        //                     ev.mItemStack ? ItemClass::newItem(ev.mItemStack)
        //                                   : Local<Value>());
        //         }
        //         IF_LISTENED_END(EVENT_TYPES::onPlayerPullFishingHook);
        //         return true;
        //       });
        //   break;

        /* DEPRECATED AND RECENTLY REMOVED - END */

    default:
        break;
    }
}

void InitBasicEventListeners() {
    using namespace ll::event;
    EventBus& bus = EventBus::getInstance();

    bus.emplaceListener<ExecutingCommandEvent>([](ExecutingCommandEvent& ev) {
        if (ev.commandContext().getCommandOrigin().getOriginType() == CommandOriginType::DedicatedServer) {
            std::string cmd = ev.commandContext().mCommand;
            if (cmd.starts_with("/")) {
                cmd.erase(0, 1);
            }

            if (!ProcessDebugEngine(cmd)) {
                ev.cancel();
                return;
            }
#ifdef LLSE_BACKEND_NODEJS
            if (!NodeJsHelper::processConsoleNpmCmd(ev.mCommand)) {
                ev.cancel();
                return;
            }
#elif defined(LLSE_BACKEND_PYTHON)
            if (!PythonHelper::processConsolePipCmd(ev.mCommand)) {
                ev.cancel();
                return;
            }
#endif
            // CallEvents
            std::vector<std::string> paras;
            bool                     isFromOtherEngine = false;
            std::string              prefix            = LLSEFindCmdReg(false, cmd, paras, &isFromOtherEngine);

            if (!prefix.empty()) {
                // LLSE Registered Cmd

                bool callbackRes = CallServerCmdCallback(prefix, paras);
                IF_LISTENED(EVENT_TYPES::onConsoleCmd) { CallEvent(EVENT_TYPES::onConsoleCmd, String::newString(cmd)); }
                IF_LISTENED_END(EVENT_TYPES::onConsoleCmd);
                if (!callbackRes) {
                    ev.cancel();
                    return;
                }
            } else {
                if (isFromOtherEngine) {
                    ev.cancel();
                    return;
                }

                // Other Cmd
                IF_LISTENED(EVENT_TYPES::onConsoleCmd) { CallEvent(EVENT_TYPES::onConsoleCmd, String::newString(cmd)); }
                IF_LISTENED_END(EVENT_TYPES::onConsoleCmd);
            }
        } else if (ev.commandContext().mOrigin->getOriginType() == CommandOriginType::Player) {
            std::string cmd = ev.commandContext().mCommand;
            if (cmd.starts_with("/")) {
                cmd.erase(0, 1);
            }
            std::vector<std::string> paras;
            bool                     isFromOtherEngine = false;
            std::string              prefix            = LLSEFindCmdReg(true, cmd, paras, &isFromOtherEngine);
            Player*                  player            = static_cast<Player*>(ev.commandContext().mOrigin->getEntity());

            if (!prefix.empty()) {
                // LLSE Registered Cmd
                int  perm             = localShareData->playerCmdCallbacks[prefix].perm;
                auto permission_level = player->getCommandPermissionLevel();
                if (static_cast<int>(permission_level) >= perm) {
                    bool callbackRes = CallPlayerCmdCallback(player, prefix, paras);
                    IF_LISTENED(EVENT_TYPES::onPlayerCmd) {
                        CallEvent(EVENT_TYPES::onPlayerCmd, PlayerClass::newPlayer(player), String::newString(cmd));
                    }
                    IF_LISTENED_END(EVENT_TYPES::onPlayerCmd);
                    if (!callbackRes) {
                        ev.cancel();
                        return;
                    }
                }
            } else {
                if (isFromOtherEngine) {
                    ev.cancel();
                    return;
                }

                // Other Cmd
                IF_LISTENED(EVENT_TYPES::onPlayerCmd) {
                    CallEvent(EVENT_TYPES::onPlayerCmd, PlayerClass::newPlayer(player), String::newString(cmd));
                }
                IF_LISTENED_END(EVENT_TYPES::onPlayerCmd);
            }
            return;
        }
        return;
    });

    //   // Plugin Hot Management
    //   Event::ScriptPluginManagerEvent::subscribe_ref(
    //       [](ScriptPluginManagerEvent &ev) {
    //         switch (ev.operation) {
    //         case ScriptPluginManagerEvent::Operation::Load:
    //           // ev.pluginType is not used
    //           // since in loadPlugin there will be check
    //           try {
    //             if (PluginManager::loadPlugin(ev.target, true, true))
    //               ev.success = true;
    //           } catch (...) {
    //           }
    //           break;

    //         case ScriptPluginManagerEvent::Operation::Unload:
    //           if (PluginManager::unloadPlugin(ev.target))
    //             ev.success = true;
    //           break;

    //         case ScriptPluginManagerEvent::Operation::Reload:
    //           if (PluginManager::reloadPlugin(ev.target))
    //             ev.success = true;
    //           break;

    //         default:
    //           break;
    //         }
    //         if (ev.success)
    //           return false; // Success. No need to spread to next engine
    //         return true;
    //       });

    // ===== onServerStarted =====
    bus.emplaceListener<ServerStartedEvent>([](ServerStartedEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onServerStarted) {
            // CallEventDelayed(EVENT_TYPES::onServerStarted);
            CallEventVoid(EVENT_TYPES::onServerStarted);
        }
        IF_LISTENED_END(EVENT_TYPES::onServerStarted);
        isCmdRegisterEnabled = true;

        // 处理延迟注册
        ProcessRegCmdQueue();
    });

    // 植入tick
    ll::schedule::ServerTimeScheduler scheduler;
    scheduler.add<ll::schedule::RepeatTask>(ll::chrono::ticks(1), []() {
#ifndef LLSE_BACKEND_NODEJS
        try {
            std::list<ScriptEngine*> tmpList;
            {
                std::shared_lock<std::shared_mutex> lock(globalShareData->engineListLock);
                // low efficiency
                tmpList = globalShareData->globalEngineList;
            }
            for (auto engine : tmpList) {
                if (EngineManager::isValid(engine) && EngineManager::getEngineType(engine) == LLSE_BACKEND_TYPE) {
                    EngineScope enter(engine);
                    engine->messageQueue()->loopQueue(script::utils::MessageQueue::LoopType::kLoopOnce);
                }
            }
        } catch (...) {
            logger.error("Error occurred in Engine Message Loop!");
            logger.error("Uncaught Exception Detected!");
        }
#endif
        // Call tick event
        IF_LISTENED(EVENT_TYPES::onTick) { CallEventVoid(EVENT_TYPES::onTick); }
        IF_LISTENED_END(EVENT_TYPES::onTick);
    });
}

/* onTurnLectern // 由于还是不能拦截掉书，暂时注释
THook(void,
"?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVLecternUpdatePacket@@@Z",
    ServerNetworkHandler* handler, NetworkIdentifier* id, Packet* pkt)
{
    IF_LISTENED(EVENT_TYPES::onTurnLectern)
    {
        // Packet* pkt = *(Packet**)pPacket;
        Player* player = Raw_GetPlayerFromPacket(handler, id, pkt);
        if (!player)
            return;

        int page = *((DWORD*)pkt + 12);
        dAccess<bool>(pkt, 56) = false;
        bool shouldDropBook = *((BYTE*)pkt + 56);
        int totalPages = *((DWORD*)pkt + 13);
        auto* bp = new BlockPos;
        bp->x = *((DWORD*)pkt + 15);
        bp->y = *((DWORD*)pkt + 16);
        bp->z = *((DWORD*)pkt + 17);
        CallEventRtnVoid(EVENT_TYPES::onTurnLectern,
PlayerClass::newPlayer(player), IntPos::newPos(bp, Raw_GetPlayerDimId(player)),
page, totalPages, Boolean::newBoolean(shouldDropBook));
    }
    IF_LISTENED_END(EVENT_TYPES::onTurnLectern);
    original(handler,id,pkt);
}
*/

bool MoneyBeforeEventCallback(LLMoneyEvent type, xuid_t from, xuid_t to, money_t value) {
    switch (type) {
    case LLMoneyEvent::Add: {
        IF_LISTENED(EVENT_TYPES::beforeMoneyAdd) {
            CallEventRtnValue(EVENT_TYPES::beforeMoneyAdd, false, String::newString(to), Number::newNumber(value));
        }
        IF_LISTENED_END(EVENT_TYPES::beforeMoneyAdd);
        break;
    }
    case LLMoneyEvent::Reduce: {
        IF_LISTENED(EVENT_TYPES::beforeMoneyReduce) {
            CallEventRtnValue(EVENT_TYPES::beforeMoneyReduce, false, String::newString(to), Number::newNumber(value));
        }
        IF_LISTENED_END(EVENT_TYPES::beforeMoneyReduce);
        break;
    }
    case LLMoneyEvent::Trans: {
        IF_LISTENED(EVENT_TYPES::beforeMoneyTrans) {
            CallEventRtnValue(
                EVENT_TYPES::beforeMoneyTrans,
                false,
                String::newString(from),
                String::newString(to),
                Number::newNumber(value)
            );
        }
        IF_LISTENED_END(EVENT_TYPES::beforeMoneyTrans);
        break;
    }
    case LLMoneyEvent::Set: {
        IF_LISTENED(EVENT_TYPES::beforeMoneySet) {
            CallEventRtnValue(EVENT_TYPES::beforeMoneySet, false, String::newString(to), Number::newNumber(value));
        }
        IF_LISTENED_END(EVENT_TYPES::beforeMoneySet);
        break;
    }
    default:
        break;
    }
    return true;
}

bool MoneyEventCallback(LLMoneyEvent type, xuid_t from, xuid_t to, money_t value) {
    switch (type) {
    case LLMoneyEvent::Add: {
        IF_LISTENED(EVENT_TYPES::onMoneyAdd) {
            CallEventRtnValue(EVENT_TYPES::onMoneyAdd, false, String::newString(to), Number::newNumber(value));
        }
        IF_LISTENED_END(EVENT_TYPES::onMoneyAdd);
        break;
    }
    case LLMoneyEvent::Reduce: {
        IF_LISTENED(EVENT_TYPES::onMoneyReduce) {
            CallEventRtnValue(EVENT_TYPES::onMoneyReduce, false, String::newString(to), Number::newNumber(value));
        }
        IF_LISTENED_END(EVENT_TYPES::onMoneyReduce);
        break;
    }
    case LLMoneyEvent::Trans: {
        IF_LISTENED(EVENT_TYPES::onMoneyTrans) {
            CallEventRtnValue(
                EVENT_TYPES::onMoneyTrans,
                false,
                String::newString(from),
                String::newString(to),
                Number::newNumber(value)
            );
        }
        IF_LISTENED_END(EVENT_TYPES::onMoneyTrans);
        break;
    }
    case LLMoneyEvent::Set: {
        IF_LISTENED(EVENT_TYPES::onMoneySet) {
            CallEventRtnValue(EVENT_TYPES::onMoneySet, false, String::newString(to), Number::newNumber(value));
        }
        IF_LISTENED_END(EVENT_TYPES::onMoneySet);
        break;
    }
    default:
        break;
    }
    return true;
}