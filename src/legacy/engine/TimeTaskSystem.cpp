#include "engine/TimeTaskSystem.h"

#include "api/APIHelp.h"
#include "engine/EngineManager.h"
#include "engine/EngineOwnData.h"
#include "engine/MessageSystem.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/thread/ThreadPoolExecutor.h"

#include <chrono>
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/service/ServerInfo.h>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <vector>

std::atomic_uint               timeTaskId = 0;
std::mutex                     locker;
ll::thread::ThreadPoolExecutor taskScheduler{"LSE_TASK", 1};

struct TimeTaskData {
    uint64                             taskId;
    script::Global<Function>           func;
    std::vector<script::Global<Value>> paras;
    script::Global<String>             code;
    ScriptEngine*                      engine;
    inline void                        swap(TimeTaskData& rhs) {
        std::swap(rhs.taskId, taskId);
        std::swap(rhs.engine, engine);
        rhs.code.swap(code);
        rhs.paras.swap(paras);
        rhs.func.swap(func);
    }
};

std::unordered_map<int, TimeTaskData> timeTaskMap;

#define TIMETASK_CATCH(TASK_TYPE)                                                                                      \
    catch (const Exception& e) {                                                                                       \
        EngineScope scope(data.engine);                                                                                \
        lse::getSelfPluginInstance().getLogger().error("Error occurred in {}", TASK_TYPE);                             \
        PrintException(e);                                                                                             \
        lse::getSelfPluginInstance().getLogger().error("In Plugin: " + getEngineData(data.engine)->pluginName);        \
    }                                                                                                                  \
    catch (const std::exception& e) {                                                                                  \
        lse::getSelfPluginInstance().getLogger().error("Error occurred in {}", TASK_TYPE);                             \
        lse::getSelfPluginInstance().getLogger().error("C++ Uncaught Exception Detected!");                            \
        lse::getSelfPluginInstance().getLogger().error(ll::string_utils::tou8str(e.what()));                           \
        lse::getSelfPluginInstance().getLogger().error("In Plugin: " + getEngineData(data.engine)->pluginName);        \
    }                                                                                                                  \
    catch (...) {                                                                                                      \
        lse::getSelfPluginInstance().getLogger().error("Error occurred in {}", TASK_TYPE);                             \
        lse::getSelfPluginInstance().getLogger().error("Uncaught Exception Detected!");                                \
        lse::getSelfPluginInstance().getLogger().error("In Plugin: " + getEngineData(data.engine)->pluginName);        \
    }

int NewTimeout(Local<Function> func, std::vector<Local<Value>> paras, int timeout) {
    int          tid = ++timeTaskId;
    TimeTaskData data;

    data.func   = func;
    data.engine = EngineScope::currentEngine();
    for (auto& para : paras) data.paras.emplace_back(std::move(para));

    taskScheduler.execute([timeout, tid, data = std::move(data)]() mutable {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
        try {
            if ((ll::getGamingStatus() == ll::GamingStatus::Stopping) || !EngineManager::isValid(data.engine)) return;

            EngineScope scope(data.engine);

            if (!data.func.isEmpty()) {
                std::vector<Local<Value>> args;
                for (auto& para : data.paras) {
                    if (para.isEmpty()) return;
                    args.emplace_back(para.get());
                }
                data.func.get().call({}, args);
            }
        }
        TIMETASK_CATCH("setTimeout-Function");
        ClearTimeTask(tid);
    });

    std::lock_guard lock(locker);
    timeTaskMap[tid] = std::move(data);
    return tid;
}

int NewTimeout(Local<String> func, int timeout) {
    int          tid = ++timeTaskId;
    TimeTaskData data;

    data.code   = func;
    data.engine = EngineScope::currentEngine();

    taskScheduler.execute([timeout, tid, data = std::move(data)]() mutable {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
        try {
            if ((ll::getGamingStatus() == ll::GamingStatus::Stopping) || !EngineManager::isValid(data.engine)) return;

            EngineScope scope(data.engine);
            {
                std::lock_guard lock(locker);
                auto            it = timeTaskMap.find(tid);
                if (it == timeTaskMap.end()) return;

                auto& taskData = it->second;

                if (!taskData.code.isEmpty()) {
                    auto code = taskData.code.get().toString();
                    data.engine->eval(code);
                }

                // 清理任务
                timeTaskMap.erase(tid);
            }
        }
        TIMETASK_CATCH("setTimeout-String");
    });

    std::lock_guard lock(locker);
    timeTaskMap[tid] = std::move(data);
    return tid;
}

int NewInterval(Local<Function> func, std::vector<Local<Value>> paras, int timeout) {
    int          tid = ++timeTaskId;
    TimeTaskData data;

    data.func   = func;
    data.engine = EngineScope::currentEngine();
    for (auto& para : paras) data.paras.emplace_back(std::move(para));

    taskScheduler.execute([timeout, tid, data = std::move(data)]() mutable {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
            try {
                if ((ll::getGamingStatus() == ll::GamingStatus::Stopping) || !EngineManager::isValid(data.engine)) {
                    ClearTimeTask(tid);
                    return;
                }

                EngineScope scope(data.engine);

                if (!data.func.isEmpty()) {
                    std::vector<Local<Value>> args;
                    for (auto& para : data.paras) {
                        if (para.isEmpty()) return;
                        args.emplace_back(para.get());
                    }
                    data.func.get().call({}, args);
                }
            }
            TIMETASK_CATCH("setInterval-Function");
        }
    });

    std::lock_guard lock(locker);
    timeTaskMap[tid] = std::move(data);
    return tid;
}

int NewInterval(Local<String> func, int timeout) {
    int          tid = ++timeTaskId;
    TimeTaskData data;

    data.code   = func;
    data.engine = EngineScope::currentEngine();

    taskScheduler.execute([timeout, tid, data = std::move(data)]() mutable {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
            try {
                if ((ll::getGamingStatus() == ll::GamingStatus::Stopping)) {
                    ClearTimeTask(tid);
                    return;
                }

                if (!EngineManager::isValid(data.engine)) {
                    ClearTimeTask(tid);
                    return;
                }

                EngineScope scope(data.engine);
                std::string code;
                {
                    std::lock_guard lock(locker);

                    auto it = timeTaskMap.find(tid);
                    if (it == timeTaskMap.end()) return;

                    auto& taskData = it->second;

                    if (taskData.code.isEmpty()) return;
                    code = taskData.code.get().toString();
                }
                if (!code.empty()) {
                    data.engine->eval(code);
                }
            }
            TIMETASK_CATCH("setInterval-String");
        }
    });

    std::lock_guard lock(locker);
    timeTaskMap[tid] = std::move(data);
    return tid;
}

bool ClearTimeTask(int id) {
    try {
        std::lock_guard lock(locker);
        auto            it = timeTaskMap.find(id);
        if (it != timeTaskMap.end()) {
            timeTaskMap.erase(id);
        }
    } catch (...) {
        lse::getSelfPluginInstance().getLogger().error("Fail in ClearTimeTask");
    }
    return true;
}

void LLSERemoveTimeTaskData(ScriptEngine* engine) {
    EngineScope enter(engine);
    try {
        std::lock_guard lock(locker);
        for (auto it = timeTaskMap.begin(); it != timeTaskMap.end();) {
            if (it->second.engine == engine) {
                it = timeTaskMap.erase(it);
            } else {
                ++it;
            }
        }
    } catch (...) {
        lse::getSelfPluginInstance().getLogger().info("Fail in LLSERemoveTimeTaskData");
    }
}
