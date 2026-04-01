#pragma once
#include "legacy/api/APIHelp.h"
#include "ll/api/thread/ThreadPoolExecutor.h"

#include <map>
#include <string>

//////////////////// Structs ////////////////////

enum class CommandPermissionLevel : uchar;

// 命令回调信息结构体
struct CommandCallbackData {
    std::shared_ptr<ScriptEngine> fromEngine;
    CommandPermissionLevel        perm;
    script::Global<Function>      func;
};

// 命令延迟注册队列
struct FakeCommandRegistrationData {
    std::string                             description;
    CommandPermissionLevel                  level;
    std::shared_ptr<ScriptEngine>           engine;
    std::optional<script::Global<Function>> playerFunc;
    std::optional<script::Global<Function>> consoleFunc;
};

// 命令回调map排序
struct CommmandCallbackMapCmp {
    bool operator()(std::string const& a, std::string const& b) const {
        if (a.size() != b.size()) return a.size() > b.size();
        return a > b;
    }
};

// DLL本地共享数据
struct LocalDataType {
    // 是否是第一个ScriptEngine实例
    bool isFirstInstance = true;

    // 真指令回调
    std::map<std::string, CommandCallbackData, CommmandCallbackMapCmp> commandCallbacks;

    // 命令延迟注册队列
    std::unordered_map<std::string, FakeCommandRegistrationData> fakeCommandsMap;
};

//////////////////// Externs ////////////////////

// DLL本地共享数据
extern std::unique_ptr<LocalDataType> localShareData;

// 线程池
extern ll::thread::ThreadPoolExecutor pool;

//////////////////// APIs ////////////////////

void InitLocalShareData();
