#include "api/LoggerAPI.h"

#include "api/APIHelp.h"
#include "api/PlayerAPI.h"
#include "engine/EngineOwnData.h"
#include "ll/api/data/IndirectValue.h"
#include "ll/api/io/DefaultSink.h"
#include "ll/api/io/FileSink.h"
#include "ll/api/io/Logger.h"
#include "ll/api/io/PatternFormatter.h"
#include "lse/api/PlayerSink.h"
#include "mc/world/actor/player/Player.h"
#include "utils/Utils.h"

#include <string>

using ll::io::LogLevel;

//////////////////// Classes ////////////////////

ClassDefine<void> LoggerClassBuilder = defineClass("logger")
                                           .function("log", &LoggerClass::log)
                                           .function("debug", &LoggerClass::debug)
                                           .function("info", &LoggerClass::info)
                                           .function("warn", &LoggerClass::warn)
                                           .function("warning", &LoggerClass::warn)
                                           .function("error", &LoggerClass::error)
                                           .function("fatal", &LoggerClass::fatal)

                                           .function("setTitle", &LoggerClass::setTitle)
                                           .function("setConsole", &LoggerClass::setConsole)
                                           .function("setFile", &LoggerClass::setFile)
                                           .function("setPlayer", &LoggerClass::setPlayer)
                                           .function("setLogLevel", &LoggerClass::setLogLevel)
                                           .build();

////////////////// Helper //////////////////
std::string inline GetTimeStrHelper() { return Raw_GetDateTimeStr(); }

std::string& StrReplace(std::string& str, std::string const& to_replaced, std::string const& new_str) {
    for (std::string::size_type pos(0); pos != std::string::npos; pos += new_str.length()) {
        pos = str.find(to_replaced, pos);
        if (pos != std::string::npos) str.replace(pos, to_replaced.length(), new_str);
        else break;
    }
    return str;
}
////////////////// Helper //////////////////

void inline LogDataHelper(LogLevel level, Arguments const& args) {
    std::string res;
    for (int i = 0; i < args.size(); ++i) res += ValueToString(args[i]);
    getEngineOwnData()->logger->log(level, res);
}

Local<Value> LoggerClass::log(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = getEngineOwnData();
        LogDataHelper(LogLevel::Info, args);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> LoggerClass::debug(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = getEngineOwnData();
        LogDataHelper(LogLevel::Debug, args);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> LoggerClass::info(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = getEngineOwnData();
        LogDataHelper(LogLevel::Info, args);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> LoggerClass::warn(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = getEngineOwnData();
        LogDataHelper(LogLevel::Warn, args);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> LoggerClass::error(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = getEngineOwnData();
        LogDataHelper(LogLevel::Error, args);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> LoggerClass::fatal(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = getEngineOwnData();
        LogDataHelper(LogLevel::Fatal, args);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

// Deprecated
Local<Value> LoggerClass::setTitle(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    getEngineOwnData()->logger = ll::io::LoggerRegistry::getInstance().getOrCreate(args[0].asString().toString());
    return Boolean::newBoolean(true);
}

///////////////// Helper /////////////////

Local<Value> LoggerClass::setConsole(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kBoolean)
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kNumber)

    try {
        if (args.size() >= 2) {
            getEngineOwnData()->logger->getSink(0)->setFlushLevel(
                static_cast<LogLevel>(args[1].asNumber().toInt32() - 1)
            ); // See LSE's definition https://lse.levimc.org/apis/ScriptAPI/Logger/
        }
        if (!args[0].asBoolean().value()) {
            getEngineOwnData()->logger->getSink(0)->setFlushLevel(LogLevel::Off);
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> LoggerClass::setFile(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kNumber)

    try {
        std::filesystem::path             newFile = std::filesystem::path(args[0].asString().toString());
        std::shared_ptr<ll::io::FileSink> sink    = std::make_shared<ll::io::FileSink>(
            newFile,
            ll::makePolymorphic<ll::io::PatternFormatter>("{3:.3%T.} {2} {1} {0}", false),
            std::ios::app
        );
        if (args.size() >= 2) {
            sink->setFlushLevel(static_cast<LogLevel>(args[1].asNumber().toInt32() - 1));
        }
        bool hasFileSink = false;
        for (auto& sk : getEngineOwnData()->logger->sinks()) {
            if (typeid(sk) == typeid(ll::io::FileSink)) {
                hasFileSink = true;
            }
        }
        auto logger = getEngineOwnData()->logger;
        if (hasFileSink) {
            logger->clearSink();
            logger->addSink(std::make_shared<ll::io::DefaultSink>());
        }
        return Boolean::newBoolean(logger->addSink(sink));
    }
    CATCH_AND_THROW
}

Local<Value> LoggerClass::setPlayer(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)
    try {
        Player* player = PlayerClass::extract(args[0]);
        if (!player) {
            return Boolean::newBoolean(false);
        }
        std::shared_ptr<lse::io::PlayerSink> sink = std::make_shared<lse::io::PlayerSink>(player->getUuid());
        if (args.size() >= 2) {
            sink->setFlushLevel(static_cast<LogLevel>(args[1].asNumber().toInt32() - 1));
        }
        return Boolean::newBoolean(getEngineOwnData()->logger->addSink(sink));
    }
    CATCH_AND_THROW
}

Local<Value> LoggerClass::setLogLevel(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber)

    try {
        auto conf = getEngineOwnData();
        conf->logger->setLevel(static_cast<LogLevel>(args[0].asNumber().toInt32() - 1));
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}
