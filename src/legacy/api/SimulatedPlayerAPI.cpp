#include "api/APIHelp.h"
#include "api/BaseAPI.h"
#include "api/BlockAPI.h"
#include "api/EntityAPI.h"
#include "api/ItemAPI.h"
#include "api/McAPI.h"
#include "api/PlayerAPI.h"
#include "engine/EngineOwnData.h"
#include "engine/GlobalShareData.h"
#include "ll/api/service/Bedrock.h"
#include "lse/api/helper/SimulatedPlayerHelper.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/scripting/modules/gametest/ScriptNavigationResult.h"
#include "mc/server/SimulatedPlayer.h"
#include "mc/server/sim/LookDuration.h"
#include "mc/world/Container.h"
#include "mc/world/Minecraft.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/dimension/Dimension.h"

#include <string>
#include <vector>

using lse::api::SimulatedPlayerHelper;

Local<Value> McClass::spawnSimulatedPlayer(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        std::string name = args[0].asString().toString();
        if (args.size() == 1) {
            if (auto sp = SimulatedPlayer::create(name, ll::service::getLevel()->getSharedSpawnPos()))
                return PlayerClass::newPlayer(sp);
            else return Local<Value>();
        }
        auto dimId = 0;
        Vec3 spawnPos;
        if (IsInstanceOf<IntPos>(args[1])) {
            auto pos = IntPos::extractPos(args[1]);
            spawnPos = pos->getBlockPos().bottomCenter();
            dimId    = pos->getDimensionId();
        } else if (IsInstanceOf<FloatPos>(args[1])) {
            auto pos = FloatPos::extractPos(args[1]);
            spawnPos = pos->getVec3();
            dimId    = pos->getDimensionId();
        } else {
            CHECK_ARGS_COUNT(args, 4);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
            if (args.size() > 4) {
                CHECK_ARG_TYPE(args[4], ValueKind::kNumber);
                dimId = args[4].asNumber().toInt32();
            }
            spawnPos =
                BlockPos(args[1].asNumber().toInt32(), args[2].asNumber().toInt32(), args[3].asNumber().toInt32())
                    .bottomCenter();
        }
        if (auto sp = SimulatedPlayer::create(name, spawnPos, dimId)) return PlayerClass::newPlayer(sp);
        else return Local<Value>();
    }
    CATCH("Fail in " __FUNCTION__ "!")
}

SimulatedPlayer* PlayerClass::asSimulatedPlayer() {
    Player* ptr = get();
    if (ptr && ptr->isSimulatedPlayer()) {
        return static_cast<SimulatedPlayer*>(ptr);
    }
    return nullptr;
}

Local<Value> PlayerClass::simulateSneak(const Arguments&) {
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();

        return Boolean::newBoolean(sp->simulateSneaking());
    }
    CATCH("Fail in " __FUNCTION__ "!")
}

Local<Value> PlayerClass::simulateAttack(const Arguments& args) {
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();

        if (args.size() == 0) return Boolean::newBoolean(sp->simulateAttack());

        if (auto actor = EntityClass::tryExtractActor(args[0])) {
            sp->_trySwing();
            return Boolean::newBoolean(sp->attack(*actor, SharedTypes::Legacy::ActorDamageCause::EntityAttack));
        }

        LOG_WRONG_ARG_TYPE(__FUNCTION__);
        return Local<Value>();
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

Local<Value> PlayerClass::simulateDestroy(const Arguments& args) {
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();

        if (args.size() == 0) return Boolean::newBoolean(sp->simulateDestroyLookAt());

        int                                 dimid = sp->getDimensionId().id;
        BlockPos                            bpos;
        size_t                              index = 0;
        ScriptModuleMinecraft::ScriptFacing face  = (ScriptModuleMinecraft::ScriptFacing)0;
        if (IsInstanceOf<IntPos>(args[0])) {
            auto pos = IntPos::extractPos(args[index]);
            if (dimid != pos->getDimensionId()) return Local<Value>();
            bpos  = pos->getBlockPos();
            index = 1;
        } else if (IsInstanceOf<FloatPos>(args[0])) {
            auto pos = FloatPos::extractPos(args[index]);
            if (dimid != pos->getDimensionId()) return Local<Value>();
            bpos  = pos->getVec3();
            index = 1;
        } else if (IsInstanceOf<BlockClass>(args[0])) {
            auto block = EngineScope::currentEngine()->getNativeInstance<BlockClass>(args[0]);
            auto pos   = IntPos::extractPos(block->getPos());
            if (dimid != pos->getDimensionId()) return Local<Value>();
            bpos  = pos->getBlockPos();
            index = 1;
        }
#ifdef ENABLE_NUMBERS_AS_POS
        else if (args[0].isNumber()) {
            CHECK_ARGS_COUNT(args, 4);
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            bpos  = {args[0].asNumber().toInt32(), args[1].asNumber().toInt32(), args[2].asNumber().toInt32()};
            index = 3;
        }
#endif // ENABLE_NUMBERS_AS_POS
        else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        if (args.size() > index) {
            CHECK_ARG_TYPE(args[index], ValueKind::kNumber);
            face = (ScriptModuleMinecraft::ScriptFacing)args[index].asNumber().toInt32();
        }

        return Boolean::newBoolean(sp->simulateDestroyBlock(bpos, face));
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

Local<Value> PlayerClass::simulateDisconnect(const Arguments&) {
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        sp->disconnect();
        sp->remove();
        sp->setGameTestHelper(nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

Local<Value> PlayerClass::simulateInteract(const Arguments& args) {
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        if (args.size() == 0) return Boolean::newBoolean(sp->simulateInteract());

        if (auto actor = EntityClass::tryExtractActor(args[0])) {
            return Boolean::newBoolean(sp->isAlive() && sp->interact(*actor, Vec3::ZERO()));
        }

        int                                 dimid = sp->getDimensionId().id;
        BlockPos                            bpos;
        size_t                              index = 0;
        ScriptModuleMinecraft::ScriptFacing face  = (ScriptModuleMinecraft::ScriptFacing)0;
        if (IsInstanceOf<IntPos>(args[0])) {
            auto pos = IntPos::extractPos(args[index]);
            if (dimid != pos->getDimensionId()) return Local<Value>();
            bpos  = pos->getBlockPos();
            index = 1;
        } else if (IsInstanceOf<FloatPos>(args[0])) {
            auto pos = FloatPos::extractPos(args[index]);
            if (dimid != pos->getDimensionId()) return Local<Value>();
            bpos  = pos->getVec3();
            index = 1;
        } else if (IsInstanceOf<BlockClass>(args[0])) {
            auto block = EngineScope::currentEngine()->getNativeInstance<BlockClass>(args[0]);
            auto pos   = IntPos::extractPos(block->getPos());
            if (dimid != pos->getDimensionId()) return Local<Value>();
            bpos  = pos->getBlockPos();
            index = 1;
        }
#ifdef ENABLE_NUMBERS_AS_POS
        else if (args[0].isNumber()) {
            CHECK_ARGS_COUNT(args, 4);
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            bpos  = {args[0].asNumber().toInt32(), args[1].asNumber().toInt32(), args[2].asNumber().toInt32()};
            index = 3;
        }
#endif // ENABLE_NUMBERS_AS_POS
        else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }

        return Boolean::newBoolean(sp->simulateInteract(bpos, face));
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

Local<Value> PlayerClass::simulateJump(const Arguments&) {
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        return Boolean::newBoolean(sp->simulateJump());
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

Local<Value> PlayerClass::simulateRespawn(const Arguments&) {
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        if (sp->simulateRespawn()) {
            auto& spawnPoint = sp->mPlayerRespawnPoint;
            get()->teleport(spawnPoint->mPlayerPosition->bottomCenter(), spawnPoint->mDimension);
            return Boolean::newBoolean(true);
        } else {
            return Boolean::newBoolean(false);
        }
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

Local<Value> PlayerClass::simulateLocalMove(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        Vec3   target;
        float  speed = 1.0f;
        size_t index = 0;
        if (IsInstanceOf<IntPos>(args[0])) {
            auto pos  = IntPos::extractPos(args[0]);
            target    = pos->getBlockPos();
            index    += 1;
        } else if (IsInstanceOf<FloatPos>(args[0])) {
            auto pos  = FloatPos::extractPos(args[0]);
            target    = pos->getVec3();
            index    += 1;
        }
#ifdef ENABLE_NUMBERS_AS_POS
        else if (args[0].isNumber()) {
            CHECK_ARGS_COUNT(args, 3);
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            Vec3 pos  = Vec3(args[0].asNumber().toFloat(), args[1].asNumber().toFloat(), args[2].asNumber().toFloat());
            index    += 3;
        }
#endif // ENABLE_NUMBERS_AS_POS
        else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }

        if (args.size() > index) {
            CHECK_ARG_TYPE(args[index], ValueKind::kNumber);
            speed = args[index].asNumber().toFloat();
        }

        sp->simulateLocalMove(target, speed);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in " __FUNCTION__ "!")
}

Local<Value> PlayerClass::simulateWorldMove(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        Vec3   target;
        float  speed = 1.0f;
        size_t index = 0;
        if (IsInstanceOf<IntPos>(args[0])) {
            auto pos  = IntPos::extractPos(args[0]);
            target    = pos->getBlockPos();
            index    += 1;
        } else if (IsInstanceOf<FloatPos>(args[0])) {
            auto pos  = FloatPos::extractPos(args[0]);
            target    = pos->getVec3();
            index    += 1;
        }
#ifdef ENABLE_NUMBERS_AS_POS
        else if (args[0].isNumber()) {
            CHECK_ARGS_COUNT(args, 3);
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            Vec3 pos  = Vec3(args[0].asNumber().toFloat(), args[1].asNumber().toFloat(), args[2].asNumber().toFloat());
            index    += 3;
        }
#endif // ENABLE_NUMBERS_AS_POS
        else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }

        if (args.size() > index) {
            CHECK_ARG_TYPE(args[index], ValueKind::kNumber);
            speed = args[index].asNumber().toFloat();
        }

        sp->simulateWorldMove(target, speed);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

Local<Value> PlayerClass::simulateMoveTo(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        Vec3   target;
        float  speed = 1.0f;
        size_t index = 0;
        if (IsInstanceOf<IntPos>(args[0])) {
            auto pos  = IntPos::extractPos(args[0]);
            target    = pos->getBlockPos();
            index    += 1;
        } else if (IsInstanceOf<FloatPos>(args[0])) {
            auto pos  = FloatPos::extractPos(args[0]);
            target    = pos->getVec3();
            index    += 1;
        }
#ifdef ENABLE_NUMBERS_AS_POS
        else if (args[0].isNumber()) {
            CHECK_ARGS_COUNT(args, 3);
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            Vec3 pos  = Vec3(args[0].asNumber().toFloat(), args[1].asNumber().toFloat(), args[2].asNumber().toFloat());
            index    += 3;
        }
#endif // ENABLE_NUMBERS_AS_POS
        else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }

        if (args.size() > index) {
            CHECK_ARG_TYPE(args[index], ValueKind::kNumber);
            speed = args[index].asNumber().toFloat();
        }

        sp->simulateMoveToLocation(target, speed, true);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

Local<Value> PlayerClass::simulateLookAt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        int  dimid        = sp->getDimensionId().id;
        auto lookDuration = sim::LookDuration::UntilMove;
        if (args.size() > 1) {
            if (!args[1].isNumber()) {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
            }
            lookDuration = static_cast<sim::LookDuration>(args[1].asNumber().toInt32());
        }
        if (IsInstanceOf<IntPos>(args[0])) {
            auto pos = IntPos::extractPos(args[0]);
            auto did = pos->getDimensionId();
            if (dimid == did || did < 0 || did > 2) {
                sp->simulateLookAt(pos->getBlockPos(), lookDuration);
                return Boolean::newBoolean(true);
            }
            lse::LegacyScriptEngine::getLogger().debug(
                "Can't simulate look at other dimension!"
            );
            return Boolean::newBoolean(false);
        } else if (IsInstanceOf<FloatPos>(args[0])) {
            auto pos = FloatPos::extractPos(args[0]);
            auto did = pos->getDimensionId();
            if (dimid == did || did < 0 || did > 2) {
                sp->simulateLookAt(pos->getVec3(), (sim::LookDuration)lookDuration);
                return Boolean::newBoolean(true);
            }
            lse::LegacyScriptEngine::getLogger().debug(
                "Can't simulate look at other dimension!"
            );
            return Boolean::newBoolean(false);
        } else if (IsInstanceOf<BlockClass>(args[0])) {
            auto block = EngineScope::currentEngine()->getNativeInstance<BlockClass>(args[0]);
            auto pos   = IntPos::extractPos(block->getPos());
            auto did   = pos->getDimensionId();
            if (dimid == did || did < 0 || did > 2) {
                sp->simulateLookAt(pos->getBlockPos(), (sim::LookDuration)lookDuration);
                return Boolean::newBoolean(true);
            }
            lse::LegacyScriptEngine::getLogger().debug(
                "Can't simulate look at other dimension!"
            );
            return Boolean::newBoolean(false);
        } else if (auto actor = EntityClass::tryExtractActor(args[0])) {
            sp->simulateLookAt(*actor, (sim::LookDuration)lookDuration);
            return Boolean::newBoolean(true);
        }
        LOG_WRONG_ARG_TYPE(__FUNCTION__);
        return Local<Value>();
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

// void simulateSetBodyRotation(float);
Local<Value> PlayerClass::simulateSetBodyRotation(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        sp->simulateSetBodyRotation(args[0].asNumber().toFloat());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in " __FUNCTION__ "!")
}

// void simulateWorldMove(class Vec3 const&, float);
// void simulateMoveToLocation(class Vec3 const&, float);

inline Local<Value> NavigateResultToObject(ScriptModuleGameTest::ScriptNavigationResult const& res) {
    auto obj = Object::newObject();
    obj.set(String::newString("isFullPath"), Boolean::newBoolean(res.mIsFullPath));
    auto path = Array::newArray();
    for (auto& pos : *res.mPath) {
        path.add(Array::newArray({Number::newNumber(pos.x), Number::newNumber(pos.y), Number::newNumber(pos.z)}));
    }
    obj.set(String::newString("path"), path);
    return obj;
}

// struct ScriptNavigationResult simulateNavigateToEntity(class Actor&, float);
// struct ScriptNavigationResult simulateNavigateToLocation(class Vec3 const&,
// float); void simulateNavigateToLocations(std::vector<class Vec3>&&, float);
Local<Value> PlayerClass::simulateNavigateTo(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {

        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        float speed = 1.f;
        if (args.size() > 1) {
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            speed = args[1].asNumber().toFloat();
        }
        if (args[0].isArray()) {
            auto              arr = args[0].asArray();
            std::vector<Vec3> path;
            for (size_t index = 0; index < arr.size(); ++index) {
                if (IsInstanceOf<IntPos>(arr.get(index)))
                    path.emplace_back(IntPos::extractPos(arr.get(index))->getBlockPos().center());
                else if (IsInstanceOf<FloatPos>(arr.get(index)))
                    path.emplace_back(FloatPos::extractPos(arr.get(index))->getVec3());
                else if (arr.get(index).isArray()) {
                    auto posArr = arr.get(index).asArray();
                    if (posArr.size() != 3 || !posArr.get(0).isNumber()) {
                        LOG_WRONG_ARG_TYPE(__FUNCTION__);
                        return Local<Value>();
                    }
                    path.emplace_back(
                        posArr.get(0).asNumber().toFloat(),
                        posArr.get(1).asNumber().toFloat(),
                        posArr.get(2).asNumber().toFloat()
                    );
                } else {
                    LOG_WRONG_ARG_TYPE(__FUNCTION__);
                    return Local<Value>();
                }
            }
            sp->simulateNavigateToLocations(std::move(path), speed);
            return Boolean::newBoolean(true);
        } else if (auto actor = EntityClass::tryExtractActor(args[0])) {
            auto res = sp->simulateNavigateToEntity(*actor, speed);
            return NavigateResultToObject(res);
        } else if (IsInstanceOf<IntPos>(args[0]) || IsInstanceOf<FloatPos>(args[0])) {
            Vec3 pos = IsInstanceOf<IntPos>(args[0]) ? IntPos::extractPos(args[0])->getBlockPos().bottomCenter()
                                                     : FloatPos::extractPos(args[0])->getVec3();
            auto res = sp->simulateNavigateToLocation(pos, speed);
            return NavigateResultToObject(res);
        }
#ifdef ENABLE_NUMBERS_AS_POS
        else if (args[0].isNumber()) {
            CHECK_ARGS_COUNT(args, 3);
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            if (args.size() > 3) {
                CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
                speed = args[3].asNumber().toFloat();
            }
            Vec3 pos = Vec3(args[0].asNumber().toFloat(), args[1].asNumber().toFloat(), args[2].asNumber().toFloat());
            auto res = sp->simulateNavigateToLocation(pos, speed);
            return NavigateResultToObject(res);
        }
#endif // ENABLE_NUMBERS_AS_POS
        else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

// bool simulateSetItem(class ItemStack&, bool, int);

// bool simulateUseItem();
// bool simulateUseItemInSlot(int);
// bool simulateUseItem(class ItemStack&);
// bool simulateUseItemInSlotOnBlock(int, class BlockPos const&, enum
// ScriptFacing, class Vec3 const&); bool simulateUseItemOnBlock(class
// ItemStack&, class BlockPos const&, enum ScriptFacing, class Vec3 const&);
Local<Value> PlayerClass::simulateUseItem(const Arguments& args) {
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();

        if (args.size() == 0) return Boolean::newBoolean(sp->simulateUseItem());

        int        slot = -1;
        ItemStack* item = nullptr;
        if (args[0].isNumber()) slot = args[0].asNumber().toInt32();
        else if (IsInstanceOf<ItemClass>(args[0])) item = ItemClass::extract(args[0]);
        else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        if (args.size() == 1) {
            if (item) return Boolean::newBoolean(SimulatedPlayerHelper::simulateUseItem(*sp, *item));
            else return Boolean::newBoolean(sp->simulateUseItemInSlot(slot));
        }

        BlockPos                            bpos;
        ScriptModuleMinecraft::ScriptFacing face        = (ScriptModuleMinecraft::ScriptFacing)0;
        Vec3                                relativePos = {0.5, 0.5, 0.5};
        if (IsInstanceOf<IntPos>(args[1])) bpos = IntPos::extractPos(args[1])->getBlockPos();
        else if (IsInstanceOf<FloatPos>(args[1])) bpos = FloatPos::extractPos(args[1])->getVec3();
        else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        if (args.size() > 2) {
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            face = (ScriptModuleMinecraft::ScriptFacing)args[2].asNumber().toInt32();
            if (args.size() > 3) {
                if (IsInstanceOf<FloatPos>(args[3])) {
                    relativePos = FloatPos::extractPos(args[3])->getVec3();
                } else {
                    LOG_WRONG_ARG_TYPE(__FUNCTION__);
                    return Local<Value>();
                }
            }
        }
        if (item) return Boolean::newBoolean(sp->simulateUseItemOnBlock(*item, bpos, face, relativePos));
        else
            return Boolean::newBoolean(
                SimulatedPlayerHelper::simulateUseItemInSlotOnBlock(*sp, slot, bpos, face, relativePos)
            );
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

Local<Value> PlayerClass::simulateStopDestroyingBlock(const Arguments&) {
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        sp->simulateStopDestroyingBlock();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

Local<Value> PlayerClass::simulateStopInteracting(const Arguments&) {
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        sp->deleteContainerManager();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

Local<Value> PlayerClass::simulateStopMoving(const Arguments&) {
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        sp->simulateStopMoving();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

Local<Value> PlayerClass::simulateStopUsingItem(const Arguments&) {
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();
        sp->simulateStopUsingItem();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in " __FUNCTION__ "!")
};

Local<Value> PlayerClass::simulateStopSneaking(const Arguments&) {
    try {
        auto sp = asSimulatedPlayer();
        if (!sp) return Local<Value>();

        return Boolean::newBoolean(sp->simulateStopSneaking());
    }
    CATCH("Fail in " __FUNCTION__ "!")
}
