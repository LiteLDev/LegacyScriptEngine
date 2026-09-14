#include "lse/events/PlayerEvents.h"

#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/event/EventBus.h"
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
#include "ll/api/service/GamingStatus.h"
#include "lse/api/Thread.h"
#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/AttributeInstanceConstRef.h"
#include "mc/world/effect/MobEffectInstance.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/VanillaItemNames.h"
#include "mc/world/level/ChangeDimensionRequest.h"
#include "mc/world/level/dimension/Dimension.h"

#include <ila/event/world/actor/ActorGetEffectEvent.h>
#include <ila/event/world/actor/ActorRideEvent.h>
#include <ila/event/world/actor/ArmorStandSwapItemEvent.h>
#include <ila/event/world/actor/player/PlayerAteEvent.h>
#include <ila/event/world/actor/player/PlayerAttackBlockEvent.h>
#include <ila/event/world/actor/player/PlayerChangeDimensionEvent.h>
#include <ila/event/world/actor/player/PlayerChangeSlotEvent.h>
#include <ila/event/world/actor/player/PlayerDropItemEvent.h>
#include <ila/event/world/actor/player/PlayerInteractEntityEvent.h>
#include <ila/event/world/actor/player/PlayerOpenContainerEvent.h>
#include <ila/event/world/actor/player/PlayerOperatedItemFrameEvent.h>
#include <ila/event/world/actor/player/PlayerStartSleepEvent.h>

namespace lse::events::player {
using namespace ll::event;
using api::thread::isServerThread;
#define bus EventBus::getInstance()

void onJoin() {
    bus.getInstance().emplaceListener<PlayerJoinEvent>([](PlayerJoinEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onJoin) {
            if (isServerThread()) {
                if (!CallEvent(EVENT_TYPES::onJoin, PlayerClass::newPlayer(&ev.self()))) {
                    ev.cancel();
                }
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onJoin);
    });
}

void onPreJoin() {
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
}

void onLeft() {
    bus.emplaceListener<PlayerDisconnectEvent>([](PlayerDisconnectEvent const& ev) {
        IF_LISTENED(EVENT_TYPES::onLeft) {
            if (isServerThread() && ll::getGamingStatus() != ll::GamingStatus::Stopping) {
                CallEvent(EVENT_TYPES::onLeft, PlayerClass::newPlayer(&ev.self())); // Not cancellable
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onLeft);
    });
}

void onChat() {
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
}

void onChangeDim() {
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
}

void onPlayerSwing() {
    bus.emplaceListener<PlayerSwingEvent>([](PlayerSwingEvent const& ev) {
        IF_LISTENED(EVENT_TYPES::onPlayerSwing) {
            if (isServerThread()) {
                CallEvent(EVENT_TYPES::onPlayerSwing, PlayerClass::newPlayer(&ev.self())); // Not cancellable
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onPlayerSwing);
    });
}

void onAttackEntity() {
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
}

void onAttackBlock() {
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
}

void onPlayerDie() {
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
}

void onRespawn() {
    bus.emplaceListener<PlayerRespawnEvent>([](PlayerRespawnEvent const& ev) {
        IF_LISTENED(EVENT_TYPES::onRespawn) {
            if (isServerThread()) {
                CallEvent(EVENT_TYPES::onRespawn, PlayerClass::newPlayer(&ev.self())); // Not cancellable
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onRespawn)
    });
}

void onStartDestroyBlock() {
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
}

void onDestroyBlock() {
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
}

void onPlaceBlock() {
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
}

void afterPlaceBlock() {
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
}

void onJump() {
    bus.emplaceListener<PlayerJumpEvent>([](PlayerJumpEvent const& ev) {
        IF_LISTENED(EVENT_TYPES::onJump) {
            if (isServerThread()) {
                CallEvent(EVENT_TYPES::onJump, PlayerClass::newPlayer(&ev.self())); // Not cancellable
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onJump);
    });
}

void onDropItem() {
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
}

void onTakeItem() {
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
}

void onOpenContainer() {
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
}

void onInventoryChange() {
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
}

void onUseItem() {
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
}

void onUseItemOn() {
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
}

void onChangeArmorStand() {
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
}

void onChangeSprinting() {
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
}

void onSneak() {
    bus.emplaceListener<PlayerSneakingEvent>([](PlayerSneakingEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onSneak) {
            if (isServerThread()) {
                if (!CallEvent(EVENT_TYPES::onSneak, PlayerClass::newPlayer(&ev.self()), Boolean::newBoolean(true))) {
                    ev.cancel();
                }
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onSneak);
    });
    bus.emplaceListener<PlayerSneakedEvent>([](PlayerSneakedEvent& ev) {
        IF_LISTENED(EVENT_TYPES::onSneak) {
            if (isServerThread()) {
                if (!CallEvent(EVENT_TYPES::onSneak, PlayerClass::newPlayer(&ev.self()), Boolean::newBoolean(false))) {
                    ev.cancel();
                }
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onSneak);
    });
}

void onEat() {
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
}

void onAte() {
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
}

void onEffectAdded() {
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
}

void onEffectUpdated() {
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
}

void onRide() {
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
}

void onUseFrameBlock() {
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
}

void onExperienceAdd() {
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
}

void onBedEnter() {
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
}

void onPlayerInteractEntity() {
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
}
} // namespace lse::events::player
