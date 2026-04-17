#pragma once
#include "legacy/api/APIHelp.h"

#include <Windows.h>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

//////////////////// Structs ////////////////////

// 导出函数表
struct ExportedFuncData {
    std::string                                          fromEngineType;
    ScriptEngine*                                        engine;
    script::Global<Function>                             func;
    std::function<std::string(std::vector<std::string>)> callback;
};

// 消息系统处理函数信息
struct MessageHandlers {
    script::utils::Message::MessageProc* handler;
    script::utils::Message::MessageProc* cleaner;
};

// 全局共享数据
struct GlobalDataType {
    // 引擎管理器表
    std::mutex                                                               engineListLock;
    std::vector<std::shared_ptr<ScriptEngine>>                               globalEngineList;
    std::shared_mutex                                                         engineSnapshotLock;
    std::shared_ptr<std::vector<std::shared_ptr<ScriptEngine>>>              globalEngineSnapshot;

    // 导出函数表
    std::unordered_map<std::string, ExportedFuncData> exportedFuncs;

    // 模块消息系统
    int                                    messageSystemNextId = 0;
    std::map<std::string, MessageHandlers> messageSystemHandlers;
    std::map<std::string, HANDLE>          messageThreads;

    // OperationCount
    std::map<std::string, int> operationCountData;
};

//////////////////// Externs ////////////////////

// 全局共享数据
extern GlobalDataType* globalShareData;

//////////////////// APIs ////////////////////

void InitGlobalShareData();
