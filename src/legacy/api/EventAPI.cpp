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
#include "legacy/events/EventHooks.h"
#include "legacy/main/NodeJsHelper.h"
#include "legacy/main/PythonHelper.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/command/ExecuteCommandEvent.h"
#include "ll/api/event/entity/ActorHurtEvent.h"
#include "ll/api/event/entity/MobDieEvent.h"
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
#include "ll/api/event/world/BlockChangedEvent.h"
#include "ll/api/event/world/FireSpreadEvent.h"
#include "ll/api/event/world/SpawnMobEvent.h"
#include "ll/api/schedule/Scheduler.h"
#include "ll/api/schedule/Task.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/utils/StringUtils.h"
#include "main/Global.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/VanillaItemNames.h"
#include "mc/world/level./BlockSource.h"
#include "mc/world/level/dimension/Dimension.h"

#include <exception>
#include <list>
#include <shared_mutex>
#include <string>

//////////////////// Listeners ////////////////////

// 监听器表
std::list<ListenerListType> listenerList[int(EVENT_TYPES::EVENT_COUNT)];

// 监听器历史
bool hasListened[int(EVENT_TYPES::EVENT_COUNT)] = {false};

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
        lse::getSelfPluginInstance().getLogger().error("Event \"" + eventName + "\" No Found!\n");
        lse::getSelfPluginInstance().getLogger().error("In Plugin: " + ENGINE_GET_DATA(engine)->pluginName);
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
            IF_LISTENED(EVENT_TYPES::onLeft) {
                CallEventUncancelable(EVENT_TYPES::onLeft, PlayerClass::newPlayer(&ev.self()));
            }
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

    case EVENT_TYPES::onChangeDim:
        lse::events::PlayerChangeDimensionEvent();
        break;

    case EVENT_TYPES::onPlayerSwing:
        bus.emplaceListener<PlayerSwingEvent>([](PlayerSwingEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onPlayerSwing) {
                CallEventUncancelable(EVENT_TYPES::onPlayerSwing, PlayerClass::newPlayer(&ev.self()));
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

    case EVENT_TYPES::onAttackBlock:
        lse::events::PlayerAttackBlockEvent();
        break;

    case EVENT_TYPES::onPlayerDie:
        bus.emplaceListener<ll::event::PlayerDieEvent>([](ll::event::PlayerDieEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onPlayerDie) {
                Actor* source = ll::service::getLevel()
                                    ->getDimension(ev.self().getDimensionId())
                                    ->fetchEntity(ev.source().getEntityUniqueID(), false);
                CallEventUncancelable(
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
                CallEventUncancelable(EVENT_TYPES::onRespawn, PlayerClass::newPlayer(&ev.self()));
            }
            IF_LISTENED_END(EVENT_TYPES::onRespawn)
        });
        break;

    case EVENT_TYPES::onStartDestroyBlock:
        lse::events::PlayerStartDestroyBlock();
        break;

    case EVENT_TYPES::onDestroyBlock:
        bus.emplaceListener<PlayerDestroyBlockEvent>([](PlayerDestroyBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onDestroyBlock) {
                CallEvent(
                    EVENT_TYPES::onDestroyBlock,
                    PlayerClass::newPlayer(&ev.self()),
                    BlockClass::newBlock(ev.pos(), ev.self().getDimensionId())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onDestroyBlock);
        });
        break;

    case EVENT_TYPES::onPlaceBlock:
        bus.emplaceListener<PlayerPlacingBlockEvent>([](PlayerPlacingBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onPlaceBlock) {
                BlockPos truePos = ev.pos();
                switch (ev.face()) {
                case 0:
                    --truePos.y;
                    break;
                case 1:
                    ++truePos.y;
                    break;
                case 2:
                    --truePos.z;
                    break;
                case 3:
                    ++truePos.z;
                    break;
                case 4:
                    --truePos.x;
                    break;
                case 5:
                    ++truePos.x;
                    break;
                }
                CallEvent(
                    EVENT_TYPES::onPlaceBlock,
                    PlayerClass::newPlayer(&ev.self()),
                    BlockClass::newBlock(truePos, ev.self().getDimensionId()),
                    Number::newNumber((schar)ev.face())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onPlaceBlock);
        });
        break;

    case EVENT_TYPES::afterPlaceBlock:
        bus.emplaceListener<PlayerPlacedBlockEvent>([](PlayerPlacedBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::afterPlaceBlock) {
                CallEventUncancelable(
                    EVENT_TYPES::afterPlaceBlock,
                    PlayerClass::newPlayer(&ev.self()),
                    BlockClass::newBlock(ev.pos(), ev.self().getDimensionId())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::afterPlaceBlock);
        });
        break;
    case EVENT_TYPES::onJump:
        bus.emplaceListener<PlayerJumpEvent>([](PlayerJumpEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onJump) {
                CallEventUncancelable(EVENT_TYPES::onJump, PlayerClass::newPlayer(&ev.self()));
            }
            IF_LISTENED_END(EVENT_TYPES::onJump);
        });
        break;

    case EVENT_TYPES::onDropItem:
        lse::events::PlayerDropItem();
        break;

    case EVENT_TYPES::onTakeItem:
        bus.emplaceListener<PlayerPickUpItemEvent>([](PlayerPickUpItemEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onTakeItem) {
                CallEvent(
                    EVENT_TYPES::onTakeItem,
                    PlayerClass::newPlayer(&ev.self()),
                    EntityClass::newEntity(&ev.itemActor()),
                    ItemClass::newItem(&ev.itemActor().item(), false)
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onTakeItem);
        });
        break;

    case EVENT_TYPES::onOpenContainer:
        lse::events::PlayerOpenContainerEvent();
        break;

    case EVENT_TYPES::onCloseContainer:
        lse::events::PlayerCloseContainerEvent();
        break;

    case EVENT_TYPES::onInventoryChange:
        lse::events::PlayerChangeSlotEvent();
        break;

    case EVENT_TYPES::onUseItem:
        bus.emplaceListener<PlayerUseItemEvent>([](PlayerUseItemEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onUseItem) {
                CallEvent(
                    EVENT_TYPES::onUseItem,
                    PlayerClass::newPlayer(&ev.self()),
                    ItemClass::newItem(&ev.item(), false)
                );
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
                    ItemClass::newItem(&ev.item(), false),
                    BlockClass::newBlock(&ev.block().get(), &ev.blockPos(), ev.self().getDimensionId()),
                    Number::newNumber((schar)ev.face()),
                    FloatPos::newPos(ev.clickPos(), ev.self().getDimensionId())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onUseItemOn);
        });
        break;

    case EVENT_TYPES::onUseBucketPlace:
        lse::events::PlayerUseBucketPlaceEvent();
        break;
    case EVENT_TYPES::onUseBucketTake:
        lse::events::PlayerUseBucketTakeEvent();
        break;

    case EVENT_TYPES::onContainerChange:
        lse::events::ContainerChangeEvent();
        break;

    case EVENT_TYPES::onChangeArmorStand:
        lse::events::ArmorStandSwapItemEvent();
        break;

    case EVENT_TYPES::onChangeSprinting:
        bus.emplaceListener<PlayerSprintingEvent>([](PlayerSprintingEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onChangeSprinting) {
                CallEventUncancelable(
                    EVENT_TYPES::onChangeSprinting,
                    PlayerClass::newPlayer(&ev.self()),
                    Boolean::newBoolean(true)
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onChangeSprinting);
        });
        bus.emplaceListener<PlayerSprintedEvent>([](PlayerSprintedEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onChangeSprinting) {
                CallEventUncancelable(
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

    case EVENT_TYPES::onOpenContainerScreen:
        lse::events::PlayerOpenContainerScreenEvent();
        break;

    case EVENT_TYPES::onSetArmor:
        lse::events::PlayerSetArmorEvent();
        break;

    case EVENT_TYPES::onEat:
        bus.emplaceListener<PlayerUseItemEvent>([](PlayerUseItemEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onEat) {
                if ((ev.item().getItem()->isFood() || ev.item().isPotionItem()
                     || ev.item().getTypeName() == VanillaItemNames::MilkBucket.c_str())
                    && (ev.self().isHungry() || ev.self().forceAllowEating())) {
                    CallEvent(
                        EVENT_TYPES::onEat,
                        PlayerClass::newPlayer(&ev.self()),
                        ItemClass::newItem(&ev.item(), false)
                    );
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onEat);
        });

        break;

    case EVENT_TYPES::onAte:
        lse::events::PlayerEatEvent();
        break;

    case EVENT_TYPES::onConsumeTotem:
        lse::events::PlayerConsumeTotemEvent();
        break;

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

    case EVENT_TYPES::onUseRespawnAnchor:
        lse::events::PlayerUseRespawnAnchorEvent();
        break;

    case EVENT_TYPES::onRide:
        lse::events::ActorRideEvent();
        break;

    case EVENT_TYPES::onEntityExplode:
        lse::events::ExplodeEvent();
        break;

    case EVENT_TYPES::onBlockExplode:
        lse::events::ExplodeEvent();
        break;

    case EVENT_TYPES::onRespawnAnchorExplode:
        lse::events::RespawnAnchorExplodeEvent();
        break;

    case EVENT_TYPES::onBlockExploded:
        lse::events::BlockExplodedEvent();
        break;

    case EVENT_TYPES::onCmdBlockExecute:
        lse::events::CommandBlockExecuteEvent();
        break;

    case EVENT_TYPES::onRedStoneUpdate:
        lse::events::RedstoneupdateEvent();
        break;

    case EVENT_TYPES::onWitherBossDestroy:
        lse::events::WitherDestroyEvent();
        break;

    case EVENT_TYPES::onMobHurt:
        bus.emplaceListener<ActorHurtEvent>([](ActorHurtEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onMobHurt) {
                Actor* source = nullptr;
                if (ev.source().isEntitySource()) {
                    if (ev.source().isChildEntitySource()) {
                        source = ll::service::getLevel()->fetchEntity(ev.source().getEntityUniqueID());
                    } else {
                        source = ll::service::getLevel()->fetchEntity(ev.source().getDamagingEntityUniqueID());
                    }
                }

                CallEvent(
                    EVENT_TYPES::onMobHurt,
                    EntityClass::newEntity(&ev.self()),
                    source ? EntityClass::newEntity(source) : Local<Value>(),
                    Number::newNumber(float(ev.damage())),
                    Number::newNumber((int)ev.source().getCause())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onMobHurt)
        });
        break;

    case EVENT_TYPES::onStepOnPressurePlate:
        lse::events::PressurePlateTriggerEvent();
        break;

    case EVENT_TYPES::onMobDie:
        bus.emplaceListener<MobDieEvent>([](MobDieEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onMobDie) {
                Actor* source = nullptr;
                if (ev.source().isEntitySource()) {
                    source = ll::service::getLevel()->fetchEntity(ev.source().getDamagingEntityUniqueID());
                    if (source) {
                        if (ev.source().isChildEntitySource()) source = source->getOwner();
                    }
                }

                CallEventUncancelable(
                    EVENT_TYPES::onMobDie,
                    EntityClass::newEntity(&ev.self()),
                    (source ? EntityClass::newEntity(source) : Local<Value>()),
                    Number::newNumber((int)ev.source().getCause())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onMobDie);
        });
        break;

    case EVENT_TYPES::onSpawnProjectile:
        lse::events::ProjectileSpawnEvent();
        break;

    case EVENT_TYPES::onProjectileCreated:
        lse::events::ProjectileCreatedEvent();
        break;

    case EVENT_TYPES::onProjectileHitEntity:
        lse::events::ProjectileHitEntityEvent();
        break;

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

    case EVENT_TYPES::onProjectileHitBlock:
        lse::events::ProjectileHitBlockEvent();
        break;

    case EVENT_TYPES::onLiquidFlow:
        lse::events::LiquidFlowEvent();
        break;

    case EVENT_TYPES::onUseFrameBlock:
        lse::events::PlayerUseFrameEvent();
        break;

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

    case EVENT_TYPES::onFarmLandDecay:
        lse::events::FarmDecayEvent();
        break;

    case EVENT_TYPES::onPistonTryPush:
        lse::events::PistonPushEvent();
        break;

    case EVENT_TYPES::onPistonPush:
        lse::events::PistonPushEvent();
        break;

    case EVENT_TYPES::onHopperSearchItem:
        lse::events::HopperEvent(true);
        break;

    case EVENT_TYPES::onHopperPushOut:
        lse::events::HopperEvent(false);
        break;

    case EVENT_TYPES::onFireSpread:
        bus.emplaceListener<FireSpreadEvent>([](FireSpreadEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onFireSpread) {
                CallEvent(EVENT_TYPES::onFireSpread, IntPos::newPos(ev.pos(), ev.blockSource().getDimensionId()));
            }
            IF_LISTENED_END(EVENT_TYPES::onFireSpread);
        });
        break;

    case EVENT_TYPES::onBlockChanged:
        bus.emplaceListener<BlockChangedEvent>([](BlockChangedEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onBlockChanged) {
                CallEventUncancelable(
                    EVENT_TYPES::onBlockChanged,
                    BlockClass::newBlock(&ev.previousBlock(), &ev.pos(), &ev.blockSource()),
                    BlockClass::newBlock(&ev.newBlock(), &ev.pos(), &ev.blockSource())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onBlockChanged);
        });
        break;

    case EVENT_TYPES::onScoreChanged:
        lse::events::ScoreChangedEvent();
        break;

    case EVENT_TYPES::onMobSpawn:
        lse::getSelfPluginInstance().getLogger().warn(
            "Event 'onMobSpawn' is outdated, please use 'onMobTrySpawn' instead."
        );
        bus.emplaceListener<SpawningMobEvent>([](SpawningMobEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onMobSpawn) {
                CallEventUncancelable(
                    EVENT_TYPES::onMobSpawn,
                    String::newString(ev.identifier().getFullName()),
                    FloatPos::newPos(ev.pos(), ev.blockSource().getDimensionId())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onMobSpawn);
        });
        break;

    case EVENT_TYPES::onMobTrySpawn:
        bus.emplaceListener<SpawningMobEvent>([](SpawningMobEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onMobTrySpawn) {
                CallEvent(
                    EVENT_TYPES::onMobTrySpawn,
                    String::newString(ev.identifier().getFullName()),
                    FloatPos::newPos(ev.pos(), ev.blockSource().getDimensionId())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onMobTrySpawn);
        });
        break;

    case EVENT_TYPES::onMobSpawned:
        bus.emplaceListener<SpawnedMobEvent>([](SpawnedMobEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onMobSpawned) {
                CallEventUncancelable(
                    EVENT_TYPES::onMobSpawned,
                    EntityClass::newEntity(ev.mob().has_value() ? ev.mob().as_ptr() : nullptr),
                    FloatPos::newPos(ev.pos(), ev.blockSource().getDimensionId())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onMobSpawned);
        });
        break;

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

    case EVENT_TYPES::onBedEnter:
        lse::events::PlayerSleepEvent();
        break;

    case EVENT_TYPES::onOpenInventory:
        lse::events::PlayerOpenInventoryEvent();
        break;
    case EVENT_TYPES::onPlayerPullFishingHook:
        lse::events::PlayerPullFishingHookEvent();
        break;
    default:
        break;
    }
}

ll::schedule::ServerTimeScheduler eventScheduler;

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
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS
            if (!NodeJsHelper::processConsoleNpmCmd(ev.commandContext().mCommand)) {
                ev.cancel();
                return;
            }
#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON)
            if (!PythonHelper::processConsolePipCmd(ev.commandContext().mCommand)) {
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

    // ===== onServerStarted =====
    bus.emplaceListener<ServerStartedEvent>([](ServerStartedEvent& ev) {
        using namespace ll::chrono_literals;
        eventScheduler.add<ll::schedule::DelayTask>(1_tick, [] {
            IF_LISTENED(EVENT_TYPES::onServerStarted) { CallEventUncancelable(EVENT_TYPES::onServerStarted); }
            IF_LISTENED_END(EVENT_TYPES::onServerStarted);
            isCmdRegisterEnabled = true;

            // 处理延迟注册
            ProcessRegCmdQueue();
        });
    });

    // 植入tick
    using namespace ll::chrono_literals;

    eventScheduler.add<ll::schedule::RepeatTask>(1_tick, [] {
#ifndef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS
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
            lse::getSelfPluginInstance().getLogger().error("Error occurred in Engine Message Loop!");
            lse::getSelfPluginInstance().getLogger().error("Uncaught Exception Detected!");
        }
#endif
        // Call tick event
        IF_LISTENED(EVENT_TYPES::onTick) { CallEventUncancelable(EVENT_TYPES::onTick); }
        IF_LISTENED_END(EVENT_TYPES::onTick);
    });
}

bool MoneyBeforeEventCallback(LLMoneyEvent type, std::string from, std::string to, long long value) {
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

bool MoneyEventCallback(LLMoneyEvent type, std::string from, std::string to, long long value) {
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
