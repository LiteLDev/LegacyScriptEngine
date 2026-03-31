#include "legacy/api/ScoreboardAPI.h"

#include "legacy/api/APIHelp.h"
#include "legacy/api/McAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/scores/DisplayObjective.h"
#include "mc/world/scores/Objective.h"
#include "mc/world/scores/ScoreInfo.h"
#include "mc/world/scores/Scoreboard.h"
#include "mc/world/scores/ScoreboardId.h"
#include "mc/world/scores/ScoreboardOperationResult.h"

//////////////////// Class Definition ////////////////////

ClassDefine<ObjectiveClass> ObjectiveClassBuilder =
    defineClass<ObjectiveClass>("LLSE_Objective")
        .constructor(nullptr)
        .instanceProperty("name", &ObjectiveClass::getName)
        .instanceProperty("displayName", &ObjectiveClass::getDisplayName)

        .instanceFunction("setDisplay", &ObjectiveClass::setDisplay)
        .instanceFunction("setScore", &ObjectiveClass::setScore)
        .instanceFunction("addScore", &ObjectiveClass::addScore)
        .instanceFunction("reduceScore", &ObjectiveClass::reduceScore)
        .instanceFunction("deleteScore", &ObjectiveClass::deleteScore)
        .instanceFunction("getScore", &ObjectiveClass::getScore)
        .build();

//////////////////// Classes ////////////////////

Local<Object> ObjectiveClass::newObjective(Objective* obj) {
    auto newp = new ObjectiveClass(obj);
    return newp->getScriptObject();
}

void ObjectiveClass::set(Objective* obj) {
    if (obj) {
        objname = obj->mName;
        isValid = true;
    }
}

Objective* ObjectiveClass::get() const {
    if (isValid) return ll::service::getLevel()->getScoreboard().getObjective(objname);
    return nullptr;
}

Local<Value> ObjectiveClass::getName() const {
    try {
        return String::newString(objname);
    }
    CATCH_AND_THROW
}

Local<Value> ObjectiveClass::getDisplayName() const {
    try {
        Objective* obj = get();
        if (!obj) return {};
        return String::newString(obj->mDisplayName);
    }
    CATCH_AND_THROW
}

Local<Value> ObjectiveClass::setDisplay(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    if (args.size() == 2) CHECK_ARG_TYPE(args[1], ValueKind::kNumber)

    try {
        Objective* obj = get();
        if (!obj) return {};

        std::string slot = args[0].asString().toString();
        int         sort = 0;
        if (args.size() == 2) sort = args[1].asNumber().toInt32();
        return Boolean::newBoolean(
            ll::service::getLevel()
                ->getScoreboard()
                .setDisplayObjective(slot, *obj, static_cast<ObjectiveSortOrder>(sort))
        );
    }
    CATCH_AND_THROW
}

///////////////////////////////////////////////////////////////////

Local<Value> ObjectiveClass::setScore(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2)
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber)

    try {
        int score = args[1].asNumber().toInt32();

        if (args[0].isString()) {
            std::string name       = args[0].asString().toString();
            Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
            Objective*  obj        = get();
            if (!obj) {
                return Boolean::newBoolean(false);
            }
            ScoreboardId const& id = scoreboard.getScoreboardId(name);
            if (id.mRawID == ScoreboardId::INVALID().mRawID) {
                scoreboard.createScoreboardId(name);
            }
            ScoreboardOperationResult isSuccess;
            scoreboard.modifyPlayerScore(isSuccess, id, *obj, score, PlayerScoreSetFunction::Set);
            if (isSuccess == ScoreboardOperationResult::Success) return Number::newNumber(score);
        } else if (IsInstanceOf<PlayerClass>(args[0])) {
            Player*     player     = PlayerClass::extract(args[0]);
            Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
            Objective*  obj        = get();
            if (!obj) {
                return Boolean::newBoolean(false);
            }
            ScoreboardId const& id = scoreboard.getScoreboardId(*player);
            if (id.mRawID == ScoreboardId::INVALID().mRawID) {
                scoreboard.createScoreboardId(*player);
            }
            ScoreboardOperationResult isSuccess;
            scoreboard
                .modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Set);
            if (isSuccess == ScoreboardOperationResult::Success) return Number::newNumber(score);
        } else {
            throw WrongArgTypeException(__FUNCTION__);
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> ObjectiveClass::addScore(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2)
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber)

    try {
        int score = args[1].asNumber().toInt32();

        if (args[0].isString()) {
            std::string name       = args[0].asString().toString();
            Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
            Objective*  obj        = get();
            if (!obj) {
                return Boolean::newBoolean(false);
            }
            ScoreboardId const& id = scoreboard.getScoreboardId(name);
            if (id.mRawID == ScoreboardId::INVALID().mRawID) {
                scoreboard.createScoreboardId(name);
            }
            ScoreboardOperationResult isSuccess;
            scoreboard
                .modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Add);
            if (isSuccess == ScoreboardOperationResult::Success) return Number::newNumber(score);
        } else if (IsInstanceOf<PlayerClass>(args[0])) {
            Player*     player     = PlayerClass::extract(args[0]);
            Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
            Objective*  obj        = get();
            if (!obj) {
                return Boolean::newBoolean(false);
            }
            ScoreboardId const& id = scoreboard.getScoreboardId(*player);
            if (id.mRawID == ScoreboardId::INVALID().mRawID) {
                scoreboard.createScoreboardId(*player);
            }
            ScoreboardOperationResult isSuccess;
            scoreboard
                .modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Add);
            if (isSuccess == ScoreboardOperationResult::Success) return Number::newNumber(score);
        } else {
            throw WrongArgTypeException(__FUNCTION__);
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> ObjectiveClass::reduceScore(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2)
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber)

    try {
        int score = args[1].asNumber().toInt32();

        if (args[0].isString()) {
            std::string name       = args[0].asString().toString();
            Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
            Objective*  obj        = get();
            if (!obj) {
                return Boolean::newBoolean(false);
            }
            ScoreboardId const& id = scoreboard.getScoreboardId(name);
            if (id.mRawID == ScoreboardId::INVALID().mRawID) {
                scoreboard.createScoreboardId(name);
            }
            ScoreboardOperationResult isSuccess;
            scoreboard
                .modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Subtract);
            if (isSuccess == ScoreboardOperationResult::Success) return Number::newNumber(score);
        } else if (IsInstanceOf<PlayerClass>(args[0])) {
            Player*     player     = PlayerClass::extract(args[0]);
            Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
            Objective*  obj        = get();
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
            if (isSuccess == ScoreboardOperationResult::Success) return Number::newNumber(score);
        } else {
            throw WrongArgTypeException(__FUNCTION__);
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> ObjectiveClass::deleteScore(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1)

    try {
        if (args[0].isString()) {
            Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
            Objective*  obj        = get();
            if (!obj) {
                return Boolean::newBoolean(false);
            }
            ScoreboardId const& id = scoreboard.getScoreboardId(args[0].asString().toString());
            if (id.mRawID == ScoreboardId::INVALID().mRawID) {
                return Boolean::newBoolean(true);
            }
            // obj->_resetPlayer(id);
            scoreboard.resetPlayerScore(id, *obj);
            return Boolean::newBoolean(true);
        }
        if (IsInstanceOf<PlayerClass>(args[0])) {
            Player*     player     = PlayerClass::extract(args[0]);
            Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
            Objective*  obj        = get();
            if (!obj) {
                return Boolean::newBoolean(false);
            }
            ScoreboardId const& id = scoreboard.getScoreboardId(*player);
            if (id.mRawID == ScoreboardId::INVALID().mRawID) {
                return Boolean::newBoolean(true);
            }
            // obj->_resetPlayer(id);
            scoreboard.resetPlayerScore(id, *obj);
            return Boolean::newBoolean(true);
        }
        throw WrongArgTypeException(__FUNCTION__);
    }
    CATCH_AND_THROW
}

Local<Value> ObjectiveClass::getScore(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1)

    try {
        if (args[0].isString()) {
            Scoreboard&  board     = ll::service::getLevel()->getScoreboard();
            Objective*   objective = board.getObjective(objname);
            ScoreboardId sid       = board.getScoreboardId(args[0].asString().toString());
            if (!objective || sid.mRawID == ScoreboardId::INVALID().mRawID || !objective->mScores->contains(sid)) {
                return {};
            }
            return Number::newNumber(objective->getPlayerScore(sid).mValue);
        }
        if (IsInstanceOf<PlayerClass>(args[0])) {
            Scoreboard&  board     = ll::service::getLevel()->getScoreboard();
            Objective*   objective = board.getObjective(objname);
            ScoreboardId sid       = board.getScoreboardId(*PlayerClass::extract(args[0]));
            if (!objective || sid.mRawID == ScoreboardId::INVALID().mRawID || !objective->mScores->contains(sid)) {
                return {};
            }
            return Number::newNumber(objective->getPlayerScore(sid).mValue);
        }
        throw WrongArgTypeException(__FUNCTION__);
    }
    CATCH_AND_THROW
}

//////////////////// APIs ////////////////////

Local<Value> McClass::getDisplayObjective(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        std::string slot = args[0].asString().toString();
        auto        res  = ll::service::getLevel()->getScoreboard().getDisplayObjective(slot);

        if (!res) return {};
        return ObjectiveClass::newObjective(const_cast<Objective*>(res->mObjective));
    }
    CATCH_AND_THROW
}

Local<Value> McClass::clearDisplayObjective(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        std::string slot = args[0].asString().toString();
        auto        res  = ll::service::getLevel()->getScoreboard().clearDisplayObjective(slot);

        if (!res) return Boolean::newBoolean(false);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::getScoreObjective(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        std::string name = args[0].asString().toString();
        if (ll::service::getLevel().has_value()) {
            auto res = ll::service::getLevel()->getScoreboard().getObjective(name);
            if (!res) {
                return {};
            };
            return ObjectiveClass::newObjective(res);
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> McClass::newScoreObjective(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kString)

    try {
        std::string name    = args[0].asString().toString();
        std::string display = name;
        if (args.size() >= 2) display = args[1].asString().toString();
        Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
        std::string criteria   = "dummy";
        Objective*  obj =
            scoreboard.addObjective(name, display, *const_cast<ObjectiveCriteria*>(scoreboard.getCriteria(criteria)));
        return obj ? ObjectiveClass::newObjective(obj) : Local<Value>();
    }
    CATCH_AND_THROW
}

Local<Value> McClass::removeScoreObjective(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        std::string name = args[0].asString().toString();
        if (auto obj = ll::service::getLevel()->getScoreboard().getObjective(name)) {
            ll::service::getLevel()->getScoreboard().removeObjective(obj);
            return Boolean::newBoolean(true);
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::getAllScoreObjectives(Arguments const&) {
    try {
        Local<Array> res = Array::newArray();

        auto objs = ll::service::getLevel()->getScoreboard().getObjectives();
        for (auto& obj : objs) {
            if (obj) res.add(ObjectiveClass::newObjective(const_cast<Objective*>(obj)));
        }
        return res;
    }
    CATCH_AND_THROW
}
