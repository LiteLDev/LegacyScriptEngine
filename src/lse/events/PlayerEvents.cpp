#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include "ll/api/service/Bedrock.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/deps/ecs/WeakEntityRef.h"
#include "mc/server/ServerPlayer.h"
#include "mc/server/module/VanillaServerGameplayEventListener.h"
#include "mc/world/ContainerID.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/FishingHook.h"
#include "mc/world/actor/item/ItemActor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/PlayerItemInUse.h"
#include "mc/world/containers/models/LevelContainerModel.h"
#include "mc/world/effect/EffectDuration.h"
#include "mc/world/effect/MobEffectInstance.h"
#include "mc/world/events/BlockEventCoordinator.h"
#include "mc/world/events/EventResult.h"
#include "mc/world/events/PlayerOpenContainerEvent.h"
#include "mc/world/gamemode/InteractionResult.h"
#include "mc/world/inventory/transaction/ComplexInventoryTransaction.h"
#include "mc/world/inventory/transaction/InventoryAction.h"
#include "mc/world/inventory/transaction/InventorySource.h"
#include "mc/world/inventory/transaction/InventoryTransaction.h"
#include "mc/world/item/BucketItem.h"
#include "mc/world/item/ItemInstance.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BedrockSpawner.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChangeDimensionRequest.h"
#include "mc/world/level/Explosion.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/BasePressurePlateBlock.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/ItemFrameBlock.h"
#include "mc/world/level/block/RespawnAnchorBlock.h"
#include "mc/world/level/block/actor/BarrelBlockActor.h"
#include "mc/world/level/block/actor/ChestBlockActor.h"
#include "mc/world/level/block/actor/PistonBlockActor.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/material/Material.h"
#include "mc/world/phys/AABB.h"
#include "mc/world/phys/HitResult.h"

namespace lse::events::player {
LL_TYPE_INSTANCE_HOOK(
    StartDestroyHook,
    HookPriority::Normal,
    BlockEventCoordinator,
    &BlockEventCoordinator::sendBlockDestructionStarted,
    void,
    ::Player&         player,
    const ::BlockPos& blockPos,
    const ::Block&    hitBlock,
    uchar             face
) {
    IF_LISTENED(EVENT_TYPES::onStartDestroyBlock) {
        if (!CallEvent(
                EVENT_TYPES::onStartDestroyBlock,
                PlayerClass::newPlayer(&player),
                BlockClass::newBlock(hitBlock, blockPos, player.getDimensionId())
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onStartDestroyBlock)
    origin(player, blockPos, hitBlock, face);
}

LL_TYPE_INSTANCE_HOOK(
    DropItemHook1,
    HookPriority::Normal,
    Player,
    &Player::$drop,
    bool,
    ItemStack const& item,
    bool             randomly
) {
    IF_LISTENED(EVENT_TYPES::onDropItem) {
        if (!CallEvent(
                EVENT_TYPES::onDropItem,
                PlayerClass::newPlayer(this),
                ItemClass::newItem(&const_cast<ItemStack&>(item))
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onDropItem);
    return origin(item, randomly);
}

LL_TYPE_INSTANCE_HOOK(
    DropItemHook2,
    HookPriority::Normal,
    ComplexInventoryTransaction,
    &ComplexInventoryTransaction::$handle,
    InventoryTransactionError,
    Player& player,
    bool    isSenderAuthority
) {
    if (mType == ComplexInventoryTransaction::Type::NormalTransaction) {
        IF_LISTENED(EVENT_TYPES::onDropItem) {
            InventorySource source{
                InventorySourceType::ContainerInventory,
                ContainerID::Inventory,
                InventorySource::InventorySourceFlags::NoFlag
            };
            auto& actions = mTransaction->getActions(source);
            if (actions.size() == 1) {
                if (!CallEvent(
                        EVENT_TYPES::onDropItem,
                        PlayerClass::newPlayer(&player),
                        ItemClass::newItem(&const_cast<ItemStack&>(player.getInventory().getItem(actions[0].mSlot)))
                    )) {
                    return InventoryTransactionError::NoError;
                }
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onDropItem);
    }
    return origin(player, isSenderAuthority);
}

LL_TYPE_INSTANCE_HOOK(
    OpenContainerHook,
    HookPriority::Normal,
    VanillaServerGameplayEventListener,
    &VanillaServerGameplayEventListener::$onEvent,
    EventResult,
    struct PlayerOpenContainerEvent const& playerOpenContainerEvent
) {
    IF_LISTENED(EVENT_TYPES::onOpenContainer) {
        Actor* actor = static_cast<WeakEntityRef*>((void*)&playerOpenContainerEvent)->tryUnwrap<Actor>();
        if (actor && actor->isType(ActorType::Player)) {
            if (!CallEvent(
                    EVENT_TYPES::onOpenContainer,
                    PlayerClass::newPlayer(static_cast<Player*>(actor)),
                    BlockClass::newBlock(playerOpenContainerEvent.mUnkb08e33.as<BlockPos>(), actor->getDimensionId())
                )) {
                return EventResult::StopProcessing;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onOpenContainer);
    return origin(playerOpenContainerEvent);
}

LL_TYPE_INSTANCE_HOOK(
    CloseContainerHook1,
    HookPriority::Normal,
    ChestBlockActor,
    &ChestBlockActor::$stopOpen,
    void,
    Player& player
) {
    IF_LISTENED(EVENT_TYPES::onCloseContainer) {
        if (!CallEvent(
                EVENT_TYPES::onCloseContainer,
                PlayerClass::newPlayer(&player),
                BlockClass::newBlock(getPosition(), player.getDimensionId())
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onCloseContainer);
    origin(player);
}

LL_TYPE_INSTANCE_HOOK(
    CloseContainerHook2,
    HookPriority::Normal,
    BarrelBlockActor,
    &BarrelBlockActor::$stopOpen,
    void,
    Player& player
) {
    IF_LISTENED(EVENT_TYPES::onCloseContainer) {
        if (!CallEvent(
                EVENT_TYPES::onCloseContainer,
                PlayerClass::newPlayer(&player),
                BlockClass::newBlock(getPosition(), player.getDimensionId())
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onCloseContainer);
    origin(player);
}

LL_TYPE_INSTANCE_HOOK(
    ChangeSlotHook,
    HookPriority::Normal,
    Player,
    &Player::inventoryChanged,
    void,
    Container&       container,
    int              slot,
    ItemStack const& oldItem,
    ItemStack const& newItem,
    bool             forceBalanced
) {
    IF_LISTENED(EVENT_TYPES::onInventoryChange) {
        if (!CallEvent(
                EVENT_TYPES::onInventoryChange,
                PlayerClass::newPlayer(this),
                slot,
                ItemClass::newItem(&const_cast<ItemStack&>(oldItem)),
                ItemClass::newItem(&const_cast<ItemStack&>(newItem))
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onInventoryChange);
    origin(container, slot, oldItem, newItem, forceBalanced);
}

LL_TYPE_INSTANCE_HOOK(
    AttackBlockHook,
    HookPriority::Normal,
    Block,
    &Block::attack,
    bool,
    Player*         player,
    BlockPos const& pos
) {
    IF_LISTENED(EVENT_TYPES::onAttackBlock) {
        ItemStack const& item = player->getSelectedItem();
        if (!CallEvent(
                EVENT_TYPES::onAttackBlock,
                PlayerClass::newPlayer(player),
                BlockClass::newBlock(pos, player->getDimensionId()),
                !item.isNull() ? ItemClass::newItem(&const_cast<ItemStack&>(item)) : Local<Value>()
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onAttackBlock);
    return origin(player, pos);
}

LL_TYPE_INSTANCE_HOOK(
    UseFrameHook1,
    HookPriority::Normal,
    ItemFrameBlock,
    &ItemFrameBlock::$use,
    bool,
    Player&         player,
    BlockPos const& pos,
    uchar           face
) {
    IF_LISTENED(EVENT_TYPES::onUseFrameBlock) {
        if (!CallEvent(
                EVENT_TYPES::onUseFrameBlock,
                PlayerClass::newPlayer(&player),
                BlockClass::newBlock(pos, player.getDimensionId())
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onUseFrameBlock);
    return origin(player, pos, face);
}

LL_TYPE_INSTANCE_HOOK(
    UseFrameHook2,
    HookPriority::Normal,
    ItemFrameBlock,
    &ItemFrameBlock::$attack,
    bool,
    Player*         player,
    BlockPos const& pos
) {
    IF_LISTENED(EVENT_TYPES::onUseFrameBlock) {
        if (!CallEvent(
                EVENT_TYPES::onUseFrameBlock,
                PlayerClass::newPlayer(player),
                BlockClass::newBlock(pos, player->getDimensionId())
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onUseFrameBlock);
    return origin(player, pos);
}

LL_TYPE_INSTANCE_HOOK(EatHook1, HookPriority::Normal, Player, &Player::eat, void, ItemStack const& instance) {
    IF_LISTENED(EVENT_TYPES::onAte) {
        if (!CallEvent(
                EVENT_TYPES::onAte,
                PlayerClass::newPlayer(this),
                ItemClass::newItem(&const_cast<ItemStack&>(instance))
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onAte);
    origin(instance);
}

LL_TYPE_INSTANCE_HOOK(
    EatHook2,
    HookPriority::Normal,
    ItemStack,
    &ItemStack::useTimeDepleted,
    ::ItemUseMethod,
    Level*  level,
    Player* player
) {
    IF_LISTENED(EVENT_TYPES::onAte) {
        if (isPotionItem() || getTypeName() == "minecraft:milk_bucket") {
            if (!CallEvent(EVENT_TYPES::onAte, PlayerClass::newPlayer(player), ItemClass::newItem(this))) {
                return ItemUseMethod::Unknown;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onAte);
    return origin(level, player);
}

LL_TYPE_INSTANCE_HOOK(
    ChangeDimensionHook,
    HookPriority::Normal,
    Level,
    &Level::$requestPlayerChangeDimension,
    void,
    Player&                  player,
    ChangeDimensionRequest&& changeRequest
) {
    IF_LISTENED(EVENT_TYPES::onChangeDim) {
        if (!CallEvent(
                EVENT_TYPES::onChangeDim,
                PlayerClass::newPlayer(&player),
                Number::newNumber(changeRequest.mToDimensionId->id)
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onChangeDim);
    origin(player, std::move(changeRequest));
}

LL_TYPE_INSTANCE_HOOK(OpenContainerScreenHook, HookPriority::Normal, Player, &Player::canOpenContainerScreen, bool) {
    IF_LISTENED(EVENT_TYPES::onOpenContainerScreen) {
        if (!CallEvent(EVENT_TYPES::onOpenContainerScreen, PlayerClass::newPlayer(this))) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onOpenContainerScreen);
    return origin();
}

LL_TYPE_STATIC_HOOK(
    UseRespawnAnchorHook,
    HookPriority::Normal,
    RespawnAnchorBlock,
    &RespawnAnchorBlock::_trySetSpawn,
    bool,
    Player&         player,
    BlockPos const& pos,
    BlockSource&    region,
    class Level&    level
) {
    IF_LISTENED(EVENT_TYPES::onUseRespawnAnchor) {
        if (!CallEvent(
                EVENT_TYPES::onUseRespawnAnchor,
                PlayerClass::newPlayer(&player),
                IntPos::newPos(pos, region.getDimensionId())
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onUseRespawnAnchor);
    return origin(player, pos, region, level);
}

LL_TYPE_INSTANCE_HOOK(
    SleepHook,
    HookPriority::Normal,
    Player,
    &Player::$startSleepInBed,
    BedSleepingResult,
    BlockPos const& pos
) {
    IF_LISTENED(EVENT_TYPES::onBedEnter) {
        if (!CallEvent(
                EVENT_TYPES::onBedEnter,
                PlayerClass::newPlayer(this),
                IntPos::newPos(pos, this->getDimensionId())
            )) {
            return BedSleepingResult::Ok;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onBedEnter);
    return origin(pos);
}
LL_TYPE_INSTANCE_HOOK(OpenInventoryHook, HookPriority::Normal, ServerPlayer, &ServerPlayer::$openInventory, void, ) {
    IF_LISTENED(EVENT_TYPES::onOpenInventory) {
        if (!CallEvent(EVENT_TYPES::onOpenInventory, PlayerClass::newPlayer(this))) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onOpenInventory);
    origin();
}

LL_TYPE_INSTANCE_HOOK(
    PullFishingHook,
    HookPriority::Normal,
    FishingHook,
    &FishingHook::_pullCloser,
    void,
    Actor& inEntity,
    float  inSpeed
) {
    IF_LISTENED(EVENT_TYPES::onPlayerPullFishingHook) {
        if (!CallEvent(
                EVENT_TYPES::onPlayerPullFishingHook,
                PlayerClass::newPlayer(this->getPlayerOwner()),
                EntityClass::newEntity(&inEntity),
                inEntity.isType(ActorType::ItemEntity) ? ItemClass::newItem(&static_cast<ItemActor&>(inEntity).item())
                                                       : Local<Value>()
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onPlayerPullFishingHook);
    origin(inEntity, inSpeed);
}

LL_TYPE_INSTANCE_HOOK(
    UseBucketPlaceHook,
    HookPriority::Normal,
    BucketItem,
    &BucketItem::_emptyBucket,
    bool,
    BlockSource&     region,
    Block const&     contents,
    BlockPos const&  pos,
    Actor*           placer,
    ItemStack const& instance,
    uchar            face
) {
    IF_LISTENED(EVENT_TYPES::onUseBucketPlace) {
        if (!CallEvent(
                EVENT_TYPES::onUseBucketPlace,
                PlayerClass::newPlayer(static_cast<Player*>(placer)),
                ItemClass::newItem(&const_cast<ItemStack&>(instance)),
                BlockClass::newBlock(contents, pos, region),
                Number::newNumber(face),
                FloatPos::newPos(pos, region.getDimensionId())
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onUseBucketPlace);
    return origin(region, contents, pos, placer, instance, face);
}

LL_TYPE_INSTANCE_HOOK(
    UseBucketTakeHook1,
    HookPriority::Normal,
    BucketItem,
    &BucketItem::_takeLiquid,
    bool,
    ItemStack&      item,
    Actor&          entity,
    BlockPos const& pos
) {
    IF_LISTENED(EVENT_TYPES::onUseBucketTake) {
        if (!CallEvent(
                EVENT_TYPES::onUseBucketTake,
                PlayerClass::newPlayer(&static_cast<Player&>(entity)),
                ItemClass::newItem(&item),
                BlockClass::newBlock(pos, entity.getDimensionId()),
                Number::newNumber(-1),
                FloatPos::newPos(pos, entity.getDimensionId())
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onUseBucketTake);
    return origin(item, entity, pos);
}

LL_TYPE_INSTANCE_HOOK(
    UseBucketTakeHook2,
    HookPriority::Normal,
    BucketItem,
    &BucketItem::_takePowderSnow,
    bool,
    ItemStack&      item,
    Actor&          entity,
    BlockPos const& pos
) {
    IF_LISTENED(EVENT_TYPES::onUseBucketTake) {
        if (!CallEvent(
                EVENT_TYPES::onUseBucketTake,
                PlayerClass::newPlayer(&static_cast<Player&>(entity)),
                ItemClass::newItem(&item),
                BlockClass::newBlock(pos, entity.getDimensionId()),
                Number::newNumber(-1),
                FloatPos::newPos(pos, entity.getDimensionId())
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onUseBucketTake);
    return origin(item, entity, pos);
}

// LL_TYPE_INSTANCE_HOOK(
//     UseBucketTakeHook3,
//     HookPriority::Normal,
//     BucketItem,
//     &BucketItem::_useOn,
//     InteractionResult,
//     ItemStack&  instance,
//     Actor&      entity,
//     BlockPos    pos,
//     uchar       face,
//     Vec3 const& clickPos
// ) {
//     IF_LISTENED(EVENT_TYPES::onUseBucketTake) {
//         CallEventRtnValue(
//             EVENT_TYPES::onUseBucketTake,
//             InteractionResult{InteractionResult::Result::Fail},
//             PlayerClass::newPlayer(),
//             ItemClass::newItem(&instance, false),
//             EntityClass::newEntity(&entity),
//             Number::newNumber(face),
//             FloatPos::newPos(pos, entity.getDimensionId())
//         );
//     }
//     IF_LISTENED_END(EVENT_TYPES::onUseBucketTake);
//     return origin(instance, entity, pos, face, clickPos);
// }

LL_TYPE_INSTANCE_HOOK(ConsumeTotemHook, HookPriority::Normal, Player, &Player::$consumeTotem, bool) {
    IF_LISTENED(EVENT_TYPES::onConsumeTotem) {
        if (!CallEvent(EVENT_TYPES::onConsumeTotem, PlayerClass::newPlayer(this))) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onConsumeTotem);
    return origin();
}

LL_TYPE_INSTANCE_HOOK(
    SetArmorHook,
    HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::$setArmor,
    void,
    ArmorSlot        armorSlot,
    ItemStack const& item
) {
    IF_LISTENED(EVENT_TYPES::onSetArmor) {
        if (!CallEvent(
                EVENT_TYPES::onSetArmor,
                PlayerClass::newPlayer(this),
                Number::newNumber((int)armorSlot),
                ItemClass::newItem(&const_cast<ItemStack&>(item))
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onSetArmor);
    origin(armorSlot, item);
}

LL_TYPE_INSTANCE_HOOK(
    InteractEntityHook,
    HookPriority::Normal,
    Player,
    &Player::interact,
    bool,
    Actor&      actor,
    Vec3 const& location
) {
    IF_LISTENED(EVENT_TYPES::onPlayerInteractEntity) {
        if (!CallEvent(
                EVENT_TYPES::onPlayerInteractEntity,
                PlayerClass::newPlayer(this),
                EntityClass::newEntity(&actor),
                FloatPos::newPos(location, getDimensionId())
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onPlayerInteractEntity)
    return origin(actor, location);
}

LL_TYPE_INSTANCE_HOOK(
    AddEffectHook,
    HookPriority::Normal,
    Player,
    &Player::addEffect,
    void,
    ::MobEffectInstance const& effect
) {
    IF_LISTENED(EVENT_TYPES::onEffectAdded) {
        if (!CallEvent(
                EVENT_TYPES::onEffectAdded,
                PlayerClass::newPlayer(this),
                String::newString(effect.getComponentName().getString()),
                Number::newNumber(effect.getAmplifier()),
                Number::newNumber(effect.getDuration().getValueForSerialization())
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onEffectAdded);
    origin(effect);
}

LL_TYPE_INSTANCE_HOOK(
    RemoveEffectHook,
    HookPriority::Normal,
    Player,
    &Player::$onEffectRemoved,
    void,
    ::MobEffectInstance& effect
) {
    IF_LISTENED(EVENT_TYPES::onEffectRemoved) {
        if (!CallEvent(
                EVENT_TYPES::onEffectRemoved,
                PlayerClass::newPlayer(this),
                String::newString(effect.getComponentName().getString())
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onEffectRemoved);
    origin(effect);
}

void StartDestroyBlock() { StartDestroyHook::hook(); }
void DropItem() {
    DropItemHook1::hook();
    DropItemHook2::hook();
}
void OpenContainerEvent() { OpenContainerHook::hook(); }
void CloseContainerEvent() {
    CloseContainerHook1::hook();
    CloseContainerHook2::hook();
}
void ChangeSlotEvent() { ChangeSlotHook::hook(); }
void AttackBlockEvent() { AttackBlockHook::hook(); }
void UseFrameEvent() {
    UseFrameHook1::hook();
    UseFrameHook2::hook();
}
void EatEvent() {
    EatHook1::hook();
    EatHook2::hook();
}
void ChangeDimensionEvent() { ChangeDimensionHook::hook(); };
void OpenContainerScreenEvent() { OpenContainerScreenHook::hook(); }
void UseRespawnAnchorEvent() { UseRespawnAnchorHook::hook(); }
void SleepEvent() { SleepHook::hook(); }
void OpenInventoryEvent() { OpenInventoryHook::hook(); }
void PullFishingHookEvent() { PullFishingHook::hook(); }
void UseBucketPlaceEvent() { UseBucketPlaceHook::hook(); }
void UseBucketTakeEvent() {
    UseBucketTakeHook1::hook();
    UseBucketTakeHook2::hook();
}
void ConsumeTotemEvent() { ConsumeTotemHook::hook(); }
void SetArmorEvent() { SetArmorHook::hook(); }
void InteractEntityEvent() { InteractEntityHook::hook(); }
void AddEffectEvent() { AddEffectHook::hook(); }
void RemoveEffectEvent() { RemoveEffectHook::hook(); }
} // namespace lse::events::player
