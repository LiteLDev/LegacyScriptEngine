#include "legacy/api/ScriptAPI.h"

#include "legacy/api/APIHelp.h"
#include "legacy/engine/EngineOwnData.h"
#include "legacy/engine/GlobalShareData.h"
#include "legacy/engine/LocalShareData.h"
#include "legacy/engine/TimeTaskSystem.h"

#include <chrono>
#include <memory>
#include <sstream>

using ll::hash_utils::doHash;

//////////////////// APIs ////////////////////

Local<Value> Log(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        std::ostringstream sout;
        for (int i = 0; i < args.size(); ++i) PrintValue(sout, args[i]);
        sout << std::endl;
        getEngineOwnData()->logger->info(sout.str());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> ColorLog(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        std::string prefix;
        switch (doHash(args[0].asString().toString())) {
        case doHash("dk_blue"):
            prefix = "\x1b[34m";
            break;
        case doHash("dk_green"):
            prefix = "\x1b[32m";
            break;
        case doHash("bt_blue"):
            prefix = "\x1b[36m";
            break;
        case doHash("dk_red"):
            prefix = "\x1b[31m";
            break;
        case doHash("purple"):
            prefix = "\x1b[35m";
            break;
        case doHash("dk_yellow"):
            prefix = "\x1b[33m";
            break;
        case doHash("grey"):
            prefix = "\x1b[37m";
            break;
        case doHash("sky_blue"):
            prefix = "\x1b[94m";
            break;
        case doHash("blue"):
            prefix = "\x1b[94m";
            break;
        case doHash("green"):
            prefix = "\x1b[92m";
            break;
        case doHash("cyan"):
            prefix = "\x1b[36m";
            break;
        case doHash("red"):
            prefix = "\x1b[91m";
            break;
        case doHash("pink"):
            prefix = "\x1b[95m";
            break;
        case doHash("yellow"):
            prefix = "\x1b[93m";
            break;
        case doHash("white"):
            prefix = "\x1b[97m";
            break;
        default:
            throw CreateExceptionWithInfo(__FUNCTION__, "Invalid color!");
        }
        std::ostringstream sout;
        sout << prefix;
        for (int i = 1; i < args.size(); ++i) PrintValue(sout, args[i]);
        sout << "\x1b[0m";
        getEngineOwnData()->logger->info(sout.str());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> FastLog(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        std::ostringstream sout;
        for (int i = 0; i < args.size(); ++i) PrintValue(sout, args[i]);

        pool.execute([str{sout.str()}, pluginName{getEngineOwnData()->pluginName}]() {
            ll::io::LoggerRegistry::getInstance().getOrCreate(pluginName)->info(str);
        });
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

//////////////////// APIs ////////////////////

Local<Value> SetTimeout(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2)
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber)
    try {
        bool isFunc = args[0].getKind() == ValueKind::kFunction;
        if (!isFunc && args[0].getKind() != ValueKind::kString) {
            throw WrongArgTypeException(__FUNCTION__);
        }

        int timeout = args[1].asNumber().toInt32();
        if (timeout <= 0) timeout = 1;

        if (isFunc) return Number::newNumber(NewTimeout(args[0].asFunction(), {}, timeout));
        return Number::newNumber(NewTimeout(args[0].asString(), timeout));
    }
    CATCH_AND_THROW
}

Local<Value> SetInterval(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2)
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber)

    try {
        bool isFunc = args[0].getKind() == ValueKind::kFunction;
        if (!isFunc && args[0].getKind() != ValueKind::kString) {
            throw WrongArgTypeException(__FUNCTION__);
        }

        int timeout = args[1].asNumber().toInt32();
        if (timeout <= 0) timeout = 1;

        if (isFunc) return Number::newNumber(NewInterval(args[0].asFunction(), {}, timeout));
        return Number::newNumber(NewInterval(args[0].asString(), timeout));
    }
    CATCH_AND_THROW
}

// ClearInterval
Local<Value> ClearInterval(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber)

    try {
        return Boolean::newBoolean(ClearTimeTask(args[0].asNumber().toInt32()));
    }
    CATCH_AND_THROW
}
