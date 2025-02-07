#include "engine/TimeTaskSystem.h"

#include "api/APIHelp.h"
#include "engine/EngineManager.h"
#include "engine/EngineOwnData.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/thread/ServerThreadExecutor.h"

#include <ll/api/service/ServerInfo.h>
#include <mutex>
#include <vector>

std::atomic_uint timeTaskId = 0;
std::mutex       locker;

struct TimeTaskData {
    script::Global<Function>           func;
    std::vector<script::Global<Value>> paras;
    script::Global<String>             code;
    ScriptEngine*                      engine;
};

std::unordered_map<uint64, ScriptEngine*> timeTaskMap;

#define TIMETASK_CATCH(TASK_TYPE)                                                                                      \
    catch (const Exception& e) {                                                                                       \
        EngineScope scope(data.engine);                                                                                \
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("Error occurred in {}", TASK_TYPE);         \
        ll::error_utils::printException(e, lse::LegacyScriptEngine::getInstance().getSelf().getLogger());              \
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(                                            \
            "In Plugin: " + getEngineData(data.engine)->pluginName                                                     \
        );                                                                                                             \
    }                                                                                                                  \
    catch (...) {                                                                                                      \
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("Error occurred in {}", TASK_TYPE);         \
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());          \
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(                                            \
            "In Plugin: " + getEngineData(data.engine)->pluginName                                                     \
        );                                                                                                             \
    }

int NewTimeout(Local<Function> func, std::vector<Local<Value>> paras, int timeout) {
    int          tid = ++timeTaskId;
    TimeTaskData data;

    data.func   = func;
    data.engine = EngineScope::currentEngine();
    for (auto& para : paras) data.paras.emplace_back(std::move(para));

    ll::coro::keepThis([timeout, tid, data]() -> ll::coro::CoroTask<> {
        co_await std::chrono::milliseconds(timeout);
        try {
            if (!CheckTimeTask(tid)) {
                co_return;
            }

            if ((ll::getGamingStatus() == ll::GamingStatus::Stopping) || !EngineManager::isValid(data.engine)) {
                ClearTimeTask(tid);
                co_return;
            }

            EngineScope scope(data.engine);
            if (!data.func.isEmpty()) {
                std::vector<Local<Value>> args;
                for (auto& para : data.paras) {
                    if (para.isEmpty()) co_return;
                    args.emplace_back(para.get());
                }
                data.func.get().call({}, args);
            }
        }
        TIMETASK_CATCH("setTimeout-Function");
        ClearTimeTask(tid);
    }).launch(ll::thread::ServerThreadExecutor::getDefault());

    std::lock_guard lock(locker);
    timeTaskMap[tid] = data.engine;
    return tid;
}

int NewTimeout(Local<String> func, int timeout) {
    int          tid = ++timeTaskId;
    TimeTaskData data;

    data.code   = func;
    data.engine = EngineScope::currentEngine();

    ll::coro::keepThis([timeout, tid, data]() -> ll::coro::CoroTask<> {
        co_await std::chrono::milliseconds(timeout);
        try {
            if (!CheckTimeTask(tid)) {
                co_return;
            }

            if ((ll::getGamingStatus() == ll::GamingStatus::Stopping) || !EngineManager::isValid(data.engine)) {
                ClearTimeTask(tid);
                co_return;
            }

            EngineScope scope(data.engine);
            if (!data.code.isEmpty()) {
                auto code = data.code.get().toString();
                data.engine->eval(code);
            }
        }
        TIMETASK_CATCH("setTimeout-String");
        ClearTimeTask(tid);
    }).launch(ll::thread::ServerThreadExecutor::getDefault());

    std::lock_guard lock(locker);
    timeTaskMap[tid] = data.engine;
    return tid;
}

int NewInterval(Local<Function> func, std::vector<Local<Value>> paras, int timeout) {
    int          tid = ++timeTaskId;
    TimeTaskData data;

    data.func   = func;
    data.engine = EngineScope::currentEngine();
    for (auto& para : paras) data.paras.emplace_back(std::move(para));

    ll::coro::keepThis([timeout, tid, data]() -> ll::coro::CoroTask<> {
        while (true) {
            co_await std::chrono::milliseconds(timeout);
            try {
                if (!CheckTimeTask(tid)) {
                    co_return;
                }

                if ((ll::getGamingStatus() == ll::GamingStatus::Stopping) || !EngineManager::isValid(data.engine)) {
                    ClearTimeTask(tid);
                    co_return;
                }

                EngineScope scope(data.engine);

                if (!data.func.isEmpty()) {
                    std::vector<Local<Value>> args;
                    for (auto& para : data.paras) {
                        if (para.isEmpty()) co_return;
                        args.emplace_back(para.get());
                    }
                    data.func.get().call({}, args);
                }
            }
            TIMETASK_CATCH("setInterval-Function");
        }
    }).launch(ll::thread::ServerThreadExecutor::getDefault());

    std::lock_guard lock(locker);
    timeTaskMap[tid] = data.engine;
    return tid;
}

int NewInterval(Local<String> func, int timeout) {
    int          tid = ++timeTaskId;
    TimeTaskData data;

    data.code   = func;
    data.engine = EngineScope::currentEngine();

    ll::coro::keepThis([timeout, tid, data]() -> ll::coro::CoroTask<> {
        while (true) {
            co_await std::chrono::milliseconds(timeout);
            try {
                if (!CheckTimeTask(tid)) {
                    co_return;
                }
                if ((ll::getGamingStatus() == ll::GamingStatus::Stopping) || !EngineManager::isValid(data.engine)) {
                    ClearTimeTask(tid);
                    co_return;
                }

                EngineScope scope(data.engine);
                if (!data.code.isEmpty()) {
                    data.engine->eval(data.code.get().toString());
                }
            }
            TIMETASK_CATCH("setInterval-String");
        }
    }).launch(ll::thread::ServerThreadExecutor::getDefault());

    std::lock_guard lock(locker);
    timeTaskMap[tid] = data.engine;
    return tid;
}

bool CheckTimeTask(int const& id) {
    std::lock_guard lock(locker);
    return timeTaskMap.find(id) != timeTaskMap.end();
}

bool ClearTimeTask(int const& id) {
    try {
        std::lock_guard lock(locker);
        if (timeTaskMap.find(id) != timeTaskMap.end()) {
            timeTaskMap.erase(id);
        }
    } catch (...) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("Fail in ClearTimeTask");
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
    }
    return true;
}

void LLSERemoveTimeTaskData(ScriptEngine* engine) {
    // enter scope to prevent script::Global::~Global() from crashing
    EngineScope enter(engine);
    try {
        std::lock_guard lock(locker);
        for (auto it = timeTaskMap.begin(); it != timeTaskMap.end();) {
            if (it->second == engine) {
                it = timeTaskMap.erase(it);
            } else {
                ++it;
            }
        }
    } catch (...) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().info("Fail in LLSERemoveTimeTaskData");
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
    }
}
