#include "api/ServerAPI.h"

#include "api/APIHelp.h"
#include "api/McAPI.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/utils/RandomUtils.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/packet/SetTimePacket.h"
#include "mc/world/level/Tick.h"
#include "mc/world/level/storage/LevelData.h"

#include <cstdint>
#include <ll/api/service/ServerInfo.h>

Local<Value> McClass::setMotd(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kString)

    try {
        return Boolean::newBoolean(ll::setServerMotd(args[0].asString().toString()));
    }
    CATCH("Fail in SetServerMotd!")
}

Local<Value> McClass::crashBDS(const Arguments&) { return Boolean::newBoolean(false); }

Local<Value> McClass::setMaxNumPlayers(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber)

    try {
        int back = ll::service::getServerNetworkHandler()->setMaxNumPlayers(args[0].asNumber().toInt32());
        ll::service::getServerNetworkHandler()->updateServerAnnouncement();
        return Boolean::newBoolean(back == 0 ? true : false);
    }
    CATCH("Fail in setMaxPlayers!")
}

Local<Value> McClass::getTime(const Arguments& args) {
    int option = 0; // option: 0: daytime, 1: gametime, 2: day

    if (args.size() > 0) {
        CHECK_ARG_TYPE(args[0], ValueKind::kNumber)
        option = args[0].asNumber().toInt32();
    }

    switch (option) {
    case 0:
        return Number::newNumber(ll::service::getLevel()->getTime() % 24000);
        break;
    case 1:
        return Number::newNumber(static_cast<int64_t>(ll::service::getLevel()->getCurrentTick().tickID));
        break;
    case 2:
        return Number::newNumber(ll::service::getLevel()->getTime() / 24000);
        break;
    default:
        throw script::Exception("The range of this argument is between 0 and 2");
    }
}

Local<Value> McClass::setTime(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber)

    try {
        int currentTime = ll::service::getLevel()->getTime();
        int targetTime  = args[0].asNumber().toInt32(); // Tick

        int newTime          = currentTime;
        int currentTimeOfDay = currentTime % 24000;

        if (targetTime > currentTime % 24000) newTime = currentTime + targetTime - currentTimeOfDay;
        else if (targetTime < currentTimeOfDay) newTime = currentTime + targetTime + 24000 - currentTimeOfDay;

        ll::service::getLevel()->setTime(newTime);
        SetTimePacket(newTime).sendToClients();
    }
    CATCH("Fail in setTime!")

    return Boolean::newBoolean(true);
}

Local<Value> McClass::getWeather(const Arguments&) { // weather: 0: Clear, 1: Rain, 2: Thunder
    if (ll::service::getLevel()->getLevelData().isLightning()) return Number::newNumber(2);
    else if (ll::service::getLevel()->getLevelData().isRaining()) return Number::newNumber(1);

    return Number::newNumber(0);
}

Local<Value> McClass::setWeather(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber)

    int duration = 0;
    int weather  = args[0].asNumber().toInt32(); // weather: 0: Clear, 1: Rain, 2: Thunder

    try {
        if (args.size() > 1) {
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            duration = args[1].asNumber().toInt32();
        } else {
            duration = 20 * (ll::random_utils::rand(600) + 300);
        }

        if (weather == 1) ll::service::getLevel()->updateWeather(1.0, duration, 0.0, duration);
        else if (weather == 2) ll::service::getLevel()->updateWeather(1065353216.0, duration, 1065353216.0, duration);
        else ll::service::getLevel()->updateWeather(0.0, duration, 0.0, duration);
    }
    CATCH("Fail in setWeather!")

    return Boolean::newBoolean(true);
}
