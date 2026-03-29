#pragma once
#include "api/APIHelp.h"

class McClass {
public:
    static Local<Value> getBDSVersion(Arguments const& args);
    static Local<Value> getServerProtocolVersion(Arguments const& args);

    static Local<Value> runcmd(Arguments const& args);
    static Local<Value> runcmdEx(Arguments const& args);
    static Local<Value> newCommand(Arguments const& args);
    static Local<Value> regPlayerCmd(Arguments const& args);
    static Local<Value> broadcast(Arguments const& args);

    static Local<Value> listen(Arguments const& args);

    static Local<Value> getPlayer(Arguments const& args);
    static Local<Value> getOnlinePlayers(Arguments const& args);
    static Local<Value> getAllEntities(Arguments const& args);
    static Local<Value> getEntities(Arguments const& args);
    static Local<Value> getEntity(Arguments const& args);

    static Local<Value> newItem(Arguments const& args);
    static Local<Value> spawnMob(Arguments const& args);
    static Local<Value> spawnItem(Arguments const& args);
    static Local<Value> spawnSimulatedPlayer(Arguments const& args);
    static Local<Value> explode(Arguments const& args);
    static Local<Value> cloneMob(Arguments const& args);

    static Local<Value> getBlock(Arguments const& args);
    static Local<Value> setBlock(Arguments const& args);
    static Local<Value> spawnParticle(Arguments const& args);

    static Local<Value> newSimpleForm(Arguments const& args);
    static Local<Value> newCustomForm(Arguments const& args);

    static Local<Value> regConsoleCmd(Arguments const& args);
    static Local<Value> setMotd(Arguments const& args);
    static Local<Value> sendCmdOutput(Arguments const& args);
    static Local<Value> crashBDS(Arguments const& args);

    static Local<Value> setMaxNumPlayers(Arguments const& args);

    static Local<Value> newIntPos(Arguments const& args);
    static Local<Value> newFloatPos(Arguments const& args);

    static Local<Value> getDisplayObjective(Arguments const& args);
    static Local<Value> clearDisplayObjective(Arguments const& args);
    static Local<Value> getScoreObjective(Arguments const& args);
    static Local<Value> newScoreObjective(Arguments const& args);
    static Local<Value> removeScoreObjective(Arguments const& args);
    static Local<Value> getAllScoreObjectives(Arguments const& args);
    static Local<Value> getStructure(Arguments const& args);
    static Local<Value> setStructure(Arguments const& args);

    static Local<Value> newParticleSpawner(Arguments const& args);

    static Local<Value> getPlayerNbt(Arguments const& args);
    static Local<Value> setPlayerNbt(Arguments const& args);
    static Local<Value> setPlayerNbtTags(Arguments const& args);
    static Local<Value> deletePlayerNbt(Arguments const& args);
    static Local<Value> getPlayerScore(Arguments const& args);
    static Local<Value> setPlayerScore(Arguments const& args);
    static Local<Value> addPlayerScore(Arguments const& args);
    static Local<Value> reducePlayerScore(Arguments const& args);
    static Local<Value> deletePlayerScore(Arguments const& args);

    static Local<Value> getTime(Arguments const& args);
    static Local<Value> setTime(Arguments const& args);
    static Local<Value> getWeather(Arguments const& args);
    static Local<Value> setWeather(Arguments const& args);
};
extern ClassDefine<> McClassBuilder;
