#include "api/DataAPI.h"

#include "api/APIHelp.h"
#include "api/DatabaseAPI.h"
#include "ll/api/io/FileUtils.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/PlayerInfo.h"
#include "ll/api/utils/Base64Utils.h"
#include "ll/api/utils/StringUtils.h"
#include "lse/api/Hash.h"
#include "main/EconomicSystem.h"
#include "utils/JsonHelper.h"

#include <ctre/ctre.hpp>
#include <fstream>
#include <string>

//////////////////// Class Definition ////////////////////

ClassDefine<void> DataClassBuilder = defineClass("data")
                                         .function("xuid2name", &DataClass::xuid2name)
                                         .function("name2xuid", &DataClass::name2xuid)
                                         .function("xuid2uuid", &DataClass::xuid2uuid)
                                         .function("name2uuid", &DataClass::name2uuid)
                                         .function("getAllPlayerInfo", &DataClass::getAllPlayerInfo)
                                         .function("fromUuid", &DataClass::fromUuid)
                                         .function("fromXuid", &DataClass::fromXuid)
                                         .function("fromName", &DataClass::fromName)
                                         .function("parseJson", &DataClass::parseJson)
                                         .function("toJson", &DataClass::toJson)
                                         .function("toMD5", &DataClass::toMD5)
                                         .function("toSHA1", &DataClass::toSHA1)
                                         .function("toBase64", &DataClass::toBase64)
                                         .function("fromBase64", &DataClass::fromBase64)

                                         // For Compatibility
                                         .function("openDB", &DataClass::openDB)
                                         .function("openConfig", &DataClass::openConfig)
                                         .build();

ClassDefine<void> MoneyClassBuilder = defineClass("money")
                                          .function("set", &MoneyClass::set)
                                          .function("get", &MoneyClass::get)
                                          .function("add", &MoneyClass::add)
                                          .function("reduce", &MoneyClass::reduce)
                                          .function("trans", &MoneyClass::trans)
                                          .function("getHistory", &MoneyClass::getHistory)
                                          .function("clearHistory", &MoneyClass::clearHistory)
                                          .build();

ClassDefine<ConfJsonClass> ConfJsonClassBuilder =
    defineClass<ConfJsonClass>("JsonConfigFile")
        .constructor(&ConfJsonClass::constructor)
        .instanceFunction("init", &ConfJsonClass::init)
        .instanceFunction("get", &ConfJsonClass::get)
        .instanceFunction("set", &ConfJsonClass::set)
        .instanceFunction("delete", &ConfJsonClass::del)
        .instanceFunction(
            "reload",
            selectOverloadedFunc<Local<Value> (ConfJsonClass::*)(Arguments const&)>(&ConfJsonClass::reload)
        )
        .instanceFunction(
            "close",
            selectOverloadedFunc<Local<Value> (ConfJsonClass::*)(Arguments const&)>(&ConfJsonClass::close)
        )
        .instanceFunction("getPath", &ConfJsonClass::getPath)
        .instanceFunction("read", &ConfJsonClass::read)
        .instanceFunction("write", &ConfJsonClass::write)
        .build();

ClassDefine<ConfIniClass> ConfIniClassBuilder =
    defineClass<ConfIniClass>("IniConfigFile")
        .constructor(&ConfIniClass::constructor)
        .instanceFunction("init", &ConfIniClass::init)
        .instanceFunction("set", &ConfIniClass::set)
        .instanceFunction("getStr", &ConfIniClass::getStr)
        .instanceFunction("getInt", &ConfIniClass::getInt)
        .instanceFunction("getFloat", &ConfIniClass::getFloat)
        .instanceFunction("getBool", &ConfIniClass::getBool)
        .instanceFunction("delete", &ConfIniClass::del)
        .instanceFunction(
            "reload",
            selectOverloadedFunc<Local<Value> (ConfIniClass::*)(Arguments const&)>(&ConfIniClass::reload)
        )
        .instanceFunction(
            "close",
            selectOverloadedFunc<Local<Value> (ConfIniClass::*)(Arguments const&)>(&ConfIniClass::close)
        )
        .instanceFunction("getPath", &ConfIniClass::getPath)
        .instanceFunction("read", &ConfIniClass::read)
        .instanceFunction("write", &ConfIniClass::write)
        .build();

//////////////////// Classes ConfBase ////////////////////

ConfBaseClass::ConfBaseClass(string const& dir) : confPath(dir) {}

Local<Value> ConfBaseClass::getPath(Arguments const&) const {
    try {
        return String::newString(confPath);
    }
    CATCH_AND_THROW
}

Local<Value> ConfBaseClass::read(Arguments const&) const {
    try {
        auto content = ll::file_utils::readFile(ll::string_utils::str2u8str(confPath));
        if (!content) return {};
        return String::newString(*content);
    }
    CATCH_AND_THROW
}

//////////////////// Classes ConfJson ////////////////////

// 生成函数
ConfJsonClass::ConfJsonClass(Local<Object> const& scriptObj, string const& path, string const& defContent)
: ScriptClass(scriptObj),
  ConfBaseClass(path) {
    jsonConf = CreateJson(path, defContent);
}

ConfJsonClass::ConfJsonClass(string const& path, string const& defContent)
: ScriptClass(ScriptClass::ConstructFromCpp<ConfJsonClass>{}),
  ConfBaseClass(path) {
    jsonConf = CreateJson(path, defContent);
}

ConfJsonClass::~ConfJsonClass() { ConfJsonClass::close(); }

ConfJsonClass* ConfJsonClass::constructor(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        string path = args[0].asString().toString();
        if (path.empty()) return nullptr;

        if (args.size() >= 2) return new ConfJsonClass(args.thiz(), path, args[1].asString().toString());
        return new ConfJsonClass(args.thiz(), path, "{}");
    }
    CATCH_AND_THROW
}

Local<Value> ConfJsonClass::init(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        return JsonToValue(jsonConf.at(args[0].asString().toString()));
    } catch (std::out_of_range const&) {
        jsonConf[args[0].asString().toString()] = ordered_json::parse(ValueToJson(args[1]));
        flush();
        return args[1];
    } catch (ordered_json::exception const&) {
        jsonConf[args[0].asString().toString()] = ordered_json::parse(ValueToJson(args[1]));
        flush();
        return args[1];
    }
    CATCH_AND_THROW
}

Local<Value> ConfJsonClass::get(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        return JsonToValue(jsonConf.at(args[0].asString().toString()));
    } catch (std::out_of_range const&) {
        return args.size() >= 2 ? args[1] : Local<Value>();
    } catch (ordered_json::exception const&) {
        return args.size() >= 2 ? args[1] : Local<Value>();
    }
    CATCH_AND_THROW
}

Local<Value> ConfJsonClass::set(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        jsonConf[args[0].asString().toString()] = ordered_json::parse(ValueToJson(args[1]));
        return Boolean::newBoolean(flush());
    } catch (ordered_json::exception const&) {
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> ConfJsonClass::del(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        if (jsonConf.erase(args[0].asString().toString()) <= 0) return Boolean::newBoolean(false);

        return Boolean::newBoolean(flush());
    } catch (ordered_json::exception const&) {
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> ConfJsonClass::reload(Arguments const&) {
    try {
        return Boolean::newBoolean(reload());
    }
    CATCH_AND_THROW
}

Local<Value> ConfJsonClass::close(Arguments const&) {
    try {
        return Boolean::newBoolean(close());
    }
    CATCH_AND_THROW
}

Local<Value> ConfJsonClass::write(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        bool res =
            ll::file_utils::writeFile(ll::string_utils::str2u8str(confPath), args[0].asString().toString(), false);
        reload();
        return Boolean::newBoolean(res);
    }
    CATCH_AND_THROW
}

bool ConfJsonClass::flush() {
    std::ofstream jsonFile(confPath);
    if (jsonFile.is_open()) {
        jsonFile << jsonConf.dump(4);
        jsonFile.close();
        return true;
    }
    return false;
}

bool ConfJsonClass::close() {
    reload();
    return true;
}

bool ConfJsonClass::reload() {
    auto jsonTexts = ll::file_utils::readFile(ll::string_utils::str2u8str(confPath));
    if (!jsonTexts) return false;
    jsonConf = ordered_json::parse(*jsonTexts, nullptr, true, true);
    return true;
}

//////////////////// Classes ConfIni ////////////////////

// 生成函数
ConfIniClass::ConfIniClass(Local<Object> const& scriptObj, string const& path, string const& defContent)
: ScriptClass(scriptObj),
  ConfBaseClass(path) {
    iniConf = SimpleIni::create(path, defContent);
}

ConfIniClass::ConfIniClass(string const& path, string const& defContent)
: ScriptClass(ScriptClass::ConstructFromCpp<ConfIniClass>{}),
  ConfBaseClass(path) {
    iniConf = SimpleIni::create(path, defContent);
}

ConfIniClass::~ConfIniClass() { ConfIniClass::close(); }

ConfIniClass* ConfIniClass::constructor(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        string path = args[0].asString().toString();
        if (path.empty()) return nullptr;

        if (args.size() >= 2) return new ConfIniClass(args.thiz(), path, args[1].asString().toString());
        return new ConfIniClass(args.thiz(), path, "");
    }
    CATCH_AND_THROW
}

bool ConfIniClass::flush() { return iniConf->SaveFile(iniConf->filePath.c_str(), true); }

bool ConfIniClass::close() {
    if (isValid()) {
        reload();
        iniConf.reset();
    }
    return true;
}
bool ConfIniClass::reload() {
    if (!isValid()) return false;

    iniConf.reset();
    iniConf = SimpleIni::create(confPath);
    return true;
}

Local<Value> ConfIniClass::init(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        if (!isValid()) return {};

        string       section = args[0].asString().toString();
        string       key     = args[1].asString().toString();
        Local<Value> res;

        switch (args[2].getKind()) {
        case ValueKind::kString: {
            string def  = args[2].asString().toString();
            string data = iniConf->getString(section, key, def);
            if (data == def) {
                iniConf->setString(section, key, def);
                flush();
            }
            res = String::newString(data);
            break;
        }
        case ValueKind::kNumber: {
            if (CheckIsFloat(args[2])) {
                // Float
                float def  = args[2].asNumber().toFloat();
                float data = iniConf->getFloat(section, key, def);
                if (data == def) {
                    iniConf->setFloat(section, key, def);
                    flush();
                }
                res = Number::newNumber(data);
            } else {
                // Int
                int def  = args[2].asNumber().toInt32();
                int data = iniConf->getInt(section, key, def);
                if (data == def) {
                    iniConf->setInt(section, key, def);
                    flush();
                }
                res = Number::newNumber(data);
            }
            break;
        }
        case ValueKind::kBoolean: {
            bool def  = args[2].asBoolean().value();
            bool data = iniConf->getBool(section, key, def);
            if (data == def) {
                iniConf->setBool(section, key, def);
                flush();
            }
            res = Boolean::newBoolean(data);
            break;
        }
        default:
            throw CreateExceptionWithInfo(__FUNCTION__, "Ini file don't support this type of data!");
        }
        return res;
    }
    CATCH_AND_THROW
}

Local<Value> ConfIniClass::set(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        if (!isValid()) return {};

        string section = args[0].asString().toString();
        string key     = args[1].asString().toString();
        switch (args[2].getKind()) {
        case ValueKind::kString:
            iniConf->setString(section, key, args[2].asString().toString());
            break;
        case ValueKind::kNumber:
            if (CheckIsFloat(args[2])) iniConf->setFloat(section, key, args[2].asNumber().toFloat());
            else iniConf->setInt(section, key, args[2].asNumber().toInt32());
            break;
        case ValueKind::kBoolean:
            iniConf->setBool(section, key, args[2].asBoolean().value());
            break;
        default:
            throw CreateExceptionWithInfo(__FUNCTION__, "Ini file don't support this type of data!");
        }
        flush();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> ConfIniClass::getStr(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kString)

    try {
        if (!isValid()) return {};

        return String::newString(iniConf->getString(
            args[0].asString().toString(),
            args[1].asString().toString(),
            args.size() >= 3 ? args[2].asString().toString() : ""
        ));
    }
    CATCH_AND_THROW
}

Local<Value> ConfIniClass::getInt(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber)

    try {
        if (!isValid()) return {};

        return Number::newNumber(iniConf->getInt(
            args[0].asString().toString(),
            args[1].asString().toString(),
            args.size() >= 3 ? args[2].asNumber().toInt32() : 0
        ));
    }
    CATCH_AND_THROW
}

Local<Value> ConfIniClass::getFloat(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);

    try {
        if (!isValid()) return {};

        return Number::newNumber(iniConf->getFloat(
            args[0].asString().toString(),
            args[1].asString().toString(),
            args.size() >= 3 ? args[2].asNumber().toFloat() : 0.0f
        ));
    }
    CATCH_AND_THROW
}

Local<Value> ConfIniClass::getBool(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kBoolean);

    try {
        if (!isValid()) return {};

        return Boolean::newBoolean(iniConf->getBool(
            args[0].asString().toString(),
            args[1].asString().toString(),
            args.size() >= 3 ? args[2].asBoolean().value() : false
        ));
    }
    CATCH_AND_THROW
}

Local<Value> ConfIniClass::del(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        if (!isValid()) return {};

        bool res = iniConf->deleteKey(args[0].asString().toString(), args[1].asString().toString());
        flush();
        return Boolean::newBoolean(res);
    }
    CATCH_AND_THROW
}

Local<Value> ConfIniClass::reload(Arguments const&) {
    try {
        return Boolean::newBoolean(reload());
    }
    CATCH_AND_THROW
}

Local<Value> ConfIniClass::write(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        bool res =
            ll::file_utils::writeFile(ll::string_utils::str2u8str(confPath), args[0].asString().toString(), false);
        reload();
        return Boolean::newBoolean(res);
    }
    CATCH_AND_THROW
}

Local<Value> ConfIniClass::close(Arguments const&) {
    try {
        return Boolean::newBoolean(close());
    }
    CATCH_AND_THROW
}

//////////////////// APIs ////////////////////

Local<Value> MoneyClass::set(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        return Boolean::newBoolean(
            EconomySystem::setMoney(args[0].asString().toString(), args[1].asNumber().toInt64())
        );
    }
    CATCH_AND_THROW
}

Local<Value> MoneyClass::get(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        return Number::newNumber(EconomySystem::getMoney(args[0].asString().toString()));
    }
    CATCH_AND_THROW
}

Local<Value> MoneyClass::add(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        return Boolean::newBoolean(
            EconomySystem::addMoney(args[0].asString().toString(), args[1].asNumber().toInt64())
        );
    }
    CATCH_AND_THROW
}

Local<Value> MoneyClass::reduce(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        return Boolean::newBoolean(
            EconomySystem::reduceMoney(args[0].asString().toString(), args[1].asNumber().toInt64())
        );
    }
    CATCH_AND_THROW
}

Local<Value> MoneyClass::trans(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 3);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kNumber);

    try {
        string note = "";
        if (args.size() >= 4 && args[3].getKind() == ValueKind::kString) note = args[3].asString().toString();
        return Boolean::newBoolean(
            EconomySystem::transMoney(
                args[0].asString().toString(),
                args[1].asString().toString(),
                args[2].asNumber().toInt64(),
                note
            )
        );
    }
    CATCH_AND_THROW
}

Local<Array> objectificationMoneyHistory(string const& res) {
    Local<Array> arr = Array::newArray();
    ll::string_utils::splitByPattern(
        [&](std::string_view str) -> bool {
            auto [whole, fromName, toName, money, time, note] =
                ctre::match<R"(^(.*) -> (.*) (\d+) (\d{4}-\d{1,2}-\d{1,2} \d{1,2}:\d{1,2}:\d{1,2}) \((.*)\)$)">(str);
            if (time.to_view().empty()) return true;
            Local<Object> obj = Object::newObject();
            obj.set("from", String::newString(fromName));
            obj.set("to", String::newString(toName));
            obj.set("money", Number::newNumber(money.to_number<llong>()));
            obj.set("time", String::newString(time));
            obj.set("note", String::newString(note));
            arr.add(obj);
            return true;
        },
        res,
        "\n"
    );
    return arr;
}

Local<Value> MoneyClass::getHistory(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        string res = EconomySystem::getMoneyHist(args[0].asString().toString(), args[1].asNumber().toInt32());
        return objectificationMoneyHistory(res);
    }
    CATCH_AND_THROW
}

Local<Value> MoneyClass::clearHistory(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        EconomySystem::clearMoneyHist(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::xuid2name(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        if (auto playerInfo = ll::service::PlayerInfo::getInstance().fromXuid(args[0].asString().toString())) {
            return String::newString(playerInfo->name);
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::name2xuid(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        if (auto playerInfo = ll::service::PlayerInfo::getInstance().fromName(args[0].asString().toString())) {
            return String::newString(playerInfo->xuid);
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::name2uuid(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        if (auto playerInfo = ll::service::PlayerInfo::getInstance().fromName(args[0].asString().toString())) {
            return String::newString(playerInfo->uuid.asString());
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::xuid2uuid(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        if (auto playerInfo = ll::service::PlayerInfo::getInstance().fromXuid(args[0].asString().toString())) {
            return String::newString(playerInfo->uuid.asString());
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::getAllPlayerInfo(Arguments const& args) {
    try {
        auto arr   = Array::newArray();
        auto level = ll::service::getLevel();
        if (level.has_value()) {
            for (auto& entry : ll::service::PlayerInfo::getInstance().entries()) {
                auto object = Object::newObject();
                object.set("xuid", entry.xuid);
                object.set("name", entry.name);
                object.set("uuid", entry.uuid.asString());
                arr.add(object);
            }
        }
        return arr;
    }
    CATCH_AND_THROW
}

// New API for LSE
Local<Value> DataClass::fromUuid(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto playerInfo =
            ll::service::PlayerInfo::getInstance().fromUuid(mce::UUID::fromString(args[0].asString().toString()));
        if (playerInfo) {
            auto object = Object::newObject();
            object.set("xuid", playerInfo->xuid);
            object.set("name", playerInfo->name);
            object.set("uuid", playerInfo->uuid.asString());
            return object;
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::fromXuid(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        if (auto playerInfo = ll::service::PlayerInfo::getInstance().fromXuid(args[0].asString().toString())) {
            auto object = Object::newObject();
            object.set("xuid", playerInfo->xuid);
            object.set("name", playerInfo->name);
            object.set("uuid", playerInfo->uuid.asString());
            return object;
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::fromName(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        if (auto playerInfo = ll::service::PlayerInfo::getInstance().fromName(args[0].asString().toString())) {
            auto object = Object::newObject();
            object.set("xuid", playerInfo->xuid);
            object.set("name", playerInfo->name);
            object.set("uuid", playerInfo->uuid.asString());
            return object;
        }
        return {};
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::toJson(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    int spaces = -1;
    if (args.size() >= 2) {
        int newSpaces = args[1].asNumber().toInt32();
        if (newSpaces > 0) spaces = newSpaces;
    }
    try {
        return String::newString(ValueToJson(args[0], spaces));
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::parseJson(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        return JsonToValue(args[0].asString().toString());
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::toMD5(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        std::string data;
        if (args[0].isString()) data = args[0].asString().toString();
        else if (args[0].isByteBuffer()) {
            Local<ByteBuffer> buf = args[0].asByteBuffer();
            data                  = string(static_cast<char*>(buf.getRawBytes()), buf.byteLength());
        } else {
            throw WrongArgTypeException(__FUNCTION__);
        }
        using namespace lse::api::hash;
        return String::newString(caculateHash(HashType::MD5, data));
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::toSHA1(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        std::string data;
        if (args[0].isString()) data = args[0].asString().toString();
        else if (args[0].isByteBuffer()) {
            Local<ByteBuffer> buf = args[0].asByteBuffer();
            data                  = string(static_cast<char*>(buf.getRawBytes()), buf.byteLength());
        } else {
            throw WrongArgTypeException(__FUNCTION__);
        }
        using namespace lse::api::hash;
        return String::newString(caculateHash(HashType::SHA1, data));
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::toBase64(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        string data;
        if (args[0].isString()) data = args[0].asString().toString();
        else if (args[0].isByteBuffer()) {
            Local<ByteBuffer> buf = args[0].asByteBuffer();
            data                  = string(static_cast<char*>(buf.getRawBytes()), buf.byteLength());
        } else {
            throw WrongArgTypeException(__FUNCTION__);
        }
        return String::newString(ll::base64_utils::encode(data));
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::fromBase64(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        bool isBinary = false;
        if (args.size() > 1) {
            CHECK_ARG_TYPE(args[1], ValueKind::kBoolean);
            isBinary = args[1].asBoolean().value();
        }
        auto data = ll::base64_utils::decode(args[0].asString().toString());
        if (isBinary) {
            return ByteBuffer::newByteBuffer(const_cast<char*>(data.c_str()), data.size());
        }
        return String::newString(data);
    }
    CATCH_AND_THROW
}

// For Compatibility

Local<Value> KVDBClass::newDb(string const& dir) {
    auto newp = new KVDBClass(dir);

    if (newp->isValid()) return newp->getScriptObject();
    delete newp;
    return {};
}

Local<Value> ConfJsonClass::newConf(string const& path, string const& defContent) {
    auto newp = new ConfJsonClass(path, defContent);
    return newp->getScriptObject();
}

Local<Value> ConfIniClass::newConf(string const& path, string const& defContent) {
    if (auto newp = new ConfIniClass(path, defContent)) return newp->getScriptObject();
    return {};
}

Local<Value> DataClass::openConfig(Arguments const& args) {
    enum GlobalConfType { json, ini };

    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    if (args.size() >= 2) CHECK_ARG_TYPE(args[1], ValueKind::kString);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kString);

    try {
        string         path     = args[0].asString().toString();
        GlobalConfType confType = GlobalConfType::ini;

        if (path.empty()) return Boolean::newBoolean(false);

        if (args.size() >= 2) {
            string fileType = args[1].asString().toString();
            if (fileType == "json" || fileType == "Json") confType = GlobalConfType::json;
        }

        if (confType == GlobalConfType::ini) {
            if (args.size() >= 3) return ConfIniClass::newConf(path, args[2].asString().toString());
            return ConfIniClass::newConf(path);
        } // json
        if (args.size() >= 3) return ConfJsonClass::newConf(path, args[2].asString().toString());
        return ConfJsonClass::newConf(path, "{}");
    }
    CATCH_AND_THROW
}

Local<Value> DataClass::openDB(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    return KVDBClass::newDb(args[0].asString().toString());
}
