#include "api/EventAPI.h"

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
#include "legacy/engine/LocalShareData.h"
#include "legacy/main/BuiltinCommands.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
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
#include "ll/api/event/player/PlayerDisconnectEvent.h"
#include "ll/api/event/player/PlayerInteractBlockEvent.h"
#include "ll/api/event/player/PlayerJoinEvent.h"
#include "ll/api/event/player/PlayerJumpEvent.h"
#include "ll/api/event/player/PlayerPickUpItemEvent.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"
#include "ll/api/event/player/PlayerRespawnEvent.h"
#include "ll/api/event/player/PlayerSneakEvent.h"
#include "ll/api/event/player/PlayerSprintEvent.h"
#include "ll/api/event/player/PlayerSwingEvent.h"
#include "ll/api/event/player/PlayerUseItemEvent.h"
#include "ll/api/event/server/ServerStartedEvent.h"
#include "ll/api/event/world/BlockChangedEvent.h"
#include "ll/api/event/world/FireSpreadEvent.h"
#include "ll/api/event/world/SpawnMobEvent.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "lse/Entry.h"
#include "lse/events/BlockEvents.h"
#include "lse/events/EntityEvents.h"
#include "lse/events/OtherEvents.h"
#include "lse/events/PlayerEvents.h"
#include "main/Global.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/VanillaItemNames.h"
#include "mc/world/level/dimension/Dimension.h"

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS
#include "legacy/main/NodeJsHelper.h"
#endif

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON
#include "legacy/main/PythonHelper.h"
#endif

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
            LLSEAddEventListener(EngineScope::currentEngine(), args[0].asString().toString(), args[1].asFunction())
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
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("Event {} not found!"_tr(eventName));
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "In Plugin: " + getEngineData(engine)->pluginName
        );
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
    for (auto& [index, cb] : getEngineData(engine)->unloadCallbacks) {
        cb(engine);
    }
    getEngineData(engine)->unloadCallbacks.clear();
    return true;
}

//////////////////// Events ////////////////////

// TODO:
void EnableEventListener(int eventId) {
    using namespace ll::event;
    EventBus& bus = EventBus::getInstance();
    switch ((EVENT_TYPES)eventId) {
    case EVENT_TYPES::onJoin:
        bus.emplaceListener<PlayerJoinEvent>([](PlayerJoinEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onJoin) {
                if (!CallEvent(EVENT_TYPES::onJoin, PlayerClass::newPlayer(&ev.self()))) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onJoin);
        });
        break;

    case EVENT_TYPES::onPreJoin:
        bus.emplaceListener<PlayerConnectEvent>([](PlayerConnectEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onPreJoin) {
                if (!CallEvent(EVENT_TYPES::onPreJoin, PlayerClass::newPlayer(&ev.self()))) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onPreJoin);
        });
        break;

    case EVENT_TYPES::onLeft:
        bus.emplaceListener<PlayerDisconnectEvent>([](PlayerDisconnectEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onLeft) {
                CallEvent(EVENT_TYPES::onLeft, PlayerClass::newPlayer(&ev.self())); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onLeft);
        });
        break;

    case EVENT_TYPES::onChat:
        bus.emplaceListener<PlayerChatEvent>([](PlayerChatEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onChat) {
                if (!CallEvent(
                        EVENT_TYPES::onChat,
                        PlayerClass::newPlayer(&ev.self()),
                        String::newString(ev.message())
                    )) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onChat);
        });

    case EVENT_TYPES::onChangeDim:
        lse::events::player::ChangeDimensionEvent();
        break;

    case EVENT_TYPES::onPlayerSwing:
        bus.emplaceListener<PlayerSwingEvent>([](PlayerSwingEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onPlayerSwing) {
                CallEvent(EVENT_TYPES::onPlayerSwing, PlayerClass::newPlayer(&ev.self())); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onPlayerSwing);
        });
        break;

    case EVENT_TYPES::onAttackEntity:
        bus.emplaceListener<PlayerAttackEvent>([](PlayerAttackEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onAttackEntity) {
                if (!CallEvent(
                        EVENT_TYPES::onAttackEntity,
                        PlayerClass::newPlayer(&ev.self()),
                        EntityClass::newEntity(&ev.target())
                    )) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onAttackEntity);
        });
        break;
        break;

    case EVENT_TYPES::onAttackBlock:
        lse::events::player::AttackBlockEvent();
        break;

    case EVENT_TYPES::onPlayerDie:
        bus.emplaceListener<ll::event::PlayerDieEvent>([](ll::event::PlayerDieEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onPlayerDie) {
                Actor* source = ll::service::getLevel()
                                    ->getDimension(ev.self().getDimensionId())
                                    ->fetchEntity(ev.source().getEntityUniqueID(), false);
                CallEvent(
                    EVENT_TYPES::onPlayerDie,
                    PlayerClass::newPlayer(&ev.self()),
                    (source ? EntityClass::newEntity(source) : Local<Value>())
                ); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onPlayerDie);
        });
        break;

    case EVENT_TYPES::onRespawn:
        bus.emplaceListener<ll::event::PlayerRespawnEvent>([](ll::event::PlayerRespawnEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onRespawn) {
                CallEvent(EVENT_TYPES::onRespawn, PlayerClass::newPlayer(&ev.self())); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onRespawn)
        });
        break;

    case EVENT_TYPES::onStartDestroyBlock:
        lse::events::player::StartDestroyBlock();
        break;

    case EVENT_TYPES::onDestroyBlock:
        bus.emplaceListener<PlayerDestroyBlockEvent>([](PlayerDestroyBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onDestroyBlock) {
                if (!CallEvent(
                        EVENT_TYPES::onDestroyBlock,
                        PlayerClass::newPlayer(&ev.self()),
                        BlockClass::newBlock(ev.pos(), ev.self().getDimensionId())
                    )) {
                    ev.cancel();
                }
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
                if (!CallEvent(
                        EVENT_TYPES::onPlaceBlock,
                        PlayerClass::newPlayer(&ev.self()),
                        BlockClass::newBlock(truePos, ev.self().getDimensionId()),
                        Number::newNumber((schar)ev.face())
                    )) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onPlaceBlock);
        });
        break;

    case EVENT_TYPES::afterPlaceBlock:
        bus.emplaceListener<PlayerPlacedBlockEvent>([](PlayerPlacedBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::afterPlaceBlock) {
                CallEvent(
                    EVENT_TYPES::afterPlaceBlock,
                    PlayerClass::newPlayer(&ev.self()),
                    BlockClass::newBlock(ev.pos(), ev.self().getDimensionId())
                ); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::afterPlaceBlock);
        });
        break;
    case EVENT_TYPES::onJump:
        bus.emplaceListener<PlayerJumpEvent>([](PlayerJumpEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onJump) {
                CallEvent(EVENT_TYPES::onJump, PlayerClass::newPlayer(&ev.self())); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onJump);
        });
        break;

    case EVENT_TYPES::onDropItem:
        lse::events::player::DropItem();
        break;

    case EVENT_TYPES::onTakeItem:
        bus.emplaceListener<PlayerPickUpItemEvent>([](PlayerPickUpItemEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onTakeItem) {
                if (!CallEvent(
                        EVENT_TYPES::onTakeItem,
                        PlayerClass::newPlayer(&ev.self()),
                        EntityClass::newEntity(&ev.itemActor()),
                        ItemClass::newItem(&ev.itemActor().item())
                    )) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onTakeItem);
        });
        break;

    case EVENT_TYPES::onOpenContainer:
        lse::events::player::OpenContainerEvent();
        break;

    case EVENT_TYPES::onCloseContainer:
        lse::events::player::CloseContainerEvent();
        break;

    case EVENT_TYPES::onInventoryChange:
        lse::events::player::ChangeSlotEvent();
        break;

    case EVENT_TYPES::onUseItem:
        bus.emplaceListener<PlayerUseItemEvent>([](PlayerUseItemEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onUseItem) {
                if (!CallEvent(
                        EVENT_TYPES::onUseItem,
                        PlayerClass::newPlayer(&ev.self()),
                        ItemClass::newItem(&ev.item())
                    )) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onUseItem);
        });
        break;

    case EVENT_TYPES::onUseItemOn:
        bus.emplaceListener<PlayerInteractBlockEvent>([](PlayerInteractBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onUseItemOn) {
                if (!CallEvent(
                        EVENT_TYPES::onUseItemOn,
                        PlayerClass::newPlayer(&ev.self()),
                        ItemClass::newItem(&ev.item()),
                        BlockClass::newBlock(ev.block(), ev.blockPos(), ev.self().getDimensionId()),
                        Number::newNumber((schar)ev.face()),
                        FloatPos::newPos(ev.clickPos(), ev.self().getDimensionId())
                    )) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onUseItemOn);
        });
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

    case EVENT_TYPES::onChangeArmorStand:
        lse::events::block::ArmorStandSwapItemEvent();
        break;

    case EVENT_TYPES::onChangeSprinting:
        bus.emplaceListener<PlayerSprintingEvent>([](PlayerSprintingEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onChangeSprinting) {
                CallEvent(
                    EVENT_TYPES::onChangeSprinting,
                    PlayerClass::newPlayer(&ev.self()),
                    Boolean::newBoolean(true)
                ); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onChangeSprinting);
        });
        bus.emplaceListener<PlayerSprintedEvent>([](PlayerSprintedEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onChangeSprinting) {
                CallEvent(
                    EVENT_TYPES::onChangeSprinting,
                    PlayerClass::newPlayer(&ev.self()),
                    Boolean::newBoolean(false)
                ); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onChangeSprinting);
        });
        break;

    case EVENT_TYPES::onSneak:
        bus.emplaceListener<PlayerSneakingEvent>([](PlayerSneakingEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onSneak) {
                if (!CallEvent(EVENT_TYPES::onSneak, PlayerClass::newPlayer(&ev.self()), Boolean::newBoolean(true))) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onSneak);
        });
        bus.emplaceListener<PlayerSneakedEvent>([](PlayerSneakedEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onSneak) {
                if (!CallEvent(EVENT_TYPES::onSneak, PlayerClass::newPlayer(&ev.self()), Boolean::newBoolean(false))) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onSneak);
        });
        break;

    case EVENT_TYPES::onOpenContainerScreen:
        lse::events::player::OpenContainerScreenEvent();
        break;

    case EVENT_TYPES::onSetArmor:
        lse::events::player::SetArmorEvent();
        break;

    case EVENT_TYPES::onEat:
        bus.emplaceListener<PlayerUseItemEvent>([](PlayerUseItemEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onEat) {
                if ((ev.item().getItem()->isFood() || ev.item().isPotionItem()
                     || ev.item().getTypeName() == VanillaItemNames::MilkBucket().getString())
                    && (ev.self().isHungry() || ev.self().forceAllowEating())) {
                    if (!CallEvent(
                            EVENT_TYPES::onEat,
                            PlayerClass::newPlayer(&ev.self()),
                            ItemClass::newItem(&ev.item())
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onEat);
        });

        break;

    case EVENT_TYPES::onAte:
        lse::events::player::EatEvent();
        break;

    case EVENT_TYPES::onConsumeTotem:
        lse::events::player::ConsumeTotemEvent();
        break;

    case EVENT_TYPES::onEffectAdded:
        lse::events::player::AddEffectEvent();
        break;
    case EVENT_TYPES::onEffectRemoved:
        lse::events::player::RemoveEffectEvent();
        break;
    case EVENT_TYPES::onEffectUpdated:
        lse::events::entity::EffectUpdateEvent();
        break;

    case EVENT_TYPES::onUseRespawnAnchor:
        lse::events::player::UseRespawnAnchorEvent();
        break;

    case EVENT_TYPES::onRide:
        lse::events::entity::ActorRideEvent();
        break;

    case EVENT_TYPES::onEntityExplode:
        lse::events::block::ExplodeEvent();
        break;

    case EVENT_TYPES::onBlockExplode:
        lse::events::block::ExplodeEvent();
        break;

    case EVENT_TYPES::onRespawnAnchorExplode:
        lse::events::block::RespawnAnchorExplodeEvent();
        break;

    case EVENT_TYPES::onBlockExploded:
        lse::events::block::BlockExplodedEvent();
        break;

    case EVENT_TYPES::onCmdBlockExecute:
        lse::events::block::CommandBlockExecuteEvent();
        break;

    case EVENT_TYPES::onRedStoneUpdate:
        lse::events::block::RedstoneUpdateEvent();
        break;

    case EVENT_TYPES::onWitherBossDestroy:
        lse::events::entity::WitherDestroyEvent();
        break;

    case EVENT_TYPES::onMobHurt:
        lse::events::entity::MobHurtEvent();
        break;

    case EVENT_TYPES::onStepOnPressurePlate:
        lse::events::block::PressurePlateTriggerEvent();
        break;

    case EVENT_TYPES::onMobDie:
        bus.emplaceListener<MobDieEvent>([](MobDieEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onMobDie) {
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
                    Number::newNumber((int)ev.source().getCause())
                ); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onMobDie);
        });
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
        lse::events::block::LiquidFlowEvent();
        break;

    case EVENT_TYPES::onUseFrameBlock:
        lse::events::player::UseFrameEvent();
        break;

    case EVENT_TYPES::onBlockInteracted:
        bus.emplaceListener<PlayerInteractBlockEvent>([](PlayerInteractBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onBlockInteracted) {
                if (!CallEvent(
                        EVENT_TYPES::onBlockInteracted,
                        PlayerClass::newPlayer(&ev.self()),
                        BlockClass::newBlock(ev.blockPos(), ev.self().getDimensionId())
                    )) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onBlockInteracted);
        });

    case EVENT_TYPES::onFarmLandDecay:
        lse::events::block::FarmDecayEvent();
        break;

    case EVENT_TYPES::onPistonTryPush:
        lse::events::block::PistonPushEvent();
        break;

    case EVENT_TYPES::onPistonPush:
        lse::events::block::PistonPushEvent();
        break;

    case EVENT_TYPES::onHopperSearchItem:
        lse::events::block::HopperEvent(true);
        break;

    case EVENT_TYPES::onHopperPushOut:
        lse::events::block::HopperEvent(false);
        break;

    case EVENT_TYPES::onFireSpread:
        bus.emplaceListener<FireSpreadEvent>([](FireSpreadEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onFireSpread) {
                if (!CallEvent(
                        EVENT_TYPES::onFireSpread,
                        IntPos::newPos(ev.pos(), ev.blockSource().getDimensionId())
                    )) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onFireSpread);
        });
        break;

    case EVENT_TYPES::onBlockChanged:
        bus.emplaceListener<BlockChangedEvent>([](BlockChangedEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onBlockChanged) {
                CallEvent(
                    EVENT_TYPES::onBlockChanged,
                    BlockClass::newBlock(ev.previousBlock(), ev.pos(), ev.blockSource()),
                    BlockClass::newBlock(ev.newBlock(), ev.pos(), ev.blockSource())
                ); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onBlockChanged);
        });
        break;

    case EVENT_TYPES::onScoreChanged:
        lse::events::other::ScoreChangedEvent();
        break;

    case EVENT_TYPES::onMobSpawn:
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().warn(
            "Event 'onMobSpawn' is outdated, please use 'onMobTrySpawn' instead."
        );
        bus.emplaceListener<SpawningMobEvent>([](SpawningMobEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onMobSpawn) {
                CallEvent(
                    EVENT_TYPES::onMobSpawn,
                    String::newString(ev.identifier().getFullName()),
                    FloatPos::newPos(ev.pos(), ev.blockSource().getDimensionId())
                ); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onMobSpawn);
        });
        break;

    case EVENT_TYPES::onMobTrySpawn:
        bus.emplaceListener<SpawningMobEvent>([](SpawningMobEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onMobTrySpawn) {
                if (!CallEvent(
                        EVENT_TYPES::onMobTrySpawn,
                        String::newString(ev.identifier().getFullName()),
                        FloatPos::newPos(ev.pos(), ev.blockSource().getDimensionId())
                    )) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onMobTrySpawn);
        });
        break;

    case EVENT_TYPES::onMobSpawned:
        bus.emplaceListener<SpawnedMobEvent>([](SpawnedMobEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onMobSpawned) {
                CallEvent(
                    EVENT_TYPES::onMobSpawned,
                    EntityClass::newEntity(ev.mob().has_value() ? ev.mob().as_ptr() : nullptr),
                    FloatPos::newPos(ev.pos(), ev.blockSource().getDimensionId())
                ); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onMobSpawned);
        });
        break;

    case EVENT_TYPES::onExperienceAdd:
        bus.emplaceListener<PlayerAddExperienceEvent>([](PlayerAddExperienceEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onExperienceAdd) {
                if (!CallEvent(
                        EVENT_TYPES::onExperienceAdd,
                        PlayerClass::newPlayer(&ev.self()),
                        Number::newNumber(ev.experience())
                    )) {
                    ev.cancel();
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onExperienceAdd);
        });
        break;

    case EVENT_TYPES::onBedEnter:
        lse::events::player::SleepEvent();
        break;

    case EVENT_TYPES::onOpenInventory:
        lse::events::player::OpenInventoryEvent();
        break;
    case EVENT_TYPES::onPlayerPullFishingHook:
        lse::events::player::PullFishingHookEvent();
        break;
    case EVENT_TYPES::onPlayerInteractEntity:
        lse::events::player::InteractEntityEvent();
        break;
    case EVENT_TYPES::onNpcCmd:
        lse::events::entity::NpcCommandEvent();
        break;
    default:
        break;
    }
}

void InitBasicEventListeners() {
    using namespace ll::event;
    EventBus& bus = EventBus::getInstance();

    bus.emplaceListener<ExecutingCommandEvent>([](ExecutingCommandEvent& ev) {
        auto originType = ev.commandContext().getCommandOrigin().getOriginType();
        if (originType == CommandOriginType::DedicatedServer) {
            std::string cmd = ev.commandContext().mCommand;
            if (cmd.starts_with("/")) {
                cmd.erase(0, 1);
            }
#ifndef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS
            if (!ProcessDebugEngine(cmd)) {
                ev.cancel();
                return;
            }
#endif
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS
            if (!NodeJsHelper::processConsoleNpmCmd(cmd)) {
                ev.cancel();
                return;
            }
#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON)
            if (!PythonHelper::processConsolePipCmd(cmd)) {
                ev.cancel();
                return;
            }
#endif
            // CallEvents
            std::vector<std::string> paras;
            bool                     isFromOtherEngine = false;
            std::string              prefix            = LLSEFindCmdReg(false, cmd, paras, &isFromOtherEngine);

            if (!prefix.empty()) {
                // LSE Registered Cmd
                bool callbackRes = CallServerCmdCallback(prefix, paras);
                IF_LISTENED(EVENT_TYPES::onConsoleCmd) {
                    if (!CallEvent(EVENT_TYPES::onConsoleCmd, String::newString(cmd))) {
                        ev.cancel();
                    }
                }
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
                IF_LISTENED(EVENT_TYPES::onConsoleCmd) {
                    if (!CallEvent(EVENT_TYPES::onConsoleCmd, String::newString(cmd))) {
                        ev.cancel();
                    }
                }
                IF_LISTENED_END(EVENT_TYPES::onConsoleCmd);
            }
        } else if (originType == CommandOriginType::Player) {
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
                        if (!CallEvent(
                                EVENT_TYPES::onPlayerCmd,
                                PlayerClass::newPlayer(player),
                                String::newString(cmd)
                            )) {
                            ev.cancel();
                        }
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
                    if (!CallEvent(EVENT_TYPES::onPlayerCmd, PlayerClass::newPlayer(player), String::newString(cmd))) {
                        ev.cancel();
                    }
                }
                IF_LISTENED_END(EVENT_TYPES::onPlayerCmd);
            }
        }
    });

    // ===== onServerStarted =====
    bus.emplaceListener<ServerStartedEvent>([](ServerStartedEvent&) {
        ll::coro::keepThis([]() -> ll::coro::CoroTask<> {
            using namespace ll::chrono_literals;
            co_await 1_tick;

            IF_LISTENED(EVENT_TYPES::onServerStarted) {
                CallEvent(EVENT_TYPES::onServerStarted); // Not cancellable
            }
            IF_LISTENED_END(EVENT_TYPES::onServerStarted);

            isCmdRegisterEnabled = true;

            // 处理延迟注册
            ProcessRegCmdQueue();
        }).launch(ll::thread::ServerThreadExecutor::getDefault());
    });

    // 植入tick
    ll::coro::keepThis([]() -> ll::coro::CoroTask<> {
        using namespace ll::chrono_literals;

        while (true) {
            co_await 1_tick;

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
                lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                    "Error occurred in Engine Message Loop!"
                );
                ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
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

bool MoneyBeforeEventCallback(LLMoneyEvent type, std::string from, std::string to, long long value) {
    switch (type) {
    case LLMoneyEvent::Add: {
        IF_LISTENED(EVENT_TYPES::beforeMoneyAdd) {
            if (!CallEvent(EVENT_TYPES::beforeMoneyAdd, String::newString(to), Number::newNumber(value))) {
                return false;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::beforeMoneyAdd);
        break;
    }
    case LLMoneyEvent::Reduce: {
        IF_LISTENED(EVENT_TYPES::beforeMoneyReduce) {
            if (!CallEvent(EVENT_TYPES::beforeMoneyReduce, String::newString(to), Number::newNumber(value))) {
                return false;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::beforeMoneyReduce);
        break;
    }
    case LLMoneyEvent::Trans: {
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
    case LLMoneyEvent::Set: {
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

bool MoneyEventCallback(LLMoneyEvent type, std::string from, std::string to, long long value) {
    switch (type) {
    case LLMoneyEvent::Add: {
        IF_LISTENED(EVENT_TYPES::onMoneyAdd) {
            if (!CallEvent(EVENT_TYPES::onMoneyAdd, String::newString(to), Number::newNumber(value))) {
                return false;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onMoneyAdd);
        break;
    }
    case LLMoneyEvent::Reduce: {
        IF_LISTENED(EVENT_TYPES::onMoneyReduce) {
            if (!CallEvent(EVENT_TYPES::onMoneyReduce, String::newString(to), Number::newNumber(value))) {
                return false;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onMoneyReduce);
        break;
    }
    case LLMoneyEvent::Trans: {
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
    case LLMoneyEvent::Set: {
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
