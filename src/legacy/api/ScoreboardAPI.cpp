#include "api/ScoreboardAPI.h"

#include "api/APIHelp.h"
#include "api/McAPI.h"
#include "api/PlayerAPI.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/scores/DisplayObjective.h"
#include "mc/world/scores/Objective.h"
#include "mc/world/scores/ScoreInfo.h"
#include "mc/world/scores/Scoreboard.h"
#include "mc/world/scores/ScoreboardId.h"

#include <optional>

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
        objname = obj->getName();
        isValid = true;
    }
}

Objective* ObjectiveClass::get() {
    if (isValid) return ll::service::getLevel()->getScoreboard().getObjective(objname);
    return nullptr;
}

Local<Value> ObjectiveClass::getName() {
    try {
        return String::newString(objname);
    }
    CATCH("Fail in getName!")
}

Local<Value> ObjectiveClass::getDisplayName() {
    try {
        Objective* obj = get();
        if (!obj) return Local<Value>();
        return String::newString(obj->getDisplayName());
    }
    CATCH("Fail in getDisplayName!")
}

Local<Value> ObjectiveClass::setDisplay(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    if (args.size() == 2) CHECK_ARG_TYPE(args[1], ValueKind::kNumber)

    try {
        Objective* obj = get();
        if (!obj) return Local<Value>();

        std::string slot = args[0].asString().toString();
        int         sort = 0;
        if (args.size() == 2) sort = args[1].asNumber().toInt32();
        return Boolean::newBoolean(
            ll::service::getLevel()->getScoreboard().setDisplayObjective(slot, *obj, (ObjectiveSortOrder)sort)
        );
    }
    CATCH("Fail in setDisplay");
}

///////////////////////////////////////////////////////////////////

Local<Value> ObjectiveClass::setScore(const Arguments& args) {
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
            const ScoreboardId& id = scoreboard.getScoreboardId(name);
            if (!id.isValid()) {
                scoreboard.createScoreboardId(name);
            }
            bool isSuccess = false;
            scoreboard.modifyPlayerScore(isSuccess, id, *obj, score, PlayerScoreSetFunction::Set);
            if (isSuccess) return Number::newNumber(score);
            return Local<Value>();
        } else if (IsInstanceOf<PlayerClass>(args[0])) {
            Player*     player     = PlayerClass::extract(args[0]);
            Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
            Objective*  obj        = get();
            if (!obj) {
                return Boolean::newBoolean(false);
            }
            const ScoreboardId& id = scoreboard.getScoreboardId(*player);
            if (!id.isValid()) {
                scoreboard.createScoreboardId(*player);
            }
            bool isSuccess = false;
            scoreboard
                .modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Set);
            if (isSuccess) return Number::newNumber(score);
            return Local<Value>();
        } else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        return Local<Value>();
    }
    CATCH("Fail in setScore");
}

Local<Value> ObjectiveClass::addScore(const Arguments& args) {
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
            const ScoreboardId& id = scoreboard.getScoreboardId(name);
            if (!id.isValid()) {
                scoreboard.createScoreboardId(name);
            }
            bool isSuccess = false;
            scoreboard
                .modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Add);
            if (isSuccess) return Number::newNumber(score);
            return Local<Value>();
        } else if (IsInstanceOf<PlayerClass>(args[0])) {
            Player*     player     = PlayerClass::extract(args[0]);
            Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
            Objective*  obj        = get();
            if (!obj) {
                return Boolean::newBoolean(false);
            }
            const ScoreboardId& id = scoreboard.getScoreboardId(*player);
            if (!id.isValid()) {
                scoreboard.createScoreboardId(*player);
            }
            bool isSuccess = false;
            scoreboard
                .modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Add);
            if (isSuccess) return Number::newNumber(score);
            return Local<Value>();
        } else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        return Local<Value>();
    }
    CATCH("Fail in addScore");
}

Local<Value> ObjectiveClass::reduceScore(const Arguments& args) {
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
            const ScoreboardId& id = scoreboard.getScoreboardId(name);
            if (!id.isValid()) {
                scoreboard.createScoreboardId(name);
            }
            bool isSuccess = false;
            scoreboard
                .modifyPlayerScore(isSuccess, id, *obj, args[1].asNumber().toInt32(), PlayerScoreSetFunction::Subtract);
            if (isSuccess) return Number::newNumber(score);
            return Local<Value>();
        } else if (IsInstanceOf<PlayerClass>(args[0])) {
            Player*     player     = PlayerClass::extract(args[0]);
            Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
            Objective*  obj        = get();
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
            if (isSuccess) return Number::newNumber(score);
            return Local<Value>();
        } else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        return Local<Value>();
    }
    CATCH("Fail in removeScore");
}

Local<Value> ObjectiveClass::deleteScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        if (args[0].isString()) {
            Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
            Objective*  obj        = get();
            if (!obj) {
                return Boolean::newBoolean(false);
            }
            const ScoreboardId& id = scoreboard.getScoreboardId(args[0].asString().toString());
            if (!id.isValid()) {
                return Boolean::newBoolean(true);
            }
            // obj->_resetPlayer(id);
            scoreboard.resetPlayerScore(id, *obj);
            return Boolean::newBoolean(true);
        } else if (IsInstanceOf<PlayerClass>(args[0])) {
            Player*     player     = PlayerClass::extract(args[0]);
            Scoreboard& scoreboard = ll::service::getLevel()->getScoreboard();
            Objective*  obj        = get();
            if (!obj) {
                return Boolean::newBoolean(false);
            }
            const ScoreboardId& id = scoreboard.getScoreboardId(*player);
            if (!id.isValid()) {
                return Boolean::newBoolean(true);
            }
            // obj->_resetPlayer(id);
            scoreboard.resetPlayerScore(id, *obj);
            return Boolean::newBoolean(true);
        } else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
    }
    CATCH("Fail in deleteScore");
}

Local<Value> ObjectiveClass::getScore(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        if (args[0].isString()) {
            Scoreboard&  board     = ll::service::getLevel()->getScoreboard();
            Objective*   objective = board.getObjective(objname);
            ScoreboardId sid       = board.getScoreboardId(args[0].asString().toString());
            if (!objective || !sid.isValid() || !objective->hasScore(sid)) {
                return {};
            }
            return Number::newNumber(objective->getPlayerScore(sid).mValue);
        } else if (IsInstanceOf<PlayerClass>(args[0])) {
            Scoreboard&  board     = ll::service::getLevel()->getScoreboard();
            Objective*   objective = board.getObjective(objname);
            ScoreboardId sid       = board.getScoreboardId(*PlayerClass::extract(args[0]));
            if (!objective || !sid.isValid() || !objective->hasScore(sid)) {
                return {};
            }
            return Number::newNumber(objective->getPlayerScore(sid).mValue);
        } else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
    }
    CATCH("Fail in getScore");
}

//////////////////// APIs ////////////////////

Local<Value> McClass::getDisplayObjective(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        std::string slot = args[0].asString().toString();
        auto        res  = ll::service::getLevel()->getScoreboard().getDisplayObjective(slot);

        if (!res) return Local<Value>();
        return ObjectiveClass::newObjective(const_cast<Objective*>(res->mObjective));
    }
    CATCH("Fail in GetDisplayObjective");
}

Local<Value> McClass::clearDisplayObjective(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        std::string slot = args[0].asString().toString();
        auto        res  = ll::service::getLevel()->getScoreboard().clearDisplayObjective(slot);

        if (!res) return Boolean::newBoolean(false);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in ClearDisplayObjective");
}

Local<Value> McClass::getScoreObjective(const Arguments& args) {
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
    CATCH("Fail in GetScoreObjective");
}

Local<Value> McClass::newScoreObjective(const Arguments& args) {
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
    CATCH("Fail in NewScoreObjective!")
}

Local<Value> McClass::removeScoreObjective(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        std::string name = args[0].asString().toString();
        auto        obj  = ll::service::getLevel()->getScoreboard().getObjective(name);
        if (obj) {
            ll::service::getLevel()->getScoreboard().removeObjective(obj);
            return Boolean::newBoolean(true);
        }
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in RemoveScoreObjective!")
}

Local<Value> McClass::getAllScoreObjectives(const Arguments&) {
    try {
        Local<Array> res = Array::newArray();

        auto objs = ll::service::getLevel()->getScoreboard().getObjectives();
        for (auto& obj : objs) {
            if (obj) res.add(ObjectiveClass::newObjective(const_cast<Objective*>(obj)));
        }
        return res;
    }
    CATCH("Fail in GetAllScoreObjectives!")
}
