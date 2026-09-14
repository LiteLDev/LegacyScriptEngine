#include "legacy/api/EventAPI.h"

#include "legacy/api/APIHelp.h"
#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/LegacyCommandAPI.h"
#include "legacy/api/McAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "legacy/engine/EngineManager.h" // IWYU pragma: keep
#include "legacy/engine/EngineOwnData.h"
#include "legacy/engine/GlobalShareData.h"
#include "legacy/main/BuiltinCommands.h" // IWYU pragma: keep
#include "legacy/main/Global.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/command/ExecuteCommandEvent.h"
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
#include "ll/api/service/GamingStatus.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "lse/Entry.h"
#include "lse/api/Thread.h"
#include "lse/events/BlockEvents.h"
#include "lse/events/EntityEvents.h"
#include "lse/events/OtherEvents.h"
#include "lse/events/PlayerEvents.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/AttributeInstanceConstRef.h"
#include "mc/world/effect/MobEffectInstance.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/VanillaItemNames.h"
#include "mc/world/level/ChangeDimensionRequest.h"
#include "mc/world/level/Explosion.h"
#include "mc/world/level/dimension/Dimension.h"

#include <ila/event/world/ExplosionEvent.h>
#include <ila/event/world/PistonPushEvent.h>
#include <ila/event/world/RedstoneUpdateEvent.h>
#include <ila/event/world/actor/ActorGetEffectEvent.h>
#include <ila/event/world/actor/ActorRideEvent.h>
#include <ila/event/world/actor/ActorTriggerPressurePlateEvent.h>
#include <ila/event/world/actor/ArmorStandSwapItemEvent.h>
#include <ila/event/world/actor/MobTakeBlockEvent.h>
#include <ila/event/world/actor/player/PlayerAteEvent.h>
#include <ila/event/world/actor/player/PlayerAttackBlockEvent.h>
#include <ila/event/world/actor/player/PlayerChangeDimensionEvent.h>
#include <ila/event/world/actor/player/PlayerChangeSlotEvent.h>
#include <ila/event/world/actor/player/PlayerDropItemEvent.h>
#include <ila/event/world/actor/player/PlayerInteractEntityEvent.h>
#include <ila/event/world/actor/player/PlayerOpenContainerEvent.h>
#include <ila/event/world/actor/player/PlayerOperatedItemFrameEvent.h>
#include <ila/event/world/actor/player/PlayerStartSleepEvent.h>
#include <ila/event/world/level/block/FarmDecayEvent.h>
#include <ila/event/world/level/block/LiquidFlowEvent.h>

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
        bus.emplaceListener<PlayerJoinEvent>([](PlayerJoinEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onJoin) {
                if (isServerThread()) {
                    if (!CallEvent(EVENT_TYPES::onJoin, PlayerClass::newPlayer(&ev.self()))) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onJoin);
        });
        break;

    case EVENT_TYPES::onPreJoin:
        bus.emplaceListener<PlayerConnectEvent>([](PlayerConnectEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onPreJoin) {
                if (isServerThread()) {
                    if (!CallEvent(EVENT_TYPES::onPreJoin, PlayerClass::newPlayer(&ev.self()))) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onPreJoin);
        });
        break;

    case EVENT_TYPES::onLeft:
        bus.emplaceListener<PlayerDisconnectEvent>([](PlayerDisconnectEvent const& ev) {
            IF_LISTENED(EVENT_TYPES::onLeft) {
                if (isServerThread() && ll::getGamingStatus() != ll::GamingStatus::Stopping) {
                    CallEvent(EVENT_TYPES::onLeft, PlayerClass::newPlayer(&ev.self())); // Not cancellable
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onLeft);
        });
        break;

    case EVENT_TYPES::onChat:
        bus.emplaceListener<PlayerChatEvent>([](PlayerChatEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onChat) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onChat,
                            PlayerClass::newPlayer(&ev.self()),
                            String::newString(ev.message())
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onChat);
        });
        break;

    case EVENT_TYPES::onChangeDim:
        bus.emplaceListener<ila::mc::PlayerChangeDimensionBeforeEvent>(
            [](ila::mc::PlayerChangeDimensionBeforeEvent& event) {
                IF_LISTENED(EVENT_TYPES::onChangeDim) {
                    if (isServerThread()) {
                        CallEvent(
                            EVENT_TYPES::onChangeDim,
                            PlayerClass::newPlayer(&event.self()),
                            Number::newNumber(event.changeDimensionRequest().mToDimensionId->mValue)
                        );
                    }
                }
                IF_LISTENED_END(EVENT_TYPES::onChangeDim);
            }
        );
        break;

    case EVENT_TYPES::onPlayerSwing:
        bus.emplaceListener<PlayerSwingEvent>([](PlayerSwingEvent const& ev) {
            IF_LISTENED(EVENT_TYPES::onPlayerSwing) {
                if (isServerThread()) {
                    CallEvent(EVENT_TYPES::onPlayerSwing, PlayerClass::newPlayer(&ev.self())); // Not cancellable
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onPlayerSwing);
        });
        break;

    case EVENT_TYPES::onAttackEntity:
        bus.emplaceListener<PlayerAttackEvent>([](PlayerAttackEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onAttackEntity) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onAttackEntity,
                            PlayerClass::newPlayer(&ev.self()),
                            EntityClass::newEntity(&ev.target())
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onAttackEntity);
        });
        break;
        break;

    case EVENT_TYPES::onAttackBlock:
        bus.emplaceListener<ila::mc::PlayerAttackBlockBeforeEvent>([](ila::mc::PlayerAttackBlockBeforeEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onAttackBlock) {
                if (isServerThread()) {
                    ItemStack const& item = ev.self().getSelectedItem();
                    if (!CallEvent(
                            EVENT_TYPES::onAttackBlock,
                            PlayerClass::newPlayer(&ev.self()),
                            BlockClass::newBlock(ev.pos(), ev.self().getDimensionId()),
                            !item.isNull() ? ItemClass::newItem(&const_cast<ItemStack&>(item)) : Local<Value>()
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onAttackBlock);
        });
        break;

    case EVENT_TYPES::onPlayerDie:
        bus.emplaceListener<PlayerDieEvent>([](PlayerDieEvent const& ev) {
            IF_LISTENED(EVENT_TYPES::onPlayerDie) {
                if (isServerThread()) {
                    Actor* source = ev.self().getDimension().fetchEntity(ev.source().getEntityUniqueID(), false);
                    CallEvent(
                        EVENT_TYPES::onPlayerDie,
                        PlayerClass::newPlayer(&ev.self()),
                        (source ? EntityClass::newEntity(source) : Local<Value>())
                    ); // Not cancellable
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onPlayerDie);
        });
        break;

    case EVENT_TYPES::onRespawn:
        bus.emplaceListener<PlayerRespawnEvent>([](PlayerRespawnEvent const& ev) {
            IF_LISTENED(EVENT_TYPES::onRespawn) {
                if (isServerThread()) {
                    CallEvent(EVENT_TYPES::onRespawn, PlayerClass::newPlayer(&ev.self())); // Not cancellable
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onRespawn)
        });
        break;

    case EVENT_TYPES::onStartDestroyBlock:
        bus.emplaceListener<ila::mc::PlayerAttackBlockBeforeEvent>([](ila::mc::PlayerAttackBlockBeforeEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onStartDestroyBlock) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onStartDestroyBlock,
                            PlayerClass::newPlayer(&ev.self()),
                            BlockClass::newBlock(ev.pos(), ev.self().getDimensionId())
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onStartDestroyBlock)
        });
        break;

    case EVENT_TYPES::onDestroyBlock:
        bus.emplaceListener<PlayerDestroyBlockEvent>([](PlayerDestroyBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onDestroyBlock) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onDestroyBlock,
                            PlayerClass::newPlayer(&ev.self()),
                            BlockClass::newBlock(ev.pos(), ev.self().getDimensionId())
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onDestroyBlock);
        });
        break;

    case EVENT_TYPES::onPlaceBlock:
        bus.emplaceListener<PlayerPlacingBlockEvent>([](PlayerPlacingBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onPlaceBlock) {
                if (isServerThread()) {
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
                    default:
                        break;
                    }
                    auto block = ev.self().getCarriedItem().mBlock;
                    if (!CallEvent(
                            EVENT_TYPES::onPlaceBlock,
                            PlayerClass::newPlayer(&ev.self()),
                            block ? BlockClass::newBlock(*block, truePos, ev.self().getDimensionId())
                                  : BlockClass::newBlock(truePos, ev.self().getDimensionId()),
                            Number::newNumber(static_cast<schar>(ev.face()))
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onPlaceBlock);
        });
        break;

    case EVENT_TYPES::afterPlaceBlock:
        bus.emplaceListener<PlayerPlacedBlockEvent>([](PlayerPlacedBlockEvent const& ev) {
            IF_LISTENED(EVENT_TYPES::afterPlaceBlock) {
                if (isServerThread()) {
                    CallEvent(
                        EVENT_TYPES::afterPlaceBlock,
                        PlayerClass::newPlayer(&ev.self()),
                        BlockClass::newBlock(ev.pos(), ev.self().getDimensionId())
                    ); // Not cancellable
                }
            }
            IF_LISTENED_END(EVENT_TYPES::afterPlaceBlock);
        });
        break;
    case EVENT_TYPES::onJump:
        bus.emplaceListener<PlayerJumpEvent>([](PlayerJumpEvent const& ev) {
            IF_LISTENED(EVENT_TYPES::onJump) {
                if (isServerThread()) {
                    CallEvent(EVENT_TYPES::onJump, PlayerClass::newPlayer(&ev.self())); // Not cancellable
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onJump);
        });
        break;

    case EVENT_TYPES::onDropItem:
        bus.emplaceListener<ila::mc::PlayerDropItemBeforeEvent>([](ila::mc::PlayerDropItemBeforeEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onDropItem) {
                if (isServerThread()) {
                    // TODO: Remove const_cast after update ila
                    if (!CallEvent(
                            EVENT_TYPES::onDropItem,
                            PlayerClass::newPlayer(&ev.self()),
                            ItemClass::newItem(&const_cast<ItemStack&>(ev.item()))
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onDropItem);
        });
        break;

    case EVENT_TYPES::onTakeItem:
        bus.emplaceListener<PlayerPickUpItemEvent>([](PlayerPickUpItemEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onTakeItem) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onTakeItem,
                            PlayerClass::newPlayer(&ev.self()),
                            EntityClass::newEntity(&ev.itemActor()),
                            ItemClass::newItem(&ev.itemActor().item())
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onTakeItem);
        });
        break;

    case EVENT_TYPES::onOpenContainer:
        bus.emplaceListener<ila::mc::PlayerOpenContainerBeforeEvent>([](ila::mc::PlayerOpenContainerBeforeEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onOpenContainer) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onOpenContainer,
                            PlayerClass::newPlayer(&ev.self()),
                            BlockClass::newBlock(ev.containerBlockPos(), ev.self().getDimensionId())
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onOpenContainer);
        });
        break;

    case EVENT_TYPES::onCloseContainer:
        lse::events::player::CloseContainerEvent();
        break;

    case EVENT_TYPES::onInventoryChange:
        bus.emplaceListener<ila::mc::PlayerChangeSlotEvent>([](ila::mc::PlayerChangeSlotEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onInventoryChange) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onInventoryChange,
                            PlayerClass::newPlayer(&ev.self()),
                            ev.slot(),
                            ItemClass::newItem(&const_cast<ItemStack&>(ev.oldItem())),
                            ItemClass::newItem(&const_cast<ItemStack&>(ev.newItem()))
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onInventoryChange);
        });
        break;

    case EVENT_TYPES::onUseItem:
        bus.emplaceListener<PlayerUseItemEvent>([](PlayerUseItemEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onUseItem) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onUseItem,
                            PlayerClass::newPlayer(&ev.self()),
                            ItemClass::newItem(&ev.item())
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onUseItem);
        });
        break;

    case EVENT_TYPES::onUseItemOn:
        bus.emplaceListener<PlayerInteractBlockEvent>([](PlayerInteractBlockEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onUseItemOn) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onUseItemOn,
                            PlayerClass::newPlayer(&ev.self()),
                            ItemClass::newItem(&ev.item()),
                            ev.block() ? BlockClass::newBlock(ev.block(), ev.blockPos(), ev.self().getDimensionId())
                                       : BlockClass::newBlock(ev.blockPos(), ev.self().getDimensionId()),
                            Number::newNumber(static_cast<schar>(ev.face())),
                            FloatPos::newPos(ev.clickPos(), ev.self().getDimensionId())
                        )) {
                        ev.cancel();
                    }
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

    case EVENT_TYPES::onDispenseItem:
        lse::events::block::DispenseItemEvent();
        break;

    case EVENT_TYPES::onChangeArmorStand:
        bus.emplaceListener<ila::mc::ArmorStandSwapItemBeforeEvent>([](ila::mc::ArmorStandSwapItemBeforeEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onChangeArmorStand) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onChangeArmorStand,
                            EntityClass::newEntity(&ev.self()),
                            PlayerClass::newPlayer(&ev.player()),
                            Number::newNumber(static_cast<int>(ev.slot()))
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onChangeArmorStand);
        });
        break;

    case EVENT_TYPES::onChangeSprinting:
        bus.emplaceListener<PlayerSprintingEvent>([](PlayerSprintingEvent const& ev) {
            IF_LISTENED(EVENT_TYPES::onChangeSprinting) {
                if (isServerThread()) {
                    CallEvent(
                        EVENT_TYPES::onChangeSprinting,
                        PlayerClass::newPlayer(&ev.self()),
                        Boolean::newBoolean(true)
                    ); // Not cancellable
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onChangeSprinting);
        });
        bus.emplaceListener<PlayerSprintedEvent>([](PlayerSprintedEvent const& ev) {
            IF_LISTENED(EVENT_TYPES::onChangeSprinting) {
                if (isServerThread()) {
                    CallEvent(
                        EVENT_TYPES::onChangeSprinting,
                        PlayerClass::newPlayer(&ev.self()),
                        Boolean::newBoolean(false)
                    ); // Not cancellable
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onChangeSprinting);
        });
        break;

    case EVENT_TYPES::onSneak:
        bus.emplaceListener<PlayerSneakingEvent>([](PlayerSneakingEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onSneak) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onSneak,
                            PlayerClass::newPlayer(&ev.self()),
                            Boolean::newBoolean(true)
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onSneak);
        });
        bus.emplaceListener<PlayerSneakedEvent>([](PlayerSneakedEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onSneak) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onSneak,
                            PlayerClass::newPlayer(&ev.self()),
                            Boolean::newBoolean(false)
                        )) {
                        ev.cancel();
                    }
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
                if (isServerThread()) {
                    if (ev.item().mItem->isFood() || ev.item().isPotionItem()
                        || ev.item().getTypeName() == VanillaItemNames::MilkBucket().getString()) {
                        auto attribute = ev.self().getAttribute(Player::HUNGER());
                        if (attribute.mPtr->mCurrentMaxValue > attribute.mPtr->mCurrentValue) {
                            if (!CallEvent(
                                    EVENT_TYPES::onEat,
                                    PlayerClass::newPlayer(&ev.self()),
                                    ItemClass::newItem(&ev.item())
                                )) {
                                ev.cancel();
                            }
                        }
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onEat);
        });
        break;

    case EVENT_TYPES::onAte:
        bus.emplaceListener<ila::mc::PlayerAteBeforeEvent>([](ila::mc::PlayerAteBeforeEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onAte) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onAte,
                            PlayerClass::newPlayer(&ev.self()),
                            ItemClass::newItem(&ev.item())
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onAte);
        });
        break;

    case EVENT_TYPES::onConsumeTotem:
        lse::events::player::ConsumeTotemEvent();
        break;

    case EVENT_TYPES::onEffectAdded:
        bus.emplaceListener<ila::mc::ActorGetEffectBeforeEvent>([](ila::mc::ActorGetEffectBeforeEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onEffectAdded) {
                if (isServerThread() && ev.self().isPlayer()) {
                    if (!CallEvent(
                            EVENT_TYPES::onEffectAdded,
                            PlayerClass::newPlayer(reinterpret_cast<Player*>(&ev.self())),
                            String::newString(
                                MobEffect::mMobEffects()[static_cast<MobEffectIds>(ev.effect().mId)]
                                    ->mComponentName->getString()
                            ),
                            Number::newNumber(ev.effect().mAmplifier),
                            Number::newNumber(ev.effect().mDuration->mValue)
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onEffectAdded);
        });
        break;
    case EVENT_TYPES::onEffectRemoved:
        lse::events::player::RemoveEffectEvent();
        break;
    case EVENT_TYPES::onEffectUpdated:
        bus.emplaceListener<ila::mc::ActorGetEffectBeforeEvent>([](ila::mc::ActorGetEffectBeforeEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onEffectUpdated) {
                if (isServerThread() && ev.self().isPlayer()) {
                    if (ev.self().getEffect(ev.effect().mId)) {
                        if (!CallEvent(
                                EVENT_TYPES::onEffectUpdated,
                                PlayerClass::newPlayer(reinterpret_cast<Player*>(&ev.self())),
                                String::newString(
                                    MobEffect::mMobEffects()[static_cast<MobEffectIds>(ev.effect().mId)]
                                        ->mComponentName->getString()
                                ),
                                Number::newNumber(ev.effect().mAmplifier),
                                Number::newNumber(ev.effect().mDuration->mValue)
                            )) {
                            ev.cancel();
                        }
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onEffectUpdated);
        });
        break;

    case EVENT_TYPES::onUseRespawnAnchor:
        lse::events::player::UseRespawnAnchorEvent();
        break;

    case EVENT_TYPES::onRide:
        bus.emplaceListener<ila::mc::ActorRideBeforeEvent>([](ila::mc::ActorRideBeforeEvent& ev) {
            if (isServerThread()) {
                if (!CallEvent(
                        EVENT_TYPES::onRide,
                        EntityClass::newEntity(&ev.self()),
                        EntityClass::newEntity(&ev.target())
                    )) {
                    ev.cancel();
                }
            }
        });
        break;

    case EVENT_TYPES::onEntityExplode:
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
        break;
    case EVENT_TYPES::onBlockExplode:
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
        break;

    case EVENT_TYPES::onWitherBossDestroy:
        lse::events::entity::WitherDestroyEvent();
        break;

    case EVENT_TYPES::onMobHurt:
        lse::events::entity::MobHurtEvent();
        break;

    case EVENT_TYPES::onStepOnPressurePlate:
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
        break;

    case EVENT_TYPES::onMobDie:
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
        break;

    case EVENT_TYPES::onUseFrameBlock:
        bus.emplaceListener<ila::mc::PlayerOperatedItemFrameBeforeEvent>(
            [](ila::mc::PlayerOperatedItemFrameBeforeEvent& ev) {
                IF_LISTENED(EVENT_TYPES::onUseFrameBlock) {
                    if (isServerThread()) {
                        if (!CallEvent(
                                EVENT_TYPES::onUseFrameBlock,
                                PlayerClass::newPlayer(&ev.self()),
                                BlockClass::newBlock(ev.blockPos(), ev.self().getDimensionId())
                            )) {
                            ev.cancel();
                        }
                    }
                }
                IF_LISTENED_END(EVENT_TYPES::onUseFrameBlock);
            }
        );
        break;

    case EVENT_TYPES::onBlockInteracted:
        lse::events::player::BlockInteractedEvent();
        break;

    case EVENT_TYPES::onFarmLandDecay:
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
        break;

    case EVENT_TYPES::onPistonTryPush:
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
        break;
    case EVENT_TYPES::onPistonPush:
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
        break;

    case EVENT_TYPES::onBlockChanged:
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
        break;

    case EVENT_TYPES::onScoreChanged:
        lse::events::other::ScoreChangedEvent();
        break;

    case EVENT_TYPES::onMobSpawn:
        lse::LegacyScriptEngine::getLogger().warn(
            "Event 'onMobSpawn' is outdated, please use 'onMobTrySpawn' instead."
        );
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
        break;

    case EVENT_TYPES::onMobTrySpawn:
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
        break;

    case EVENT_TYPES::onMobSpawned:
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
        break;

    case EVENT_TYPES::onPortalTrySpawnPigZombie:
        lse::events::entity::PortalTrySpawnPigZombieEvent();
        break;

    case EVENT_TYPES::onExperienceAdd:
        bus.emplaceListener<PlayerAddExperienceEvent>([](PlayerAddExperienceEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onExperienceAdd) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onExperienceAdd,
                            PlayerClass::newPlayer(&ev.self()),
                            Number::newNumber(ev.experience())
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onExperienceAdd);
        });
        break;

    case EVENT_TYPES::onBedEnter:
        bus.emplaceListener<ila::mc::PlayerStartSleepBeforeEvent>([](ila::mc::PlayerStartSleepBeforeEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onBedEnter) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onBedEnter,
                            PlayerClass::newPlayer(&ev.self()),
                            IntPos::newPos(ev.pos(), ev.self().getDimensionId())
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onBedEnter);
        });
        break;

    case EVENT_TYPES::onOpenInventory:
        lse::events::player::OpenInventoryEvent();
        break;
    case EVENT_TYPES::onPlayerPullFishingHook:
        lse::events::player::PullFishingHookEvent();
        break;
    case EVENT_TYPES::onPlayerInteractEntity:
        bus.emplaceListener<ila::mc::PlayerInteractEntityBeforeEvent>([](ila::mc::PlayerInteractEntityBeforeEvent& ev) {
            IF_LISTENED(EVENT_TYPES::onPlayerInteractEntity) {
                if (isServerThread()) {
                    if (!CallEvent(
                            EVENT_TYPES::onPlayerInteractEntity,
                            PlayerClass::newPlayer(&ev.self()),
                            EntityClass::newEntity(&ev.target()),
                            FloatPos::newPos(ev.pos(), ev.self().getDimensionId())
                        )) {
                        ev.cancel();
                    }
                }
            }
            IF_LISTENED_END(EVENT_TYPES::onPlayerInteractEntity)
        });
        break;
    case EVENT_TYPES::onNpcCmd:
        lse::events::entity::NpcCommandEvent();
        break;
    case EVENT_TYPES::onEndermanTakeBlock:
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
