#include "api/LoggerAPI.h"

#include "api/APIHelp.h"
#include "api/PlayerAPI.h"
#include "engine/EngineOwnData.h"
#include "ll/api/io/Logger.h"
#include "ll/api/io/FileSink.h"
#include "ll/api/io/PatternFormatter.h"
#include "ll/api/data/IndirectValue.h"
#include "mc/world/actor/player/Player.h"
#include "utils/Utils.h"
#include "lse/api/PlayerSink.h"

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
string inline GetTimeStrHelper() { return Raw_GetDateTimeStr(); }

string& StrReplace(string& str, const string& to_replaced, const string& new_str) {
    for (string::size_type pos(0); pos != string::npos; pos += new_str.length()) {
        pos = str.find(to_replaced, pos);
        if (pos != string::npos) str.replace(pos, to_replaced.length(), new_str);
        else break;
    }
    return str;
}
////////////////// Helper //////////////////

void inline LogDataHelper(LogLevel level, const Arguments& args) {
    std::string res;
    for (int i = 0; i < args.size(); ++i) res += ValueToString(args[i]);
    ENGINE_OWN_DATA()->getModInstance()->getLogger().log(level, res);
}

Local<Value> LoggerClass::log(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = ENGINE_OWN_DATA();
        LogDataHelper(LogLevel::Info, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerLog!")
}

Local<Value> LoggerClass::debug(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = ENGINE_OWN_DATA();
        LogDataHelper(LogLevel::Debug, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerDebug!")
}

Local<Value> LoggerClass::info(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = ENGINE_OWN_DATA();
        LogDataHelper(LogLevel::Info, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerInfo!")
}

Local<Value> LoggerClass::warn(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = ENGINE_OWN_DATA();
        LogDataHelper(LogLevel::Warn, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerWarn!")
}

Local<Value> LoggerClass::error(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = ENGINE_OWN_DATA();
        LogDataHelper(LogLevel::Error, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerError!")
}

Local<Value> LoggerClass::fatal(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = ENGINE_OWN_DATA();
        LogDataHelper(LogLevel::Fatal, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerFatal!")
}

// Deprecated
Local<Value> LoggerClass::setTitle(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    return Boolean::newBoolean(false);

}

///////////////// Helper /////////////////

Local<Value> LoggerClass::setConsole(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kBoolean)
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kNumber)

    try {
        if (args.size() >= 2) {
            ENGINE_OWN_DATA()->getModInstance()->getLogger().getSink(0)->setFlushLevel(static_cast<ll::io::LogLevel>(args[1].toInt() + 1
            )); // See LSE's definition https://legacy-script-engine.levimc.org/apis/ScriptAPI/Logger/
        }
        if (!args[0].asBoolean().value()) {
            ENGINE_OWN_DATA()->getModInstance()->getLogger().getSink(0)->setFlushLevel(ll::io::LogLevel::Off);
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
        std::filesystem::path newFile = std::filesystem::path(args[0].asString().toString());
        std::shared_ptr<ll::io::FileSink> sink = std::make_shared<ll::io::FileSink>(newFile, ll::makePolymorphic<ll::io::PatternFormatter>("{3:.3%T.} {2} {1} {0}", ll::io::Formatter::supportColorLog(), 0b0010), std::ios::app);
        if (args.size() >= 2) {
            sink->setFlushLevel(static_cast<LogLevel>(args[1].toInt() + 1));
        }
        return Boolean::newBoolean(ENGINE_OWN_DATA()->getModInstance()->getLogger().addSink(sink));
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
            sink->setFlushLevel(static_cast<LogLevel>(args[1].toInt() + 1));
        }
        return Boolean::newBoolean(ENGINE_OWN_DATA()->getModInstance()->getLogger().addSink(sink));
    }
    CATCH("Fail in LoggerSetPlayer!")
}

Local<Value> LoggerClass::setLogLevel(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber)

    try {
        auto conf         = ENGINE_OWN_DATA();
        conf->getModInstance()->getLogger().setLevel(static_cast<LogLevel>(args[0].toInt() + 1));
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in SetLogLevel!")
}
