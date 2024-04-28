#include "api/LoggerAPI.h"

#include "api/APIHelp.h"
#include "api/PlayerAPI.h"
#include "engine/EngineOwnData.h"
#include "ll/api/Logger.h"
#include "mc/world/actor/player/Player.h"
#include "utils/Utils.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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

void inline LogDataHelper(ll::OutputStream* outStream, const Arguments& args) {
    std::string res;
    for (int i = 0; i < args.size(); ++i) res += ValueToString(args[i]);
    (*outStream)(res);
}

Local<Value> LoggerClass::log(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = ENGINE_OWN_DATA();
        LogDataHelper(&globalConf->logger.info, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerLog!")
}

Local<Value> LoggerClass::debug(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = ENGINE_OWN_DATA();
        LogDataHelper(&globalConf->logger.debug, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerDebug!")
}

Local<Value> LoggerClass::info(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = ENGINE_OWN_DATA();
        LogDataHelper(&globalConf->logger.info, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerInfo!")
}

Local<Value> LoggerClass::warn(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = ENGINE_OWN_DATA();
        LogDataHelper(&globalConf->logger.warn, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerWarn!")
}

Local<Value> LoggerClass::error(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = ENGINE_OWN_DATA();
        LogDataHelper(&globalConf->logger.error, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerError!")
}

Local<Value> LoggerClass::fatal(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        auto globalConf = ENGINE_OWN_DATA();
        LogDataHelper(&globalConf->logger.fatal, args);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerFatal!")
}

Local<Value> LoggerClass::setTitle(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        ENGINE_OWN_DATA()->logger.title = args[0].asString().toString();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerSetTitle!")
}

///////////////// Helper /////////////////
void UpdateMaxLogLevel() {
    auto data         = ENGINE_OWN_DATA();
    data->maxLogLevel = data->logger.consoleLevel;
    if (data->maxLogLevel < data->logger.fileLevel) data->maxLogLevel = data->logger.fileLevel;
    if (data->maxLogLevel < data->logger.playerLevel) data->maxLogLevel = data->logger.playerLevel;
}
///////////////// Helper /////////////////

Local<Value> LoggerClass::setConsole(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kBoolean)
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kNumber)

    try {
        if (args.size() >= 2) {
            ENGINE_OWN_DATA()->logger.consoleLevel = args[1].toInt();
        }
        if (!args[0].asBoolean().value()) {
            ENGINE_OWN_DATA()->logger.consoleLevel = 0;
        }
        UpdateMaxLogLevel();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in LoggerSetConsole!")
}

Local<Value> LoggerClass::setFile(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kNumber)

    try {
        string newFile = args[0].asString().toString();
        ENGINE_OWN_DATA()->logger.setFile(newFile, std::ios::app);

        if (args.size() >= 2) {
            ENGINE_OWN_DATA()->logger.fileLevel = args[1].toInt();
            UpdateMaxLogLevel();
        }
        return Boolean::newBoolean(ENGINE_OWN_DATA()->logger.ofs.value().is_open());
    }
    CATCH("Fail in LoggerSetFile!")
}

Local<Value> LoggerClass::setPlayer(const Arguments& args) {
    try {
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in LoggerSetPlayer!")
}

Local<Value> LoggerClass::setLogLevel(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber)

    try {
        auto conf         = ENGINE_OWN_DATA();
        conf->maxLogLevel = conf->logger.consoleLevel = conf->logger.fileLevel = conf->logger.playerLevel =
            args[0].toInt();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in SetLogLevel!")
}
