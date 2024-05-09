#include "api/PlayerAPI.h"

#include "EventAPI.h"
#include "MoreGlobal.h"
#include "ScriptX/ScriptX.h"
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
#include "api/NativeAPI.h"
#include "api/NbtAPI.h"
#include "api/PacketAPI.h"
#include "engine/EngineOwnData.h"
#include "engine/GlobalShareData.h"
#include "legacyapi/form/FormPacketHelper.h"
#include "legacyapi/form/FormUI.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/PlayerInfo.h"
#include "ll/api/service/ServerInfo.h"
#include "lse/api/NetworkPacket.h"
#include "main/EconomicSystem.h"
#include "main/SafeGuardRecord.h"
#include "mc/certificates/WebToken.h"
#include "mc/dataloadhelper/DataLoadHelper.h"
#include "mc/dataloadhelper/DefaultDataLoadHelper.h"
#include "mc/entity/EntityIdTraits.h"
#include "mc/entity/gamerefs_entity/EntityRegistry.h"
#include "mc/entity/utilities/ActorDataIDs.h"
#include "mc/enums/BossBarColor.h"
#include "mc/enums/MinecraftPacketIds.h"
#include "mc/enums/ScorePacketType.h"
#include "mc/enums/TextPacketType.h"
#include "mc/enums/d_b_helpers/Category.h"
#include "mc/nbt/ListTag.h"
#include "mc/network/ConnectionRequest.h"
#include "mc/network/MinecraftPackets.h"
#include "mc/network/NetworkIdentifier.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/packet/BossEventPacket.h"
#include "mc/network/packet/LevelChunkPacket.h"
#include "mc/network/packet/ModalFormRequestPacket.h"
#include "mc/network/packet/RemoveObjectivePacket.h"
#include "mc/network/packet/ScorePacketInfo.h"
#include "mc/network/packet/SetDisplayObjectivePacket.h"
#include "mc/network/packet/SetScorePacket.h"
#include "mc/network/packet/SetTitlePacket.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/network/packet/ToastRequestPacket.h"
#include "mc/network/packet/TransferPacket.h"
#include "mc/network/packet/UpdateAbilitiesPacket.h"
#include "mc/network/packet/UpdateAdventureSettingsPacket.h"
#include "mc/server/ServerPlayer.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/PlayerCommandOrigin.h"
#include "mc/world/ActorUniqueID.h"
#include "mc/world/Container.h"
#include "mc/world/Minecraft.h"
#include "mc/world/SimpleContainer.h"
#include "mc/world/actor/player/EnderChestContainer.h"
#include "mc/world/actor/player/PlayerScoreSetFunction.h"
#include "mc/world/actor/player/PlayerUISlot.h"
#include "mc/world/effect/MobEffectInstance.h"
#include "mc/world/events/BossEventUpdateType.h"
#include "mc/world/item/registry/ItemStack.h"
#include "mc/world/level/LayeredAbilities.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/storage/DBStorage.h"
#include "mc/world/phys/HitResult.h"
#include "mc/world/scores/PlayerScoreboardId.h"
#include "mc/world/scores/ScoreInfo.h"
#include "mc/world/scores/Scoreboard.h"
#include "mc/world/scores/ScoreboardId.h"

#include <algorithm>
#include <mc/entity/EntityContext.h>
#include <mc/entity/utilities/ActorEquipment.h>
#include <mc/entity/utilities/ActorMobilityUtils.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/SimpleContainer.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/SynchedActorData.h>
#include <mc/world/actor/SynchedActorDataEntityWrapper.h>
#include <mc/world/actor/components/SynchedActorDataAccess.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/attribute/Attribute.h>
#include <mc/world/attribute/AttributeInstance.h>
#include <mc/world/attribute/SharedAttributes.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Command.h>
#include <mc/world/level/IConstBlockSource.h>
#include <mc/world/level/biome/Biome.h>
#include <mc/world/level/material/Material.h>
#include <mc/world/scores/Objective.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

//////////////////// Class Definition ////////////////////

ClassDefine<PlayerClass> PlayerClassBuilder =
    defineClass<PlayerClass>("LLSE_Player")
        .constructor(nullptr)
        .instanceFunction("asPointer", &PlayerClass::asPointer)

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
        .build();

//////////////////// Classes ////////////////////

// 生成函数
PlayerClass::PlayerClass(Player* p) : ScriptClass(ScriptClass::ConstructFromCpp<PlayerClass>{}) { set(p); }

Local<Object> PlayerClass::newPlayer(Player* p) {
    auto newp = new PlayerClass(p);
    return newp->getScriptObject();
}

Player* PlayerClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<PlayerClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<PlayerClass>(v)->get();
    else return nullptr;
}

// 公用API
Local<Value> McClass::getPlayerNbt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto                         uuid   = mce::UUID::fromString(args[0].asString().toString());
        std::unique_ptr<CompoundTag> tag    = std::make_unique<CompoundTag>();
        Player*                      player = ll::service::getLevel()->getPlayer(uuid);
        if (player) {
            player->save(*tag);
        } else {
            DBStorage* db = MoreGlobal::db;
            if (db) {
                if (db->hasKey("player_" + uuid.asString(), DBHelpers::Category::Player)) {
                    std::unique_ptr<CompoundTag> playerTag =
                        db->getCompoundTag("player_" + uuid.asString(), DBHelpers::Category::Player);
                    if (playerTag) {
                        std::string serverId = playerTag->at("ServerId");
                        if (!serverId.empty()) {
                            if (db->hasKey(serverId, DBHelpers::Category::Player)) {
                                tag = db->getCompoundTag(serverId, DBHelpers::Category::Player);
                            }
                        }
                    }
                }
            }
        }
        if (tag && !tag->isEmpty()) {
            return NbtCompoundClass::pack(std::move(tag));
        }
        return Local<Value>();
    }
    CATCH("Fail in getPlayerNbt!")
}

Local<Value> McClass::setPlayerNbt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto    uuid   = mce::UUID::fromString(args[0].asString().toString());
        auto    tag    = NbtCompoundClass::extract(args[1]);
        Player* player = ll::service::getLevel()->getPlayer(uuid);
        if (player) {
            DefaultDataLoadHelper defaultDataLoadHelper;
            player->load(*tag, defaultDataLoadHelper);
            return Boolean::newBoolean(true);
        } else {
            DBStorage* db = MoreGlobal::db;
            if (db) {
                if (db->hasKey("player_" + uuid.asString(), DBHelpers::Category::Player)) {
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
        auto         uuid   = mce::UUID::fromString(args[0].asString().toString());
        CompoundTag* nbt    = NbtCompoundClass::extract(args[1]);
        auto         arr    = args[2].asArray();
        Player*      player = ll::service::getLevel()->getPlayer(uuid);
        CompoundTag  playerNbt;
        player->save(playerNbt);
        if (player) {
            for (int i = 0; i < arr.size(); ++i) {
                auto value = arr.get(i);
                if (value.getKind() == ValueKind::kString) {
                    std::string tagName = value.asString().toString();
                    if (!nbt->at(tagName).is_null()) {
                        playerNbt.at(tagName) = nbt->at(tagName);
                    }
                }
            }
            DefaultDataLoadHelper defaultDataLoadHelper;
            player->load(playerNbt, defaultDataLoadHelper);
            player->refreshInventory();
            return Boolean::newBoolean(true);
        }
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in setPlayerNbtTags!")
}

Local<Value> McClass::deletePlayerNbt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto uuid = mce::UUID::fromString(args[0].asString().toString());
        ll::service::getLevel()->getLevelStorage().deleteData("player_" + uuid.asString(), DBHelpers::Category::Player);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in deletePlayerNbt!")
}

Local<Value> McClass::getPlayerScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    try {
        auto         obj       = args[1].asString().toString();
        Scoreboard&  board     = ll::service::getLevel()->getScoreboard();
        Objective*   objective = board.getObjective(obj);
        ScoreboardId sid = board.getScoreboardId(PlayerScoreboardId(std::atoll(args[0].asString().toString().c_str())));
        if (!objective || !sid.isValid() || !objective->hasScore(sid)) {
            return {};
        }
        return Number::newNumber(objective->getPlayerScore(sid).mScore);
    }
    CATCH("Fail in getPlayerScore!")
}

Local<Value> McClass::setPlayerScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    try {
        auto        uuid       = mce::UUID::fromString(args[0].asString().toString());
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  objective  = scoreboard.getObjective(args[1].asString().toString());
        if (!objective) {
            return Boolean::newBoolean(false);
        }
        const ScoreboardId& id =
            scoreboard.getScoreboardId(PlayerScoreboardId(std::atoll(args[0].asString().toString().c_str())));
        if (!id.isValid()) {
            Player* pl = ll::service::getLevel()->getPlayer(uuid);
            if (pl) {
                scoreboard.createScoreboardId(*pl);
            } else {
                return Boolean::newBoolean(false);
            }
        }
        bool isSuccess = false;
        scoreboard
            .modifyPlayerScore(isSuccess, id, *objective, args[2].asNumber().toInt32(), PlayerScoreSetFunction::Set);
        return Boolean::newBoolean(isSuccess);
    }
    CATCH("Fail in setPlayerScore!")
}

Local<Value> McClass::addPlayerScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    try {
        auto        uuid       = mce::UUID::fromString(args[0].asString().toString());
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  objective  = scoreboard.getObjective(args[1].asString().toString());
        if (!objective) {
            return Boolean::newBoolean(false);
        }
        const ScoreboardId& id =
            scoreboard.getScoreboardId(PlayerScoreboardId(std::atoll(args[0].asString().toString().c_str())));
        if (!id.isValid()) {
            Player* pl = ll::service::getLevel()->getPlayer(uuid);
            if (pl) {
                scoreboard.createScoreboardId(*pl);
            } else {
                return Boolean::newBoolean(false);
            }
        }
        bool isSuccess = false;
        scoreboard
            .modifyPlayerScore(isSuccess, id, *objective, args[2].asNumber().toInt32(), PlayerScoreSetFunction::Add);
        return Boolean::newBoolean(isSuccess);
    }
    CATCH("Fail in addPlayerScore!")
}

Local<Value> McClass::reducePlayerScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
    try {
        auto        uuid       = mce::UUID::fromString(args[0].asString().toString());
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        Objective*  objective  = scoreboard.getObjective(args[1].asString().toString());
        if (!objective) {
            return Boolean::newBoolean(false);
        }
        const ScoreboardId& id =
            scoreboard.getScoreboardId(PlayerScoreboardId(std::atoll(args[0].asString().toString().c_str())));
        if (!id.isValid()) {
            Player* pl = ll::service::getLevel()->getPlayer(uuid);
            if (pl) {
                scoreboard.createScoreboardId(*pl);
            } else {
                return Boolean::newBoolean(false);
            }
        }
        bool isSuccess = false;
        scoreboard.modifyPlayerScore(
            isSuccess,
            id,
            *objective,
            args[2].asNumber().toInt32(),
            PlayerScoreSetFunction::Subtract
        );
        return Boolean::newBoolean(isSuccess);
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
        if (!objective) {
            return Boolean::newBoolean(false);
        }
        const ScoreboardId& id =
            scoreboard.getScoreboardId(PlayerScoreboardId(std::atoll(args[0].asString().toString().c_str())));
        if (!id.isValid()) {
            return Boolean::newBoolean(true);
        }
        return Boolean::newBoolean(scoreboard.getScoreboardIdentityRef(id)->removeFromObjective(scoreboard, *objective)
        );
    }
    CATCH("Fail in deletePlayerScore!")
}

Local<Value> McClass::getPlayer(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        string target = args[0].toStr();
        if (target.empty()) return Local<Value>();

        transform(target.begin(), target.end(), target.begin(),
                  ::tolower); // lower case the string
        std::vector<Player*> playerList;
        int                  delta = 2147483647; // c++ int max
        Player*              found = nullptr;
        ll::service::getLevel()->forEachPlayer([&](Player& player) {
            playerList.push_back(&player);
            return true;
        });

        for (Player* p : playerList) {
            if (p->getXuid() == target || std::to_string(p->getOrCreateUniqueID().id) == target)
                return PlayerClass::newPlayer(p);

            string pName = p->getName();
            transform(pName.begin(), pName.end(), pName.begin(), ::tolower);

            if (pName.find(target) == 0) {
                // 0 ís the index where the "target" appear in "pName"
                int curDelta = pName.length() - target.length();
                if (curDelta == 0) return PlayerClass::newPlayer(p);

                if (curDelta < delta) {
                    found = p;
                    delta = curDelta;
                }
            }
        }
        return found ? PlayerClass::newPlayer(found) : Local<Value>(); // Player/Null
    }
    CATCH("Fail in GetPlayer!")
}

Local<Value> McClass::getOnlinePlayers(const Arguments& args) {
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
void PlayerClass::set(Player* player) {
    try {
        id = player->getOrCreateUniqueID();
    } catch (...) {
        isValid = false;
    }
}

Player* PlayerClass::get() {
    if (!isValid) return nullptr;
    else return ll::service::getLevel()->getPlayer(id);
}

Local<Value> PlayerClass::getName() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return String::newString(player->getName());
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
        if (!pos.has_value() || !dim.has_value() || dim == -1) {
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
            lse::getSelfPluginInstance().getLogger().debug("Fail in getXuid!");
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
            lse::getSelfPluginInstance().getLogger().debug("Fail in getUuid!");
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

        return Boolean::newBoolean(player->canBeSeenOnMap());
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

        return Boolean::newBoolean(player->getCanPickupItems());
    }
    CATCH("Fail in getCanPickupItems!")
}

Local<Value> PlayerClass::isSneaking() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->isSneaking());
    }
    CATCH("Fail in isSneaking!")
}

Local<Value> PlayerClass::getSpeed() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Number::newNumber(player->getSpeed());
    }
    CATCH("Fail in getSpeed!")
}

Local<Value> PlayerClass::getDirection() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        Vec2 rot = player->getRotation();
        return DirectionAngle::newAngle(rot.x, rot.y);
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

        return Number::newNumber(player->getHealth());
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

        return Boolean::newBoolean(ActorMobilityUtils::shouldApplyLava(
            *(IConstBlockSource*)&player->getDimensionBlockSourceConst(),
            player->getEntityContext()
        ));
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

        return Boolean::newBoolean(player->isInWall());
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

        return Boolean::newBoolean(player->isInClouds());
    }
    CATCH("Fail in getInClouds!")
}

Local<Value> PlayerClass::asPointer(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        else return NativePointer::newNativePointer(player);
    }
    CATCH("Fail in asPointer!")
}

Local<Value> PlayerClass::getUniqueID() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        else return String::newString(std::to_string(player->getOrCreateUniqueID().id));
    }
    CATCH("Fail in getUniqueID!")
}

Local<Value> PlayerClass::getLangCode() {
    try {
        Json::Value& requestJson = ll::service::getServerNetworkHandler()
                                       ->fetchConnectionRequest(get()->getNetworkIdentifier())
                                       .mRawToken->mDataInfo;

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

        return Boolean::newBoolean(player->isInsidePortal());
    }
    CATCH("Fail in isInsidePortal!")
}

Local<Value> PlayerClass::isHurt() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isHurt());
    }
    CATCH("Fail in isHurt!")
}

Local<Value> PlayerClass::isTrusting() {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }

        return Boolean::newBoolean(player->isTrusting());
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

        return Boolean::newBoolean(player->isHungry());
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

        return Boolean::newBoolean(player->isOnHotBlock());
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

        return Boolean::newBoolean(player->isGliding());
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

        return Boolean::newBoolean(player->isMoving());
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
                    pos.x   = posObj->x;
                    pos.y   = posObj->y;
                    pos.z   = posObj->z;
                    pos.dim = posObj->dim;
                }
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = *posObj;
                }
            } else {
                LOG_WRONG_ARG_TYPE();
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
            pos.dim = args[3].toInt();
            if (args.size() == 5 && IsInstanceOf<DirectionAngle>(args[4])) {
                auto ang        = DirectionAngle::extract(args[4]);
                angle.x         = ang->pitch;
                angle.y         = ang->yaw;
                rotationIsValid = true;
            }
        } else {
            LOG_WRONG_ARG_TYPE();
            return Boolean::newBoolean(false);
        }
        if (!rotationIsValid) {
            angle = player->getRotation();
        }
        player->teleport(pos.getVec3(), pos.dim, angle);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in TeleportPlayer!")
}

Local<Value> PlayerClass::kill(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->kill();
        return Boolean::newBoolean(true); //=======???
    }
    CATCH("Fail in KillPlayer!")
}

Local<Value> PlayerClass::isOP(const Arguments& args) {
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
                ENGINE_OWN_DATA()->pluginName,
                "Set Permission Level",
                fmt::format("Set Player {} Permission Level as {}.", player->getRealName(), newPerm)
            );
            player->setPermissions((CommandPermissionLevel)newPerm);
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
        if (newMode >= 0 || newMode <= 3) {
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
            std::make_unique<PlayerCommandOrigin>(PlayerCommandOrigin(*get()))
        );
        ll::service::getMinecraft()->getCommands().executeCommand(context);
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
        if (args.size() >= 1) msg = args[0].toStr();

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
        pkt.mMessage   = args[0].asString().toString();
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
            content = args[0].toStr();
        }
        if (args.size() >= 2) {
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            type = (SetTitlePacket::TitleType)args[1].toInt();
        }
        if (args.size() >= 5) {
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[4], ValueKind::kNumber);
            fadeInTime  = args[2].toInt();
            stayTime    = args[3].toInt();
            fadeOutTime = args[4].toInt();
        }

        SetTitlePacket pkt = SetTitlePacket(type, content);
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

        TextPacket pkt =
            TextPacket::createChat(player->getName(), args[0].asString().toString(), player->getXuid(), "");
        if (ll::service::getLevel().has_value()) {
            IF_LISTENED(EVENT_TYPES::onChat) {
                CallEventRtnValue(
                    EVENT_TYPES::onChat,
                    Boolean::newBoolean(false),
                    PlayerClass::newPlayer(player),
                    String::newString(args[0].asString().toString())
                );
            }
            IF_LISTENED_END(EVENT_TYPES::onChat);
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
            TextPacket::createChat(player->getRealName(), args[0].asString().toString(), player->getXuid(), "");
        target->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in talkTo!");
}

Local<Value> PlayerClass::getHand(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return ItemClass::newItem(&const_cast<ItemStack&>(player->getSelectedItem()), false);
    }
    CATCH("Fail in getHand!");
}

Local<Value> PlayerClass::getOffHand(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return ItemClass::newItem((ItemStack*)&player->getOffhandSlot(), false);
    }
    CATCH("Fail in getOffHand!");
}

Local<Value> PlayerClass::getInventory(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return ContainerClass::newContainer(&player->getInventory());
    }
    CATCH("Fail in getInventory!");
}

Local<Value> PlayerClass::getArmor(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return ContainerClass::newContainer(&ActorEquipment::getArmorContainer(player->getEntityContext()));
    }
    CATCH("Fail in getArmor!");
}

Local<Value> PlayerClass::getEnderChest(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        auto chest = player->getEnderChestContainer();
        if (chest) {
            return ContainerClass::newContainer(chest);
        } else {
            EnderChestContainer* enderChest = ll::memory::dAccess<EnderChestContainer*>(
                player,
                3248
            ); // Todo: please remove it when LeviLamina 0.11.1 released
            if (enderChest) {
                return ContainerClass::newContainer(enderChest);
            }
        }
        return {};
    }
    CATCH("Fail in getEnderChest!");
}

Local<Value> PlayerClass::getRespawnPosition(const Arguments& args) {
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
                LOG_WRONG_ARG_TYPE();
                return Local<Value>();
            }
        } else if (args.size() == 4) {
            // Number Pos
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
            pos = {args[0].toInt(), args[1].toInt(), args[2].toInt(), args[3].toInt()};
        } else {
            LOG_WRONG_ARGS_COUNT();
            return Local<Value>();
        }
        player->setRespawnPosition(pos.getBlockPos(), pos.dim);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setRespawnPosition!")
}

Local<Value> PlayerClass::refreshItems(const Arguments& args) {
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
        player->setNameTag(args[0].toStr());
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

        player->addLevels(args[0].toInt());
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

        player->addLevels(-args[0].toInt());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in reduceLevel!");
}

Local<Value> PlayerClass::getLevel(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Number::newNumber(player->getPlayerLevel());
    }
    CATCH("Fail in getLevel!")
}

Local<Value> PlayerClass::setLevel(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->addLevels(args[0].toInt() - player->getPlayerLevel());
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

Local<Value> PlayerClass::resetLevel(const Arguments& args) {
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

        player->addExperience(args[0].toInt());
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

        player->addExperience(-args[0].toInt());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in reduceExperience!");
}

Local<Value> PlayerClass::getCurrentExperience(const Arguments& arg) {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }
        return Number::newNumber(player->getXpEarnedAtCurrentLevel());
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

        AttributeInstance* attr = player->getMutableAttribute(Player::EXPERIENCE);
        attr->setCurrentValue(args[0].asNumber().toFloat()); // Not sure about that
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setCurrentExperience!");
}

Local<Value> PlayerClass::getTotalExperience(const Arguments& arg) {
    try {
        Player* player = get();
        if (!player) {
            return Local<Value>();
        }
        return Number::newNumber(
            (int32_t)player->getXpNeededForLevelRange(0, player->getPlayerLevel()) + player->getXpEarnedAtCurrentLevel()
        );
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

Local<Value> PlayerClass::getXpNeededForNextLevel(const Arguments& args) {
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

        TransferPacket pkt = TransferPacket(args[0].toStr(), args[1].toInt());
        player->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in transServer!");
}

Local<Value> PlayerClass::crash(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        RecordOperation(
            ENGINE_OWN_DATA()->pluginName,
            "Crash Player",
            "Execute player.crash() to crash player <" + player->getRealName() + ">"
        );
        LevelChunkPacket pkt = LevelChunkPacket();
        pkt.mCacheEnabled    = true;
        player->sendNetworkPacket(pkt);
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in crashPlayer!");
}

Local<Value> PlayerClass::getBlockStandingOn(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return BlockClass::newBlock(player->getBlockPosCurrentlyStandingOn(nullptr), player->getDimensionId());
    }
    CATCH("Fail in getBlockStandingOn!");
}

Local<Value> PlayerClass::getDevice(const Arguments& args) {
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
        if (!id.isValid()) {
            scoreboard.createScoreboardId(*player);
        }
        return Number::newNumber(obj->getPlayerScore(id).mScore);
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
                scoreboard.getDefaultCriteria()
            );
        }
        const ScoreboardId& id = scoreboard.getScoreboardId(*player);
        if (!id.isValid()) {
            scoreboard.createScoreboardId(*player);
        }
        bool isSuccess = false;
        scoreboard.modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Set);
        return Boolean::newBoolean(isSuccess);
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
        if (!id.isValid()) {
            scoreboard.createScoreboardId(*player);
        }
        bool isSuccess = false;
        scoreboard.modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Add);
        return Boolean::newBoolean(isSuccess);
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
        if (!id.isValid()) {
            scoreboard.createScoreboardId(*player);
        }
        bool isSuccess = false;
        scoreboard
            .modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Subtract);
        return Boolean::newBoolean(isSuccess);
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
        if (!id.isValid()) {
            return Boolean::newBoolean(true);
        }
        return Boolean::newBoolean(scoreboard.getScoreboardIdentityRef(id)->removeFromObjective(scoreboard, *obj));
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
            data.push_back(make_pair(key, source.get(key).toInt()));
        }

        int sortOrder = 1;
        if (args.size() >= 3) sortOrder = args[2].toInt();

        SetDisplayObjectivePacket
            disObjPkt("sidebar", "FakeScoreObj", args[0].asString().toString(), "dummy", (ObjectiveSortOrder)sortOrder);
        disObjPkt.sendTo(*player);
        std::vector<ScorePacketInfo> info;
        for (auto& i : data) {
            ScorePacketInfo pktInfo;
            pktInfo.mScoreboardId   = ScoreboardId(i.second);
            pktInfo.mObjectiveName  = "FakeScoreObj";
            pktInfo.mIdentityType   = IdentityDefinition::Type::FakePlayer;
            pktInfo.mScoreValue     = i.second;
            pktInfo.mFakePlayerName = i.first;
            info.emplace_back(pktInfo);
        }
        SetScorePacket setPkt = SetScorePacket::change(info);
        setPkt.sendTo(*player);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setSidebar!")
}

Local<Value> PlayerClass::removeSidebar(const Arguments& args) {
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
            int     percent = args[2].toInt();
            if (percent < 0) percent = 0;
            else if (percent > 100) percent = 100;
            float value = (float)percent / 100;

            // Remove BossBar firstly
            BossEventPacket removePkt = BossEventPacket();
            removePkt.mBossID         = ActorUniqueID(uid);
            removePkt.mEventType      = BossEventUpdateType::Remove;
            player->sendNetworkPacket(removePkt);

            BinaryStream bs;
            bs.writeVarInt64(uid);
            bs.writeUnsignedVarInt64(uid);
            bs.writeString("player");
            bs.writeFloat(player->getPosition().x);
            bs.writeFloat(-60.0f);
            bs.writeFloat(player->getPosition().z);
            bs.writeFloat(0.0f);
            bs.writeFloat(0.0f);
            bs.writeFloat(0.0f);
            bs.writeFloat(0.0f);
            bs.writeFloat(0.0f);
            bs.writeFloat(0.0f);
            bs.writeFloat(0.0f);
            // Atrribute
            bs.writeUnsignedVarInt(0);
            // DataItem
            bs.writeUnsignedVarInt(0);
            // PropertySyncIntEntry
            bs.writeUnsignedVarInt(0);
            // PropertySyncFloatEntry
            bs.writeUnsignedVarInt(0);
            // Links
            bs.writeUnsignedVarInt(0);
            auto addPkt = lse::api::NetworkPacket<MinecraftPacketIds::AddActor>(std::move(*bs.mBuffer));

            BossBarColor    color = (BossBarColor)args[3].toInt();
            BossEventPacket pkt;
            pkt.mEventType     = BossEventUpdateType::Add;
            pkt.mBossID        = ActorUniqueID(uid);
            pkt.mName          = args[1].asString().toString();
            pkt.mHealthPercent = value;
            pkt.mColor         = color;
            player->sendNetworkPacket(addPkt);
            player->sendNetworkPacket(pkt);
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

        int percent = args[1].toInt();
        if (percent < 0) percent = 0;
        else if (percent > 100) percent = 100;
        float value = (float)percent / 100;

        BossBarColor color = BossBarColor::Red;
        if (args.size() >= 3) color = (BossBarColor)args[2].toInt();
        BossEventPacket pkt;
        pkt.mEventType     = BossEventUpdateType::Add;
        pkt.mName          = args[0].asString().toString();
        pkt.mHealthPercent = value;
        pkt.mColor         = color;
        player->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setBossBar!")
}

Local<Value> PlayerClass::removeBossBar(const Arguments& args) {
    if (args.size() == 0) {
        try {
            Player* player = get();
            if (!player) return Local<Value>();

            BossEventPacket pkt = BossEventPacket();
            pkt.mEventType      = BossEventUpdateType::Remove;
            pkt.mColor          = BossBarColor::Red;
            player->sendNetworkPacket(pkt);
            return Boolean::newBoolean(true);
        }
        CATCH("Fail in removeBossBar!")
    } else {
        CHECK_ARGS_COUNT(args, 1);
        CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
        try {
            Player* player = get();
            if (!player) return Local<Value>();
            int64_t         uid = args[0].asNumber().toInt64();
            BossEventPacket pkt = BossEventPacket();
            pkt.mBossID         = ActorUniqueID(uid);
            pkt.mEventType      = BossEventUpdateType::Remove;
            player->sendNetworkPacket(pkt);
            return Boolean::newBoolean(true);
        }
        CATCH("Fail in removeBossBar!")
    }
}

Local<Value> PlayerClass::sendSimpleForm(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 4);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kArray);
    CHECK_ARG_TYPE(args[3], ValueKind::kArray);
    CHECK_ARG_TYPE(args[4], ValueKind::kFunction);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        // 普通格式
        auto textsArr = args[2].asArray();
        if (textsArr.size() == 0 || !textsArr.get(0).isString()) return Local<Value>();
        auto imagesArr = args[3].asArray();
        if (imagesArr.size() != textsArr.size() || !imagesArr.get(0).isString()) return Local<Value>();

        lse::form::SimpleForm form(args[0].asString().toString(), args[1].asString().toString());
        for (int i = 0; i < textsArr.size(); ++i) {
            Local<Value> img = imagesArr.get(i);
            if (img.isString()) {
                form.addButton(textsArr.get(i).asString().toString(), img.asString().toString());
            } else {
                form.addButton(textsArr.get(i).asString().toString());
            }
        }
        form.sendTo(
            player,
            [engine{EngineScope::currentEngine()},
             callback{script::Global(args[4].asFunction())}](Player* pl, int chosen) {
                if ((ll::getServerStatus() != ll::ServerStatus::Running)) return;
                if (!EngineManager::isValid(engine)) return;

                EngineScope scope(engine);
                try {
                    callback.get()
                        .call({}, PlayerClass::newPlayer(pl), chosen >= 0 ? Number::newNumber(chosen) : Local<Value>());
                }
                CATCH_IN_CALLBACK("sendSimpleForm")
            }
        );

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

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        lse::form::ModalForm form(
            args[0].asString().toString(),
            args[1].asString().toString(),
            args[2].asString().toString(),
            args[3].asString().toString()
        );
        form.sendTo(
            player,
            [engine{EngineScope::currentEngine()},
             callback{script::Global(args[4].asFunction())}](Player* pl, bool chosen) {
                if ((ll::getServerStatus() != ll::ServerStatus::Running)) return;
                if (!EngineManager::isValid(engine)) return;

                EngineScope scope(engine);
                try {
                    callback.get()
                        .call({}, PlayerClass::newPlayer(pl), chosen ? Boolean::newBoolean(chosen) : Local<Value>());
                }
                CATCH_IN_CALLBACK("sendModalForm")
            }
        );

        return Number::newNumber(2);
    }
    CATCH("Fail in sendModalForm!");
}

Local<Value> PlayerClass::sendCustomForm(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        std::string data = ordered_json::parse(args[0].toStr()).dump();

        unsigned               formId = lse::form::NewFormId();
        ModalFormRequestPacket packet(formId, data);
        player->sendNetworkPacket(packet);
        lse::form::SetCustomFormPacketCallback(
            formId,
            [id{player->getOrCreateUniqueID()},
             engine{EngineScope::currentEngine()},
             callback{script::Global(args[1].asFunction())}](Player* player, string result) {
                if ((ll::getServerStatus() != ll::ServerStatus::Running)) return;
                if (!EngineManager::isValid(engine)) return;

                EngineScope scope(engine);
                try {
                    callback.get().call(
                        {},
                        PlayerClass::newPlayer(player),
                        result != "null" ? JsonToValue(result) : Local<Value>()
                    );
                }
                CATCH_IN_CALLBACK("sendCustomForm")
            }
        );
        return Number::newNumber(3);
    } catch (const ordered_json::exception& e) {
        lse::getSelfPluginInstance().getLogger().error("Fail to parse Json string in sendCustomForm!");
        lse::getSelfPluginInstance().getLogger().error(ll::string_utils::tou8str(e.what()));
        PrintScriptStackTrace();
        return {};
    }
    CATCH("Fail in sendCustomForm!");
}

Local<Value> PlayerClass::sendForm(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        lse::form::SimpleForm* form = SimpleFormClass::extract(args[0]);
        if (IsInstanceOf<SimpleFormClass>(args[0])) {
            Local<Function> callback = args[1].asFunction();
            SimpleFormClass::sendForm(SimpleFormClass::extract(args[0]), player, callback);
        } else if (IsInstanceOf<CustomFormClass>(args[0])) {
            Local<Function> callback = args[1].asFunction();
            CustomFormClass::sendForm(CustomFormClass::extract(args[0]), player, callback);
        } else {
            LOG_WRONG_ARG_TYPE();
            return Local<Value>();
        }
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
    return Local<Value>();
}

Local<Value> PlayerClass::setExtraData(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        string key = args[0].toStr();
        if (key.empty()) return Boolean::newBoolean(false);

        ENGINE_OWN_DATA()->playerDataDB[player->getRealName() + "-" + key] = args[1];
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

        string key = args[0].toStr();
        if (key.empty()) return Local<Value>();

        auto& db  = ENGINE_OWN_DATA()->playerDataDB;
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

        string key = args[0].toStr();
        if (key.empty()) return Boolean::newBoolean(false);

        ENGINE_OWN_DATA()->playerDataDB.erase(player->getRealName() + "-" + key);
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
            auto source = EntityClass::extract(args[2]);
            if (!source) {
                return Boolean::newBoolean(false);
            }
            return Boolean::newBoolean(player->hurtByCause(damage, (ActorDamageCause)type, source));
        }
        return Boolean::newBoolean(player->hurtByCause(damage, (ActorDamageCause)type));
    }
    CATCH("Fail in hurt!");
}

Local<Value> PlayerClass::heal(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->heal(args[0].toInt());
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

        AttributeInstance* healthAttribute = player->getMutableAttribute(SharedAttributes::HEALTH);

        healthAttribute->setCurrentValue(args[0].asNumber().toFloat());

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

        AttributeInstance* healthAttribute = player->getMutableAttribute(SharedAttributes::HEALTH);

        healthAttribute->setMaxValue(args[0].asNumber().toFloat());

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

        AttributeInstance* absorptionAttribute = player->getMutableAttribute(SharedAttributes::ABSORPTION);

        absorptionAttribute->setCurrentValue(args[0].asNumber().toFloat());

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

        AttributeInstance* attactDamageAttribute = player->getMutableAttribute(SharedAttributes::ATTACK_DAMAGE);

        attactDamageAttribute->setCurrentValue(args[0].asNumber().toFloat());

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

        AttributeInstance* attactDamageAttribute = player->getMutableAttribute(SharedAttributes::ATTACK_DAMAGE);

        attactDamageAttribute->setMaxValue(args[0].asNumber().toFloat());

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

        AttributeInstance* followRangeAttribute = player->getMutableAttribute(SharedAttributes::FOLLOW_RANGE);

        followRangeAttribute->setCurrentValue(args[0].asNumber().toFloat());

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

        AttributeInstance* knockbackResistanceAttribute =
            player->getMutableAttribute(SharedAttributes::KNOCKBACK_RESISTANCE);

        knockbackResistanceAttribute->setCurrentValue(args[0].asNumber().toFloat());

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

        AttributeInstance* luckAttribute = player->getMutableAttribute(SharedAttributes::LUCK);

        luckAttribute->setCurrentValue(args[0].asNumber().toFloat());

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

        AttributeInstance* movementSpeedAttribute = player->getMutableAttribute(SharedAttributes::MOVEMENT_SPEED);
        if (movementSpeedAttribute) {
            movementSpeedAttribute->setCurrentValue(args[0].asNumber().toFloat());
            return Boolean::newBoolean(true);
        } else {
            player->_registerPlayerAttributes();
            movementSpeedAttribute = player->getMutableAttribute(SharedAttributes::MOVEMENT_SPEED); // Check again
            if (movementSpeedAttribute) {
                movementSpeedAttribute->setCurrentValue(args[0].asNumber().toFloat());
                return Boolean::newBoolean(true);
            }
        }
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

        AttributeInstance* underwaterMovementSpeedAttribute =
            player->getMutableAttribute(SharedAttributes::UNDERWATER_MOVEMENT_SPEED);

        underwaterMovementSpeedAttribute->setCurrentValue(args[0].asNumber().toFloat());

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

        AttributeInstance* lavaMovementSpeedAttribute =
            player->getMutableAttribute(SharedAttributes::LAVA_MOVEMENT_SPEED);

        lavaMovementSpeedAttribute->setCurrentValue(args[0].asNumber().toFloat());

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

        AttributeInstance* healthAttribute = player->getMutableAttribute(player->HUNGER);

        healthAttribute->setCurrentValue(args[0].asNumber().toFloat());

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

        int  time          = args[0].toInt();
        bool isEffectValue = args[1].asBoolean().value();

        player->setOnFire(time, isEffectValue);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setFire!");
}

Local<Value> PlayerClass::stopFire(const Arguments& args) {
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

        int time = args[0].toInt();

        player->setOnFire(time, true);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setOnFire!");
}

Local<Value> PlayerClass::refreshChunks(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        player->resendAllChunks();
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
        unsigned int clearCount = 1;
        if (args.size() > 1) {
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            clearCount = args[1].asNumber().toInt32();
        }
        int   result         = 0;
        auto& inventorySlots = player->getInventory().getSlots();
        for (unsigned short slot = 0; slot < inventorySlots.size(); ++slot) {
            if (inventorySlots[slot]->getTypeName() == args[0].asString().toString()) {
                if (inventorySlots[slot]->mCount < clearCount) {
                    result += inventorySlots[slot]->mCount;
                } else {
                    result += clearCount;
                }
                player->getInventory().removeItem(slot, clearCount);
            }
        }
        auto& handSlots = ActorEquipment::getHandContainer(player->getEntityContext()).getSlots();
        for (unsigned short slot = 0; slot < handSlots.size(); ++slot) {
            if (handSlots[slot]->getTypeName() == args[0].asString().toString()) {
                if (handSlots[slot]->mCount < clearCount) {
                    result += handSlots[slot]->mCount;
                } else {
                    result += clearCount;
                }
                ActorEquipment::getHandContainer(player->getEntityContext()).removeItem(slot, clearCount);
            }
        }
        auto& armorSlots = ActorEquipment::getArmorContainer(player->getEntityContext()).getSlots();
        for (unsigned short slot = 0; slot < armorSlots.size(); ++slot) {
            if (armorSlots[slot]->getTypeName() == args[0].asString().toString()) {
                if (armorSlots[slot]->mCount < clearCount) {
                    result += armorSlots[slot]->mCount;
                } else {
                    result += clearCount;
                }
                ActorEquipment::getArmorContainer(player->getEntityContext()).removeItem(slot, clearCount);
            }
        }
        player->refreshInventory();
        return Number::newNumber(result);
    }
    CATCH("Fail in clearItem!");
}

Local<Value> PlayerClass::isSprinting(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 0);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->isSprinting());
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

Local<Value> PlayerClass::getNbt(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        CompoundTag* tag = new CompoundTag();
        player->save(*tag);
        return NbtCompoundClass::pack(tag);
    }
    CATCH("Fail in getNbt!")
}

Local<Value> PlayerClass::setNbt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        auto nbt = NbtCompoundClass::extract(args[0]);
        if (!nbt) return Local<Value>(); // Null

        DefaultDataLoadHelper helper = DefaultDataLoadHelper();
        return Boolean::newBoolean(player->load(*nbt, helper));
    }
    CATCH("Fail in setNbt!")
}

Local<Value> PlayerClass::addTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->addTag(args[0].toStr()));
    }
    CATCH("Fail in addTag!");
}

Local<Value> PlayerClass::removeTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->removeTag(args[0].toStr()));
    }
    CATCH("Fail in removeTag!");
}

Local<Value> PlayerClass::hasTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Player* player = get();
        if (!player) return Local<Value>();

        return Boolean::newBoolean(player->hasTag(args[0].toStr()));
    }
    CATCH("Fail in hasTag!");
}

Local<Value> PlayerClass::getAllTags(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        Local<Array> arr = Array::newArray();
        for (auto& tag : player->getTags()) {
            arr.add(String::newString(tag));
        }
        return arr;
    }
    CATCH("Fail in getAllTags!");
}

Local<Value> PlayerClass::getAbilities(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        CompoundTag tag = CompoundTag();
        player->save(tag);
        try {
            return Tag2Value(&tag.at("abilities").get<CompoundTag>(), true);
        } catch (...) {
            return Object::newObject();
        }
    }
    CATCH("Fail in getAbilities!");
}

Local<Value> PlayerClass::getAttributes(const Arguments& args) {
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
        if (entity) {
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
        bool  includeLiquid      = false;
        bool  solidOnly          = false;
        float maxDistance        = 5.25f;
        bool  ignoreBorderBlocks = true;
        bool  fullOnly           = false;
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
            [&solidOnly, &fullOnly, &includeLiquid](BlockSource const& source, Block const& block, bool idk) {
                if (solidOnly && !block.isSolid()) {
                    return false;
                }
                if (fullOnly && !block.isSlabBlock()) {
                    return false;
                }
                if (!includeLiquid && block.getMaterial().isLiquid()) {
                    return false;
                }
                return true;
            }
        );

        BlockPos bp;
        if (includeLiquid && res.mIsHitLiquid) {
            bp = res.mLiquidPos;
        } else {
            bp = res.mBlockPos;
        }
        Block const& bl = player->getDimensionBlockSource().getBlock(bp);
        if (bl.isEmpty()) {
            return Local<Value>();
        }
        return BlockClass::newBlock(std::move(&bl), std::move(&bp), &player->getDimensionBlockSource());
    }
    CATCH("Fail in getBlockFromViewVector!");
}

Local<Value> PlayerClass::isSimulatedPlayer(const Arguments& args) {
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
        return Number::newNumber(actor->evalMolang(args[0].toStr()));
    }
    CATCH("Fail in quickEvalMolangScript!");
}

//////////////////// For LLMoney ////////////////////

Local<Value> PlayerClass::getMoney(const Arguments& args) {
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
            targetXuid = args[0].toStr();
        } else {
            targetXuid = PlayerClass::extract(args[0])->getXuid();
        }
        if (args.size() >= 3) {
            CHECK_ARG_TYPE(args[2], ValueKind::kString);
            note = args[2].toStr();
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
        return xuid.empty() ? Local<Value>()
                            : objectificationMoneyHistory(EconomySystem::getMoneyHist(xuid, args[0].toInt()));
    }
    CATCH("Fail in getMoneyHistory!");
}

//////////////////// For Compatibility ////////////////////

Local<Value> PlayerClass::getAllItems(const Arguments& args) {
    try {
        Player* player = get();
        if (!player) return Local<Value>();

        const ItemStack&              hand      = player->getCarriedItem();
        const ItemStack&              offHand   = player->getOffhandSlot();
        std::vector<const ItemStack*> inventory = player->getInventory().getSlots();
        std::vector<const ItemStack*> armor = ActorEquipment::getArmorContainer(player->getEntityContext()).getSlots();
        std::vector<const ItemStack*> endChest = player->getEnderChestContainer()->getSlots();

        Local<Object> result = Object::newObject();

        // hand
        result.set("hand", ItemClass::newItem(&const_cast<ItemStack&>(hand), false));

        // offHand
        result.set("offHand", ItemClass::newItem(&const_cast<ItemStack&>(offHand), false));

        // inventory
        Local<Array> inventoryArr = Array::newArray();
        for (const ItemStack* item : inventory) {
            if (item) {
                inventoryArr.add(ItemClass::newItem(const_cast<ItemStack*>(item), false));
            }
        }
        result.set("inventory", inventoryArr);

        // armor
        Local<Array> armorArr = Array::newArray();
        for (const ItemStack* item : armor) {
            if (item) {
                armorArr.add(ItemClass::newItem(const_cast<ItemStack*>(item), false));
            }
        }
        result.set("armor", armorArr);

        // endChest
        Local<Array> endChestArr = Array::newArray();
        for (const ItemStack* item : endChest) {
            if (item) {
                endChestArr.add(ItemClass::newItem(const_cast<ItemStack*>(item), false));
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

        int inventoryId = args[0].toInt();
        int count       = args[1].toInt();

        Container& container = player->getInventory();
        if (inventoryId > container.getContainerSize()) return Boolean::newBoolean(false);
        container.removeItem(inventoryId, count);
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

        ToastRequestPacket pkt = ToastRequestPacket(args[0].asString().toString(), args[1].asString().toString());
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
                    pos.x   = posObj->x;
                    pos.y   = posObj->y;
                    pos.z   = posObj->z;
                    pos.dim = posObj->dim;
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

                Actor* targetActor = EntityClass::tryExtractActor(args[0]).value();
                if (!targetActor) return Local<Value>();

                Vec3 targetActorPos = targetActor->getPosition();

                pos.x   = targetActorPos.x;
                pos.y   = targetActorPos.y;
                pos.z   = targetActorPos.z;
                pos.dim = targetActor->getDimensionId();
            } else {
                LOG_WRONG_ARG_TYPE();
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
            pos.dim = args[3].toInt();
        } else {
            LOG_WRONG_ARGS_COUNT();
            return Local<Value>();
        }

        if (player->getDimensionId() != pos.dim) return Number::newNumber(INT_MAX);

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
                    pos.x   = posObj->x;
                    pos.y   = posObj->y;
                    pos.z   = posObj->z;
                    pos.dim = posObj->dim;
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

                Actor* targetActor = EntityClass::tryExtractActor(args[0]).value();
                if (!targetActor) return Local<Value>();

                Vec3 targetActorPos = targetActor->getPosition();

                pos.x   = targetActorPos.x;
                pos.y   = targetActorPos.y;
                pos.z   = targetActorPos.z;
                pos.dim = targetActor->getDimensionId();
            } else {
                LOG_WRONG_ARG_TYPE();
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
            pos.dim = args[3].toInt();
        } else {
            LOG_WRONG_ARGS_COUNT();
            return Local<Value>();
        }

        if (player->getDimensionId() != pos.dim) return Number::newNumber(INT_MAX);

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
        bool           value     = args[1].asBoolean().value();
        AbilitiesIndex index     = AbilitiesIndex(args[0].asNumber().toInt32());
        ActorUniqueID  uid       = player->getOrCreateUniqueID();
        auto&          abilities = player->getAbilities();
        bool           flying    = abilities.getAbility(AbilitiesIndex::Flying).getBool();
        if (index == AbilitiesIndex::Flying && value && player->isOnGround()) {
            abilities.setAbility(AbilitiesIndex::MayFly, value);
        }
        if (index == AbilitiesIndex::MayFly && value == false && flying) {
            abilities.setAbility(AbilitiesIndex::Flying, false);
        }
        abilities.setAbility(index, value);
        auto mayfly = abilities.getAbility(AbilitiesIndex::MayFly).getBool();
        auto noclip = abilities.getAbility(AbilitiesIndex::NoClip).getBool();
        player->setCanFly(mayfly || noclip);
        if (index == AbilitiesIndex::NoClip) {
            abilities.setAbility(AbilitiesIndex::Flying, value);
        }
        flying      = abilities.getAbility(AbilitiesIndex::Flying).getBool();
        Ability& ab = abilities.getAbility(AbilitiesLayer(1), AbilitiesIndex::Flying);
        ab.setBool(0);
        if (flying) ab.setBool(1);
        UpdateAbilitiesPacket         pkt(uid, abilities);
        UpdateAdventureSettingsPacket pkt2 = UpdateAdventureSettingsPacket(AdventureSettings());
        abilities.setAbility(AbilitiesIndex::Flying, flying);
        player->sendNetworkPacket(pkt2);
        player->sendNetworkPacket(pkt);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setAbility!");
}

Local<Value> PlayerClass::getBiomeId() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        Biome& bio = player->getDimensionBlockSource().getBiome(player->getFeetBlockPos());
        return Number::newNumber(ll::memory::dAccess<int>(&bio, 0x80));
    }
    CATCH("Fail in getBiomeId!");
}

Local<Value> PlayerClass::getBiomeName() {
    try {
        Player* player = get();
        if (!player) return Local<Value>();
        Biome& bio = player->getDimensionBlockSource().getBiome(player->getFeetBlockPos());
        return String::newString(ll::memory::dAccess<HashedString>(&bio, 0x08).getString());
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
        for (unsigned int i = 0; i <= 30; i++) {
            if (player->getEffect(i)) {
                effectList.add(Number::newNumber((int)i));
            }
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
        unsigned int      id            = args[0].asNumber().toInt32();
        int               tick          = args[1].asNumber().toInt32();
        int               level         = args[2].asNumber().toInt32();
        bool              showParticles = args[3].asBoolean().value();
        MobEffectInstance effect        = MobEffectInstance(id, tick, level, false, showParticles, false);
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
