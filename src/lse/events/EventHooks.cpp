#include "EventHooks.h"

#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/service/Bedrock.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/server/ServerPlayer.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/world/ContainerID.h"
#include "mc/world/actor/ActorDamageCause.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/inventory/transaction/InventoryAction.h"
#include "mc/world/inventory/transaction/InventorySource.h"
#include "mc/world/inventory/transaction/InventoryTransaction.h"
#include "mc/world/phys/HitResult.h"
#include "mc/world/scores/IdentityDefinition.h"
#include "mc/world/scores/Objective.h"
#include "mc/world/scores/PlayerScoreboardId.h"
#include "mc/world/scores/ScoreInfo.h"
#include "mc/world/scores/ScoreboardId.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/memory/Memory.h"
#include "mc/deps/ecs/WeakEntityRef.h"
#include "mc/entity/components_json_legacy/ProjectileComponent.h"
#include "mc/server/module/VanillaServerGameplayEventListener.h"
#include "mc/world/actor/ActorDefinitionIdentifier.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/ArmorStand.h"
#include "mc/world/actor/FishingHook.h"
#include "mc/world/actor/Hopper.h"
#include "mc/world/actor/VanillaActorRendererId.h"
#include "mc/world/actor/boss/WitherBoss.h"
#include "mc/world/actor/item/ItemActor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/containers/models/LevelContainerModel.h"
#include "mc/world/events/BlockEventCoordinator.h"
#include "mc/world/events/EventResult.h"
#include "mc/world/gamemode/InteractionResult.h"
#include "mc/world/inventory/transaction/ComplexInventoryTransaction.h"
#include "mc/world/item/BucketItem.h"
#include "mc/world/item/CrossbowItem.h"
#include "mc/world/item/ItemInstance.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/TridentItem.h"
#include "mc/world/level/BedrockSpawner.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChangeDimensionRequest.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/BasePressurePlateBlock.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/ComparatorBlock.h"
#include "mc/world/level/block/DiodeBlock.h"
#include "mc/world/level/block/FarmBlock.h"
#include "mc/world/level/block/ItemFrameBlock.h"
#include "mc/world/level/block/LiquidBlockDynamic.h"
#include "mc/world/level/block/RedStoneWireBlock.h"
#include "mc/world/level/block/RedstoneTorchBlock.h"
#include "mc/world/level/block/RespawnAnchorBlock.h"
#include "mc/world/level/block/actor/BarrelBlockActor.h"
#include "mc/world/level/block/actor/BaseCommandBlock.h"
#include "mc/world/level/block/actor/ChestBlockActor.h"
#include "mc/world/level/block/actor/PistonBlockActor.h"
#include "mc/world/phys/AABB.h"
#include "mc/world/scores/ServerScoreboard.h"
#include "mc/world/level/Explosion.h"
#include "mc/world/events/PlayerOpenContainerEvent.h"
#include "mc/world/level/material/Material.h"
#include "mc/world/level/dimension/Dimension.h"
#include "ll/api/thread/ThreadName.h"

namespace lse::events {

// NOLINTBEGIN(modernize-use-trailing-return-type)
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)

LL_TYPE_INSTANCE_HOOK(
    PlayerStartDestroyHook,
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
    PlayerDropItemHook1,
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
    PlayerDropItemHook2,
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
    PlayerOpenContainerHook,
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
    PlayerCloseContainerHook1,
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
    PlayerCloseContainerHook2,
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
    PlayerChangeSlotHook,
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
    ContainerChangeHook,
    HookPriority::Normal,
    LevelContainerModel,
    &LevelContainerModel::$_onItemChanged,
    void,
    int              slotNumber,
    ItemStack const& oldItem,
    ItemStack const& newItem
) {
    IF_LISTENED(EVENT_TYPES::onContainerChange) {
        if (*reinterpret_cast<void***>(this) != LevelContainerModel::$vftable())
            return origin(slotNumber, oldItem, newItem);

        Player& player = mUnk84d147.as<Player&>();
        if (!CallEvent(
                EVENT_TYPES::onContainerChange,
                PlayerClass::newPlayer(&player),
                BlockClass::newBlock(mUnk74419a.as<BlockPos>(), player.getDimensionId()),
                Number::newNumber(slotNumber + this->_getContainerOffset()),
                ItemClass::newItem(&const_cast<ItemStack&>(oldItem)),
                ItemClass::newItem(&const_cast<ItemStack&>(newItem))
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onContainerChange);
    origin(slotNumber, oldItem, newItem);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerAttackBlockHook,
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
    ArmorStandSwapItemHook,
    HookPriority::Normal,
    ArmorStand,
    &ArmorStand::_trySwapItem,
    bool,
    Player&                              player,
    ::SharedTypes::Legacy::EquipmentSlot slot
) {
    IF_LISTENED(EVENT_TYPES::onChangeArmorStand) {
        if (!CallEvent(
                EVENT_TYPES::onChangeArmorStand,
                EntityClass::newEntity(this),
                PlayerClass::newPlayer(&player),
                Number::newNumber((int)slot)
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onChangeArmorStand);
    return origin(player, slot);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerUseFrameHook1,
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
    PlayerUseFrameHook2,
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

LL_TYPE_INSTANCE_HOOK(
    ProjectileSpawnHook1,
    HookPriority::Normal,
    BedrockSpawner,
    &BedrockSpawner::$spawnProjectile,
    Actor*,
    BlockSource&                     region,
    ActorDefinitionIdentifier const& id,
    Actor*                           spawner,
    Vec3 const&                      position,
    Vec3 const&                      direction
) {
    IF_LISTENED(EVENT_TYPES::onSpawnProjectile) {
        if (id._getLegacyActorType() != ActorType::Trident) {
            if (!CallEvent(
                    EVENT_TYPES::onSpawnProjectile,
                    EntityClass::newEntity(spawner),
                    String::newString(id.getCanonicalName())
                )) {
                return nullptr;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onSpawnProjectile);
    Actor* projectile = origin(region, id, spawner, position, direction);
    IF_LISTENED(EVENT_TYPES::onProjectileCreated) {
        CallEvent( // Not nancellable
            EVENT_TYPES::onProjectileCreated,
            EntityClass::newEntity(spawner),
            EntityClass::newEntity(projectile)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onProjectileCreated);
    return projectile;
}

LL_TYPE_INSTANCE_HOOK(
    ProjectileSpawnHook2,
    HookPriority::Normal,
    CrossbowItem,
    &CrossbowItem::_shootFirework,
    void,
    ItemInstance const& projectileInstance,
    Player&             player
) {
    IF_LISTENED(EVENT_TYPES::onSpawnProjectile) {
        if (!CallEvent(
                EVENT_TYPES::onSpawnProjectile,
                EntityClass::newEntity(&player),
                String::newString(projectileInstance.getTypeName())
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onSpawnProjectile);
    origin(projectileInstance, player);
}

LL_TYPE_INSTANCE_HOOK(
    ProjectileSpawnHook3,
    HookPriority::Normal,
    TridentItem,
    &TridentItem::$releaseUsing,
    void,
    ItemStack& item,
    Player*    player,
    int        durationLeft
) {
    IF_LISTENED(EVENT_TYPES::onSpawnProjectile) {
        if (!CallEvent(
                EVENT_TYPES::onSpawnProjectile,
                EntityClass::newEntity(player),
                String::newString(VanillaActorRendererId::trident().getString())
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onSpawnProjectile);
    origin(item, player, durationLeft);
}

LL_TYPE_INSTANCE_HOOK(
    PressurePlateTriggerHook,
    HookPriority::Normal,
    BasePressurePlateBlock,
    &BasePressurePlateBlock::$shouldTriggerEntityInside,
    bool,
    BlockSource&    region,
    BlockPos const& pos,
    Actor&          entity
) {
    IF_LISTENED(EVENT_TYPES::onStepOnPressurePlate) {
        if (!CallEvent(
                EVENT_TYPES::onStepOnPressurePlate,
                EntityClass::newEntity(&entity),
                BlockClass::newBlock(pos, region.getDimensionId())
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onStepOnPressurePlate);
    return origin(region, pos, entity);
}

LL_TYPE_INSTANCE_HOOK(ActorRideHook, HookPriority::Normal, Actor, &Actor::$canAddPassenger, bool, Actor& passenger) {
    IF_LISTENED(EVENT_TYPES::onRide) {
        if (!CallEvent(EVENT_TYPES::onRide, EntityClass::newEntity(&passenger), EntityClass::newEntity(this))) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onRide);
    return origin(passenger);
}

LL_TYPE_INSTANCE_HOOK(
    WitherDestroyHook,
    HookPriority::Normal,
    WitherBoss,
    &WitherBoss::_destroyBlocks,
    void,
    Level&                       level,
    AABB const&                  bb,
    BlockSource&                 region,
    int                          range,
    WitherBoss::WitherAttackType type
) {
    IF_LISTENED(EVENT_TYPES::onWitherBossDestroy) {
        if (!CallEvent(
                EVENT_TYPES::onWitherBossDestroy,
                EntityClass::newEntity(this),
                IntPos::newPos(bb.min, region.getDimensionId()),
                IntPos::newPos(bb.max, region.getDimensionId())
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onWitherBossDestroy);
    origin(level, bb, region, range, type);
}

LL_TYPE_INSTANCE_HOOK(
    FarmDecayHook,
    HookPriority::Normal,
    FarmBlock,
    &FarmBlock::$transformOnFall,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    Actor*          actor,
    float           fallDistance
) {
    IF_LISTENED(EVENT_TYPES::onFarmLandDecay) {
        CallEvent(
            EVENT_TYPES::onFarmLandDecay,
            IntPos::newPos(pos, region.getDimensionId()),
            EntityClass::newEntity(actor)
        );
    }
    IF_LISTENED_END(EVENT_TYPES::onFarmLandDecay);
    origin(region, pos, actor, fallDistance);
}

LL_TYPE_INSTANCE_HOOK(
    PistonPushHook,
    HookPriority::Normal,
    PistonBlockActor,
    &PistonBlockActor::_attachedBlockWalker,
    bool,
    BlockSource&    region,
    BlockPos const& curPos,
    uchar           curBranchFacing,
    uchar           pistonMoveFacing
) {
    IF_LISTENED(EVENT_TYPES::onPistonTryPush) {
        if (region.getBlock(curPos).isAir()) {
            return origin(region, curPos, curBranchFacing, pistonMoveFacing);
        }
        if (!CallEvent(
                EVENT_TYPES::onPistonTryPush,
                IntPos::newPos(this->getPosition(), region.getDimensionId()),
                BlockClass::newBlock(curPos, region.getDimensionId())
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onPistonTryPush);
    bool shouldPush = origin(region, curPos, curBranchFacing, pistonMoveFacing);
    IF_LISTENED(EVENT_TYPES::onPistonPush) {
        if (shouldPush) {
            CallEvent( // Not cancellable
                EVENT_TYPES::onPistonPush,
                IntPos::newPos(this->getPosition(), region.getDimensionId()),
                BlockClass::newBlock(curPos, region.getDimensionId())
            );
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onPistonPush);
    return shouldPush;
}

LL_TYPE_INSTANCE_HOOK(PlayerEatHook, HookPriority::Normal, Player, &Player::eat, void, ItemStack const& instance) {
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

LL_TYPE_INSTANCE_HOOK(ExplodeHook, HookPriority::Normal, Level, &Level::$explode, bool, ::Explosion& explosion) {
    IF_LISTENED(EVENT_TYPES::onEntityExplode) {
        if (explosion.mSourceID->rawID != ActorUniqueID::INVALID_ID().rawID) {
            if (!CallEvent(
                    EVENT_TYPES::onEntityExplode,
                    EntityClass::newEntity(ll::service::getLevel()->fetchEntity(explosion.mSourceID, false)),
                    FloatPos::newPos(explosion.mPos, explosion.mRegion.getDimensionId()),
                    Number::newNumber(explosion.mRadius),
                    Number::newNumber(explosion.mMaxResistance),
                    Boolean::newBoolean(explosion.mBreaking),
                    Boolean::newBoolean(explosion.mFire)
                )) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onEntityExplode);

    IF_LISTENED(EVENT_TYPES::onBlockExplode) {
        if (!CallEvent(
                EVENT_TYPES::onBlockExplode,
                BlockClass::newBlock(*explosion.mPos, explosion.mRegion.getDimensionId()),
                FloatPos::newPos(explosion.mPos, explosion.mRegion.getDimensionId()),
                Number::newNumber(explosion.mRadius),
                Number::newNumber(explosion.mMaxResistance),
                Boolean::newBoolean(explosion.mBreaking),
                Boolean::newBoolean(explosion.mFire)
            )) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onBlockExplode);
    return origin(explosion);
}

LL_TYPE_STATIC_HOOK(
    RespawnAnchorExplodeHook,
    HookPriority::Normal,
    RespawnAnchorBlock,
    &RespawnAnchorBlock::_explode,
    void,
    Player&         player,
    BlockPos const& pos,
    BlockSource&    region,
    Level&          level
) {
    IF_LISTENED(EVENT_TYPES::onRespawnAnchorExplode) {
        if (!CallEvent(
                EVENT_TYPES::onRespawnAnchorExplode,
                IntPos::newPos(pos, region.getDimensionId()),
                PlayerClass::newPlayer(&player)
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onRespawnAnchorExplode);
    origin(player, pos, region, level);
}

LL_TYPE_INSTANCE_HOOK(
    BlockExplodedHook,
    HookPriority::Normal,
    Block,
    &Block::onExploded,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    Actor*          entitySource
) {
    IF_LISTENED(EVENT_TYPES::onBlockExploded) {
        if (!CallEvent(
                EVENT_TYPES::onBlockExploded,
                BlockClass::newBlock(pos, region.getDimensionId()),
                EntityClass::newEntity(entitySource)
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onBlockExploded);
    origin(region, pos, entitySource);
}

inline bool RedstoneUpdateEvent(BlockSource& region, BlockPos const& pos, int& strength, bool& isFirstTime) {
    if (!CallEvent(
            EVENT_TYPES::onRedStoneUpdate,
            BlockClass::newBlock(pos, region.getDimensionId()),
            Number::newNumber(strength),
            Boolean::newBoolean(isFirstTime)
        )) {
        return false;
    }
    return true;
}

LL_TYPE_INSTANCE_HOOK(
    RedstoneUpdateHook1,
    HookPriority::Normal,
    RedStoneWireBlock,
    &RedStoneWireBlock::$onRedstoneUpdate,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    int             strength,
    bool            isFirstTime
) {
    IF_LISTENED(EVENT_TYPES::onRedStoneUpdate) {
        if (!RedstoneUpdateEvent(region, pos, strength, isFirstTime)) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onRedStoneUpdate);
    origin(region, pos, strength, isFirstTime);
}

LL_TYPE_INSTANCE_HOOK(
    RedstoneUpdateHook2,
    HookPriority::Normal,
    DiodeBlock,
    &DiodeBlock::$onRedstoneUpdate,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    int             strength,
    bool            isFirstTime
) {
    IF_LISTENED(EVENT_TYPES::onRedStoneUpdate) {
        if (!RedstoneUpdateEvent(region, pos, strength, isFirstTime)) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onRedStoneUpdate);
    origin(region, pos, strength, isFirstTime);
}

LL_TYPE_INSTANCE_HOOK(
    RedstoneUpdateHook3,
    HookPriority::Normal,
    RedstoneTorchBlock,
    &RedstoneTorchBlock::$onRedstoneUpdate,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    int             strength,
    bool            isFirstTime
) {
    IF_LISTENED(EVENT_TYPES::onRedStoneUpdate) {
        if (!RedstoneUpdateEvent(region, pos, strength, isFirstTime)) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onRedStoneUpdate);
    origin(region, pos, strength, isFirstTime);
}

LL_TYPE_INSTANCE_HOOK(
    RedstoneUpdateHook4,
    HookPriority::Normal,
    ComparatorBlock,
    &ComparatorBlock::$onRedstoneUpdate,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    int             strength,
    bool            isFirstTime
) {
    IF_LISTENED(EVENT_TYPES::onRedStoneUpdate) {
        if (!RedstoneUpdateEvent(region, pos, strength, isFirstTime)) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onRedStoneUpdate);
    origin(region, pos, strength, isFirstTime);
}

LL_TYPE_INSTANCE_HOOK(
    LiquidFlowHook,
    HookPriority::Normal,
    LiquidBlockDynamic,
    &LiquidBlockDynamic::_trySpreadTo,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    int               neighbor,
    ::BlockPos const& flowFromPos,
    uchar             flowFromDirection
) {
    IF_LISTENED(EVENT_TYPES::onLiquidFlow) {
        if (!CallEvent(
                EVENT_TYPES::onLiquidFlow,
                region.isInstaticking(pos) ? Local<Value>() : BlockClass::newBlock(pos, region.getDimensionId()),
                IntPos::newPos(pos, region.getDimensionId())
            )) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onLiquidFlow);
    return origin(region, pos, neighbor, flowFromPos, flowFromDirection);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerChangeDimensionHook,
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

LL_TYPE_INSTANCE_HOOK(
    PlayerOpenContainerScreenHook,
    HookPriority::Normal,
    Player,
    &Player::canOpenContainerScreen,
    bool
) {
    IF_LISTENED(EVENT_TYPES::onOpenContainerScreen) {
        if (!CallEvent(EVENT_TYPES::onOpenContainerScreen, PlayerClass::newPlayer(this))) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onOpenContainerScreen);
    return origin();
}

LL_TYPE_INSTANCE_HOOK(
    CommandBlockExecuteHook,
    HookPriority::Normal,
    BaseCommandBlock,
    &BaseCommandBlock::_performCommand,
    bool,
    BlockSource&         region,
    CommandOrigin const& commandOrigin,
    bool&                markForSaving
) {
    IF_LISTENED(EVENT_TYPES::onCmdBlockExecute) {
        if (commandOrigin.getOriginType() == CommandOriginType::MinecartCommandBlock) {
            if (!CallEvent(
                    EVENT_TYPES::onCmdBlockExecute,
                    String::newString(this->getCommand()),
                    FloatPos::newPos(commandOrigin.getEntity()->getPosition(), region.getDimensionId()),
                    Boolean::newBoolean(true)
                )) {
                return false;
            }
        } else {
            if (!CallEvent(
                    EVENT_TYPES::onCmdBlockExecute,
                    String::newString(this->getCommand()),
                    FloatPos::newPos(commandOrigin.getBlockPosition(), region.getDimensionId()),
                    Boolean::newBoolean(false)
                )) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onCmdBlockExecute);
    return origin(region, commandOrigin, markForSaving);
}

LL_TYPE_STATIC_HOOK(
    PlayerUseRespawnAnchorHook,
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
    PlayerSleepHook,
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
LL_TYPE_INSTANCE_HOOK(
    PlayerOpenInventoryHook,
    HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::$openInventory,
    void,
) {
    IF_LISTENED(EVENT_TYPES::onOpenInventory) {
        if (!CallEvent(EVENT_TYPES::onOpenInventory, PlayerClass::newPlayer(this))) {
            return;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onOpenInventory);
    origin();
}

LL_TYPE_INSTANCE_HOOK(
    PlayerPullFishingHook,
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
    ScoreChangedHook,
    HookPriority::Normal,
    ServerScoreboard,
    &ServerScoreboard::$onScoreChanged,
    void,
    ScoreboardId const& id,
    Objective const&    obj
) {
    IF_LISTENED(EVENT_TYPES::onScoreChanged) {
        if (id.getIdentityDef().isPlayerType()) {
            if (!CallEvent(
                    EVENT_TYPES::onScoreChanged,
                    PlayerClass::newPlayer(ll::service::getLevel()->getPlayer(
                        ActorUniqueID(id.getIdentityDef().getPlayerId().mActorUniqueId)
                    )),
                    Number::newNumber(obj.getPlayerScore(id).mValue),
                    String::newString(obj.getName()),
                    String::newString(obj.getDisplayName())
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onScoreChanged);
    origin(id, obj);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerUseBucketPlaceHook,
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
    PlayerUseBucketTakeHook1,
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
    PlayerUseBucketTakeHook2,
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
//     PlayerUseBucketTakeHook3,
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

LL_TYPE_INSTANCE_HOOK(PlayerConsumeTotemHook, HookPriority::Normal, Player, &Player::$consumeTotem, bool) {
    IF_LISTENED(EVENT_TYPES::onConsumeTotem) {
        if (!CallEvent(EVENT_TYPES::onConsumeTotem, PlayerClass::newPlayer(this))) {
            return false;
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onConsumeTotem);
    return origin();
}

LL_TYPE_INSTANCE_HOOK(
    PlayerSetArmorHook,
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
    ProjectileHitEntityHook,
    HookPriority::Normal,
    ProjectileComponent,
    &ProjectileComponent::onHit,
    void,
    Actor&           owner,
    HitResult const& res
) {
    IF_LISTENED(EVENT_TYPES::onProjectileHitEntity) {
        if (res.getEntity()) {
            if (!CallEvent(
                    EVENT_TYPES::onProjectileHitEntity,
                    EntityClass::newEntity(res.getEntity()),
                    EntityClass::newEntity(&owner)
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onProjectileHitEntity);
    origin(owner, res);
}

LL_TYPE_INSTANCE_HOOK(
    ProjectileHitBlockHook,
    HookPriority::Normal,
    Block,
    &Block::onProjectileHit,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    Actor const&    projectile
) {
    IF_LISTENED(EVENT_TYPES::onProjectileHitBlock) {
        if (pos != BlockPos::ZERO() && !this->isAir()) {
            if (!CallEvent(
                    EVENT_TYPES::onProjectileHitBlock,
                    BlockClass::newBlock(*this, pos, region),
                    EntityClass::newEntity(&const_cast<Actor&>(projectile))
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onProjectileHitBlock);
    origin(region, pos, projectile);
}

namespace HopperEvents {
enum class HopperStatus { None, PullIn, PullOut } hopperStatus = HopperStatus::None;
Vec3 hopperPos;

LL_TYPE_INSTANCE_HOOK(
    HopperPullInHook,
    HookPriority::Normal,
    Hopper,
    &Hopper::_tryPullInItemsFromAboveContainer,
    bool,
    BlockSource& region,
    Container&   toContainer,
    Vec3 const&  pos
) {
    hopperStatus = HopperStatus::PullIn;
    hopperPos    = pos;
    return origin(region, toContainer, pos);
}

LL_TYPE_INSTANCE_HOOK(
    HopperPushOutHook,
    HookPriority::Normal,
    Hopper,
    &Hopper::_pushOutItems,
    bool,
    BlockSource& region,
    Container&   fromContainer,
    Vec3 const&  position,
    int          attachedFace
) {
    hopperStatus = HopperStatus::PullOut;
    hopperPos    = position;
    return origin(region, fromContainer, position, attachedFace);
}

LL_TYPE_INSTANCE_HOOK(
    HopperAddItemHook,
    HookPriority::Normal,
    Hopper,
    &Hopper::_tryMoveInItem,
    bool,
    ::BlockSource& region,
    ::Container&   container,
    ::ItemStack&   item,
    int            slot,
    int            face,
    int            itemCount
) {
    IF_LISTENED(EVENT_TYPES::onHopperSearchItem) {
        if (hopperStatus == HopperStatus::PullIn) {
            if (!CallEvent(
                    EVENT_TYPES::onHopperSearchItem,
                    FloatPos::newPos(hopperPos, region.getDimensionId()),
                    Boolean::newBoolean(this->mIsEntity),
                    ItemClass::newItem(&item)
                )) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onHopperSearchItem);
    IF_LISTENED(EVENT_TYPES::onHopperPushOut) {
        if (hopperStatus == HopperStatus::PullOut) {
            if (!CallEvent(
                    EVENT_TYPES::onHopperPushOut,
                    FloatPos::newPos(hopperPos, region.getDimensionId()),
                    Boolean::newBoolean(this->mIsEntity),
                    ItemClass::newItem(&item)
                )) {
                return false;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onHopperPushOut);
    hopperStatus = HopperStatus::None;
    return origin(region, container, item, slot, face, itemCount);
}
} // namespace HopperEvents

LL_TYPE_INSTANCE_HOOK(
    MobHurtEffectHook,
    HookPriority::Normal,
    Mob,
    &Mob::getDamageAfterResistanceEffect,
    float,
    ActorDamageSource const& source,
    float                    damage
) {
    IF_LISTENED(EVENT_TYPES::onMobHurt) {
        if (source.getCause() == ActorDamageCause::Magic || source.getCause() == ActorDamageCause::Wither) {
            Actor* damageSource = nullptr;
            if (source.isEntitySource()) {
                if (source.isChildEntitySource()) {
                    damageSource = ll::service::getLevel()->fetchEntity(source.getEntityUniqueID(), false);
                } else {
                    damageSource = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
                }
            }

            if (!CallEvent(
                    EVENT_TYPES::onMobHurt,
                    EntityClass::newEntity(this),
                    damageSource ? EntityClass::newEntity(damageSource) : Local<Value>(),
                    Number::newNumber(damage < 0.0f ? -damage : damage),
                    Number::newNumber((int)source.getCause())
                )) {
                return 0.0f;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onMobHurt)
    return origin(source, damage);
}

void PlayerStartDestroyBlock() { PlayerStartDestroyHook::hook(); }
void PlayerDropItem() {
    PlayerDropItemHook1::hook();
    PlayerDropItemHook2::hook();
}
void PlayerOpenContainerEvent() { PlayerOpenContainerHook::hook(); }
void PlayerCloseContainerEvent() {
    PlayerCloseContainerHook1::hook();
    PlayerCloseContainerHook2::hook();
}
void PlayerChangeSlotEvent() { PlayerChangeSlotHook::hook(); }
void ContainerChangeEvent() { ContainerChangeHook::hook(); }
void PlayerAttackBlockEvent() { PlayerAttackBlockHook::hook(); }
void ArmorStandSwapItemEvent() { ArmorStandSwapItemHook::hook(); }
void PlayerUseFrameEvent() {
    PlayerUseFrameHook1::hook();
    PlayerUseFrameHook2::hook();
}
void ProjectileSpawnEvent() {
    ProjectileSpawnHook1::hook();
    ProjectileSpawnHook2::hook();
    ProjectileSpawnHook3::hook();
};
void ProjectileCreatedEvent() { ProjectileSpawnHook1::hook(); };
void PressurePlateTriggerEvent() { PressurePlateTriggerHook::hook(); }
void ActorRideEvent() { ActorRideHook::hook(); }
void WitherDestroyEvent() { WitherDestroyHook::hook(); }
void FarmDecayEvent() { FarmDecayHook::hook(); }
void PistonPushEvent() { PistonPushHook::hook(); }
void PlayerEatEvent() { PlayerEatHook::hook(); }
void ExplodeEvent() { ExplodeHook::hook(); }
void RespawnAnchorExplodeEvent() { RespawnAnchorExplodeHook::hook(); }
void BlockExplodedEvent() { BlockExplodedHook ::hook(); }
void RedstoneupdateEvent() {
    RedstoneUpdateHook1::hook();
    RedstoneUpdateHook2::hook();
    RedstoneUpdateHook3::hook();
    RedstoneUpdateHook4::hook();
}
void LiquidFlowEvent() { LiquidFlowHook::hook(); }
void PlayerChangeDimensionEvent() { PlayerChangeDimensionHook::hook(); };
void PlayerOpenContainerScreenEvent() { PlayerOpenContainerScreenHook::hook(); }
void CommandBlockExecuteEvent() { CommandBlockExecuteHook::hook(); }
void PlayerUseRespawnAnchorEvent() { PlayerUseRespawnAnchorHook::hook(); }
void PlayerSleepEvent() { PlayerSleepHook::hook(); }
void PlayerOpenInventoryEvent() { PlayerOpenInventoryHook::hook(); }
void PlayerPullFishingHookEvent() { PlayerPullFishingHook::hook(); }
void ScoreChangedEvent() { ScoreChangedHook::hook(); }
void PlayerUseBucketPlaceEvent() { PlayerUseBucketPlaceHook::hook(); }
void PlayerUseBucketTakeEvent() {
    PlayerUseBucketTakeHook1::hook();
    PlayerUseBucketTakeHook2::hook();
}
void PlayerConsumeTotemEvent() { PlayerConsumeTotemHook::hook(); }
void PlayerSetArmorEvent() { PlayerSetArmorHook::hook(); }
void ProjectileHitEntityEvent() { ProjectileHitEntityHook::hook(); }
void ProjectileHitBlockEvent() { ProjectileHitBlockHook::hook(); }
void HopperEvent(bool pullIn) {
    HopperEvents::HopperAddItemHook::hook();
    if (pullIn) {
        HopperEvents::HopperPullInHook::hook();
    } else {
        HopperEvents::HopperPushOutHook::hook();
    }
}
void MobHurtEvent() { MobHurtEffectHook::hook(); }

// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
// NOLINTEND(modernize-use-trailing-return-type)

} // namespace lse::events
