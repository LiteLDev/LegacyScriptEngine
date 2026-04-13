#include "legacy/api/PlayerAPI.h"

#include "legacy/api/APIHelp.h"
#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/ContainerAPI.h"
#include "legacy/api/DataAPI.h"
#include "legacy/api/DeviceAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/GuiAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/McAPI.h"
#include "legacy/api/NbtAPI.h"
#include "legacy/api/PacketAPI.h"
#include "legacy/engine/EngineManager.h"
#include "legacy/engine/EngineOwnData.h"
#include "legacy/engine/GlobalShareData.h"
#include "legacy/main/EconomicSystem.h"
#include "legacy/main/SafeGuardRecord.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerChatEvent.h"
#include "ll/api/form/ModalForm.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/service/PlayerInfo.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "lse/api/MoreGlobal.h"
#include "lse/api/NetworkPacket.h"
#include "lse/api/helper/AttributeHelper.h"
#include "lse/api/helper/PlayerHelper.h"
#include "lse/api/helper/ScoreboardHelper.h"
#include "mc/deps/core/math/Vec2.h"
#include "mc/deps/core/utility/MCRESULT.h"
#include "mc/deps/nbt/CompoundTag.h"
#include "mc/deps/nbt/ListTag.h"
#include "mc/deps/nbt/StringTag.h"
#include "mc/entity/components/ActorRotationComponent.h"
#include "mc/entity/components/AttributesComponent.h"
#include "mc/entity/components/InsideBlockComponent.h"
#include "mc/entity/components/IsOnHotBlockFlagComponent.h"
#include "mc/entity/components/TagsComponent.h"
#include "mc/entity/components/WasInWaterFlagComponent.h"
#include "mc/entity/utilities/ActorMobilityUtils.h"
#include "mc/legacy/ActorRuntimeID.h"
#include "mc/legacy/ActorUniqueID.h"
#include "mc/network/ConnectionRequest.h"
#include "mc/network/MinecraftPacketIds.h"
#include "mc/network/MinecraftPackets.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/packet/BossEventPacket.h"
#include "mc/network/packet/ClientboundCloseFormPacket.h"
#include "mc/network/packet/LevelChunkPacket.h"
#include "mc/network/packet/RemoveObjectivePacket.h"
#include "mc/network/packet/ScorePacketInfo.h"
#include "mc/network/packet/SetDisplayObjectivePacket.h"
#include "mc/network/packet/SetScorePacket.h"
#include "mc/network/packet/SetTitlePacket.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/network/packet/TextPacketType.h"
#include "mc/network/packet/ToastRequestPacket.h"
#include "mc/network/packet/TransferPacket.h"
#include "mc/network/packet/UpdateAbilitiesPacket.h"
#include "mc/network/packet/UpdateAdventureSettingsPacket.h"
#include "mc/platform/UUID.h"
#include "mc/server/NetworkChunkPublisher.h"
#include "mc/server/ServerPlayer.h"
#include "mc/server/commands/CommandContext.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/PlayerCommandOrigin.h"
#include "mc/util/BlockUtils.h"
#include "mc/util/LootTableUtils.h"
#include "mc/world/Container.h"
#include "mc/world/Minecraft.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorDamageByActorSource.h"
#include "mc/world/actor/ActorHurtResult.h"
#include "mc/world/actor/ai/util/BossBarColor.h"
#include "mc/world/actor/ai/util/BossEventUpdateType.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/LayeredAbilities.h"
#include "mc/world/actor/player/PermissionsHandler.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/PlayerInventory.h"
#include "mc/world/actor/provider/ActorEquipment.h"
#include "mc/world/actor/provider/SynchedActorDataAccess.h"
#include "mc/world/attribute/Attribute.h"
#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/AttributeInstanceConstRef.h"
#include "mc/world/attribute/AttributeInstanceHandle.h" // IWYU pragma: keep
#include "mc/world/attribute/AttributeInstanceRef.h"
#include "mc/world/attribute/SharedAttributes.h"
#include "mc/world/effect/EffectDuration.h"
#include "mc/world/effect/MobEffectInstance.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/biome/Biome.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/VanillaBlockTypeIds.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/material/Material.h"
#include "mc/world/level/storage/DBStorage.h"
#include "mc/world/level/storage/db_helpers/Category.h"
#include "mc/world/phys/HitResult.h"
#include "mc/world/scores/IdentityDefinition.h"
#include "mc/world/scores/Objective.h"
#include "mc/world/scores/PlayerScoreSetFunction.h"
#include "mc/world/scores/PlayerScoreboardId.h"
#include "mc/world/scores/ScoreInfo.h"
#include "mc/world/scores/Scoreboard.h"
#include "mc/world/scores/ScoreboardId.h"
#include "mc/world/scores/ScoreboardOperationResult.h"

SetScorePacket::SetScorePacket() { mType = ScorePacketType::Change; }
ToastRequestPacket::ToastRequestPacket() { mSerializationMode = SerializationMode::ManualOnly; }
ToastRequestPacketPayload::ToastRequestPacketPayload() = default;
SetDisplayObjectivePacket::SetDisplayObjectivePacket() { mSerializationMode = SerializationMode::ManualOnly; }
SetDisplayObjectivePacketPayload::SetDisplayObjectivePacketPayload() { mSortOrder = ObjectiveSortOrder::Ascending; }
RemoveObjectivePacketPayload::RemoveObjectivePacketPayload() = default;
RemoveObjectivePacket::RemoveObjectivePacket() { mSerializationMode = SerializationMode::ManualOnly; }

//////////////////// Class Definition ////////////////////

ClassDefine<PlayerClass> PlayerClassBuilder =
    defineClass<PlayerClass>("LLSE_Player")
        .constructor(nullptr)
        .instanceProperty("name", &PlayerClass::getName)
        .instanceProperty("pos", &PlayerClass::getPos)
        .instanceProperty("feetPos", &PlayerClass::getFeetPos)
        .instanceProperty("blockPos", &PlayerClass::getBlockPos)
        .instanceProperty("lastDeathPos", &PlayerClass::getLastDeathPos)
        .instanceProperty("realName", &PlayerClass::getRealName)
        .instanceProperty("xuid", &PlayerClass::getXuid)
        .instanceProperty("uuid", &PlayerClass::getUuid)
        .instanceProperty("permLevel", &PlayerClass::getPermLevel)
        .instanceProperty("gameMode", &PlayerClass::getGameMode)
        .instanceProperty("canSleep", &PlayerClass::getCanSleep)
        .instanceProperty("canFly", &PlayerClass::getCanFly)
        .instanceProperty("canBeSeenOnMap", &PlayerClass::getCanBeSeenOnMap)
        .instanceProperty("canFreeze", &PlayerClass::getCanFreeze)
        .instanceProperty("canSeeDaylight", &PlayerClass::getCanSeeDaylight)
        .instanceProperty("canShowNameTag", &PlayerClass::getCanShowNameTag)
        .instanceProperty("canStartSleepInBed", &PlayerClass::getCanStartSleepInBed)
        .instanceProperty("canPickupItems", &PlayerClass::getCanPickupItems)
        .instanceProperty("maxHealth", &PlayerClass::getMaxHealth)
        .instanceProperty("health", &PlayerClass::getHealth)
        .instanceProperty("inAir", &PlayerClass::getInAir)
        .instanceProperty("inWater", &PlayerClass::getInWater)
        .instanceProperty("inLava", &PlayerClass::getInLava)
        .instanceProperty("inRain", &PlayerClass::getInRain)
        .instanceProperty("inSnow", &PlayerClass::getInSnow)
        .instanceProperty("inWall", &PlayerClass::getInWall)
        .instanceProperty("inWaterOrRain", &PlayerClass::getInWaterOrRain)
        .instanceProperty("inWorld", &PlayerClass::getInWorld)
        .instanceProperty("inClouds", &PlayerClass::getInClouds)
        .instanceProperty("speed", &PlayerClass::getSpeed)
        .instanceProperty("direction", &PlayerClass::getDirection)
        .instanceProperty("uniqueId", &PlayerClass::getUniqueID)
        .instanceProperty("runtimeId", &PlayerClass::getRuntimeID)
        .instanceProperty("langCode", &PlayerClass::getLangCode)
        .instanceProperty("isLoading", &PlayerClass::isLoading)
        .instanceProperty("isInvisible", &PlayerClass::isInvisible)
        .instanceProperty("isInsidePortal", &PlayerClass::isInsidePortal)
        .instanceProperty("isHurt", &PlayerClass::isHurt)
        .instanceProperty("isTrusting", &PlayerClass::isTrusting)
        .instanceProperty("isTouchingDamageBlock", &PlayerClass::isTouchingDamageBlock)
        .instanceProperty("isHungry", &PlayerClass::isHungry)
        .instanceProperty("isOnFire", &PlayerClass::isOnFire)
        .instanceProperty("isOnGround", &PlayerClass::isOnGround)
        .instanceProperty("isOnHotBlock", &PlayerClass::isOnHotBlock)
        .instanceProperty("isTrading", &PlayerClass::isTrading)
        .instanceProperty("isAdventure", &PlayerClass::isAdventure)
        .instanceProperty("isGliding", &PlayerClass::isGliding)
        .instanceProperty("isSurvival", &PlayerClass::isSurvival)
        .instanceProperty("isSpectator", &PlayerClass::isSpectator)
        .instanceProperty("isRiding", &PlayerClass::isRiding)
        .instanceProperty("isDancing", &PlayerClass::isDancing)
        .instanceProperty("isCreative", &PlayerClass::isCreative)
        .instanceProperty("isFlying", &PlayerClass::isFlying)
        .instanceProperty("isSleeping", &PlayerClass::isSleeping)
        .instanceProperty("isMoving", &PlayerClass::isMoving)
        .instanceProperty("isSneaking", &PlayerClass::isSneaking)

        .instanceFunction("isOP", &PlayerClass::isOP)
        .instanceFunction("setPermLevel", &PlayerClass::setPermLevel)
        .instanceFunction("setGameMode", &PlayerClass::setGameMode)

        .instanceFunction("runcmd", &PlayerClass::runcmd)
        .instanceFunction("teleport", &PlayerClass::teleport)
        .instanceFunction("kill", &PlayerClass::kill)
        .instanceFunction("kick", &PlayerClass::kick)
        .instanceFunction("disconnect", &PlayerClass::kick)
        .instanceFunction("tell", &PlayerClass::tell)
        .instanceFunction("talkAs", &PlayerClass::talkAs)
        .instanceFunction("talkTo", &PlayerClass::talkTo)
        .instanceFunction("sendText", &PlayerClass::tell)
        .instanceFunction("setTitle", &PlayerClass::setTitle)
        .instanceFunction("rename", &PlayerClass::rename)
        .instanceFunction("setFire", &PlayerClass::setFire)
        .instanceFunction("stopFire", &PlayerClass::stopFire)
        .instanceFunction("transServer", &PlayerClass::transServer)
        .instanceFunction("crash", &PlayerClass::crash)
        .instanceFunction("hurt", &PlayerClass::hurt)
        .instanceFunction("heal", &PlayerClass::heal)
        .instanceFunction("setHealth", &PlayerClass::setHealth)
        .instanceFunction("setMaxHealth", &PlayerClass::setMaxHealth)
        .instanceFunction("setAbsorption", &PlayerClass::setAbsorption)
        .instanceFunction("setAttackDamage", &PlayerClass::setAttackDamage)
        .instanceFunction("setMaxAttackDamage", &PlayerClass::setMaxAttackDamage)
        .instanceFunction("setFollowRange", &PlayerClass::setFollowRange)
        .instanceFunction("setKnockbackResistance", &PlayerClass::setKnockbackResistance)
        .instanceFunction("setLuck", &PlayerClass::setLuck)
        .instanceFunction("setMovementSpeed", &PlayerClass::setMovementSpeed)
        .instanceFunction("setUnderwaterMovementSpeed", &PlayerClass::setUnderwaterMovementSpeed)
        .instanceFunction("setLavaMovementSpeed", &PlayerClass::setLavaMovementSpeed)
        .instanceFunction("setHungry", &PlayerClass::setHungry)
        .instanceFunction("refreshChunks", &PlayerClass::refreshChunks)
        .instanceFunction("giveItem", &PlayerClass::giveItem)
        .instanceFunction("clearItem", &PlayerClass::clearItem)
        .instanceFunction("isSprinting", &PlayerClass::isSprinting)
        .instanceFunction("setSprinting", &PlayerClass::setSprinting)
        .instanceFunction("sendToast", &PlayerClass::sendToast)
        .instanceFunction("distanceTo", &PlayerClass::distanceTo)
        .instanceFunction("distanceToSqr", &PlayerClass::distanceToSqr)

        .instanceFunction("getBlockStandingOn", &PlayerClass::getBlockStandingOn)
        .instanceFunction("getDevice", &PlayerClass::getDevice)
        .instanceFunction("getHand", &PlayerClass::getHand)
        .instanceFunction("getOffHand", &PlayerClass::getOffHand)
        .instanceFunction("getInventory", &PlayerClass::getInventory)
        .instanceFunction("getArmor", &PlayerClass::getArmor)
        .instanceFunction("getEnderChest", &PlayerClass::getEnderChest)
        .instanceFunction("getRespawnPosition", &PlayerClass::getRespawnPosition)
        .instanceFunction("setRespawnPosition", &PlayerClass::setRespawnPosition)
        .instanceFunction("refreshItems", &PlayerClass::refreshItems)

        .instanceFunction("getScore", &PlayerClass::getScore)
        .instanceFunction("setScore", &PlayerClass::setScore)
        .instanceFunction("addScore", &PlayerClass::addScore)
        .instanceFunction("reduceScore", &PlayerClass::reduceScore)
        .instanceFunction("deleteScore", &PlayerClass::deleteScore)
        .instanceFunction("setSidebar", &PlayerClass::setSidebar)
        .instanceFunction("removeSidebar", &PlayerClass::removeSidebar)
        .instanceFunction("setBossBar", &PlayerClass::setBossBar)
        .instanceFunction("removeBossBar", &PlayerClass::removeBossBar)
        .instanceFunction("addLevel", &PlayerClass::addLevel)
        .instanceFunction("reduceLevel", &PlayerClass::reduceLevel)
        .instanceFunction("getLevel", &PlayerClass::getLevel)
        .instanceFunction("setLevel", &PlayerClass::setLevel)
        .instanceFunction("setScale", &PlayerClass::setScale)
        .instanceFunction("resetLevel", &PlayerClass::resetLevel)
        .instanceFunction("addExperience", &PlayerClass::addExperience)
        .instanceFunction("reduceExperience", &PlayerClass::reduceExperience)
        .instanceFunction("getCurrentExperience", &PlayerClass::getCurrentExperience)
        .instanceFunction("setCurrentExperience", &PlayerClass::setCurrentExperience)
        .instanceFunction("getTotalExperience", &PlayerClass::getTotalExperience)
        .instanceFunction("setTotalExperience", &PlayerClass::setTotalExperience)
        .instanceFunction("getXpNeededForNextLevel", &PlayerClass::getXpNeededForNextLevel)
        .instanceFunction("setAbility", &PlayerClass::setAbility)
        .instanceFunction("getBiomeId", &PlayerClass::getBiomeId)
        .instanceFunction("getBiomeName", &PlayerClass::getBiomeName)

        .instanceFunction("getAllEffects", &PlayerClass::getAllEffects)
        .instanceFunction("addEffect", &PlayerClass::addEffect)
        .instanceFunction("removeEffect", &PlayerClass::removeEffect)

        .instanceFunction("sendSimpleForm", &PlayerClass::sendSimpleForm)
        .instanceFunction("sendModalForm", &PlayerClass::sendModalForm)
        .instanceFunction("sendCustomForm", &PlayerClass::sendCustomForm)
        .instanceFunction("sendForm", &PlayerClass::sendForm)
        .instanceFunction("closeForm", &PlayerClass::closeForm)
        .instanceFunction("sendPacket", &PlayerClass::sendPacket)

        .instanceFunction("setExtraData", &PlayerClass::setExtraData)
        .instanceFunction("getExtraData", &PlayerClass::getExtraData)
        .instanceFunction("delExtraData", &PlayerClass::delExtraData)

        .instanceFunction("setNbt", &PlayerClass::setNbt)
        .instanceFunction("getNbt", &PlayerClass::getNbt)
        .instanceFunction("addTag", &PlayerClass::addTag)
        .instanceFunction("removeTag", &PlayerClass::removeTag)
        .instanceFunction("hasTag", &PlayerClass::hasTag)
        .instanceFunction("getAllTags", &PlayerClass::getAllTags)
        .instanceFunction("getAbilities", &PlayerClass::getAbilities)
        .instanceFunction("getAttributes", &PlayerClass::getAttributes)
        .instanceFunction("getEntityFromViewVector", &PlayerClass::getEntityFromViewVector)
        .instanceFunction("getBlockFromViewVector", &PlayerClass::getBlockFromViewVector)
        .instanceFunction("quickEvalMolangScript", &PlayerClass::quickEvalMolangScript)

        // LLMoney
        .instanceFunction("getMoney", &PlayerClass::getMoney)
        .instanceFunction("setMoney", &PlayerClass::setMoney)
        .instanceFunction("addMoney", &PlayerClass::addMoney)
        .instanceFunction("reduceMoney", &PlayerClass::reduceMoney)
        .instanceFunction("transMoney", &PlayerClass::transMoney)
        .instanceFunction("getMoneyHistory", &PlayerClass::getMoneyHistory)

        // SimulatedPlayer API
        .instanceFunction("isSimulatedPlayer", &PlayerClass::isSimulatedPlayer)
        .instanceFunction("simulateSneak", &PlayerClass::simulateSneak)
        .instanceFunction("simulateAttack", &PlayerClass::simulateAttack)
        .instanceFunction("simulateDestroy", &PlayerClass::simulateDestroy)
        .instanceFunction("simulateDisconnect", &PlayerClass::simulateDisconnect)
        .instanceFunction("simulateInteract", &PlayerClass::simulateInteract)
        .instanceFunction("simulateRespawn", &PlayerClass::simulateRespawn)
        .instanceFunction("simulateJump", &PlayerClass::simulateJump)
        .instanceFunction("simulateLocalMove", &PlayerClass::simulateLocalMove)
        .instanceFunction("simulateWorldMove", &PlayerClass::simulateWorldMove)
        .instanceFunction("simulateMoveTo", &PlayerClass::simulateMoveTo)
        .instanceFunction("simulateLookAt", &PlayerClass::simulateLookAt)
        .instanceFunction("simulateSetBodyRotation", &PlayerClass::simulateSetBodyRotation)
        .instanceFunction("simulateNavigateTo", &PlayerClass::simulateNavigateTo)
        .instanceFunction("simulateUseItem", &PlayerClass::simulateUseItem)
        .instanceFunction("simulateStopDestroyingBlock", &PlayerClass::simulateStopDestroyingBlock)
        .instanceFunction("simulateStopInteracting", &PlayerClass::simulateStopInteracting)
        .instanceFunction("simulateStopMoving", &PlayerClass::simulateStopMoving)
        .instanceFunction("simulateStopUsingItem", &PlayerClass::simulateStopUsingItem)
        .instanceFunction("simulateStopSneaking", &PlayerClass::simulateStopSneaking)

        // For Compatibility
        .instanceProperty("sneaking", &PlayerClass::isSneaking)
        .instanceProperty("ip", &PlayerClass::getIP)
        .instanceFunction("setTag", &PlayerClass::setNbt)
        .instanceFunction("getTag", &PlayerClass::getNbt)
        .instanceFunction("setOnFire", &PlayerClass::setOnFire)
        .instanceFunction("removeItem", &PlayerClass::removeItem)
        .instanceFunction("getAllItems", &PlayerClass::getAllItems)
        .instanceFunction("removeScore", &PlayerClass::deleteScore)
        .instanceFunction("distanceToPos", &PlayerClass::distanceTo)

        .instanceFunction("toEntity", &PlayerClass::toEntity)
        .build();

//////////////////// Classes ////////////////////

// 生成函数
PlayerClass::PlayerClass(Player const* player) : ScriptClass(ScriptClass::ConstructFromCpp<PlayerClass>{}) {
    try {
        if (player) {
            mWeakEntity = player->getWeakEntity();
            mValid      = true;
        }
    } catch (...) {}
}

Local<Object> PlayerClass::newPlayer(Player const* player) {
    auto newp = new PlayerClass(player);
    return newp->getScriptObject();
}

Player* PlayerClass::extract(Local<Value> const& v) {
    if (EngineScope::currentEngine()->isInstanceOf<PlayerClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<PlayerClass>(v)->get();
    return nullptr;
}

// 公用API
using namespace lse::api;
Local<Value> McClass::getPlayerNbt(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto uuid = mce::UUID::fromString(args[0].asString().toString());
        auto db   = ll::service::getDBStorage();
        if (db && db->hasKey("player_" + uuid.asString(), DBHelpers::Category::Player)) {
            std::unique_ptr<CompoundTag> playerTag =
                db->getCompoundTag("player_" + uuid.asString(), DBHelpers::Category::Player);
            if (playerTag) {
                std::string serverId = playerTag->at("ServerId");
                if (!serverId.empty() && db->hasKey(serverId, DBHelpers::Category::Player)) {
                    return NbtCompoundClass::pack(db->getCompoundTag(serverId, DBHelpers::Category::Player));
                }
            }
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> McClass::setPlayerNbt(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        mce::UUID uuid   = mce::UUID::fromString(args[0].asString().toString());
        auto      tag    = NbtCompoundClass::extract(args[1]);
        Player*   player = ll::service::getLevel()->getPlayer(uuid);
        if (player && tag) {
            player->load(*tag, MoreGlobal::defaultDataLoadHelper());
        } else if (tag) {
            auto db = ll::service::getDBStorage();
            if (db && db->hasKey("player_" + uuid.asString(), DBHelpers::Category::Player)) {
                std::unique_ptr<CompoundTag> playerTag =
                    db->getCompoundTag("player_" + uuid.asString(), DBHelpers::Category::Player);
                if (playerTag) {
                    std::string serverId = playerTag->at("ServerId");
                    if (!serverId.empty()) {
                        db->saveData(serverId, tag->toBinaryNbt(), DBHelpers::Category::Player);
                        return Boolean::newBoolean(true);
                    }
                }
            }
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::setPlayerNbtTags(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kArray);
    try {
        mce::UUID    uuid   = mce::UUID::fromString(args[0].asString().toString());
        auto         tag    = NbtCompoundClass::extract(args[1]);
        Local<Array> arr    = args[2].asArray();
        Player*      player = ll::service::getLevel()->getPlayer(uuid);
        if (player && tag) {
            CompoundTag loadedTag;
            player->save(loadedTag);
            for (size_t i = 0; i < arr.size(); ++i) {
                auto value = arr.get(i);
                if (value.getKind() == ValueKind::kString) {
                    std::string tagName = value.asString().toString();
                    if (!tag->at(tagName).is_null()) {
                        loadedTag.at(tagName) = tag->at(tagName);
                    }
                }
            }
            player->load(loadedTag, MoreGlobal::defaultDataLoadHelper());
        } else if (tag) {
            auto db = ll::service::getDBStorage();
            if (db && db->hasKey("player_" + uuid.asString(), DBHelpers::Category::Player)) {
                std::unique_ptr<CompoundTag> playerTag =
                    db->getCompoundTag("player_" + uuid.asString(), DBHelpers::Category::Player);
                if (playerTag) {
                    std::string serverId = playerTag->at("ServerId");
                    if (!serverId.empty() && db->hasKey(serverId, DBHelpers::Category::Player)) {
                        if (auto loadedTag = db->getCompoundTag(serverId, DBHelpers::Category::Player)) {
                            for (size_t i = 0; i < arr.size(); ++i) {
                                auto value = arr.get(i);
                                if (value.getKind() == ValueKind::kString) {
                                    std::string tagName = value.asString().toString();
                                    if (!tag->at(tagName).is_null()) {
                                        loadedTag->at(tagName) = tag->at(tagName);
                                    }
                                }
                            }
                            db->saveData(serverId, loadedTag->toBinaryNbt(), DBHelpers::Category::Player);
                            return Boolean::newBoolean(true);
                        }
                        return Boolean::newBoolean(true);
                    }
                }
            }
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::deletePlayerNbt(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        mce::UUID uuid = mce::UUID::fromString(args[0].asString().toString());
        if (uuid == mce::UUID::EMPTY()) {
            throw std::invalid_argument(args[0].asString().toString() + " is not a valid UUID");
        }
        auto storage = ll::service::getLevel().transform([](auto& level) { return &level.getLevelStorage(); });
        if (!storage) {
            return Boolean::newBoolean(false);
        }
        auto playerIds = storage->getCompoundTag("player_" + uuid.asString(), DBHelpers::Category::Player);
        if (!playerIds) {
            return Boolean::newBoolean(false);
        }
        for (auto& [type, id] : *playerIds) {
            if (!id.is_string()) {
                continue;
            }
            std::string& key = id.get<StringTag>();
            if (type == "ServerId") {
                storage->deleteData(key, ::DBHelpers::Category::Player);
            } else {
                storage->deleteData("player_" + key, ::DBHelpers::Category::Player);
            }
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::getPlayerScore(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    try {
        auto        obj        = args[1].asString().toString();
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  objective  = scoreboard.getObjective(obj);
        auto        db         = ll::service::getDBStorage();
        if (!objective || !db || !db->hasKey("player_" + args[0].asString().toString(), DBHelpers::Category::Player)) {
            return Number::newNumber(0);
        }
        std::unique_ptr<CompoundTag> playerTag =
            db->getCompoundTag("player_" + args[0].asString().toString(), DBHelpers::Category::Player);
        if (!playerTag) {
            return Number::newNumber(0);
        }
        std::string serverId = playerTag->at("ServerId");
        if (serverId.empty() || !db->hasKey(serverId, DBHelpers::Category::Player)) {
            return Number::newNumber(0);
        }
        std::unique_ptr<CompoundTag> serverIdTag = db->getCompoundTag(serverId, DBHelpers::Category::Player);
        if (!serverIdTag || !serverIdTag->contains("UniqueID")) {
            return Number::newNumber(0);
        }
        int64        uniqueId = serverIdTag->at("UniqueID");
        ScoreboardId sid      = ScoreboardHelper::getId(scoreboard, PlayerScoreboardId(uniqueId));
        if (sid.mRawID == ScoreboardId::INVALID().mRawID || !objective->mScores->contains(sid)) {
            return Number::newNumber(0);
        }
        return Number::newNumber(objective->getPlayerScore(sid).mValue);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::setPlayerScore(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    try {
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  objective  = scoreboard.getObjective(args[1].asString().toString());
        auto        db         = ll::service::getDBStorage();
        if (!objective || !db || !db->hasKey("player_" + args[0].asString().toString(), DBHelpers::Category::Player)) {
            return Boolean::newBoolean(false);
        }
        std::unique_ptr<CompoundTag> playerTag =
            db->getCompoundTag("player_" + args[0].asString().toString(), DBHelpers::Category::Player);
        if (!playerTag) {
            return Boolean::newBoolean(false);
        }
        std::string serverId = playerTag->at("ServerId");
        if (serverId.empty() || !db->hasKey(serverId, DBHelpers::Category::Player)) {
            return Boolean::newBoolean(false);
        }
        std::unique_ptr<CompoundTag> serverIdTag = db->getCompoundTag(serverId, DBHelpers::Category::Player);
        if (!serverIdTag || !serverIdTag->contains("UniqueID")) {
            return Boolean::newBoolean(false);
        }
        int64        uniqueId = serverIdTag->at("UniqueID");
        ScoreboardId sid      = ScoreboardHelper::getId(scoreboard, PlayerScoreboardId(uniqueId));
        if (sid.mRawID == ScoreboardId::INVALID().mRawID) {
            return Boolean::newBoolean(false);
        }
        ScoreboardOperationResult isSuccess;
        scoreboard
            .modifyPlayerScore(isSuccess, sid, *objective, args[2].asNumber().toInt32(), PlayerScoreSetFunction::Set);
        return Boolean::newBoolean(isSuccess == ScoreboardOperationResult::Success);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::addPlayerScore(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    try {
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  objective  = scoreboard.getObjective(args[1].asString().toString());
        auto        db         = ll::service::getDBStorage();
        if (!objective || !db || !db->hasKey("player_" + args[0].asString().toString(), DBHelpers::Category::Player)) {
            return Boolean::newBoolean(false);
        }
        std::unique_ptr<CompoundTag> playerTag =
            db->getCompoundTag("player_" + args[0].asString().toString(), DBHelpers::Category::Player);
        if (!playerTag) {
            return Boolean::newBoolean(false);
        }
        std::string serverId = playerTag->at("ServerId");
        if (serverId.empty() || !db->hasKey(serverId, DBHelpers::Category::Player)) {
            return Boolean::newBoolean(false);
        }
        std::unique_ptr<CompoundTag> serverIdTag = db->getCompoundTag(serverId, DBHelpers::Category::Player);
        if (!serverIdTag || !serverIdTag->contains("UniqueID")) {
            return Boolean::newBoolean(false);
        }
        int64        uniqueId = serverIdTag->at("UniqueID");
        ScoreboardId sid      = ScoreboardHelper::getId(scoreboard, PlayerScoreboardId(uniqueId));
        if (sid.mRawID == ScoreboardId::INVALID().mRawID) {
            return Boolean::newBoolean(false);
        }
        ScoreboardOperationResult isSuccess;
        scoreboard
            .modifyPlayerScore(isSuccess, sid, *objective, args[2].asNumber().toInt32(), PlayerScoreSetFunction::Add);
        return Boolean::newBoolean(isSuccess == ScoreboardOperationResult::Success);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::reducePlayerScore(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    try {
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  objective  = scoreboard.getObjective(args[1].asString().toString());
        auto        db         = ll::service::getDBStorage();
        if (!objective || !db || !db->hasKey("player_" + args[0].asString().toString(), DBHelpers::Category::Player)) {
            return Boolean::newBoolean(false);
        }
        std::unique_ptr<CompoundTag> playerTag =
            db->getCompoundTag("player_" + args[0].asString().toString(), DBHelpers::Category::Player);
        if (!playerTag) {
            return Boolean::newBoolean(false);
        }
        std::string serverId = playerTag->at("ServerId");
        if (serverId.empty() || !db->hasKey(serverId, DBHelpers::Category::Player)) {
            return Boolean::newBoolean(false);
        }
        std::unique_ptr<CompoundTag> serverIdTag = db->getCompoundTag(serverId, DBHelpers::Category::Player);
        if (!serverIdTag || !serverIdTag->contains("UniqueID")) {
            return Boolean::newBoolean(false);
        }
        int64        uniqueId = serverIdTag->at("UniqueID");
        ScoreboardId sid      = ScoreboardHelper::getId(scoreboard, PlayerScoreboardId(uniqueId));
        if (sid.mRawID == ScoreboardId::INVALID().mRawID) {
            return Boolean::newBoolean(false);
        }
        ScoreboardOperationResult isSuccess;
        scoreboard.modifyPlayerScore(
            isSuccess,
            sid,
            *objective,
            args[2].asNumber().toInt32(),
            PlayerScoreSetFunction::Subtract
        );
        return Boolean::newBoolean(isSuccess == ScoreboardOperationResult::Success);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::deletePlayerScore(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    try {
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  objective  = scoreboard.getObjective(args[1].asString().toString());
        auto        db         = ll::service::getDBStorage();
        if (!objective || !db || !db->hasKey("player_" + args[0].asString().toString(), DBHelpers::Category::Player)) {
            return Boolean::newBoolean(false);
        }
        std::unique_ptr<CompoundTag> playerTag =
            db->getCompoundTag("player_" + args[0].asString().toString(), DBHelpers::Category::Player);
        if (!playerTag) {
            return Boolean::newBoolean(false);
        }
        std::string serverId = playerTag->at("ServerId");
        if (serverId.empty() || !db->hasKey(serverId, DBHelpers::Category::Player)) {
            return Boolean::newBoolean(false);
        }
        std::unique_ptr<CompoundTag> serverIdTag = db->getCompoundTag(serverId, DBHelpers::Category::Player);
        if (!serverIdTag || !serverIdTag->contains("UniqueID")) {
            return Boolean::newBoolean(false);
        }
        int64        uniqueId = serverIdTag->at("UniqueID");
        ScoreboardId sid      = ScoreboardHelper::getId(scoreboard, PlayerScoreboardId(uniqueId));
        if (sid.mRawID == ScoreboardId::INVALID().mRawID) {
            return Boolean::newBoolean(false);
        }
        scoreboard.resetPlayerScore(sid, *objective);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::getPlayer(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        std::string target = args[0].asString().toString();
        if (target.empty()) return {};
        Player* found = nullptr;
        if (mce::UUID::canParse(target)) { // If target is UUID, then get player by using UUID
            found = ll::service::getLevel()->getPlayer(mce::UUID(target));
            if (found) {
                return PlayerClass::newPlayer(found);
            }
            return {};
        }

        std::ranges::transform(target, target.begin(),
                               ::tolower); // lower case the string
        size_t delta = INT_MAX;
        ll::service::getLevel()->forEachPlayer([&](Player& player) {
            if (player.getXuid() == target || std::to_string(player.getOrCreateUniqueID().rawID) == target
                || std::to_string(player.getRuntimeID().rawID) == target) {
                found = &player;
                return false;
            }
            std::string pName = player.mName;
            std::ranges::transform(pName, pName.begin(), ::tolower);

            if (pName.find(target) == 0) {
                // 0 ís the index where the "target" appear in "pName"
                size_t curDelta = pName.length() - target.length();
                if (curDelta == 0) {
                    found = &player;
                    return false;
                }

                if (curDelta < delta) {
                    found = &player;
                    delta = curDelta;
                }
            }
            return true;
        });
        return found ? PlayerClass::newPlayer(found) : Local<Value>(); // Player/Null
    }
    CATCH_AND_THROW
}

Local<Value> McClass::getOnlinePlayers(Arguments const&) {
    try {
        Local<Array> list  = Array::newArray();
        auto         level = ll::service::getLevel();
        if (level.has_value()) {
            level->forEachPlayer([&](Player const& player) {
                list.add(PlayerClass::newPlayer(&player));
                return true;
            });
        }
        return list;
    }
    CATCH_AND_THROW
}

Local<Value> McClass::broadcast(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        TextPacketType type = TextPacketType::Raw;
        if (args.size() >= 2 && args[1].isNumber()) {
            int newType = args[1].asNumber().toInt32();
            if (newType >= 0 && newType <= 11) type = static_cast<TextPacketType>(newType);
        }

        TextPacket pkt;
        switch (type) {
        case TextPacketType::Raw:
        case TextPacketType::Tip:
        case TextPacketType::SystemMessage:
        case TextPacketType::TextObject:
        case TextPacketType::TextObjectWhisper:
        case TextPacketType::TextObjectAnnouncement:
            pkt.mBody = TextPacket::MessageOnly(type, args[0].asString().toString());
            break;
        case TextPacketType::Chat:
        case TextPacketType::Whisper:
        case TextPacketType::Announcement:
            pkt.mBody = TextPacket::AuthorAndMessage(type, "", args[0].asString().toString());
            break;
        case TextPacketType::Translate:
        case TextPacketType::Popup:
        case TextPacketType::JukeboxPopup:
            pkt.mBody = TextPacket::MessageAndParams(type, args[0].asString().toString(), {});
        }

        pkt.sendToClients();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

// 成员函数
Player* PlayerClass::get() const {
    if (mValid) {
        return mWeakEntity.tryUnwrap<Player>().as_ptr();
    }
    return nullptr;
}

Local<Value> PlayerClass::getName() const {
    try {
        Player* player = get();
        if (!player) return {};

        return String::newString(player->mName);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getPos() const {
    try {
        Player* player = get();
        if (!player) return {};

        return FloatPos::newPos(player->getPosition(), player->getDimensionId().id);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getFeetPos() const {
    try {
        Player* player = get();
        if (!player) return {};

        return FloatPos::newPos(player->getFeetPos(), player->getDimensionId().id);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getBlockPos() const {
    try {
        Player* player = get();
        if (!player) return {};

        return IntPos::newPos(player->getFeetBlockPos(), player->getDimensionId().id);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getLastDeathPos() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }
        auto pos = player->getLastDeathPos();
        auto dim = player->getLastDeathDimension();
        if (!pos.has_value() || !dim.has_value() || dim->id == -1) {
            return {};
        }
        return IntPos::newPos(pos.value(), dim->id);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getXuid() const {
    try {
        Player* player = get();
        if (!player) return {};

        string xuid;
        try {
            xuid = player->getXuid();
        } catch (...) {
            lse::LegacyScriptEngine::getLogger().debug("Fail in getXuid!");
            xuid = ll::service::PlayerInfo::getInstance().fromName(player->getRealName())->xuid;
        }
        return String::newString(xuid);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getUuid() const {
    try {
        Player* player = get();
        if (!player) return {};

        string uuid;
        try {
            uuid = player->getUuid().asString();
        } catch (...) {
            lse::LegacyScriptEngine::getLogger().debug("Fail in getUuid!");
            uuid = ll::service::PlayerInfo::getInstance().fromName(player->getRealName())->uuid.asString();
        }
        return String::newString(uuid);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getRealName() const {
    try {
        Player* player = get();
        if (!player) return {};

        return String::newString(player->getRealName());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getIP() const {
    try {
        Player* player = get();
        if (!player) return {};

        return String::newString(player->getNetworkIdentifier().getAddress());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getPermLevel() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Number::newNumber(magic_enum::enum_integer(player->getCommandPermissionLevel()));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getGameMode() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Number::newNumber(static_cast<int>(player->getPlayerGameType())); //==========???
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getCanSleep() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(player->canSleep());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getCanFly() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(player->canFly());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getCanBeSeenOnMap() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        if (!player->isAlive() || player->isSpectator()) {
            return Boolean::newBoolean(false);
        }
        ItemStack const& item = player->getItemSlot(SharedTypes::Legacy::EquipmentSlot::Legs);
        return Boolean::newBoolean(item.isHumanoidWearableBlockItem());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getCanFreeze() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->canFreeze());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getCanSeeDaylight() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->canSeeDaylight());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getCanShowNameTag() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->canShowNameTag());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getCanStartSleepInBed() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->canStartSleepInBed());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getCanPickupItems() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->mCanPickupItems);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isSneaking() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(
            SynchedActorDataAccess::getActorFlag(player->getEntityContext(), ActorFlags::Sneaking)
        );
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getSpeed() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Number::newNumber(player->getPosDeltaPerSecLength());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getDirection() const {
    try {
        Player* player = get();
        if (!player) return {};

        // getRotation()
        Vec2 vec = player->mBuiltInComponents->mActorRotationComponent->mRot;
        return DirectionAngle::newAngle(vec.x, vec.y);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getMaxHealth() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Number::newNumber(player->getMaxHealth());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getHealth() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Number::newNumber(player->getHealth());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getInAir() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(
            !player->isOnGround() && !player->getEntityContext().hasComponent<WasInWaterFlagComponent>()
        );
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getInWater() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(player->getEntityContext().hasComponent<WasInWaterFlagComponent>());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getInLava() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(
            ActorMobilityUtils::shouldApplyLava(player->getDimensionBlockSourceConst(), player->getEntityContext())
        );
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getInRain() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(player->isInRain());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getInSnow() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(player->isInSnow());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getInWall() const {
    try {
        Player* player = get();
        if (!player) return {};

        // The original Actor::isInWall() was moved to MobSuffocationSystemImpl::isInWall() in 1.21.60.10, but the later
        // needs too many parameters.
        return Boolean::newBoolean(player->getDimensionBlockSource().isInWall(
            player->getAttachPos(SharedTypes::Legacy::ActorLocation::BreathingPoint)
        ));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getInWaterOrRain() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(player->isInWaterOrRain());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getInWorld() const {
    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(player->isInWorld());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getInClouds() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        short cloudHeight = player->getDimension().getCloudHeight();
        float y           = player->getPosition().y;
        return Boolean::newBoolean(y > cloudHeight && y < cloudHeight + 4.0f);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getUniqueID() const {
    try {
        Player* player = get();
        if (!player) return {};
        return String::newString(std::to_string(player->getOrCreateUniqueID().rawID));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getRuntimeID() const {
    try {
        Player* player = get();
        if (!player) return {};
        return String::newString(std::to_string(player->getRuntimeID().rawID));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getLangCode() const {
    try {
        Player* player = get();
        if (!player) return {};

        auto language = player->getLocaleCode();
        return String::newString(language.empty() ? "unknown" : language);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isLoading() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->isLoading());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isInvisible() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->isInvisible());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isInsidePortal() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        if (auto component = player->getEntityContext().tryGetComponent<InsideBlockComponent>()) {
            auto& fullName = component->mInsideBlock->getBlockType().mNameInfo->mFullName;
            return Boolean::newBoolean(
                *fullName == VanillaBlockTypeIds::Portal() || *fullName == VanillaBlockTypeIds::EndPortal()
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isHurt() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        int health = player->getHealth();
        if (health > 0 && health < player->getMaxHealth()) {
            return Boolean::newBoolean(true);
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isTrusting() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(
            SynchedActorDataAccess::getActorFlag(player->getEntityContext(), ActorFlags::Trusting)
        );
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isTouchingDamageBlock() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->isTouchingDamageBlock());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isHungry() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        if (auto attribute = player->getAttribute(Player::HUNGER()).mPtr) {
            return Boolean::newBoolean(attribute->mCurrentMaxValue > attribute->mCurrentValue);
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isOnFire() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->isOnFire());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isOnGround() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->isOnGround());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isOnHotBlock() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->getEntityContext().hasComponent<IsOnHotBlockFlagComponent>());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isTrading() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->isTrading());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isAdventure() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->isAdventure());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isGliding() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->getStatusFlag(ActorFlags::Gliding));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isSurvival() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->isSurvival());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isSpectator() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->isSpectator());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isRiding() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->isRiding());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isDancing() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(
            SynchedActorDataAccess::getActorFlag(player->getEntityContext(), ActorFlags::Dancing)
        );
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isCreative() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->isCreative());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isFlying() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->isFlying());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isSleeping() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(player->isSleeping());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isMoving() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Boolean::newBoolean(
            SynchedActorDataAccess::getActorFlag(player->getEntityContext(), ActorFlags::Moving)
        );
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::teleport(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1)

    try {
        Player* player = get();
        if (!player) return Boolean::newBoolean(false);
        Vec2      angle;
        FloatVec4 pos;
        bool      rotationIsValid = false;

        if (args.size() <= 2) {
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = *posObj;
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = static_cast<FloatVec4>(*posObj);
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
            if (args.size() == 2 && IsInstanceOf<DirectionAngle>(args[1])) {
                auto ang        = DirectionAngle::extract(args[1]);
                angle.x         = ang->pitch;
                angle.y         = ang->yaw;
                rotationIsValid = true;
            }
        } else if (args.size() <= 5) { // teleport(x,y,z,dimid[,rot])
            // number pos
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);

            pos.x   = args[0].asNumber().toFloat();
            pos.y   = args[1].asNumber().toFloat();
            pos.z   = args[2].asNumber().toFloat();
            pos.dim = args[3].asNumber().toInt32();
            if (args.size() == 5 && IsInstanceOf<DirectionAngle>(args[4])) {
                auto ang        = DirectionAngle::extract(args[4]);
                angle.x         = ang->pitch;
                angle.y         = ang->yaw;
                rotationIsValid = true;
            }
        } else {
            throw WrongArgTypeException(__FUNCTION__);
        }
        if (!rotationIsValid) {
            angle = player->mBuiltInComponents->mActorRotationComponent->mRot;
        }
        player->teleport(pos.getVec3(), pos.dim, angle);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::kill(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        player->kill();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isOP(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(player->isOperator());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setPermLevel(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        bool res     = false;
        int  newPerm = args[0].asNumber().toInt32();
        if (newPerm >= 0 && newPerm <= 4) {
            RecordOperation(
                getEngineOwnData()->pluginName,
                "Set Permission Level",
                fmt::format("Set Player {} Permission Level as {}.", player->getRealName(), newPerm)
            );
            player->getAbilities().mPermissions->mCommandPermissions = static_cast<CommandPermissionLevel>(newPerm);
            auto& perm    = player->getAbilities().mPermissions->mPlayerPermissions;
            auto  oriPerm = perm;
            if (newPerm >= 1) {
                perm = PlayerPermissionLevel::Operator;
            } else {
                perm = PlayerPermissionLevel::Member;
            }
            player->getAbilities()._handlePlayerPermissionsChange(oriPerm, perm);
            UpdateAbilitiesPacket uPkt(player->getOrCreateUniqueID(), player->getAbilities());
            player->sendNetworkPacket(uPkt);
            res = true;
        }
        return Boolean::newBoolean(res);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setGameMode(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        bool res     = false;
        int  newMode = args[0].asNumber().toInt32();
        if ((newMode >= 0 && newMode <= 2) || (newMode >= 5 && newMode <= 6)) {
            player->setPlayerGameType(static_cast<GameType>(newMode));
            res = true;
        }
        return Boolean::newBoolean(res);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::runcmd(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return {};
        CommandContext context = CommandContext(
            args[0].asString().toString(),
            std::make_unique<PlayerCommandOrigin>(ll::service::getLevel(), player->getOrCreateUniqueID()),
            static_cast<int>(CurrentCmdVersion::Latest)
        );
        ll::service::getMinecraft()->mCommands->executeCommand(context, false);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::kick(Arguments const& args) const {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return {};

        string msg = "disconnectionScreen.disconnected";
        if (args.size() >= 1) msg = args[0].asString().toString();

        player->disconnect(msg);
        return Boolean::newBoolean(true); //=======???
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::tell(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return {};

        TextPacketType type = TextPacketType::Raw;
        if (args.size() >= 2 && args[1].isNumber()) {
            int newType = args[1].asNumber().toInt32();
            if (newType >= 0 && newType <= 11) type = static_cast<TextPacketType>(newType);
        }

        TextPacket pkt;
        switch (type) {
        case TextPacketType::Raw:
        case TextPacketType::Tip:
        case TextPacketType::SystemMessage:
        case TextPacketType::TextObject:
        case TextPacketType::TextObjectWhisper:
        case TextPacketType::TextObjectAnnouncement:
            pkt.mBody = TextPacket::MessageOnly(type, args[0].asString().toString());
            break;
        case TextPacketType::Chat:
        case TextPacketType::Whisper:
        case TextPacketType::Announcement:
            pkt.mBody = TextPacket::AuthorAndMessage(type, "", args[0].asString().toString());
            break;
        case TextPacketType::Translate:
        case TextPacketType::Popup:
        case TextPacketType::JukeboxPopup:
            pkt.mBody = TextPacket::MessageAndParams(type, args[0].asString().toString(), {});
        }

        player->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setTitle(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);

    try {
        Player* player = get();
        if (!player) return {};

        string                    content;
        SetTitlePacket::TitleType type        = SetTitlePacket::TitleType::Title;
        int                       fadeInTime  = 10;
        int                       stayTime    = 70;
        int                       fadeOutTime = 20;

        if (args.size() >= 1) {
            CHECK_ARG_TYPE(args[0], ValueKind::kString);
            content = args[0].asString().toString();
        }
        if (args.size() >= 2) {
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            type = static_cast<SetTitlePacket::TitleType>(args[1].asNumber().toInt32());
        }
        if (args.size() >= 5) {
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[4], ValueKind::kNumber);
            fadeInTime  = args[2].asNumber().toInt32();
            stayTime    = args[3].asNumber().toInt32();
            fadeOutTime = args[4].asNumber().toInt32();
        }

        SetTitlePacket pkt(type, content, std::nullopt);
        pkt.mFadeInTime  = fadeInTime;
        pkt.mStayTime    = stayTime;
        pkt.mFadeOutTime = fadeOutTime;
        player->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::talkAs(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return {};
        if (ll::service::getLevel().has_value()) {
            auto                       msg = args[0].asString().toString();
            ll::event::PlayerChatEvent event{*reinterpret_cast<ServerPlayer*>(player), msg};
            ll::event::EventBus::getInstance().publish(event);
            if (event.isCancelled()) return Boolean::newBoolean(false);
            TextPacket pkt = TextPacket::createChat(player->getRealName(), msg, {}, player->getXuid(), {});
            ll::service::getLevel()->forEachPlayer([&pkt](Player const& player) {
                player.sendNetworkPacket(pkt);
                return true;
            });
        } else {
            Boolean::newBoolean(false);
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::talkTo(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* target = PlayerClass::extract(args[1]);
        if (!target) return {};
        Player* player = get();
        if (!player) return {};

        TextPacket pkt;
        pkt.mXuid = player->getXuid();
        pkt.mBody =
            TextPacket::AuthorAndMessage(TextPacketType::Whisper, player->getRealName(), args[0].asString().toString());
        target->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getHand(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        return ItemClass::newItem(&const_cast<ItemStack&>(player->getSelectedItem()));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getOffHand(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        return ItemClass::newItem(const_cast<ItemStack*>(&player->getOffhandSlot()));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getInventory(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        return ContainerClass::newContainer(player->mInventory->mInventory.get());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getArmor(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        return ContainerClass::newContainer(&ActorEquipment::getArmorContainer(player->getEntityContext()));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getEnderChest(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};
        if (auto chest = player->getEnderChestContainer()) {
            return ContainerClass::newContainer(chest);
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getRespawnPosition(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};
        BlockPos      position = player->getExpectedSpawnPosition();
        DimensionType dim      = player->getExpectedSpawnDimensionId();
        return IntPos::newPos(position, dim);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setRespawnPosition(Arguments const& args) const {
    try {
        Player* player = get();
        if (!player) return {};
        IntVec4 pos;
        if (args.size() == 1) {
            // IntPos
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = static_cast<IntVec4>(*posObj);
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = posObj->toIntVec4();
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
        } else if (args.size() == 4) {
            // Number Pos
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
            pos = {
                args[0].asNumber().toInt32(),
                args[1].asNumber().toInt32(),
                args[2].asNumber().toInt32(),
                args[3].asNumber().toInt32()
            };
        } else {
            throw WrongArgsCountException(__FUNCTION__);
        }
        player->setRespawnPosition(pos.getBlockPos(), pos.dim);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::refreshItems(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        player->refreshInventory();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::rename(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return {};
        player->setNameTag(args[0].asString().toString());
        player->_sendDirtyActorData();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::addLevel(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        player->addLevels(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::reduceLevel(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        player->addLevels(-args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getLevel(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        return Number::newNumber(player->getAttribute(Player::LEVEL()).mPtr->mCurrentValue);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setLevel(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        player->addLevels(
            args[0].asNumber().toInt32() - static_cast<int>(player->getAttribute(Player::LEVEL()).mPtr->mCurrentValue)
        );
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setScale(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        SynchedActorDataAccess::setBoundingBoxScale(player->getEntityContext(), args[0].asNumber().toFloat());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::resetLevel(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        player->resetPlayerLevel();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::addExperience(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        player->addExperience(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::reduceExperience(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        float exp = args[0].asNumber().toFloat();
        if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
            auto instance = component->mAttributes->getMutableInstance(Player::EXPERIENCE().mIDValue).mPtr;
            if (!instance) {
                return Boolean::newBoolean(false);
            }
            int neededExp  = player->getXpNeededForNextLevel();
            int currentExp = static_cast<int>(instance->mCurrentValue * neededExp);
            if (exp <= currentExp) {
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    Player::EXPERIENCE(),
                    static_cast<float>(currentExp - exp) / neededExp
                );
                return Boolean::newBoolean(true);
            }
            AttributeHelper::setCurrentValue(component->mAttributes, Player::EXPERIENCE(), 0.0f);
            size_t needExp = exp - currentExp;
            int    level   = player->getAttribute(Player::LEVEL()).mPtr->mCurrentValue;
            while (level > 0) {
                player->addLevels(-1);
                int levelXp = player->getXpNeededForNextLevel();
                if (needExp < levelXp) {
                    AttributeHelper::setCurrentValue(
                        component->mAttributes,
                        Player::EXPERIENCE(),
                        static_cast<float>(levelXp - needExp) / levelXp
                    );
                    return Boolean::newBoolean(true);
                }
                needExp -= levelXp;
                level    = player->getAttribute(Player::LEVEL()).mPtr->mCurrentValue;
            }
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getCurrentExperience(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Number::newNumber(static_cast<long long>(PlayerHelper::getXpEarnedAtCurrentLevel(player)));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setCurrentExperience(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        PlayerHelper::setXpEarnedAtCurrentLevel(player, args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getTotalExperience(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        int          startLevel = 0;
        int          endLevel   = static_cast<int>(player->getAttribute(Player::LEVEL()).mPtr->mCurrentValue);
        unsigned int totalXp    = 0;

        for (int level = startLevel; level < endLevel; ++level) {
            int xpForLevel;
            if (level / 15 == 1) {
                xpForLevel = level * 4 - 38;
            } else if (level / 15 > 1) {
                xpForLevel = level * 8 - 158;
            } else {
                xpForLevel = level * 2 + 7;
            }
            totalXp += xpForLevel;
        }
        return Number::newNumber(static_cast<long long>(totalXp + PlayerHelper::getXpEarnedAtCurrentLevel(player)));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setTotalExperience(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) {
            return {};
        }
        player->resetPlayerLevel();
        player->addExperience(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getXpNeededForNextLevel(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }

        return Number::newNumber(player->getXpNeededForNextLevel());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::transServer(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber)

    try {
        Player* player = get();
        if (!player) return {};

        TransferPacket packet(args[0].asString().toString(), args[1].asNumber().toInt32());
        player->sendNetworkPacket(packet);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::crash(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};
        RecordOperation(
            getEngineOwnData()->pluginName,
            "Crash Player",
            "Execute player.crash() to crash player <" + player->getRealName() + ">"
        );
        LevelChunkPacket pkt;
        pkt.mCacheEnabled = true;
        player->sendNetworkPacket(pkt);
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getBlockStandingOn(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        return BlockClass::newBlock(player->getBlockPosCurrentlyStandingOn(nullptr), player->getDimensionId().id);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getDevice(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};
        return DeviceClass::newDevice(player);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getScore(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return {};

        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  obj        = scoreboard.getObjective(args[0].asString().toString());
        if (!obj) {
            throw std::invalid_argument("Objective " + args[0].asString().toString() + " not found");
        }
        ScoreboardId const& id = scoreboard.getScoreboardId(*player);
        if (id.mRawID == ScoreboardId::INVALID().mRawID) { // !isValid
            scoreboard.createScoreboardId(*player);
        }
        return Number::newNumber(obj->getPlayerScore(id).mValue);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setScore(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  obj        = scoreboard.getObjective(args[0].asString().toString());
        if (!obj) {
            obj = scoreboard.addObjective(
                args[0].asString().toString(),
                args[0].asString().toString(),
                *scoreboard.getCriteria(Scoreboard::DEFAULT_CRITERIA())
            );
        }
        ScoreboardId const& id = scoreboard.getScoreboardId(*player);
        if (id.mRawID == ScoreboardId::INVALID().mRawID) {
            scoreboard.createScoreboardId(*player);
        }
        ScoreboardOperationResult isSuccess;
        scoreboard.modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Set);
        return Boolean::newBoolean(isSuccess == ScoreboardOperationResult::Success);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::addScore(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  obj        = scoreboard.getObjective(args[0].asString().toString());
        if (!obj) {
            return Boolean::newBoolean(false);
        }
        ScoreboardId const& id = scoreboard.getScoreboardId(*player);
        if (id.mRawID == ScoreboardId::INVALID().mRawID) {
            scoreboard.createScoreboardId(*player);
        }
        ScoreboardOperationResult isSuccess;
        scoreboard.modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Add);
        return Boolean::newBoolean(isSuccess == ScoreboardOperationResult::Success);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::reduceScore(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  obj        = scoreboard.getObjective(args[0].asString().toString());
        if (!obj) {
            return Boolean::newBoolean(false);
        }
        ScoreboardId const& id = scoreboard.getScoreboardId(*player);
        if (id.mRawID == ScoreboardId::INVALID().mRawID) {
            scoreboard.createScoreboardId(*player);
        }
        ScoreboardOperationResult isSuccess;
        scoreboard
            .modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Subtract);
        return Boolean::newBoolean(isSuccess == ScoreboardOperationResult::Success);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::deleteScore(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return {};

        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  obj        = scoreboard.getObjective(args[0].asString().toString());
        if (!obj) {
            return Boolean::newBoolean(false);
        }
        ScoreboardId const& id = scoreboard.getScoreboardId(*player);
        if (id.mRawID == ScoreboardId::INVALID().mRawID) {
            return Boolean::newBoolean(true);
        }
        scoreboard.resetPlayerScore(id, *obj);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setSidebar(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kObject);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        std::vector<std::pair<std::string, int>> data;
        auto                                     source = args[1].asObject();
        auto                                     keys   = source.getKeyNames();
        for (auto& key : keys) {
            data.emplace_back(key, source.get(key).asNumber().toInt32());
        }

        int sortOrder = 1;
        if (args.size() >= 3) sortOrder = args[2].asNumber().toInt32();

        SetDisplayObjectivePacket disObjPkt;
        disObjPkt.mDisplaySlotName      = "sidebar";
        disObjPkt.mObjectiveName        = "FakeScoreObj";
        disObjPkt.mObjectiveDisplayName = args[0].asString().toString();
        disObjPkt.mCriteriaName         = "dummy";
        disObjPkt.mSortOrder            = static_cast<ObjectiveSortOrder>(sortOrder);
        disObjPkt.sendTo(*player);
        std::vector<ScorePacketInfo> info;
        for (auto& i : data) {
            ScorePacketInfo pktInfo;
            pktInfo.mScoreboardId->mRawID = i.second;
            pktInfo.mObjectiveName        = "FakeScoreObj";
            pktInfo.mIdentityType         = IdentityDefinition::Type::FakePlayer;
            pktInfo.mScoreValue           = i.second;
            pktInfo.mFakePlayerName       = i.first;
            info.emplace_back(pktInfo);
        }
        SetScorePacket setPkt;
        setPkt.mType      = ScorePacketType::Change;
        setPkt.mScoreInfo = info;
        setPkt.sendTo(*player);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::removeSidebar(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};
        RemoveObjectivePacket pkt;
        pkt.mObjectiveName = "FakeScoreObj";
        player->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setBossBar(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    if (args[0].getKind() == ValueKind::kNumber) {
        CHECK_ARGS_COUNT(args, 4);
        CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
        CHECK_ARG_TYPE(args[1], ValueKind::kString);
        CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
        CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
        try {
            Player* player = get();
            if (!player) return {};

            int64_t uid     = args[0].asNumber().toInt64();
            int     percent = args[2].asNumber().toInt32();
            if (percent < 0) percent = 0;
            else if (percent > 100) percent = 100;
            float value = static_cast<float>(percent) / 100;

            // Remove BossBar firstly
            auto removePkt =
                static_pointer_cast<BossEventPacket>(MinecraftPackets::createPacket(MinecraftPacketIds::BossEvent));
            removePkt->mBossID    = ActorUniqueID(uid);
            removePkt->mEventType = BossEventUpdateType::Remove;
            removePkt->sendTo(*player);

            BinaryStream bs;
            bs.writeVarInt64(uid, nullptr, nullptr);
            bs.writeUnsignedVarInt64(uid, nullptr, nullptr);
            bs.writeString("player", nullptr, nullptr);
            bs.writeFloat(player->getPosition().x, nullptr, nullptr);
            bs.writeFloat(-60.0f, nullptr, nullptr);
            bs.writeFloat(player->getPosition().z, nullptr, nullptr);
            bs.writeFloat(0.0f, nullptr, nullptr);
            bs.writeFloat(0.0f, nullptr, nullptr);
            bs.writeFloat(0.0f, nullptr, nullptr);
            bs.writeFloat(0.0f, nullptr, nullptr);
            bs.writeFloat(0.0f, nullptr, nullptr);
            bs.writeFloat(0.0f, nullptr, nullptr);
            bs.writeFloat(0.0f, nullptr, nullptr);
            // Attribute
            bs.writeUnsignedVarInt(0, nullptr, nullptr);
            // DataItem
            bs.writeUnsignedVarInt(0, nullptr, nullptr);
            // PropertySyncIntEntry
            bs.writeUnsignedVarInt(0, nullptr, nullptr);
            // PropertySyncFloatEntry
            bs.writeUnsignedVarInt(0, nullptr, nullptr);
            // Links
            bs.writeUnsignedVarInt(0, nullptr, nullptr);
            auto addPkt = lse::api::NetworkPacket<MinecraftPacketIds::AddActor>(std::move(bs.mBuffer));

            BossBarColor color = static_cast<BossBarColor>(args[3].asNumber().toInt32());
            auto         pkt =
                static_pointer_cast<BossEventPacket>(MinecraftPackets::createPacket(MinecraftPacketIds::BossEvent));
            pkt->mEventType     = BossEventUpdateType::Add;
            pkt->mBossID        = ActorUniqueID(uid);
            pkt->mName          = args[1].asString().toString();
            pkt->mHealthPercent = value;
            pkt->mColor         = color;
            addPkt.sendTo(*player);
            pkt->sendTo(*player);
            return Boolean::newBoolean(true);
        }
        CATCH_AND_THROW
    }
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    try {
        Player* player = get();
        if (!player) return {};

        int percent = args[1].asNumber().toInt32();
        if (percent < 0) percent = 0;
        else if (percent > 100) percent = 100;
        float value = static_cast<float>(percent) / 100;

        BossBarColor color = BossBarColor::Red;
        if (args.size() >= 3) color = static_cast<BossBarColor>(args[2].asNumber().toInt32());
        auto pkt = static_pointer_cast<BossEventPacket>(MinecraftPackets::createPacket(MinecraftPacketIds::BossEvent));
        pkt->mEventType     = BossEventUpdateType::Add;
        pkt->mName          = args[0].asString().toString();
        pkt->mHealthPercent = value;
        pkt->mColor         = color;
        pkt->sendTo(*player);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::removeBossBar(Arguments const& args) const {
    if (args.size() == 0) {
        try {
            Player* player = get();
            if (!player) return {};

            auto pkt =
                static_pointer_cast<BossEventPacket>(MinecraftPackets::createPacket(MinecraftPacketIds::BossEvent));
            pkt->mEventType = BossEventUpdateType::Remove;
            pkt->mColor     = BossBarColor::Red;
            pkt->sendTo(*player);
            return Boolean::newBoolean(true);
        }
        CATCH_AND_THROW
    }
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        Player* player = get();
        if (!player) return {};
        int64_t uid = args[0].asNumber().toInt64();
        auto pkt = static_pointer_cast<BossEventPacket>(MinecraftPackets::createPacket(MinecraftPacketIds::BossEvent));
        pkt->mBossID    = ActorUniqueID(uid);
        pkt->mEventType = BossEventUpdateType::Remove;
        pkt->sendTo(*player);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::sendSimpleForm(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 5);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kArray);
    CHECK_ARG_TYPE(args[3], ValueKind::kArray);
    CHECK_ARG_TYPE(args[4], ValueKind::kFunction);
    if (args.size() > 5) CHECK_ARG_TYPE(args[5], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return {};
        bool update = args.size() > 5 ? args[5].asBoolean().value() : false;

        // 普通格式
        auto textsArr = args[2].asArray();
        if (textsArr.size() == 0 || !textsArr.get(0).isString()) return {};
        auto imagesArr = args[3].asArray();
        if (imagesArr.size() != textsArr.size() || !imagesArr.get(0).isString()) return {};

        ll::form::SimpleForm form(args[0].asString().toString(), args[1].asString().toString());
        for (size_t i = 0; i < textsArr.size(); ++i) {
            Local<Value> img = imagesArr.get(i);
            if (img.isString()) {
                auto path = img.asString().toString();
                auto type = path.starts_with("http") ? "url" : "path";
                form.appendButton(textsArr.get(i).asString().toString(), path, type);
            } else {
                form.appendButton(textsArr.get(i).asString().toString());
            }
        }
        auto formCallback = [engine{EngineScope::currentEngine()},
                             callback{
                                 script::Global(args[4].asFunction())
                             }](Player const& pl, int chosen, ll::form::FormCancelReason reason) {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running)) return;
            if (!EngineManager::isValid(engine)) return;

            EngineScope scope(engine);
            try {
                callback.get().call(
                    {},
                    PlayerClass::newPlayer(&pl),
                    chosen >= 0 ? Number::newNumber(chosen) : Local<Value>(),
                    reason.has_value() ? Number::newNumber(static_cast<uchar>(reason.value())) : Local<Value>()
                );
            }
            CATCH_IN_CALLBACK("sendSimpleForm")
        };
        if (update) form.sendUpdate(*player, std::move(formCallback));
        else form.sendTo(*player, std::move(formCallback));

        return Number::newNumber(1);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::sendModalForm(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 5);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kString);
    CHECK_ARG_TYPE(args[3], ValueKind::kString);
    CHECK_ARG_TYPE(args[4], ValueKind::kFunction);
    if (args.size() > 5) CHECK_ARG_TYPE(args[5], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return {};
        bool update = args.size() > 5 ? args[5].asBoolean().value() : false;

        ll::form::ModalForm form(
            args[0].asString().toString(),
            args[1].asString().toString(),
            args[2].asString().toString(),
            args[3].asString().toString()
        );
        auto formCallback = [engine{EngineScope::currentEngine()}, callback{script::Global(args[4].asFunction())}](
                                Player const&                    pl,
                                ll::form::ModalFormResult const& chosen,
                                ll::form::FormCancelReason       reason
                            ) {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running)) return;
            if (!EngineManager::isValid(engine)) return;

            EngineScope scope(engine);
            try {
                callback.get().call(
                    {},
                    PlayerClass::newPlayer(&pl),
                    chosen ? Boolean::newBoolean(static_cast<bool>(*chosen)) : Local<Value>(),
                    reason.has_value() ? Number::newNumber(static_cast<uchar>(reason.value())) : Local<Value>()
                );
            }
            CATCH_IN_CALLBACK("sendModalForm")
        };

        if (update) form.sendUpdate(*player, std::move(formCallback));
        else form.sendTo(*player, std::move(formCallback));

        return Number::newNumber(2);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::sendCustomForm(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);
    if (args.size() > 2) CHECK_ARG_TYPE(args[2], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return {};
        bool update = args.size() > 2 ? args[2].asBoolean().value() : false;

        auto formData     = ordered_json::parse(args[0].asString().toString());
        auto formCallback = [engine{EngineScope::currentEngine()}, callback{script::Global(args[1].asFunction())}](
                                Player const&                     player,
                                std::optional<std::string> const& data,
                                ll::form::FormCancelReason        reason
                            ) {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running)) return;
            if (!EngineManager::isValid(engine)) return;

            EngineScope scope(engine);
            try {
                Local<Value> result;
                if (data) {
                    auto dataJson = nlohmann::ordered_json::parse(*data);
                    result        = JsonToValue(dataJson);
                    if (result.isNull()) result = Array::newArray();
                }
                callback.get().call(
                    {},
                    PlayerClass::newPlayer(&player),
                    result,
                    reason.has_value() ? Number::newNumber(static_cast<uchar>(reason.value())) : Local<Value>()
                );
            }
            CATCH_IN_CALLBACK("sendCustomForm")
        };
        if (update) ll::form::Form::sendRawUpdate(*player, formData.dump(), std::move(formCallback));
        else ll::form::Form::sendRawTo(*player, formData.dump(), std::move(formCallback));

        return Number::newNumber(3);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::sendForm(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);
    if (args.size() > 2) CHECK_ARG_TYPE(args[2], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return {};
        bool update = args.size() > 2 ? args[2].asBoolean().value() : false;

        if (IsInstanceOf<SimpleFormClass>(args[0])) {
            Local<Function> callback = args[1].asFunction();
            SimpleFormClass::sendForm(SimpleFormClass::extract(args[0]), player, callback, update);
        } else if (IsInstanceOf<CustomFormClass>(args[0])) {
            Local<Function> callback = args[1].asFunction();
            CustomFormClass::sendForm(CustomFormClass::extract(args[0]), player, callback, update);
        } else {
            throw WrongArgTypeException(__FUNCTION__);
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::closeForm(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        ClientboundCloseFormPacket().sendTo(*player);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::sendPacket(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kObject);

    try {
        Player* player = get();
        if (!player) return {};
        auto pkt = PacketClass::extract(args[0]);
        if (!pkt) return Boolean::newBoolean(false);
        player->sendNetworkPacket(*pkt);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setExtraData(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return {};

        string key = args[0].asString().toString();
        if (key.empty()) return Boolean::newBoolean(false);

        getEngineOwnData()->playerDataDB[player->getRealName() + "-" + key] = args[1];
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getExtraData(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return {};

        string key = args[0].asString().toString();
        if (key.empty()) return {};

        auto& db  = getEngineOwnData()->playerDataDB;
        auto  res = db.find(player->getRealName() + "-" + key);
        if (res == db.end() || res->second.isEmpty()) return {};
        return res->second.get();
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::delExtraData(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        Player* player = get();
        if (!player) return {};

        string key = args[0].asString().toString();
        if (key.empty()) return Boolean::newBoolean(false);

        getEngineOwnData()->playerDataDB.erase(player->getRealName() + "-" + key);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::hurt(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) {
            return Boolean::newBoolean(false);
        }
        float damage = args[0].asNumber().toFloat();
        int   type   = 0;
        if (args.size() >= 2) {
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            type = args[1].asNumber().toInt32();
        }
        if (args.size() == 3) {
            Actor* source = EntityClass::tryExtractActor(args[2]);
            if (!source) {
                return Boolean::newBoolean(false);
            }
            ActorDamageByActorSource damageBySource =
                ActorDamageByActorSource(*source, static_cast<SharedTypes::Legacy::ActorDamageCause>(type));
            return Boolean::newBoolean(player->_hurt(damageBySource, damage, true, false));
        }
        ActorDamageSource damageSource(static_cast<SharedTypes::Legacy::ActorDamageCause>(type), {});
        damageSource.mCause = static_cast<SharedTypes::Legacy::ActorDamageCause>(type);
        return Boolean::newBoolean(player->_hurt(damageSource, damage, true, false));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::heal(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        Player* player = get();
        if (!player) return {};

        player->heal(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setHealth(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::HEALTH(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setMaxHealth(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setMaxValue(
                    component->mAttributes,
                    SharedAttributes::HEALTH(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setAbsorption(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::ABSORPTION(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setAttackDamage(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::ATTACK_DAMAGE(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setMaxAttackDamage(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setMaxValue(
                    component->mAttributes,
                    SharedAttributes::ATTACK_DAMAGE(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setFollowRange(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::FOLLOW_RANGE(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setKnockbackResistance(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::KNOCKBACK_RESISTANCE(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setLuck(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::LUCK(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setMovementSpeed(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::MOVEMENT_SPEED(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setUnderwaterMovementSpeed(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::UNDERWATER_MOVEMENT_SPEED(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setLavaMovementSpeed(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(
                    component->mAttributes,
                    SharedAttributes::LAVA_MOVEMENT_SPEED(),
                    args[0].asNumber().toFloat()
                )
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setHungry(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
            return Boolean::newBoolean(
                AttributeHelper::setCurrentValue(component->mAttributes, Player::HUNGER(), args[0].asNumber().toFloat())
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setFire(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return {};

        int  time          = args[0].asNumber().toInt32();
        bool isEffectValue = args[1].asBoolean().value();

        player->setOnFire(time, isEffectValue);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::stopFire(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        player->stopFire();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

// For Compatibility
Local<Value> PlayerClass::setOnFire(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};

        int time = args[0].asNumber().toInt32();

        player->setOnFire(time, true);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::refreshChunks(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        player->mChunkPublisherView->clearRegion();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::giveItem(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);

    try {
        Player* player = get();
        if (!player) return {};

        ItemStack* item = ItemClass::extract(args[0]);
        if (!item) return {}; // Null
        std::vector<ItemStack> items = {*item};
        if (args.size() >= 2) {
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            auto count    = args[1].asNumber().toInt32();
            auto maxCount = item->getMaxStackSize();
            if (count > maxCount) {
                items[0].mCount = count % maxCount;
                for (int i = 0; i < count / maxCount; i++) {
                    items.emplace_back(*item).mCount = maxCount;
                }
            }
        }
        return Boolean::newBoolean(Util::LootTableUtils::givePlayer(*player, items, true));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::clearItem(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) {
            return {};
        }
        int clearCount = 1;
        if (args.size() > 1) {
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            clearCount = args[1].asNumber().toInt32();
        }
        int         result        = 0;
        std::string typeName      = args[0].asString().toString();
        auto        clearFunction = [&result, &typeName, &clearCount](Container& container) {
            auto slots = container.getSlots();
            for (size_t slot = 0; slot < slots.size() && clearCount > 0; ++slot) {
                if (slots[slot]->getTypeName() == typeName) {
                    auto count = slots[slot]->mCount;
                    if (count <= clearCount) {
                        result     += count;
                        clearCount -= count;
                        container.setItem(static_cast<int>(slot), ItemStack::EMPTY_ITEM());
                    } else {
                        result += clearCount;
                        container.removeItem(static_cast<int>(slot), clearCount);
                        clearCount = 0;
                    }
                }
            }
        };
        clearFunction(*player->mInventory->mInventory);
        clearFunction(ActorEquipment::getHandContainer(player->getEntityContext()));
        clearFunction(ActorEquipment::getArmorContainer(player->getEntityContext()));
        player->refreshInventory();
        return Number::newNumber(result);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isSprinting(Arguments const& args) const {
    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(player->getStatusFlag(ActorFlags::Sprinting));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setSprinting(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return {};

        player->setSprinting(args[0].asBoolean().value());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getNbt(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        std::unique_ptr<CompoundTag> tag = std::make_unique<CompoundTag>();
        player->save(*tag);
        return NbtCompoundClass::pack(std::move(tag));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setNbt(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);

    try {
        Player* player = get();
        if (!player) return {};

        auto nbt = NbtCompoundClass::extract(args[0]);
        if (!nbt) {
            return {};
        }
        return Boolean::newBoolean(player->load(*nbt, MoreGlobal::defaultDataLoadHelper()));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::addTag(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(player->addTag(args[0].asString().toString()));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::removeTag(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(player->removeTag(args[0].asString().toString()));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::hasTag(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return {};

        return Boolean::newBoolean(player->hasTag(args[0].asString().toString()));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getAllTags(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        Local<Array> arr = Array::newArray();
        if (auto component = player->getEntityContext().tryGetComponent<TagsComponent<IDType<LevelTagSetIDType>>>()) {
            for (auto& tag : get()->getLevel().getTagRegistry().getTagsInSet(component->mTagSetID)) {
                arr.add(String::newString(tag));
            }
            return arr;
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getAbilities(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        CompoundTag tag;
        player->save(tag);

        try {
            return Tag2Value(&tag.at("abilities").get<CompoundTag>(), true);
        } catch (...) {
            return Object::newObject();
        }
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getAttributes(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        Local<Array> res = Array::newArray();

        CompoundTag tag = CompoundTag();
        player->save(tag);
        try {
            Local<Array> arr = Array::newArray();
            for (auto& tagP : tag.at("Attributes").get<ListTag>()) {
                arr.add(Tag2Value(tagP.get(), true));
            }
            return arr;
        } catch (...) {
            return Array::newArray();
        }
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getEntityFromViewVector(Arguments const& args) const {
    try {
        Player* player = get();
        if (!player) return {};
        float maxDistance = 5.25f;
        if (args.size() > 0) {
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            maxDistance = args[0].asNumber().toFloat();
        }
        HitResult result = player->traceRay(maxDistance, true, false);
        Actor*    entity = result.getEntity();
        if (result.mType != HitResultType::NoHit && entity) {
            return EntityClass::newEntity(entity);
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getBlockFromViewVector(Arguments const& args) const {
    try {
        Player* player = get();
        if (!player) return {};
        bool  includeLiquid = false;
        bool  solidOnly     = false;
        float maxDistance   = 5.25f;
        bool  fullOnly      = false;
        if (args.size() > 0) {
            CHECK_ARG_TYPE(args[0], ValueKind::kBoolean);
            includeLiquid = args[0].asBoolean().value();
        }
        if (args.size() > 1) {
            CHECK_ARG_TYPE(args[1], ValueKind::kBoolean);
            solidOnly = args[1].asBoolean().value();
        }
        if (args.size() > 2) {
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            maxDistance = args[2].asNumber().toFloat();
        }
        if (args.size() > 3) {
            CHECK_ARG_TYPE(args[3], ValueKind::kBoolean);
            fullOnly = args[3].asBoolean().value();
        }
        HitResult res = player->traceRay(
            maxDistance,
            false,
            true,
            [&solidOnly, &fullOnly, &includeLiquid](BlockSource const&, Block const& block, bool) {
                if (solidOnly && !block.mCachedComponentData->mIsSolid) {
                    return false;
                }
                if (fullOnly && !block.getBlockType().isSlabBlock()) {
                    return false;
                }
                if (!includeLiquid && BlockUtils::isLiquidSource(block)) {
                    return false;
                }
                return true;
            }
        );
        if (res.mType == HitResultType::NoHit) {
            return {};
        }
        BlockPos bp;
        if (includeLiquid && res.mIsHitLiquid) {
            bp = res.mLiquidPos;
        } else {
            bp = res.mBlock;
        }
        Block const&     bl     = player->getDimensionBlockSource().getBlock(bp);
        BlockType const& legacy = bl.getBlockType();
        // isEmpty()
        if (bl.isAir() || (legacy.mProperties == BlockProperty::None && legacy.mMaterial.mType == MaterialType::Any)) {
            return {};
        }
        return BlockClass::newBlock(bl, bp, player->getDimensionBlockSource());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::isSimulatedPlayer(Arguments const&) const {
    try {
        Player* actor = get();
        if (!actor) return {};
        return Boolean::newBoolean(actor->isSimulatedPlayer());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::quickEvalMolangScript(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        Player* actor = get();
        if (!actor) return {};
        return Number::newNumber(actor->evalMolang(args[0].asString().toString()));
    }
    CATCH_AND_THROW
}

//////////////////// For LLMoney ////////////////////

Local<Value> PlayerClass::getMoney(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};
        auto xuid = player->getXuid();
        return xuid.empty() ? Local<Value>() : Number::newNumber(EconomySystem::getMoney(xuid));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::reduceMoney(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};
        auto xuid = player->getXuid();
        return xuid.empty() ? Local<Value>()
                            : Boolean::newBoolean(EconomySystem::reduceMoney(xuid, args[0].asNumber().toInt64()));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setMoney(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};
        auto xuid = player->getXuid();
        return xuid.empty() ? Local<Value>()
                            : Boolean::newBoolean(EconomySystem::setMoney(xuid, args[0].asNumber().toInt64()));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::addMoney(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};
        auto xuid = player->getXuid();
        return xuid.empty() ? Local<Value>()
                            : Boolean::newBoolean(EconomySystem::addMoney(xuid, args[0].asNumber().toInt64()));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::transMoney(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    // nocheck: args[0] maybe Player or XUID.
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};
        std::string xuid = player->getXuid();
        std::string targetXuid;
        std::string note;
        if (args[0].getKind() == ValueKind::kString) {
            targetXuid = args[0].asString().toString();
        } else {
            targetXuid = PlayerClass::extract(args[0])->getXuid();
        }
        if (args.size() >= 3) {
            CHECK_ARG_TYPE(args[2], ValueKind::kString);
            note = args[2].asString().toString();
        }
        return xuid.empty()
                 ? Local<Value>()
                 : Boolean::newBoolean(EconomySystem::transMoney(xuid, targetXuid, args[1].asNumber().toInt64(), note));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getMoneyHistory(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return {};
        auto xuid = player->getXuid();
        return xuid.empty()
                 ? Local<Value>()
                 : objectificationMoneyHistory(EconomySystem::getMoneyHist(xuid, args[0].asNumber().toInt32()));
    }
    CATCH_AND_THROW
}

//////////////////// For Compatibility ////////////////////

Local<Value> PlayerClass::getAllItems(Arguments const&) const {
    try {
        Player* player = get();
        if (!player) return {};

        ItemStack const&              hand      = player->getCarriedItem();
        ItemStack const&              offHand   = player->getOffhandSlot();
        std::vector<ItemStack const*> inventory = player->mInventory->mInventory->getSlots();
        std::vector<ItemStack const*> armor = ActorEquipment::getArmorContainer(player->getEntityContext()).getSlots();
        std::vector<ItemStack const*> endChest = player->getEnderChestContainer()->getSlots();

        Local<Object> result = Object::newObject();

        // hand
        result.set("hand", ItemClass::newItem(&const_cast<ItemStack&>(hand)));

        // offHand
        result.set("offHand", ItemClass::newItem(&const_cast<ItemStack&>(offHand)));

        // inventory
        Local<Array> inventoryArr = Array::newArray();
        for (ItemStack const* item : inventory) {
            if (item) {
                inventoryArr.add(ItemClass::newItem(const_cast<ItemStack*>(item)));
            }
        }
        result.set("inventory", inventoryArr);

        // armor
        Local<Array> armorArr = Array::newArray();
        for (ItemStack const* item : armor) {
            if (item) {
                armorArr.add(ItemClass::newItem(const_cast<ItemStack*>(item)));
            }
        }
        result.set("armor", armorArr);

        // endChest
        Local<Array> endChestArr = Array::newArray();
        for (ItemStack const* item : endChest) {
            if (item) {
                endChestArr.add(ItemClass::newItem(const_cast<ItemStack*>(item)));
            }
        }
        result.set("endChest", endChestArr);

        return result;
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::removeItem(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    try {
        Player* player = get();
        if (!player) return {};

        int inventoryId = args[0].asNumber().toInt32();
        int count       = args[1].asNumber().toInt32();

        auto& container = player->mInventory->mInventory;
        if (inventoryId > container->getContainerSize()) return Boolean::newBoolean(false);
        container->removeItem(inventoryId, count);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::sendToast(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    try {
        Player* player = get();

        if (!player) return {};

        ToastRequestPacket pkt;
        pkt.mTitle   = args[0].asString().toString();
        pkt.mContent = args[1].asString().toString();
        player->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::distanceTo(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);

    try {
        FloatVec4 pos{};

        Player* player = get();
        if (!player) return {};

        if (args.size() == 1) { // pos | player | entity
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return {};
                pos = *posObj;
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return {};
                pos = static_cast<FloatVec4>(*posObj);
            } else if (IsInstanceOf<PlayerClass>(args[0]) || IsInstanceOf<EntityClass>(args[0])) {
                // Player or Entity

                Actor* targetActor = EntityClass::tryExtractActor(args[0]);
                if (!targetActor) return {};

                Vec3 targetActorPos = targetActor->getPosition();

                pos.x   = targetActorPos.x;
                pos.y   = targetActorPos.y;
                pos.z   = targetActorPos.z;
                pos.dim = targetActor->getDimensionId().id;
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
        } else if (args.size() == 4) { // x, y, z, dimId
            // number pos
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);

            pos.x   = args[0].asNumber().toFloat();
            pos.y   = args[1].asNumber().toFloat();
            pos.z   = args[2].asNumber().toFloat();
            pos.dim = args[3].asNumber().toInt32();
        } else {
            throw WrongArgsCountException(__FUNCTION__);
        }

        if (player->getDimensionId().id != pos.dim) return Number::newNumber(INT_MAX);

        return Number::newNumber(player->getPosition().distanceTo(pos.getVec3()));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::distanceToSqr(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);

    try {
        FloatVec4 pos;

        Player* player = get();
        if (!player) return {};

        if (args.size() == 1) {
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return {};
                pos = *posObj;
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return {};
                pos = static_cast<FloatVec4>(*posObj);
            } else if (IsInstanceOf<PlayerClass>(args[0]) || IsInstanceOf<EntityClass>(args[0])) {
                // Player or Entity

                Actor* targetActor = EntityClass::tryExtractActor(args[0]);
                if (!targetActor) return {};

                Vec3 targetActorPos = targetActor->getPosition();

                pos.x   = targetActorPos.x;
                pos.y   = targetActorPos.y;
                pos.z   = targetActorPos.z;
                pos.dim = targetActor->getDimensionId().id;
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
        } else if (args.size() == 4) {
            // number pos
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);

            pos.x   = args[0].asNumber().toFloat();
            pos.y   = args[1].asNumber().toFloat();
            pos.z   = args[2].asNumber().toFloat();
            pos.dim = args[3].asNumber().toInt32();
        } else {
            throw WrongArgsCountException(__FUNCTION__);
        }

        if (player->getDimensionId().id != pos.dim) return Number::newNumber(INT_MAX);

        return Number::newNumber(player->getPosition().distanceToSqr(pos.getVec3()));
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::setAbility(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return {};
        player->setAbility(static_cast<AbilitiesIndex>(args[0].asNumber().toInt32()), args[1].asBoolean().value());
        if (!player->isPlayerInitialized()) {
            ll::coro::keepThis([uuid(player->getOrCreateUniqueID())]() -> ll::coro::CoroTask<> {
                using namespace ll::chrono_literals;
                co_await 1_tick;
                auto player = ll::service::getLevel()->getPlayer(uuid);
                if (!player) co_return;
                UpdateAbilitiesPacket(uuid, player->getAbilities()).sendTo(*player);
                UpdateAdventureSettingsPacket{player->getLevel().getAdventureSettings()}.sendTo(*player);
            }).launch(ll::thread::ServerThreadExecutor::getDefault());
        }
        return Boolean::newBoolean(true);
    }

    CATCH_AND_THROW
}

Local<Value> PlayerClass::getBiomeId() const {
    try {
        Player* player = get();
        if (!player) return {};
        Biome const& bio = player->getDimensionBlockSource().getBiome(player->getFeetBlockPos());
        return Number::newNumber(bio.mId->mValue);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getBiomeName() const {
    try {
        Player* player = get();
        if (!player) return {};
        Biome const& bio = player->getDimensionBlockSource().getBiome(player->getFeetBlockPos());
        return String::newString(bio.mHash->getString());
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::getAllEffects() const {
    try {
        Player* player = get();
        if (!player) {
            return {};
        }
        Local<Array> effectList = Array::newArray();
        for (auto& effect : player->_getAllEffectsNonConst()) {
            effectList.add(Number::newNumber(static_cast<long long>(effect.mId)));
        }
        return effectList;
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::addEffect(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 4);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[3], ValueKind::kBoolean);
    try {
        Player* player = get();
        if (!player) {
            return Boolean::newBoolean(false);
        }
        unsigned int      id = args[0].asNumber().toInt32();
        EffectDuration    duration{args[1].asNumber().toInt32()};
        int               level         = args[2].asNumber().toInt32();
        bool              showParticles = args[3].asBoolean().value();
        MobEffectInstance effect(id);
        effect.mDuration      = duration;
        effect.mAmplifier     = level;
        effect.mEffectVisible = showParticles;
        player->addEffect(effect);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::removeEffect(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        Player* player = get();
        if (!player) {
            return Boolean::newBoolean(false);
        }
        int id = args[0].asNumber().toInt32();
        player->removeEffect(id);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PlayerClass::toEntity(Arguments const&) const {
    try {
        return EntityClass::newEntity(get());
    }
    CATCH_AND_THROW
}
