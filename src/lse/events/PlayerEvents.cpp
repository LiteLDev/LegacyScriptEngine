#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "lse/api/Thread.h"
#include "mc/deps/ecs/WeakEntityRef.h"
#include "mc/network/ServerPlayerBlockUseHandler.h"
#include "mc/server/ServerPlayer.h"
#include "mc/server/module/VanillaServerGameplayEventListener.h"
#include "mc/world/ContainerID.h"
#include "mc/world/actor/ActorHurtResult.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/FishingHook.h"
#include "mc/world/actor/item/ItemActor.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/PlayerInventory.h"
#include "mc/world/containers/managers/models/ContainerManagerModel.h"
#include "mc/world/effect/MobEffectInstance.h"
#include "mc/world/events/EventResult.h"
#include "mc/world/events/PlayerOpenContainerEvent.h"
#include "mc/world/gamemode/InteractionResult.h"
#include "mc/world/inventory/network/ItemStackNetManagerBase.h"
#include "mc/world/inventory/transaction/ComplexInventoryTransaction.h"
#include "mc/world/inventory/transaction/InventoryAction.h"
#include "mc/world/inventory/transaction/InventorySource.h"
#include "mc/world/inventory/transaction/InventoryTransaction.h"
#include "mc/world/item/BucketItem.h"
#include "mc/world/item/ItemInstance.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/PotionItem.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChangeDimensionRequest.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/BasePressurePlateBlock.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/ItemFrameBlock.h"
#include "mc/world/level/block/RespawnAnchorBlock.h"
#include "mc/world/level/block/VanillaBlockTypeIds.h"
#include "mc/world/level/block/VanillaStates.h"
#include "mc/world/level/block/actor/PistonBlockActor.h"
#include "mc/world/level/block/block_events/BlockPlayerInteractEvent.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/material/Material.h"

namespace lse::events::player {

using api::thread::isServerThread;

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
        if (isServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onDropItem,
                    PlayerClass::newPlayer(this),
                    ItemClass::newItem(&const_cast<ItemStack&>(item))
                )) {
                return false;
            }
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

    IF_LISTENED(EVENT_TYPES::onDropItem) {
        if (isServerThread() && mType == ComplexInventoryTransaction::Type::NormalTransaction) {
            InventorySource source{
                InventorySourceType::ContainerInventory,
                ContainerID::Inventory,
                InventorySource::InventorySourceFlags::NoFlag
            };
            auto& actions = mTransaction->mActions->at(source);
            if (actions.size() == 1) {
                if (!CallEvent(
                        EVENT_TYPES::onDropItem,
                        PlayerClass::newPlayer(&player),
                        ItemClass::newItem(
                            &const_cast<ItemStack&>(player.mInventory->mInventory->getItem(actions[0].mSlot))
                        )
                    )) {
                    return InventoryTransactionError::NoError;
                }
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onDropItem);
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
        if (isServerThread()) {
            Actor* actor = static_cast<WeakEntityRef*>((void*)&playerOpenContainerEvent)->tryUnwrap<Actor>();
            if (actor && actor->isType(ActorType::Player)) {
                if (!CallEvent(
                        EVENT_TYPES::onOpenContainer,
                        PlayerClass::newPlayer(static_cast<Player*>(actor)),
                        BlockClass::newBlock(playerOpenContainerEvent.mBlockPos, actor->getDimensionId())
                    )) {
                    return EventResult::StopProcessing;
                }
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onOpenContainer);
    return origin(playerOpenContainerEvent);
}

LL_TYPE_INSTANCE_HOOK( // When player leaves or closes the container
    CloseContainerHook1,
    HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::doDeleteContainerManager,
    void,
    bool forceDisconnect
) {
    IF_LISTENED(EVENT_TYPES::onCloseContainer) {
        if (mContainerManager) {
            if (auto* pos = std::get_if<BlockPos>(&*mContainerManager->mScreenContext->mOwner); pos) {
                if (getDimensionBlockSource().getBlock(*pos).mBlockType->isContainerBlock()) {
                    CallEvent(
                        EVENT_TYPES::onCloseContainer,
                        PlayerClass::newPlayer(this),
                        BlockClass::newBlock(*pos, getDimensionId())
                    );
                }
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onCloseContainer);
    origin(forceDisconnect);
}

LL_TYPE_INSTANCE_HOOK( // Container closed caused by piston pushing
    CloseContainerHook2,
    HookPriority::Normal,
    PistonBlockActor,
    &PistonBlockActor::_spawnMovingBlock,
    void,
    BlockSource&    region,
    BlockPos const& blockPos
) {
    if (region.getBlock(blockPos).mBlockType->isContainerBlock()) {
        IF_LISTENED(EVENT_TYPES::onCloseContainer) {
            region.mDimension.forEachPlayer([&](Player const& player) -> bool {
                if (player.mContainerManager) {
                    if (auto* pos = std::get_if<BlockPos>(&*player.mContainerManager->mScreenContext->mOwner);
                        pos && *pos == blockPos) {
                        CallEvent(
                            EVENT_TYPES::onCloseContainer,
                            PlayerClass::newPlayer(&player),
                            BlockClass::newBlock(*pos, player.getDimensionId())
                        );
                    }
                }
                return true;
            });
        }
        IF_LISTENED_END(EVENT_TYPES::onCloseContainer);
    }
    origin(region, blockPos);
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
        if (isServerThread()) {
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
    }
    IF_LISTENED_END(EVENT_TYPES::onInventoryChange);
    origin(container, slot, oldItem, newItem, forceBalanced);
}

LL_STATIC_HOOK(
    StartDestroyBlockHook,
    HookPriority::Normal,
    &ServerPlayerBlockUseHandler::onStartDestroyBlock,
    void,
    ServerPlayer&   player,
    BlockPos const& pos,
    int             face
) {
    if (isServerThread()) {
        bool isCancelled = false;
        IF_LISTENED(EVENT_TYPES::onAttackBlock) {
            ItemStack const& item = player.getSelectedItem();
            if (!CallEvent(
                    EVENT_TYPES::onAttackBlock,
                    PlayerClass::newPlayer(&player),
                    BlockClass::newBlock(pos, player.getDimensionId()),
                    !item.isNull() ? ItemClass::newItem(&const_cast<ItemStack&>(item)) : Local<Value>()
                )) {
                isCancelled = true;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onAttackBlock);
        IF_LISTENED(EVENT_TYPES::onStartDestroyBlock) {
            if (!CallEvent(
                    EVENT_TYPES::onStartDestroyBlock,
                    PlayerClass::newPlayer(&player),
                    BlockClass::newBlock(pos, player.getDimensionId())
                )) {
                isCancelled = true;
            }
        }
        IF_LISTENED_END(EVENT_TYPES::onStartDestroyBlock)
        if (isCancelled) {
            return;
        }
    }
    return origin(player, pos, face);
}

LL_TYPE_INSTANCE_HOOK(
    UseFrameHook1,
    HookPriority::Normal,
    ItemFrameBlock,
    &ItemFrameBlock::use,
    void,
    BlockEvents::BlockPlayerInteractEvent& eventData
) {
    IF_LISTENED(EVENT_TYPES::onUseFrameBlock) {
        if (isServerThread()) {
            Player& player = eventData.mPlayer;
            if (!CallEvent(
                    EVENT_TYPES::onUseFrameBlock,
                    PlayerClass::newPlayer(&player),
                    BlockClass::newBlock(eventData.mPos, player.getDimensionId())
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onUseFrameBlock);
    return origin(eventData);
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
        if (isServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onUseFrameBlock,
                    PlayerClass::newPlayer(player),
                    BlockClass::newBlock(pos, player->getDimensionId())
                )) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onUseFrameBlock);
    return origin(player, pos);
}

LL_TYPE_INSTANCE_HOOK(EatHook, HookPriority::Normal, Player, &Player::completeUsingItem, void) {
    IF_LISTENED(EVENT_TYPES::onAte) {
        if (isServerThread()) {
            std::set<std::string> const item_names{"minecraft:potion", "minecraft:milk_bucket", "minecraft:medicine"};
            auto checked = mItemInUse->mItem->mItem->isFood() || item_names.contains(mItemInUse->mItem->getTypeName());
            if (checked
                && !CallEvent(
                    EVENT_TYPES::onAte,
                    PlayerClass::newPlayer(this),
                    ItemClass::newItem(&*mItemInUse->mItem)
                ))
                stopUsingItem();
            else origin();
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onAte);
    origin();
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
        if (isServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onChangeDim,
                    PlayerClass::newPlayer(&player),
                    Number::newNumber(changeRequest.mToDimensionId->mValue)
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onChangeDim);
    origin(player, std::move(changeRequest));
}

LL_TYPE_INSTANCE_HOOK(
    OpenContainerScreenHook,
    HookPriority::Normal,
    ItemStackNetManagerBase,
    &ItemStackNetManagerBase::$onContainerScreenOpen,
    void,
    ContainerScreenContext const& screenContext
) {
    IF_LISTENED(EVENT_TYPES::onOpenContainerScreen) {
        if (isServerThread()) {
            if (!CallEvent(EVENT_TYPES::onOpenContainerScreen, PlayerClass::newPlayer(&mPlayer))) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onOpenContainerScreen);
    return origin(screenContext);
}

LL_TYPE_INSTANCE_HOOK(
    UseRespawnAnchorHook,
    HookPriority::Normal,
    RespawnAnchorBlock,
    &RespawnAnchorBlock::use,
    void,
    ::BlockEvents::BlockPlayerInteractEvent& eventData
) {
    IF_LISTENED(EVENT_TYPES::onUseRespawnAnchor) {
        if (isServerThread()) {
            // 原版逻辑：手持萤石且未满时 _tryCharge 会优先充能并短路；
            // 充能大于 0 且位于下界（维度 1）时 _trySetSpawn 才会设置重生点，重生点已是此锚时无事发生
            auto& block = eventData.mPlayer.getDimensionBlockSource().getBlock(eventData.mPos);
            int   charge = block.getState<int>(VanillaStates::RespawnAnchorCharge().mID).value_or(0);
            auto& item = eventData.mPlayer.getSelectedItem();
            auto* itemBlockType = item.mItem ? item.mItem->mBlockType.get().get() : nullptr;
            bool  isCharging = itemBlockType
                && *itemBlockType->mNameInfo->mFullName == VanillaBlockTypeIds::Glowstone()
                && charge < static_cast<int>(VanillaStates::RespawnAnchorCharge().mVariationCount) - 1;
            auto& spawnPoint = eventData.mPlayer.mPlayerRespawnPoint;
            if (charge > 0 && !isCharging && eventData.mPlayer.getDimensionId() == 1
                && !(spawnPoint->mDimension->mValue == 1
                     && *spawnPoint->mSpawnBlockPos == *eventData.mPos)) {
                if (!CallEvent(
                        EVENT_TYPES::onUseRespawnAnchor,
                        PlayerClass::newPlayer(&eventData.mPlayer),
                        IntPos::newPos(eventData.mPos, eventData.mPlayer.getDimensionId())
                    )) {
                    return;
                }
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onUseRespawnAnchor);
    origin(eventData);
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
        if (isServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onBedEnter,
                    PlayerClass::newPlayer(this),
                    IntPos::newPos(pos, this->getDimensionId())
                )) {
                return BedSleepingResult::Ok;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onBedEnter);
    return origin(pos);
}
LL_TYPE_INSTANCE_HOOK(OpenInventoryHook, HookPriority::Normal, ServerPlayer, &ServerPlayer::$openInventory, void, ) {
    IF_LISTENED(EVENT_TYPES::onOpenInventory) {
        if (isServerThread()) {
            if (!CallEvent(EVENT_TYPES::onOpenInventory, PlayerClass::newPlayer(this))) {
                return;
            }
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
        if (isServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onPlayerPullFishingHook,
                    PlayerClass::newPlayer(this->getPlayerOwner()),
                    EntityClass::newEntity(&inEntity),
                    inEntity.isType(ActorType::ItemEntity)
                        ? ItemClass::newItem(&static_cast<ItemActor&>(inEntity).item())
                        : Local<Value>()
                )) {
                return;
            }
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
        if (isServerThread()) {
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
    }
    IF_LISTENED_END(EVENT_TYPES::onUseBucketPlace);
    return origin(region, contents, pos, placer, instance, face);
}

LL_TYPE_INSTANCE_HOOK(
    UseBucketTakeHook,
    HookPriority::Normal,
    BucketItem,
    &BucketItem::$_useOn,
    InteractionResult,
    ::ItemStack&  instance,
    ::Actor&      entity,
    ::BlockPos    pos,
    uchar         face,
    ::Vec3 const& clickPos
) {
    IF_LISTENED(EVENT_TYPES::onUseBucketTake) {
        if (isServerThread()) {
            auto& bs = entity.getDimensionBlockSource();
            using ::SharedTypes::v1_26_20::MaterialType;
            if (auto& type = bs.getMaterial(pos).mType; type != MaterialType::Water && type != MaterialType::Lava) {
                if (auto& bl = bs.getBlock(pos).mBlockType;
                    *bl->mNameInfo->mFullName != VanillaBlockTypeIds::PowderSnow()) {
                    return origin(instance, entity, pos, face, clickPos);
                }
            }
            if (!CallEvent(
                    EVENT_TYPES::onUseBucketTake,
                    PlayerClass::newPlayer(&static_cast<Player&>(entity)),
                    ItemClass::newItem(&instance),
                    BlockClass::newBlock(pos, entity.getDimensionId()),
                    Number::newNumber(-1),
                    FloatPos::newPos(pos, entity.getDimensionId())
                )) {
                return InteractionResult{false, true};
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onUseBucketTake);
    return origin(instance, entity, pos, face, clickPos);
}

LL_TYPE_INSTANCE_HOOK(ConsumeTotemHook, HookPriority::Normal, Player, &Player::$consumeTotem, bool) {
    IF_LISTENED(EVENT_TYPES::onConsumeTotem) {
        if (isServerThread()) {
            if (!CallEvent(EVENT_TYPES::onConsumeTotem, PlayerClass::newPlayer(this))) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onConsumeTotem);
    return origin();
}

LL_TYPE_INSTANCE_HOOK(
    SetArmorHook,
    HookPriority::Normal,
    Actor,
    &Actor::$setArmor,
    void,
    SharedTypes::Legacy::ArmorSlot const armorSlot,
    ItemStack const&                     item
) {

    IF_LISTENED(EVENT_TYPES::onSetArmor) {
        if (isServerThread() && isPlayer()) {
            if (!CallEvent(
                    EVENT_TYPES::onSetArmor,
                    PlayerClass::newPlayer(reinterpret_cast<Player*>(this)),
                    Number::newNumber(static_cast<int>(armorSlot)),
                    ItemClass::newItem(&const_cast<ItemStack&>(item))
                )) {
                return;
            }
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
    InteractionResult,
    Actor&      actor,
    Vec3 const& location
) {
    IF_LISTENED(EVENT_TYPES::onPlayerInteractEntity) {
        if (isServerThread()) {
            if (!CallEvent(
                    EVENT_TYPES::onPlayerInteractEntity,
                    PlayerClass::newPlayer(this),
                    EntityClass::newEntity(&actor),
                    FloatPos::newPos(location, getDimensionId())
                )) {
                return {false, true};
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onPlayerInteractEntity)
    return origin(actor, location);
}

LL_TYPE_INSTANCE_HOOK(
    AddEffectHook,
    HookPriority::Normal,
    Actor,
    &Actor::addEffect,
    void,
    ::MobEffectInstance const& effect
) {
    IF_LISTENED(EVENT_TYPES::onEffectAdded) {
        if (isServerThread() && isPlayer()) {
            if (!CallEvent(
                    EVENT_TYPES::onEffectAdded,
                    PlayerClass::newPlayer(reinterpret_cast<Player*>(this)),
                    String::newString(MobEffect::mMobEffects()[effect.mId]->mComponentName->getString()),
                    Number::newNumber(effect.mAmplifier),
                    Number::newNumber(effect.mDuration->mValue)
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onEffectAdded);
    origin(effect);
}

LL_TYPE_INSTANCE_HOOK(
    RemoveEffectHook,
    HookPriority::Normal,
    Actor,
    &Actor::$onEffectRemoved,
    void,
    ::MobEffectInstance& effect
) {
    IF_LISTENED(EVENT_TYPES::onEffectRemoved) {
        if (isServerThread() && isPlayer()) {
            if (!CallEvent(
                    EVENT_TYPES::onEffectRemoved,
                    PlayerClass::newPlayer(reinterpret_cast<Player*>(this)),
                    String::newString(MobEffect::mMobEffects()[effect.mId]->mComponentName->getString())
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onEffectRemoved);
    origin(effect);
}

LL_TYPE_INSTANCE_HOOK(
    BlockInteractedHook,
    HookPriority::Normal,
    Block,
    &Block::use,
    bool,
    Player&             player,
    BlockPos const&     pos,
    uchar               face,
    std::optional<Vec3> hit
) {
    if (!isServerThread() || (!mBlockType->isInteractiveBlock() && !mBlockType->isCraftingBlock())) {
        return origin(player, pos, face, hit); // 提前把不可交互方块过滤掉
    }
    IF_LISTENED(EVENT_TYPES::onBlockInteracted) {
        if (!CallEvent(
                EVENT_TYPES::onBlockInteracted,
                PlayerClass::newPlayer(&player),
                BlockClass::newBlock(pos, player.getDimensionId())
            )) {
            /**
             * 这里的返回值意为是否交互成功
             * 如果返回false，上层GameMode::useItemOn函数会认为方块没法交互或者交互失败，回退到物品使用，导致放置方块等行为
             * 如果返回true，则认为交互成功，并推送给sapi ItemUsedOnEvent 事件（会导致容器类方块界面被打开），然后返回
             */
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onBlockInteracted);
    return origin(player, pos, face, hit);
}

void StartDestroyBlock() { static ll::memory::HookRegistrar<StartDestroyBlockHook> reg; }
void DropItem() { static ll::memory::HookRegistrar<DropItemHook1, DropItemHook2> reg; }
void OpenContainerEvent() { static ll::memory::HookRegistrar<OpenContainerHook> reg; }
void CloseContainerEvent() { static ll::memory::HookRegistrar<CloseContainerHook1, CloseContainerHook2> reg; }
void ChangeSlotEvent() { static ll::memory::HookRegistrar<ChangeSlotHook> reg; }
void AttackBlockEvent() { static ll::memory::HookRegistrar<StartDestroyBlockHook> reg; }
void UseFrameEvent() { static ll::memory::HookRegistrar<UseFrameHook1, UseFrameHook2> reg; }
void EatEvent() { static ll::memory::HookRegistrar<EatHook> reg; }
void ChangeDimensionEvent() { static ll::memory::HookRegistrar<ChangeDimensionHook> reg; };
void OpenContainerScreenEvent() { static ll::memory::HookRegistrar<OpenContainerScreenHook> reg; }
void UseRespawnAnchorEvent() { static ll::memory::HookRegistrar<UseRespawnAnchorHook> reg; }
void SleepEvent() { static ll::memory::HookRegistrar<SleepHook> reg; }
void OpenInventoryEvent() { static ll::memory::HookRegistrar<OpenInventoryHook> reg; }
void PullFishingHookEvent() { static ll::memory::HookRegistrar<PullFishingHook> reg; }
void UseBucketPlaceEvent() { static ll::memory::HookRegistrar<UseBucketPlaceHook> reg; }
void UseBucketTakeEvent() { static ll::memory::HookRegistrar<UseBucketTakeHook> reg; }
void ConsumeTotemEvent() { static ll::memory::HookRegistrar<ConsumeTotemHook> reg; }
void SetArmorEvent() { static ll::memory::HookRegistrar<SetArmorHook> reg; }
void InteractEntityEvent() { static ll::memory::HookRegistrar<InteractEntityHook> reg; }
void AddEffectEvent() { static ll::memory::HookRegistrar<AddEffectHook> reg; }
void RemoveEffectEvent() { static ll::memory::HookRegistrar<RemoveEffectHook> reg; }
void BlockInteractedEvent() { static ll::memory::HookRegistrar<BlockInteractedHook> reg; }
} // namespace lse::events::player
