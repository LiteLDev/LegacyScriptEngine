#include "api/PlayerAPI.h"

#include "api/APIHelp.h"
#include "api/BaseAPI.h"
#include "api/BlockAPI.h"
#include "api/ContainerAPI.h"
#include "api/DataAPI.h"
#include "api/DeviceAPI.h"
#include "api/EntityAPI.h"
#include "api/GuiAPI.h"
#include "api/ItemAPI.h"
#include "api/McAPI.h"
#include "api/NbtAPI.h"
#include "api/PacketAPI.h"
#include "engine/EngineOwnData.h"
#include "engine/GlobalShareData.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerChatEvent.h"
#include "ll/api/form/ModalForm.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/service/PlayerInfo.h"
#include "ll/api/service/ServerInfo.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "lse/api/MoreGlobal.h"
#include "lse/api/NetworkPacket.h"
#include "lse/api/helper/AttributeHelper.h"
#include "lse/api/helper/PlayerHelper.h"
#include "lse/api/helper/ScoreboardHelper.h"
#include "main/EconomicSystem.h"
#include "main/SafeGuardRecord.h"
#include "mc/certificates/WebToken.h"
#include "mc/deps/core/math/Vec2.h"
#include "mc/deps/core/utility/MCRESULT.h"
#include "mc/entity/components/ActorRotationComponent.h"
#include "mc/entity/components/InsideBlockComponent.h"
#include "mc/entity/components/IsOnHotBlockFlagComponent.h"
#include "mc/entity/components/TagsComponent.h"
#include "mc/entity/utilities/ActorMobilityUtils.h"
#include "mc/legacy/ActorUniqueID.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/ListTag.h"
#include "mc/nbt/StringTag.h"
#include "mc/network/ConnectionRequest.h"
#include "mc/network/MinecraftPacketIds.h"
#include "mc/network/MinecraftPackets.h"
#include "mc/network/NetEventCallback.h"
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
#include "mc/server/commands/CommandVersion.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/PlayerCommandOrigin.h"
#include "mc/util/BlockUtils.h"
#include "mc/world/Container.h"
#include "mc/world/Minecraft.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorDamageByActorSource.h"
#include "mc/world/actor/ai/util/BossBarColor.h"
#include "mc/world/actor/ai/util/BossEventUpdateType.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/LayeredAbilities.h"
#include "mc/world/actor/player/PermissionsHandler.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/PlayerInventory.h"
#include "mc/world/actor/provider/ActorAttribute.h"
#include "mc/world/actor/provider/ActorEquipment.h"
#include "mc/world/actor/provider/SynchedActorDataAccess.h"
#include "mc/world/attribute/AttributeInstance.h"
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
#include "mc/world/scores/ObjectiveCriteria.h"
#include "mc/world/scores/PlayerScoreSetFunction.h"
#include "mc/world/scores/PlayerScoreboardId.h"
#include "mc/world/scores/ScoreInfo.h"
#include "mc/world/scores/Scoreboard.h"
#include "mc/world/scores/ScoreboardId.h"
#include "mc/world/scores/ScoreboardOperationResult.h"

SetScorePacket::SetScorePacket() = default;

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
PlayerClass::PlayerClass(Player* player) : ScriptClass(ScriptClass::ConstructFromCpp<PlayerClass>{}) {
    try {
        if (player) {
            mWeakEntity = player->getWeakEntity();
            mValid      = true;
        }
    } catch (...) {}
}

Local<Object> PlayerClass::newPlayer(Player* player) {
    auto newp = new PlayerClass(player);
    return newp->getScriptObject();
}

Player* PlayerClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<PlayerClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<PlayerClass>(v)->get();
    else return nullptr;
}

// 公用API
using namespace lse::api;
Local<Value> McClass::getPlayerNbt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto       uuid = mce::UUID::fromString(args[0].asString().toString());
        DBStorage* db   = MoreGlobal::dbStorage;
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
        return Local<Value>();
    }
    CATCH("Fail in getPlayerNbt!")
}

Local<Value> McClass::setPlayerNbt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        mce::UUID uuid   = mce::UUID::fromString(args[0].asString().toString());
        auto      tag    = NbtCompoundClass::extract(args[1]);
        Player*   player = ll::service::getLevel()->getPlayer(uuid);
        if (player && tag) {
            player->load(*tag, MoreGlobal::defaultDataLoadHelper());
        } else if (tag) {
            DBStorage* db = MoreGlobal::dbStorage;
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
    CATCH("Fail in setPlayerNbt!")
}

Local<Value> McClass::setPlayerNbtTags(const Arguments& args) {
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
            DBStorage* db = MoreGlobal::dbStorage;
            if (db && db->hasKey("player_" + uuid.asString(), DBHelpers::Category::Player)) {
                std::unique_ptr<CompoundTag> playerTag =
                    db->getCompoundTag("player_" + uuid.asString(), DBHelpers::Category::Player);
                if (playerTag) {
                    std::string serverId = playerTag->at("ServerId");
                    if (!serverId.empty() && db->hasKey(serverId, DBHelpers::Category::Player)) {
                        auto loadedTag = db->getCompoundTag(serverId, DBHelpers::Category::Player);
                        if (loadedTag) {
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
    CATCH("Fail in setPlayerNbtTags!")
}

Local<Value> McClass::deletePlayerNbt(const Arguments& args) {
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
    CATCH("Fail in deletePlayerNbt!")
}

Local<Value> McClass::getPlayerScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    try {
        auto        obj        = args[1].asString().toString();
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  objective  = scoreboard.getObjective(obj);
        DBStorage*  db         = MoreGlobal::dbStorage;
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
        if (sid.mRawID == ScoreboardId::INVALID().mRawID || !objective->hasScore(sid)) {
            return Number::newNumber(0);
        }
        return Number::newNumber(objective->getPlayerScore(sid).mValue);
    }
    CATCH("Fail in getPlayerScore!")
}

Local<Value> McClass::setPlayerScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    try {
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  objective  = scoreboard.getObjective(args[1].asString().toString());
        DBStorage*  db         = MoreGlobal::dbStorage;
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
    CATCH("Fail in setPlayerScore!")
}

Local<Value> McClass::addPlayerScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    try {
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  objective  = scoreboard.getObjective(args[1].asString().toString());
        DBStorage*  db         = MoreGlobal::dbStorage;
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
    CATCH("Fail in addPlayerScore!")
}

Local<Value> McClass::reducePlayerScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    try {
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  objective  = scoreboard.getObjective(args[1].asString().toString());
        DBStorage*  db         = MoreGlobal::dbStorage;
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
    CATCH("Fail in reducePlayerScore!")
}

Local<Value> McClass::deletePlayerScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    try {
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  objective  = scoreboard.getObjective(args[1].asString().toString());
        DBStorage*  db         = MoreGlobal::dbStorage;
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
    CATCH("Fail in deletePlayerScore!")
}

Local<Value> McClass::getPlayer(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        std::string target = args[0].asString().toString();
        if (target.empty()) return Local<Value>();
        Player* found = nullptr;
        if (mce::UUID::canParse(target)) { // If target is UUID, then get player by using UUID
            found = ll::service::getLevel()->getPlayer(mce::UUID(target));
            if (found) {
                return PlayerClass::newPlayer(found);
            } else {
                return Local<Value>();
            }
        }

        transform(target.begin(), target.end(), target.begin(),
                  ::tolower); // lower case the string
        size_t delta = INT_MAX;
        ll::service::getLevel()->forEachPlayer([&](Player& player) {
            if (player.getXuid() == target || std::to_string(player.getOrCreateUniqueID().rawID) == target
                || std::to_string(player.getRuntimeID().rawID) == target) {
                found = &player;
                return false;
            }
            std::string pName = player.mName;
            transform(pName.begin(), pName.end(), pName.begin(), ::tolower);

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
    CATCH("Fail in GetPlayer!")
}

Local<Value> McClass::getOnlinePlayers(const Arguments&) {
    try {
        Local<Array> list  = Array::newArray();
        auto         level = ll::service::getLevel();
        if (level.has_value()) {
            level->forEachPlayer([&](Player& player) {
                list.add(PlayerClass::newPlayer(&player));
                return true;
            });
        }
        return list;
    }
    CATCH("Fail in GetOnlinePlayers!")
}

Local<Value> McClass::broadcast(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        TextPacketType type = TextPacketType::Raw;
        if (args.size() >= 2 && args[1].isNumber()) {
            int newType = args[1].asNumber().toInt32();
            if (newType >= 0 && newType <= 11) type = (TextPacketType)newType;
        }
        TextPacket pkt = TextPacket();
        pkt.mType      = type;
        pkt.mMessage   = args[0].asString().toString();
        pkt.sendToClients();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in Broadcast!")
}

// 成员函数
Player* PlayerClass::get() {
    if (mValid) {
        return mWeakEntity.tryUnwrap<Player>().as_ptr();
    } else {
        return nullptr;
    }
}

Local<Value> PlayerClass::getName() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return String::newString(player->mName);
    }
    CATCH("Fail in getPlayerName!")
}

Local<Value> PlayerClass::getPos() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return FloatPos::newPos(player->getPosition(), player->getDimensionId());
    }
    CATCH("Fail in getPlayerPos!")
}

Local<Value> PlayerClass::getFeetPos() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return FloatPos::newPos(player->getFeetPos(), player->getDimensionId());
    }
    CATCH("Fail in getPlayerFeetPos!")
}

Local<Value> PlayerClass::getBlockPos() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return IntPos::newPos(player->getFeetBlockPos(), player->getDimensionId());
    }
    CATCH("Fail in getPlayerBlockPos!")
}

Local<Value> PlayerClass::getLastDeathPos() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }
        auto pos = player->getLastDeathPos();
        auto dim = player->getLastDeathDimension();
        if (!pos.has_value() || !dim.has_value() || dim->id == -1) {
            return Local<Value>();
        }
        return IntPos::newPos(pos.value(), dim->id);
    }
    CATCH("Fail in getLastDeathPos!")
}

Local<Value> PlayerClass::getXuid() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        string xuid;
        try {
            xuid = player->getXuid();
        } catch (...) {
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().debug("Fail in getXuid!");
            xuid = ll::service::PlayerInfo::getInstance().fromName(player->getRealName())->xuid;
        }
        return String::newString(xuid);
    }
    CATCH("Fail in getXuid!")
}

Local<Value> PlayerClass::getUuid() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        string uuid;
        try {
            uuid = player->getUuid().asString();
        } catch (...) {
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().debug("Fail in getUuid!");
            uuid = ll::service::PlayerInfo::getInstance().fromName(player->getRealName())->uuid.asString();
        }
        return String::newString(uuid);
    }
    CATCH("Fail in getUuid!")
}

Local<Value> PlayerClass::getRealName() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return String::newString(player->getRealName());
    }
    CATCH("Fail in getRealName!")
}

Local<Value> PlayerClass::getIP() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return String::newString(player->getNetworkIdentifier().getAddress());
    }
    CATCH("Fail in GetIP!")
}

Local<Value> PlayerClass::getPermLevel() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Number::newNumber(magic_enum::enum_integer(player->getCommandPermissionLevel()));
    }
    CATCH("Fail in getPlayerPermLevel!")
}

Local<Value> PlayerClass::getGameMode() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Number::newNumber((int)player->getPlayerGameType()); //==========???
    }
    CATCH("Fail in getGameMode!")
}

Local<Value> PlayerClass::getCanSleep() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->canSleep());
    }
    CATCH("Fail in getCanSleep!")
}

Local<Value> PlayerClass::getCanFly() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->canFly());
    }
    CATCH("Fail in getCanFly!")
}

Local<Value> PlayerClass::getCanBeSeenOnMap() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        if (!player->isAlive() || player->isSpectator()) {
            return Boolean::newBoolean(false);
        }
        ItemStack const& item = player->getItemSlot(SharedTypes::Legacy::EquipmentSlot::Legs);
        return Boolean::newBoolean(item.isHumanoidWearableBlockItem());
    }
    CATCH("Fail in getCanBeSeenOnMap!")
}

Local<Value> PlayerClass::getCanFreeze() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->canFreeze());
    }
    CATCH("Fail in getCanFreeze!")
}

Local<Value> PlayerClass::getCanSeeDaylight() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->canSeeDaylight());
    }
    CATCH("Fail in getCanSeeDaylight!")
}

Local<Value> PlayerClass::getCanShowNameTag() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->canShowNameTag());
    }
    CATCH("Fail in getCanShowNameTag!")
}

Local<Value> PlayerClass::getCanStartSleepInBed() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->canStartSleepInBed());
    }
    CATCH("Fail in getCanStartSleepInBed!")
}

Local<Value> PlayerClass::getCanPickupItems() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->mCanPickupItems);
    }
    CATCH("Fail in getCanPickupItems!")
}

Local<Value> PlayerClass::isSneaking() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(
            SynchedActorDataAccess::getActorFlag(player->getEntityContext(), ActorFlags::Sneaking)
        );
    }
    CATCH("Fail in isSneaking!")
}

Local<Value> PlayerClass::getSpeed() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Number::newNumber(player->getPosDeltaPerSecLength());
    }
    CATCH("Fail in getSpeed!")
}

Local<Value> PlayerClass::getDirection() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        // getRotation()
        Vec2 vec = player->mBuiltInComponents->mActorRotationComponent->mRotationDegree;
        return DirectionAngle::newAngle(vec.x, vec.y);
    }
    CATCH("Fail in getDirection!")
}

Local<Value> PlayerClass::getMaxHealth() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Number::newNumber(player->getMaxHealth());
    }
    CATCH("Fail in GetMaxHealth!")
}

Local<Value> PlayerClass::getHealth() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Number::newNumber(ActorAttribute::getHealth(player->getEntityContext()));
    }
    CATCH("Fail in GetHealth!")
}

Local<Value> PlayerClass::getInAir() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(!player->isOnGround() && !player->isInWater());
    }
    CATCH("Fail in GetInAir!")
}

Local<Value> PlayerClass::getInWater() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->isInWater());
    }
    CATCH("Fail in getInWater!")
}

Local<Value> PlayerClass::getInLava() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(
            ActorMobilityUtils::shouldApplyLava(player->getDimensionBlockSourceConst(), player->getEntityContext())
        );
    }
    CATCH("Fail in getInLava!")
}

Local<Value> PlayerClass::getInRain() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->isInRain());
    }
    CATCH("Fail in getInRain!")
}

Local<Value> PlayerClass::getInSnow() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->isInSnow());
    }
    CATCH("Fail in getInSnow!")
}

Local<Value> PlayerClass::getInWall() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        // The original Actor::isInWall() was moved to MobSuffocationSystemImpl::isInWall() in 1.21.60.10, but the later
        // needs too many parameters.
        return Boolean::newBoolean(player->getDimensionBlockSource().isInWall(
            player->getAttachPos(SharedTypes::Legacy::ActorLocation::BreathingPoint)
        ));
    }
    CATCH("Fail in getInWall!")
}

Local<Value> PlayerClass::getInWaterOrRain() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->isInWaterOrRain());
    }
    CATCH("Fail in getInWaterOrRain!")
}

Local<Value> PlayerClass::getInWorld() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->isInWorld());
    }
    CATCH("Fail in getInWorld!")
}

Local<Value> PlayerClass::getInClouds() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        short cloudHeight = player->getDimension().getCloudHeight();
        float y           = player->getPosition().y;
        return Boolean::newBoolean(y > cloudHeight && y < cloudHeight + 4.0f);
    }
    CATCH("Fail in getInClouds!")
}

Local<Value> PlayerClass::getUniqueID() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        else return String::newString(std::to_string(player->getOrCreateUniqueID().rawID));
    }
    CATCH("Fail in getUniqueID!")
}

Local<Value> PlayerClass::getRuntimeID() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        else return String::newString(std::to_string(player->getRuntimeID().rawID));
    }
    CATCH("Fail in getUniqueID!")
}

Local<Value> PlayerClass::getLangCode() {
    try {
        Json::Value& requestJson = get()->getConnectionRequest()->mRawToken->mDataInfo;

        return String::newString(requestJson.get("LanguageCode", "unknown").asString("unknown"));
    }
    CATCH("Fail in getLangCode!");
}

Local<Value> PlayerClass::isLoading() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isLoading());
    }
    CATCH("Fail in isLoading!")
}

Local<Value> PlayerClass::isInvisible() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isInvisible());
    }
    CATCH("Fail in isInvisible!")
}

Local<Value> PlayerClass::isInsidePortal() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        auto component = player->getEntityContext().tryGetComponent<InsideBlockComponent>();
        if (component) {
            auto& fullName = component->mInsideBlock->getLegacyBlock().mNameInfo->mFullName;
            return Boolean::newBoolean(
                *fullName == VanillaBlockTypeIds::Portal() || *fullName == VanillaBlockTypeIds::EndPortal()
            );
        }
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in isInsidePortal!")
}

Local<Value> PlayerClass::isHurt() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        int health = ActorAttribute::getHealth(player->getEntityContext());
        if (health > 0 && health < player->getMaxHealth()) {
            return Boolean::newBoolean(true);
        }
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in isHurt!")
}

Local<Value> PlayerClass::isTrusting() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(
            SynchedActorDataAccess::getActorFlag(player->getEntityContext(), ActorFlags::Trusting)
        );
    }
    CATCH("Fail in isTrusting!")
}

Local<Value> PlayerClass::isTouchingDamageBlock() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isTouchingDamageBlock());
    }
    CATCH("Fail in isTouchingDamageBlock!")
}

Local<Value> PlayerClass::isHungry() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        auto attribute = player->getAttribute(Player::HUNGER());
        return Boolean::newBoolean(attribute.mCurrentMaxValue > attribute.mCurrentValue);
    }
    CATCH("Fail in isHungry!")
}

Local<Value> PlayerClass::isOnFire() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isOnFire());
    }
    CATCH("Fail in isOnFire!")
}

Local<Value> PlayerClass::isOnGround() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isOnGround());
    }
    CATCH("Fail in isOnGround!")
}

Local<Value> PlayerClass::isOnHotBlock() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->getEntityContext().hasComponent<IsOnHotBlockFlagComponent>());
    }
    CATCH("Fail in isOnHotBlock!")
}

Local<Value> PlayerClass::isTrading() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isTrading());
    }
    CATCH("Fail in isTrading!")
}

Local<Value> PlayerClass::isAdventure() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isAdventure());
    }
    CATCH("Fail in isAdventure!")
}

Local<Value> PlayerClass::isGliding() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->getStatusFlag(ActorFlags::Gliding));
    }
    CATCH("Fail in isGliding!")
}

Local<Value> PlayerClass::isSurvival() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isSurvival());
    }
    CATCH("Fail in isSurvival!")
}

Local<Value> PlayerClass::isSpectator() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isSpectator());
    }
    CATCH("Fail in isSpectator!")
}

Local<Value> PlayerClass::isRiding() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isRiding());
    }
    CATCH("Fail in isRiding!")
}

Local<Value> PlayerClass::isDancing() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isDancing());
    }
    CATCH("Fail in isDancing!")
}

Local<Value> PlayerClass::isCreative() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isCreative());
    }
    CATCH("Fail in isCreative!")
}

Local<Value> PlayerClass::isFlying() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isFlying());
    }
    CATCH("Fail in isFlying!")
}

Local<Value> PlayerClass::isSleeping() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isSleeping());
    }
    CATCH("Fail in isSleeping!")
}

Local<Value> PlayerClass::isMoving() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(SynchedActorDataAccess::getActorFlag(player->getEntityContext(), ActorFlags::Moving)
        );
    }
    CATCH("Fail in isMoving!")
}

Local<Value> PlayerClass::teleport(const Arguments& args) {
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
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = *posObj;
                }
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Boolean::newBoolean(false);
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
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Boolean::newBoolean(false);
        }
        if (!rotationIsValid) {
            angle = player->mBuiltInComponents->mActorRotationComponent->mRotationDegree;
        }
        player->teleport(pos.getVec3(), pos.dim, angle);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in TeleportPlayer!")
}

Local<Value> PlayerClass::kill(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->kill();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in KillPlayer!")
}

Local<Value> PlayerClass::isOP(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->isOperator());
    }
    CATCH("Fail in IsOP!")
}

Local<Value> PlayerClass::setPermLevel(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        bool res     = false;
        int  newPerm = args[0].asNumber().toInt32();
        if (newPerm >= 0 && newPerm <= 4) {
            RecordOperation(
                getEngineOwnData()->pluginName,
                "Set Permission Level",
                fmt::format("Set Player {} Permission Level as {}.", player->getRealName(), newPerm)
            );
            player->getAbilities().mPermissions->mCommandPermissions = (CommandPermissionLevel)newPerm;
            if (newPerm >= 1) {
                player->getAbilities().setPlayerPermissions(PlayerPermissionLevel::Operator);
            } else {
                player->getAbilities().setPlayerPermissions(PlayerPermissionLevel::Member);
            }
            UpdateAbilitiesPacket uPkt(player->getOrCreateUniqueID(), player->getAbilities());
            player->sendNetworkPacket(uPkt);
            res = true;
        }
        return Boolean::newBoolean(res);
    }
    CATCH("Fail in setPlayerPermLevel!");
}

Local<Value> PlayerClass::setGameMode(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        bool res     = false;
        int  newMode = args[0].asNumber().toInt32();
        if ((newMode >= 0 && newMode <= 2) || (newMode >= 5 && newMode <= 6)) {
            player->setPlayerGameType((GameType)newMode);
            res = true;
        }
        return Boolean::newBoolean(res);
    }
    CATCH("Fail in setGameMode!");
}

Local<Value> PlayerClass::runcmd(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
        CommandContext context = CommandContext(
            args[0].asString().toString(),
            std::make_unique<PlayerCommandOrigin>(*get()),
            CommandVersion::CurrentVersion()
        );
        ll::service::getMinecraft()->mCommands->executeCommand(context, false);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in runcmd!");
}

Local<Value> PlayerClass::kick(const Arguments& args) {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        string msg = "disconnectionScreen.disconnected";
        if (args.size() >= 1) msg = args[0].asString().toString();

        player->disconnect(msg);
        return Boolean::newBoolean(true); //=======???
    }
    CATCH("Fail in kickPlayer!");
}

Local<Value> PlayerClass::tell(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        TextPacketType type = TextPacketType::Raw;
        if (args.size() >= 2 && args[1].isNumber()) {
            int newType = args[1].asNumber().toInt32();
            if (newType >= 0 && newType <= 11) type = (TextPacketType)newType;
        }

        TextPacket pkt = TextPacket();
        pkt.mType      = type;
        pkt.mMessage.assign(args[0].asString().toString());
        player->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in tell!");
}

Local<Value> PlayerClass::setTitle(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

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
            type = (SetTitlePacket::TitleType)args[1].asNumber().toInt32();
        }
        if (args.size() >= 5) {
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[4], ValueKind::kNumber);
            fadeInTime  = args[2].asNumber().toInt32();
            stayTime    = args[3].asNumber().toInt32();
            fadeOutTime = args[4].asNumber().toInt32();
        }

        SetTitlePacket pkt = SetTitlePacket(type, content, std::nullopt);
        pkt.mFadeInTime    = fadeInTime;
        pkt.mStayTime      = stayTime;
        pkt.mFadeOutTime   = fadeOutTime;
        player->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setTitle!");
}

Local<Value> PlayerClass::talkAs(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
        if (ll::service::getLevel().has_value()) {
            auto                       msg = args[0].asString().toString();
            ll::event::PlayerChatEvent event{*reinterpret_cast<ServerPlayer*>(player), msg};
            ll::event::EventBus::getInstance().publish(event);
            if (event.isCancelled()) return Boolean::newBoolean(false);
            TextPacket pkt = TextPacket::createChat(player->getRealName(), msg, {}, player->getXuid(), {});
            ll::service::getLevel()->forEachPlayer([&pkt](Player& player) {
                player.sendNetworkPacket(pkt);
                return true;
            });
        } else {
            Boolean::newBoolean(false);
        }
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in talkAs!");
}

Local<Value> PlayerClass::talkTo(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* target = PlayerClass::extract(args[1]);
        if (!target) return Local<Value>();
        Player* player = get();
        if (!player) return Local<Value>();

        TextPacket pkt =
            TextPacket::createWhisper(player->getRealName(), args[0].asString().toString(), {}, player->getXuid(), {});
        target->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in talkTo!");
}

Local<Value> PlayerClass::getHand(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return ItemClass::newItem(&const_cast<ItemStack&>(player->getSelectedItem()));
    }
    CATCH("Fail in getHand!");
}

Local<Value> PlayerClass::getOffHand(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return ItemClass::newItem(const_cast<ItemStack*>(&player->getOffhandSlot()));
    }
    CATCH("Fail in getOffHand!");
}

Local<Value> PlayerClass::getInventory(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return ContainerClass::newContainer(player->mInventory->mInventory.get());
    }
    CATCH("Fail in getInventory!");
}

Local<Value> PlayerClass::getArmor(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return ContainerClass::newContainer(&ActorEquipment::getArmorContainer(player->getEntityContext()));
    }
    CATCH("Fail in getArmor!");
}

Local<Value> PlayerClass::getEnderChest(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        auto chest = player->getEnderChestContainer();
        if (chest) {
            return ContainerClass::newContainer(chest);
        }
        return {};
    }
    CATCH("Fail in getEnderChest!");
}

Local<Value> PlayerClass::getRespawnPosition(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        BlockPos      position = player->getExpectedSpawnPosition();
        DimensionType dim      = player->getExpectedSpawnDimensionId();
        return IntPos::newPos(position, dim);
    }
    CATCH("Fail in getRespawnPosition!")
}

Local<Value> PlayerClass::setRespawnPosition(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        IntVec4 pos;
        if (args.size() == 1) {
            // IntPos
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = posObj->toIntVec4();
                }
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
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
            LOG_WRONG_ARGS_COUNT(__FUNCTION__);
            return Local<Value>();
        }
        player->setRespawnPosition(pos.getBlockPos(), pos.dim);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setRespawnPosition!")
}

Local<Value> PlayerClass::refreshItems(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->refreshInventory();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in refreshItems!");
}

Local<Value> PlayerClass::rename(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
        player->setNameTag(args[0].asString().toString());
        player->_sendDirtyActorData();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in RenamePlayer!");
}

Local<Value> PlayerClass::addLevel(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->addLevels(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in addLevel!");
}

Local<Value> PlayerClass::reduceLevel(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        player->addLevels(-args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in reduceLevel!");
}

Local<Value> PlayerClass::getLevel(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Number::newNumber(player->getAttribute(Player::LEVEL()).mCurrentValue);
    }
    CATCH("Fail in getLevel!")
}

Local<Value> PlayerClass::setLevel(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->addLevels(args[0].asNumber().toInt32() - (int)player->getAttribute(Player::LEVEL()).mCurrentValue);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setLevel!");
}

Local<Value> PlayerClass::setScale(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        SynchedActorDataAccess::setBoundingBoxScale(player->getEntityContext(), args[0].asNumber().toFloat());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setScale!");
}

Local<Value> PlayerClass::resetLevel(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->resetPlayerLevel();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in resetLevel!")
}

Local<Value> PlayerClass::addExperience(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->addExperience(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in addExperience!");
}

Local<Value> PlayerClass::reduceExperience(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        float exp       = args[0].asNumber().toFloat();
        auto  attribute = player->getMutableAttribute(Player::EXPERIENCE());
        auto  instance  = attribute.mInstance;
        if (!instance) {
            return Boolean::newBoolean(false);
        }
        int neededExp  = player->getXpNeededForNextLevel();
        int currentExp = static_cast<int>(instance->mCurrentValue * neededExp);
        if (exp <= currentExp) {
            AttributeHelper::setCurrentValue(attribute, static_cast<float>(currentExp - exp) / neededExp);
            return Boolean::newBoolean(true);
        }
        AttributeHelper::setCurrentValue(attribute, 0.0f);
        size_t needExp = exp - currentExp;
        int    level   = player->getAttribute(Player::LEVEL()).mCurrentValue;
        while (level > 0) {
            player->addLevels(-1);
            int levelXp = player->getXpNeededForNextLevel();
            if (needExp < levelXp) {
                AttributeHelper::setCurrentValue(attribute, static_cast<float>(levelXp - needExp) / levelXp);
                return Boolean::newBoolean(true);
            }
            needExp -= levelXp;
            level    = player->getAttribute(Player::LEVEL()).mCurrentValue;
        }
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in reduceExperience!");
}

Local<Value> PlayerClass::getCurrentExperience(const Arguments&) {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Number::newNumber((long long)PlayerHelper::getXpEarnedAtCurrentLevel(player));
    }
    CATCH("Fail in getCurrentExperience!")
}

Local<Value> PlayerClass::setCurrentExperience(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        PlayerHelper::setXpEarnedAtCurrentLevel(player, args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setCurrentExperience!");
}

Local<Value> PlayerClass::getTotalExperience(const Arguments&) {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        int          startLevel = 0;
        int          endLevel   = (int)player->getAttribute(Player::LEVEL()).mCurrentValue;
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
        return Number::newNumber((long long)(totalXp + PlayerHelper::getXpEarnedAtCurrentLevel(player)));
    }
    CATCH("Fail in getTotalExperience!")
}

Local<Value> PlayerClass::setTotalExperience(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }
        player->resetPlayerLevel();
        player->addExperience(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setTotalExperience!");
}

Local<Value> PlayerClass::getXpNeededForNextLevel(const Arguments&) {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Number::newNumber(player->getXpNeededForNextLevel());
    }
    CATCH("Fail in getXpNeededForNextLevel!")
}

Local<Value> PlayerClass::transServer(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber)

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        TransferPacket packet(args[0].asString().toString(), args[1].asNumber().toInt32());
        player->sendNetworkPacket(packet);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in transServer!");
}

Local<Value> PlayerClass::crash(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
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
    CATCH("Fail in crashPlayer!");
}

Local<Value> PlayerClass::getBlockStandingOn(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return BlockClass::newBlock(player->getBlockPosCurrentlyStandingOn(nullptr), player->getDimensionId());
    }
    CATCH("Fail in getBlockStandingOn!");
}

Local<Value> PlayerClass::getDevice(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        return DeviceClass::newDevice(player);
    }
    CATCH("Fail in getDevice!");
}

Local<Value> PlayerClass::getScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  obj        = scoreboard.getObjective(args[0].asString().toString());
        if (!obj) {
            throw std::invalid_argument("Objective " + args[0].asString().toString() + " not found");
        }
        const ScoreboardId& id = scoreboard.getScoreboardId(*player);
        if (id.mRawID == ScoreboardId::INVALID().mRawID) { // !isValid
            scoreboard.createScoreboardId(*player);
        }
        return Number::newNumber(obj->getPlayerScore(id).mValue);
    }
    CATCH("Fail in getScore!");
}

Local<Value> PlayerClass::setScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  obj        = scoreboard.getObjective(args[0].asString().toString());
        if (!obj) {
            obj = scoreboard.addObjective(
                args[0].asString().toString(),
                args[0].asString().toString(),
                *scoreboard.getCriteria(Scoreboard::DEFAULT_CRITERIA())
            );
        }
        const ScoreboardId& id = scoreboard.getScoreboardId(*player);
        if (id.mRawID == ScoreboardId::INVALID().mRawID) {
            scoreboard.createScoreboardId(*player);
        }
        ScoreboardOperationResult isSuccess;
        scoreboard.modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Set);
        return Boolean::newBoolean(isSuccess == ScoreboardOperationResult::Success);
    }
    CATCH("Fail in setScore!");
}

Local<Value> PlayerClass::addScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  obj        = scoreboard.getObjective(args[0].asString().toString());
        if (!obj) {
            return Boolean::newBoolean(false);
        }
        const ScoreboardId& id = scoreboard.getScoreboardId(*player);
        if (id.mRawID == ScoreboardId::INVALID().mRawID) {
            scoreboard.createScoreboardId(*player);
        }
        ScoreboardOperationResult isSuccess;
        scoreboard.modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Add);
        return Boolean::newBoolean(isSuccess == ScoreboardOperationResult::Success);
    }
    CATCH("Fail in addScore!");
}

Local<Value> PlayerClass::reduceScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  obj        = scoreboard.getObjective(args[0].asString().toString());
        if (!obj) {
            return Boolean::newBoolean(false);
        }
        const ScoreboardId& id = scoreboard.getScoreboardId(*player);
        if (id.mRawID == ScoreboardId::INVALID().mRawID) {
            scoreboard.createScoreboardId(*player);
        }
        ScoreboardOperationResult isSuccess;
        scoreboard
            .modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Subtract);
        return Boolean::newBoolean(isSuccess == ScoreboardOperationResult::Success);
    }
    CATCH("Fail in reduceScore!");
}

Local<Value> PlayerClass::deleteScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  obj        = scoreboard.getObjective(args[0].asString().toString());
        if (!obj) {
            return Boolean::newBoolean(false);
        }
        const ScoreboardId& id = scoreboard.getScoreboardId(*player);
        if (id.mRawID == ScoreboardId::INVALID().mRawID) {
            return Boolean::newBoolean(true);
        }
        scoreboard.resetPlayerScore(id, *obj);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in deleteScore!");
}

Local<Value> PlayerClass::setSidebar(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kObject);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        std::vector<std::pair<std::string, int>> data;
        auto                                     source = args[1].asObject();
        auto                                     keys   = source.getKeyNames();
        for (auto& key : keys) {
            data.push_back(make_pair(key, source.get(key).asNumber().toInt32()));
        }

        int sortOrder = 1;
        if (args.size() >= 3) sortOrder = args[2].asNumber().toInt32();

        SetDisplayObjectivePacket disObjPkt;
        disObjPkt.mDisplaySlotName      = "sidebar";
        disObjPkt.mObjectiveName        = "FakeScoreObj";
        disObjPkt.mObjectiveDisplayName = args[0].asString().toString();
        disObjPkt.mCriteriaName         = "dummy";
        disObjPkt.mSortOrder            = (ObjectiveSortOrder)sortOrder;
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
    CATCH("Fail in setSidebar!")
}

Local<Value> PlayerClass::removeSidebar(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        RemoveObjectivePacket pkt;
        pkt.mObjectiveName = "FakeScoreObj";
        player->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in removeSidebar!")
}

Local<Value> PlayerClass::setBossBar(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    if (args[0].getKind() == ValueKind::kNumber) {
        CHECK_ARGS_COUNT(args, 4);
        CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
        CHECK_ARG_TYPE(args[1], ValueKind::kString);
        CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
        CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
        try {
            Player* player = get();
            if (!player) return Local<Value>();

            int64_t uid     = args[0].asNumber().toInt64();
            int     percent = args[2].asNumber().toInt32();
            if (percent < 0) percent = 0;
            else if (percent > 100) percent = 100;
            float value = (float)percent / 100;

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

            BossBarColor color = (BossBarColor)args[3].asNumber().toInt32();
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
        CATCH("Fail in addBossBar!")
    }
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        int percent = args[1].asNumber().toInt32();
        if (percent < 0) percent = 0;
        else if (percent > 100) percent = 100;
        float value = (float)percent / 100;

        BossBarColor color = BossBarColor::Red;
        if (args.size() >= 3) color = (BossBarColor)args[2].asNumber().toInt32();
        auto pkt = static_pointer_cast<BossEventPacket>(MinecraftPackets::createPacket(MinecraftPacketIds::BossEvent));
        pkt->mEventType     = BossEventUpdateType::Add;
        pkt->mName          = args[0].asString().toString();
        pkt->mHealthPercent = value;
        pkt->mColor         = color;
        pkt->sendTo(*player);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setBossBar!")
}

Local<Value> PlayerClass::removeBossBar(const Arguments& args) {
    if (args.size() == 0) {
        try {
            Player* player = get();
            if (!player) return Local<Value>();

            auto pkt =
                static_pointer_cast<BossEventPacket>(MinecraftPackets::createPacket(MinecraftPacketIds::BossEvent));
            pkt->mEventType = BossEventUpdateType::Remove;
            pkt->mColor     = BossBarColor::Red;
            pkt->sendTo(*player);
            return Boolean::newBoolean(true);
        }
        CATCH("Fail in removeBossBar!")
    } else {
        CHECK_ARGS_COUNT(args, 1);
        CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
        try {
            Player* player = get();
            if (!player) return Local<Value>();
            int64_t uid = args[0].asNumber().toInt64();
            auto    pkt =
                static_pointer_cast<BossEventPacket>(MinecraftPackets::createPacket(MinecraftPacketIds::BossEvent));
            pkt->mBossID    = ActorUniqueID(uid);
            pkt->mEventType = BossEventUpdateType::Remove;
            pkt->sendTo(*player);
            return Boolean::newBoolean(true);
        }
        CATCH("Fail in removeBossBar!")
    }
}

Local<Value> PlayerClass::sendSimpleForm(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 5);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kArray);
    CHECK_ARG_TYPE(args[3], ValueKind::kArray);
    CHECK_ARG_TYPE(args[4], ValueKind::kFunction);
    if (args.size() > 5) CHECK_ARG_TYPE(args[5], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
        bool update = args.size() > 5 ? args[5].asBoolean().value() : false;

        // 普通格式
        auto textsArr = args[2].asArray();
        if (textsArr.size() == 0 || !textsArr.get(0).isString()) return Local<Value>();
        auto imagesArr = args[3].asArray();
        if (imagesArr.size() != textsArr.size() || !imagesArr.get(0).isString()) return Local<Value>();

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
                             callback{script::Global(args[4].asFunction())
                             }](Player& pl, int chosen, ll::form::FormCancelReason reason) {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running)) return;
            if (!EngineManager::isValid(engine)) return;

            EngineScope scope(engine);
            try {
                callback.get().call(
                    {},
                    PlayerClass::newPlayer(&pl),
                    chosen >= 0 ? Number::newNumber(chosen) : Local<Value>(),
                    reason.has_value() ? Number::newNumber((uchar)reason.value()) : Local<Value>()
                );
            }
            CATCH_IN_CALLBACK("sendSimpleForm")
        };
        if (update) form.sendUpdate(*player, std::move(formCallback));
        else form.sendTo(*player, std::move(formCallback));

        return Number::newNumber(1);
    }
    CATCH("Fail in sendSimpleForm!");
}

Local<Value> PlayerClass::sendModalForm(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 5);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kString);
    CHECK_ARG_TYPE(args[3], ValueKind::kString);
    CHECK_ARG_TYPE(args[4], ValueKind::kFunction);
    if (args.size() > 5) CHECK_ARG_TYPE(args[5], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
        bool update = args.size() > 5 ? args[5].asBoolean().value() : false;

        ll::form::ModalForm form(
            args[0].asString().toString(),
            args[1].asString().toString(),
            args[2].asString().toString(),
            args[3].asString().toString()
        );
        auto formCallback = [engine{EngineScope::currentEngine()},
                             callback{script::Global(args[4].asFunction())
                             }](Player& pl, ll::form::ModalFormResult const& chosen, ll::form::FormCancelReason reason
                            ) {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running)) return;
            if (!EngineManager::isValid(engine)) return;

            EngineScope scope(engine);
            try {
                callback.get().call(
                    {},
                    PlayerClass::newPlayer(&pl),
                    chosen ? Boolean::newBoolean(static_cast<bool>(*chosen)) : Local<Value>(),
                    reason.has_value() ? Number::newNumber((uchar)reason.value()) : Local<Value>()
                );
            }
            CATCH_IN_CALLBACK("sendModalForm")
        };

        if (update) form.sendUpdate(*player, std::move(formCallback));
        else form.sendTo(*player, std::move(formCallback));

        return Number::newNumber(2);
    }
    CATCH("Fail in sendModalForm!");
}

Local<Value> PlayerClass::sendCustomForm(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);
    if (args.size() > 2) CHECK_ARG_TYPE(args[2], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
        bool update = args.size() > 2 ? args[2].asBoolean().value() : false;

        auto formData = ordered_json::parse(args[0].asString().toString());
        auto formCallback =
            [id{player->getOrCreateUniqueID()},
             engine{EngineScope::currentEngine()},
             callback{script::Global(args[1].asFunction())},
             formData](Player& player, std::optional<std::string> const& data, ll::form::FormCancelReason reason) {
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
                        reason.has_value() ? Number::newNumber((uchar)reason.value()) : Local<Value>()
                    );
                }
                CATCH_IN_CALLBACK("sendCustomForm")
            };
        if (update) ll::form::Form::sendRawUpdate(*player, formData.dump(), std::move(formCallback));
        else ll::form::Form::sendRawTo(*player, formData.dump(), std::move(formCallback));

        return Number::newNumber(3);
    } catch (const ordered_json::exception& e) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to parse Json string in sendCustomForm!"
        );
        ll::error_utils::printException(e, lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
        return {};
    }
    CATCH("Fail in sendCustomForm!");
}

Local<Value> PlayerClass::sendForm(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);
    if (args.size() > 2) CHECK_ARG_TYPE(args[2], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
        bool update = args.size() > 2 ? args[2].asBoolean().value() : false;

        if (IsInstanceOf<SimpleFormClass>(args[0])) {
            Local<Function> callback = args[1].asFunction();
            SimpleFormClass::sendForm(SimpleFormClass::extract(args[0]), player, callback, update);
        } else if (IsInstanceOf<CustomFormClass>(args[0])) {
            Local<Function> callback = args[1].asFunction();
            CustomFormClass::sendForm(CustomFormClass::extract(args[0]), player, callback, update);
        } else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in sendForm!");
}

Local<Value> PlayerClass::closeForm(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        ClientboundCloseFormPacket().sendTo(*player);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in sendForm!");
}

Local<Value> PlayerClass::sendPacket(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kObject);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
        auto pkt = PacketClass::extract(args[0]);
        if (!pkt) return Boolean::newBoolean(false);
        player->sendNetworkPacket(*pkt);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in sendPacket");
}

Local<Value> PlayerClass::setExtraData(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        string key = args[0].asString().toString();
        if (key.empty()) return Boolean::newBoolean(false);

        getEngineOwnData()->playerDataDB[player->getRealName() + "-" + key] = args[1];
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setExtraData!");
}

Local<Value> PlayerClass::getExtraData(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        string key = args[0].asString().toString();
        if (key.empty()) return Local<Value>();

        auto& db  = getEngineOwnData()->playerDataDB;
        auto  res = db.find(player->getRealName() + "-" + key);
        if (res == db.end() || res->second.isEmpty()) return Local<Value>();
        else return res->second.get();
    }
    CATCH("Fail in getExtraData!");
}

Local<Value> PlayerClass::delExtraData(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        string key = args[0].asString().toString();
        if (key.empty()) return Boolean::newBoolean(false);

        getEngineOwnData()->playerDataDB.erase(player->getRealName() + "-" + key);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in delExtraData!")
}

Local<Value> PlayerClass::hurt(const Arguments& args) {
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
                ActorDamageByActorSource(*source, (SharedTypes::Legacy::ActorDamageCause)type);
            return Boolean::newBoolean(player->_hurt(damageBySource, damage, true, false));
        }
        ActorDamageSource damageSource;
        damageSource.mCause = (SharedTypes::Legacy::ActorDamageCause)type;
        return Boolean::newBoolean(player->_hurt(damageSource, damage, true, false));
    }
    CATCH("Fail in hurt!");
}

Local<Value> PlayerClass::heal(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->heal(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in heal!");
}

Local<Value> PlayerClass::setHealth(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto attribute = player->getMutableAttribute(SharedAttributes::HEALTH());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setHealth!");
}

Local<Value> PlayerClass::setMaxHealth(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto attribute = player->getMutableAttribute(SharedAttributes::HEALTH());
        AttributeHelper::setMaxValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setMaxHealth!");
}

Local<Value> PlayerClass::setAbsorption(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto attribute = player->getMutableAttribute(SharedAttributes::ABSORPTION());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setAbsorptionAttribute!");
}

Local<Value> PlayerClass::setAttackDamage(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto attribute = player->getMutableAttribute(SharedAttributes::ATTACK_DAMAGE());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setAttackDamage!");
}

Local<Value> PlayerClass::setMaxAttackDamage(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto attribute = player->getMutableAttribute(SharedAttributes::ATTACK_DAMAGE());
        AttributeHelper::setMaxValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setMaxAttackDamage!");
}

Local<Value> PlayerClass::setFollowRange(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto attribute = player->getMutableAttribute(SharedAttributes::FOLLOW_RANGE());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setFollowRange!");
}

Local<Value> PlayerClass::setKnockbackResistance(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto attribute = player->getMutableAttribute(SharedAttributes::KNOCKBACK_RESISTANCE());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setKnockbackResistance!");
}

Local<Value> PlayerClass::setLuck(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto attribute = player->getMutableAttribute(SharedAttributes::LUCK());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setLuck!");
}

Local<Value> PlayerClass::setMovementSpeed(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto attribute = player->getMutableAttribute(SharedAttributes::MOVEMENT_SPEED());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(false);
    }
    CATCH("Fail in setMovementSpeed!");
}

Local<Value> PlayerClass::setUnderwaterMovementSpeed(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto attribute = player->getMutableAttribute(SharedAttributes::UNDERWATER_MOVEMENT_SPEED());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setUnderwaterMovementSpeed!");
}

Local<Value> PlayerClass::setLavaMovementSpeed(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto attribute = player->getMutableAttribute(SharedAttributes::LAVA_MOVEMENT_SPEED());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setLavaMovementSpeed!");
}

Local<Value> PlayerClass::setHungry(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto attribute = player->getMutableAttribute(Player::HUNGER());
        AttributeHelper::setCurrentValue(attribute, args[0].asNumber().toFloat());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setHungry!");
}

Local<Value> PlayerClass::setFire(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        int  time          = args[0].asNumber().toInt32();
        bool isEffectValue = args[1].asBoolean().value();

        player->setOnFire(time, isEffectValue);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setFire!");
}

Local<Value> PlayerClass::stopFire(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->stopFire();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in stopFire!");
}

// For Compatibility
Local<Value> PlayerClass::setOnFire(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        int time = args[0].asNumber().toInt32();

        player->setOnFire(time, true);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setOnFire!");
}

Local<Value> PlayerClass::refreshChunks(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->mChunkPublisherView->clearRegion();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in refreshChunks!");
}

Local<Value> PlayerClass::giveItem(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        ItemStack* item = ItemClass::extract(args[0]);
        if (!item) return Local<Value>(); // Null
        if (args.size() >= 2) {
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            item->set(args[1].asNumber().toInt32());
        }
        bool result = player->add(*item);
        if (!result) {
            player->drop(*item, false);
        }
        player->sendInventory(true);
        return Boolean::newBoolean(result);
    }
    CATCH("Fail in giveItem!");
}

Local<Value> PlayerClass::clearItem(const Arguments& args) {
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
                        container.setItem((int)slot, ItemStack::EMPTY_ITEM());
                    } else {
                        result += clearCount;
                        container.removeItem((int)slot, clearCount);
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
    CATCH("Fail in clearItem!");
}

Local<Value> PlayerClass::isSprinting(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->getStatusFlag(ActorFlags::Sprinting));
    }
    CATCH("Fail in isSprinting!");
}

Local<Value> PlayerClass::setSprinting(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->setSprinting(args[0].asBoolean().value());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setSprinting!");
}

Local<Value> PlayerClass::getNbt(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        std::unique_ptr<CompoundTag> tag = std::make_unique<CompoundTag>();
        player->save(*tag);
        return NbtCompoundClass::pack(std::move(tag));
    }
    CATCH("Fail in getNbt!")
}

Local<Value> PlayerClass::setNbt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto nbt = NbtCompoundClass::extract(args[0]);
        if (!nbt) {
            return Local<Value>();
        }
        return Boolean::newBoolean(player->load(*nbt, MoreGlobal::defaultDataLoadHelper()));
    }
    CATCH("Fail in setNbt!")
}

Local<Value> PlayerClass::addTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->addTag(args[0].asString().toString()));
    }
    CATCH("Fail in addTag!");
}

Local<Value> PlayerClass::removeTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->removeTag(args[0].asString().toString()));
    }
    CATCH("Fail in removeTag!");
}

Local<Value> PlayerClass::hasTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->hasTag(args[0].asString().toString()));
    }
    CATCH("Fail in hasTag!");
}

Local<Value> PlayerClass::getAllTags(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        Local<Array> arr       = Array::newArray();
        auto         component = player->getEntityContext().tryGetComponent<TagsComponent<IDType<LevelTagSetIDType>>>();
        if (component) {
            for (auto& tag : get()->getLevel().getTagRegistry().getTagsInSet(component->mTagSetID)) {
                arr.add(String::newString(tag));
            }
            return arr;
        }
        return Local<Value>();
    }
    CATCH("Fail in getAllTags!");
}

Local<Value> PlayerClass::getAbilities(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        CompoundTag tag;
        player->save(tag);

        try {
            return Tag2Value(&tag.at("abilities").get<CompoundTag>(), true);
        } catch (...) {
            return Object::newObject();
        }
    }
    CATCH("Fail in getAbilities!");
}

Local<Value> PlayerClass::getAttributes(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        Local<Array> res = Array::newArray();

        CompoundTag tag = CompoundTag();
        player->save(tag);
        try {
            Local<Array> arr = Array::newArray();
            tag.at("Attributes").get<ListTag>().forEachCompoundTag([&](const CompoundTag& tag) {
                arr.add(Tag2Value(&const_cast<CompoundTag&>(tag), true));
            });
            return arr;
        } catch (...) {
            return Array::newArray();
        }
    }
    CATCH("Fail in getAttributes!");
}

Local<Value> PlayerClass::getEntityFromViewVector(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
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
        return Local<Value>();
    }
    CATCH("Fail in getEntityFromViewVector!");
}

Local<Value> PlayerClass::getBlockFromViewVector(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
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
                if (solidOnly && !block.mCachedComponentData->mUnkd6c5eb.as<bool>()) {
                    return false;
                }
                if (fullOnly && !block.isSlabBlock()) {
                    return false;
                }
                if (!includeLiquid && BlockUtils::isLiquidSource(block)) {
                    return false;
                }
                return true;
            }
        );
        if (res.mType == HitResultType::NoHit) {
            return Local<Value>();
        }
        BlockPos bp;
        if (includeLiquid && res.mIsHitLiquid) {
            bp = res.mLiquidPos;
        } else {
            bp = res.mBlock;
        }
        Block const&       bl     = player->getDimensionBlockSource().getBlock(bp);
        BlockLegacy const& legacy = bl.getLegacyBlock();
        // isEmpty()
        if (bl.isAir() || (legacy.mProperties == BlockProperty::None && legacy.mMaterial.mType == MaterialType::Any)) {
            return Local<Value>();
        }
        return BlockClass::newBlock(bl, bp, player->getDimensionBlockSource());
    }
    CATCH("Fail in getBlockFromViewVector!");
}

Local<Value> PlayerClass::isSimulatedPlayer(const Arguments&) {
    try {
        Player* actor = get();
        if (!actor) return Local<Value>();
        return Boolean::newBoolean(actor->isSimulatedPlayer());
    }
    CATCH("Fail in isSimulatedPlayer!");
}

Local<Value> PlayerClass::quickEvalMolangScript(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        Player* actor = get();
        if (!actor) return Local<Value>();
        return Number::newNumber(actor->evalMolang(args[0].asString().toString()));
    }
    CATCH("Fail in quickEvalMolangScript!");
}

//////////////////// For LLMoney ////////////////////

Local<Value> PlayerClass::getMoney(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        auto xuid = player->getXuid();
        return xuid.empty() ? Local<Value>() : Number::newNumber(EconomySystem::getMoney(xuid));
    }
    CATCH("Fail in getMoney!");
}

Local<Value> PlayerClass::reduceMoney(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
        auto xuid = player->getXuid();
        return xuid.empty() ? Local<Value>()
                            : Boolean::newBoolean(EconomySystem::reduceMoney(xuid, args[0].asNumber().toInt64()));
    }
    CATCH("Fail in reduceMoney!");
}

Local<Value> PlayerClass::setMoney(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
        auto xuid = player->getXuid();
        return xuid.empty() ? Local<Value>()
                            : Boolean::newBoolean(EconomySystem::setMoney(xuid, args[0].asNumber().toInt64()));
    }
    CATCH("Fail in setMoney!");
}

Local<Value> PlayerClass::addMoney(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
        auto xuid = player->getXuid();
        return xuid.empty() ? Local<Value>()
                            : Boolean::newBoolean(EconomySystem::addMoney(xuid, args[0].asNumber().toInt64()));
    }
    CATCH("Fail in addMoney!");
}

Local<Value> PlayerClass::transMoney(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    // nocheck: args[0] maybe Player or XUID.
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
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
    CATCH("Fail in transMoney!");
}

Local<Value> PlayerClass::getMoneyHistory(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
        auto xuid = player->getXuid();
        return xuid.empty()
                 ? Local<Value>()
                 : objectificationMoneyHistory(EconomySystem::getMoneyHist(xuid, args[0].asNumber().toInt32()));
    }
    CATCH("Fail in getMoneyHistory!");
}

//////////////////// For Compatibility ////////////////////

Local<Value> PlayerClass::getAllItems(const Arguments&) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        const ItemStack&              hand      = player->getCarriedItem();
        const ItemStack&              offHand   = player->getOffhandSlot();
        std::vector<const ItemStack*> inventory = player->mInventory->mInventory->getSlots();
        std::vector<const ItemStack*> armor = ActorEquipment::getArmorContainer(player->getEntityContext()).getSlots();
        std::vector<const ItemStack*> endChest = player->getEnderChestContainer()->getSlots();

        Local<Object> result = Object::newObject();

        // hand
        result.set("hand", ItemClass::newItem(&const_cast<ItemStack&>(hand)));

        // offHand
        result.set("offHand", ItemClass::newItem(&const_cast<ItemStack&>(offHand)));

        // inventory
        Local<Array> inventoryArr = Array::newArray();
        for (const ItemStack* item : inventory) {
            if (item) {
                inventoryArr.add(ItemClass::newItem(const_cast<ItemStack*>(item)));
            }
        }
        result.set("inventory", inventoryArr);

        // armor
        Local<Array> armorArr = Array::newArray();
        for (const ItemStack* item : armor) {
            if (item) {
                armorArr.add(ItemClass::newItem(const_cast<ItemStack*>(item)));
            }
        }
        result.set("armor", armorArr);

        // endChest
        Local<Array> endChestArr = Array::newArray();
        for (const ItemStack* item : endChest) {
            if (item) {
                endChestArr.add(ItemClass::newItem(const_cast<ItemStack*>(item)));
            }
        }
        result.set("endChest", endChestArr);

        return result;
    }
    CATCH("Fail in getAllItems!")
}

Local<Value> PlayerClass::removeItem(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        int inventoryId = args[0].asNumber().toInt32();
        int count       = args[1].asNumber().toInt32();

        auto& container = player->mInventory->mInventory;
        if (inventoryId > container->getContainerSize()) return Boolean::newBoolean(false);
        container->removeItem(inventoryId, count);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in removeItem!")
}

Local<Value> PlayerClass::sendToast(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    try {
        Player* player = get();

        if (!player) return Local<Value>();

        ToastRequestPacket pkt;
        pkt.mTitle   = args[0].asString().toString();
        pkt.mContent = args[1].asString().toString();
        player->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in sendToast!");
}

Local<Value> PlayerClass::distanceTo(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        FloatVec4 pos{};

        Player* player = get();
        if (!player) return Local<Value>();

        if (args.size() == 1) { // pos | player | entity
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Local<Value>();
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Local<Value>();
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<PlayerClass>(args[0]) || IsInstanceOf<EntityClass>(args[0])) {
                // Player or Entity

                Actor* targetActor = EntityClass::tryExtractActor(args[0]);
                if (!targetActor) return Local<Value>();

                Vec3 targetActorPos = targetActor->getPosition();

                pos.x   = targetActorPos.x;
                pos.y   = targetActorPos.y;
                pos.z   = targetActorPos.z;
                pos.dim = targetActor->getDimensionId();
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
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
            LOG_WRONG_ARGS_COUNT(__FUNCTION__);
            return Local<Value>();
        }

        if (player->getDimensionId().id != pos.dim) return Number::newNumber(INT_MAX);

        return Number::newNumber(player->distanceTo(pos.getVec3()));
    }
    CATCH("Fail in distanceTo!")
}

Local<Value> PlayerClass::distanceToSqr(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        FloatVec4 pos;

        Player* player = get();
        if (!player) return Local<Value>();

        if (args.size() == 1) {
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Local<Value>();
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Local<Value>();
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<PlayerClass>(args[0]) || IsInstanceOf<EntityClass>(args[0])) {
                // Player or Entity

                Actor* targetActor = EntityClass::tryExtractActor(args[0]);
                if (!targetActor) return Local<Value>();

                Vec3 targetActorPos = targetActor->getPosition();

                pos.x   = targetActorPos.x;
                pos.y   = targetActorPos.y;
                pos.z   = targetActorPos.z;
                pos.dim = targetActor->getDimensionId();
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
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
            LOG_WRONG_ARGS_COUNT(__FUNCTION__);
            return Local<Value>();
        }

        if (player->getDimensionId().id != pos.dim) return Number::newNumber(INT_MAX);

        return Number::newNumber(player->distanceToSqr(pos.getVec3()));
    }
    CATCH("Fail in distanceToSqr!")
}

Local<Value> PlayerClass::setAbility(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kBoolean);

    try {
        Player* player = get();
        if (!player) return Local<Value>();
        player->setAbility(AbilitiesIndex(args[0].asNumber().toInt32()), args[1].asBoolean().value());
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

    CATCH("Fail in setAbility!");
}

Local<Value> PlayerClass::getBiomeId() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        Biome const& bio = player->getDimensionBlockSource().getBiome(player->getFeetBlockPos());
        return Number::newNumber(bio.mId);
    }
    CATCH("Fail in getBiomeId!");
}

Local<Value> PlayerClass::getBiomeName() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        Biome const& bio = player->getDimensionBlockSource().getBiome(player->getFeetBlockPos());
        return String::newString(bio.mHash->getString());
    }
    CATCH("Fail in getBiomeName!");
}

Local<Value> PlayerClass::getAllEffects() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }
        Local<Array> effectList = Array::newArray();
        for (auto& effect : player->_getAllEffectsNonConst()) {
            effectList.add(Number::newNumber((long long)effect.mId));
        }
        return effectList;
    }
    CATCH("Fail in getAllEffects!")
}

Local<Value> PlayerClass::addEffect(const Arguments& args) {
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
    CATCH("Fail in addEffect!");
}

Local<Value> PlayerClass::removeEffect(const Arguments& args) {
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
    CATCH("Fail in removeEffect!");
}

Local<Value> PlayerClass::toEntity(const Arguments&) {
    try {
        return EntityClass::newEntity(get());
    }
    CATCH("Fail in toEntity!");
}
