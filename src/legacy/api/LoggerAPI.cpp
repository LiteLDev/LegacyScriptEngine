#include "api/LoggerAPI.h"

#include "api/APIHelp.h"
#include "api/PlayerAPI.h"
#include "engine/EngineOwnData.h"
#include "ll/api/data/IndirectValue.h"
#include "ll/api/io/FileSink.h"
#include "ll/api/io/Logger.h"
#include "ll/api/io/PatternFormatter.h"
#include "lse/api/PlayerSink.h"
#include "mc/world/actor/player/Player.h"
#include "utils/Utils.h"

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

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

std::string& StrReplace(std::string& str, const std::string& to_replaced, const std::string& new_str) {
    for (std::string::size_type pos(0); pos != std::string::npos; pos += new_str.length()) {
        pos = str.find(to_replaced, pos);
        if (pos != std::string::npos) str.replace(pos, to_replaced.length(), new_str);
        else break;
    }
    return str;
}
////////////////// Helper //////////////////

void inline LogDataHelper(LogLevel level, const Arguments& args) {
    std::string res;
    for (int i = 0; i < args.size(); ++i) res += ValueToString(args[i]);
    getEngineOwnData()->logger->log(level, res);
}

Local<Value> LoggerClass::log(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = getEngineOwnData();
        LogDataHelper(LogLevel::Info, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerLog!")
}

Local<Value> LoggerClass::debug(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = getEngineOwnData();
        LogDataHelper(LogLevel::Debug, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerDebug!")
}

Local<Value> LoggerClass::info(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = getEngineOwnData();
        LogDataHelper(LogLevel::Info, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerInfo!")
}

Local<Value> LoggerClass::warn(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = getEngineOwnData();
        LogDataHelper(LogLevel::Warn, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerWarn!")
}

Local<Value> LoggerClass::error(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = getEngineOwnData();
        LogDataHelper(LogLevel::Error, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerError!")
}

Local<Value> LoggerClass::fatal(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = getEngineOwnData();
        LogDataHelper(LogLevel::Fatal, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerFatal!")
}

// Deprecated
Local<Value> LoggerClass::setTitle(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    getEngineOwnData()->logger = ll::io::LoggerRegistry::getInstance().getOrCreate(args[0].asString().toString());
    return Boolean::newBoolean(true);
}

///////////////// Helper /////////////////

Local<Value> LoggerClass::setConsole(const Arguments& args) {
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
    CATCH("Fail in LoggerSetConsole!")
}

Local<Value> LoggerClass::setFile(const Arguments& args) {
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
        return Boolean::newBoolean(getEngineOwnData()->logger->addSink(sink));
    }
    CATCH("Fail in LoggerSetFile!")
}

Local<Value> LoggerClass::setPlayer(const Arguments& args) {
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
    CATCH("Fail in LoggerSetPlayer!")
}

Local<Value> LoggerClass::setLogLevel(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber)

    try {
        auto conf = getEngineOwnData();
        conf->logger->setLevel(static_cast<LogLevel>(args[0].asNumber().toInt32() - 1));
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in SetLogLevel!")
}
