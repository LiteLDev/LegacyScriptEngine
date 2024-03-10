#include "engine/TimeTaskSystem.h"

#include "api/APIHelp.h"
#include "engine/EngineManager.h"
#include "engine/EngineOwnData.h"
#include "engine/MessageSystem.h"
#include "ll/api/schedule/scheduler.h"

#include <chrono>
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/schedule/Task.h>
#include <ll/api/service/ServerInfo.h>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <vector>

std::atomic_uint                timeTaskId = 0;
std::shared_mutex               locker;
ll::schedule::GameTickScheduler taskScheduler;
struct TimeTaskData {
    uint64                        task;
    script::Global<Function>      func;
    vector<script::Global<Value>> paras;
    script::Global<String>        code;
    ScriptEngine*                 engine;
    inline void                   swap(TimeTaskData& rhs) {
        std::swap(rhs.task, task);
        std::swap(rhs.engine, engine);
        rhs.code.swap(code);
        rhs.paras.swap(paras);
        rhs.func.swap(func);
    }
};
std::unordered_map<int, TimeTaskData> timeTaskMap;

#define TIMETASK_CATCH(TASK_TYPE)                                                                                      \
    catch (const Exception& e) {                                                                                       \
        EngineScope scope(engine);                                                                                     \
        lse::getSelfPluginInstance().getLogger().error("Error occurred in {}", TASK_TYPE);                             \
        PrintException(e);                                                                                             \
        lse::getSelfPluginInstance().getLogger().error("In Plugin: " + ENGINE_GET_DATA(engine)->pluginName);           \
    }                                                                                                                  \
    catch (const std::exception& e) {                                                                                  \
        lse::getSelfPluginInstance().getLogger().error("Error occurred in {}", TASK_TYPE);                             \
        lse::getSelfPluginInstance().getLogger().error("C++ Uncaught Exception Detected!");                            \
        lse::getSelfPluginInstance().getLogger().error(ll::string_utils::tou8str(e.what()));                           \
        lse::getSelfPluginInstance().getLogger().error("In Plugin: " + ENGINE_GET_DATA(engine)->pluginName);           \
    }                                                                                                                  \
    catch (...) {                                                                                                      \
        lse::getSelfPluginInstance().getLogger().error("Error occurred in {}", TASK_TYPE);                             \
        lse::getSelfPluginInstance().getLogger().error("Uncaught Exception Detected!");                                \
        lse::getSelfPluginInstance().getLogger().error("In Plugin: " + ENGINE_GET_DATA(engine)->pluginName);           \
    }

//////////////////// API ////////////////////

// void NewTimeout_s(
//     script::Global<Function>     func,
//     vector<script::Local<Value>> paras,
//     int                          timeout,
//     ScriptEngine*                engine
// ) {
//     std::vector<script::Global<Value>> tmp;
//     if (paras.size() > 0) {
//         EngineScope enter(engine);
//         for (auto& para : paras) tmp.emplace_back(std::move(para));
//     }
//     taskScheduler.add<ll::schedule::DelayTask>(
//         std::chrono::milliseconds(timeout),
//         [engine, func = std::move(func), paras = std::move(tmp)]() {
//             if ((ll::getServerStatus() != ll::ServerStatus::Running)) return;
//             if (!EngineManager::isValid(engine)) return;
//             EngineScope enter(engine);
//             if (paras.empty()) {
//                 func.get().call();
//             } else {
//                 vector<Local<Value>> args;
//                 for (auto& para : paras)
//                     if (para.isEmpty()) return;
//                     else args.emplace_back(para.get());
//                 func.get().call({}, args);
//             }
//         }
//     );
// }

int NewTimeout(Local<Function> func, vector<Local<Value>> paras, int timeout) {
    int          tid = ++timeTaskId;
    TimeTaskData data;

    data.func   = func;
    data.engine = EngineScope::currentEngine();
    for (auto& para : paras) data.paras.emplace_back(std::move(para));
    data.task = taskScheduler
                    .add<ll::schedule::DelayTask>(
                        std::chrono::milliseconds(timeout),
                        [engine{EngineScope::currentEngine()}, id{tid}]() {
                            try {
                                if ((ll::getServerStatus() != ll::ServerStatus::Running)) return;
                                if (!EngineManager::isValid(engine)) return;
                                // lock after enter EngineScope to prevent deadlock
                                EngineScope  scope(engine);
                                TimeTaskData taskData;
                                {
                                    std::unique_lock<std::shared_mutex> lock(locker);

                                    auto t = timeTaskMap.find(id);
                                    if (t == timeTaskMap.end()) return;
                                    t->second.swap(taskData);
                                    timeTaskMap.erase(id);
                                }

                                if (taskData.func.isEmpty()) return;
                                auto func = taskData.func.get();
                                if (taskData.paras.empty()) {
                                    func.call();
                                } else {
                                    std::vector<Local<Value>> args;
                                    for (auto& para : taskData.paras)
                                        if (para.isEmpty()) return;
                                        else args.emplace_back(para.get());
                                    func.call({}, args);
                                }
                            }
                            TIMETASK_CATCH("setTimeout-Function");
                        }
                    )
                    ->getId();
    std::unique_lock<std::shared_mutex> lock(locker);
    data.swap(timeTaskMap[tid]);
    return tid;
}

int NewTimeout(Local<String> func, int timeout) {
    int          tid = ++timeTaskId;
    TimeTaskData data;

    data.code   = func;
    data.engine = EngineScope::currentEngine();

    data.task = taskScheduler
                    .add<ll::schedule::DelayTask>(
                        std::chrono::milliseconds(timeout),
                        [engine{EngineScope::currentEngine()}, id{tid}]() {
                            try {
                                if ((ll::getServerStatus() != ll::ServerStatus::Running)) return;
                                if (!EngineManager::isValid(engine)) return;
                                EngineScope  scope(engine);
                                TimeTaskData taskData;
                                {
                                    std::unique_lock<std::shared_mutex> lock(locker);

                                    auto t = timeTaskMap.find(id);
                                    if (t == timeTaskMap.end()) return;
                                    t->second.swap(taskData);
                                    timeTaskMap.erase(id);
                                }

                                if (taskData.code.isEmpty()) return;
                                auto code = taskData.code.get().toString();
                                engine->eval(code);
                            }
                            TIMETASK_CATCH("setTimeout-String");
                        }
                    )
                    ->getId();

    std::unique_lock<std::shared_mutex> lock(locker);
    data.swap(timeTaskMap[tid]);
    return tid;
}

int NewInterval(Local<Function> func, vector<Local<Value>> paras, int timeout) {
    int          tid = ++timeTaskId;
    TimeTaskData data;

    data.func   = func;
    data.engine = EngineScope::currentEngine();
    for (auto& para : paras) data.paras.emplace_back(std::move(para));

    data.task = taskScheduler
                    .add<ll::schedule::RepeatTask>(
                        std::chrono::milliseconds(timeout),
                        [engine{EngineScope::currentEngine()}, id{tid}]() {
                            try {
                                if ((ll::getServerStatus() != ll::ServerStatus::Running)) return;
                                if (!EngineManager::isValid(engine)) {
                                    ClearTimeTask(id);
                                    return;
                                }
                                EngineScope          scope(engine);
                                Local<Value>         func = Local<Value>();
                                vector<Local<Value>> args;
                                {
                                    std::unique_lock<std::shared_mutex> lock(locker);

                                    auto t = timeTaskMap.find(id);
                                    if (t == timeTaskMap.end()) return;

                                    TimeTaskData& taskData = t->second;

                                    if (taskData.func.isEmpty()) return;
                                    func = taskData.func.get();
                                    if (!taskData.paras.empty()) {
                                        vector<Local<Value>> args;
                                        for (auto& para : taskData.paras)
                                            if (para.isEmpty()) return;
                                            else args.emplace_back(para.get());
                                    }
                                }
                                if (!func.isFunction()) return;
                                if (args.size() > 0) func.asFunction().call({}, args);
                                else func.asFunction().call();
                            }
                            TIMETASK_CATCH("setInterval-Function");
                        }
                    )
                    ->getId();

    std::unique_lock<std::shared_mutex> lock(locker);
    data.swap(timeTaskMap[tid]);
    return tid;
}

int NewInterval(Local<String> func, int timeout) {
    int          tid = ++timeTaskId;
    TimeTaskData data;

    data.code   = func;
    data.engine = EngineScope::currentEngine();

    data.task = taskScheduler
                    .add<ll::schedule::RepeatTask>(
                        std::chrono::milliseconds(timeout),
                        [engine{EngineScope::currentEngine()}, id{tid}]() {
                            try {
                                if ((ll::getServerStatus() != ll::ServerStatus::Running)) return;
                                if (!EngineManager::isValid(engine)) {
                                    ClearTimeTask(id);
                                    return;
                                }
                                EngineScope scope(engine);
                                std::string code;
                                {
                                    std::unique_lock<std::shared_mutex> lock(locker);

                                    auto t = timeTaskMap.find(id);
                                    if (t == timeTaskMap.end()) return;
                                    TimeTaskData& taskData = t->second;

                                    if (taskData.code.isEmpty()) return;
                                    code = taskData.code.get().toString();
                                }
                                if (!code.empty()) engine->eval(code);
                            }
                            TIMETASK_CATCH("setInterval-String");
                        }
                    )
                    ->getId();

    std::unique_lock<std::shared_mutex> lock(locker);
    data.swap(timeTaskMap[tid]);
    return tid;
}

bool ClearTimeTask(int id) {
    assert(EngineScope::currentEngine() != nullptr);
    try {
        std::unique_lock<std::shared_mutex> lock(locker);
        auto                                it = timeTaskMap.find(id);
        if (it != timeTaskMap.end()) {
            taskScheduler.remove(timeTaskMap[id].task);
            timeTaskMap.erase(id);
        }
    } catch (...) {
        lse::getSelfPluginInstance().getLogger().error("Fail in ClearTimeTask");
    }
    return true;
}

///////////////////////// Func /////////////////////////

void LLSERemoveTimeTaskData(ScriptEngine* engine) {
    // enter scope to prevent script::Global::~Global() from crashing
    EngineScope enter(engine);
    try {
        std::unique_lock<std::shared_mutex> lock(locker);
        for (auto it = timeTaskMap.begin(); it != timeTaskMap.end();) {
            if (it->second.engine == engine) {
                taskScheduler.remove(it->second.task);
                it = timeTaskMap.erase(it);
            } else ++it;
        }
    } catch (...) {
        lse::getSelfPluginInstance().getLogger().info("Fail in LLSERemoveTimeTaskData");
    }
}
