#include "engine/TimeTaskSystem.h"
#include "api/APIHelp.h"
#include "engine/EngineManager.h"
#include "engine/EngineOwnData.h"
#include "engine/MessageSystem.h"
#include "legacyapi/utils/STLHelper.h"
#include "ll/api/schedule/Scheduler.h"
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/schedule/Task.h>
#include <ll/api/service/ServerInfo.h>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <vector>

std::atomic_uint timeTaskId = 0;
std::shared_mutex locker;
ll::schedule::ServerTimeScheduler scheduler;
struct TimeTaskData {
  std::shared_ptr<ll::schedule::Task<ll::chrono::ServerClock>> task;
  script::Global<Function> func;
  vector<script::Global<Value>> paras;
  script::Global<String> code;
  ScriptEngine *engine;
  inline void swap(TimeTaskData &rhs) {
    std::swap(rhs.task, task);
    std::swap(rhs.engine, engine);
    rhs.code.swap(code);
    rhs.paras.swap(paras);
    rhs.func.swap(func);
  }
};
std::unordered_map<int, TimeTaskData> timeTaskMap;

#define TIMETASK_CATCH(TASK_TYPE)                                              \
  catch (const Exception &e) {                                                 \
    EngineScope scope(engine);                                                 \
    logger.error("Error occurred in {}", TASK_TYPE);                           \
    PrintException(e);                                                         \
    logger.error("In Plugin: " + ENGINE_GET_DATA(engine)->pluginName);         \
  }                                                                            \
  catch (const std::exception &e) {                                            \
    logger.error("Error occurred in {}", TASK_TYPE);                           \
    logger.error("C++ Uncaught Exception Detected!");                          \
    logger.error(ll::string_utils::tou8str(e.what()));                         \
    logger.error("In Plugin: " + ENGINE_GET_DATA(engine)->pluginName);         \
  }                                                                            \
  catch (...) {                                                                \
    logger.error("Error occurred in {}", TASK_TYPE);                           \
    logger.error("Uncaught Exception Detected!");                              \
    logger.error("In Plugin: " + ENGINE_GET_DATA(engine)->pluginName);         \
  }

//////////////////// API ////////////////////

// void NewTimeout_s(script::Global<Function> func, vector<script::Local<Value>>
// paras, int timeout, ScriptEngine* engine)
//{
//     std::vector<script::Global<Value>> tmp;
//     if (paras.size() > 0) {
//         EngineScope enter(engine);
//         for (auto& para : paras)
//             tmp.emplace_back(std::move(para));
//     }
//     Schedule::delay(
//         [engine, func = std::move(func), paras = std::move(tmp)]() {
//             if ((ll::getServerStatus() != ll::ServerStatus::Running))
//                 return;
//             if (!EngineManager::isValid(engine))
//                 return;
//             EngineScope enter(engine);
//             if (paras.empty()) {
//                 func.get().call();
//             }
//             else
//             {
//                 vector<Local<Value>> args;
//                 for (auto& para : paras)
//                     if (para.isEmpty())
//                         return;
//                     else
//                         args.emplace_back(para.get());
//                 func.get().call({}, args);
//             }
//         },
//         timeout / 50);
// }

int NewTimeout(Local<Function> func, vector<Local<Value>> paras, int timeout) {
  int tid = ++timeTaskId;
  TimeTaskData data;

  data.func = func;
  data.engine = EngineScope::currentEngine();
  for (auto &para : paras)
    data.paras.emplace_back(std::move(para));
  data.task = scheduler.add<ll::schedule::DelayTask>(
      ll::chrono::ticks(timeout / 50),
      [engine{EngineScope::currentEngine()}, id{tid}]() {
        try {
          if ((ll::getServerStatus() != ll::ServerStatus::Running))
            return;
          if (!EngineManager::isValid(engine))
            return;
          // lock after enter EngineScope to prevent deadlock
          EngineScope scope(engine);
          TimeTaskData taskData;
          {
            std::unique_lock<std::shared_mutex> lock(locker);

            auto t = timeTaskMap.find(id);
            if (t == timeTaskMap.end())
              return;
            t->second.swap(taskData);
            timeTaskMap.erase(id);
          }

          if (taskData.func.isEmpty())
            return;
          auto func = taskData.func.get();
          if (taskData.paras.empty()) {
            func.call();
          } else {
            vector<Local<Value>> args;
            for (auto &para : taskData.paras)
              if (para.isEmpty())
                return;
              else
                args.emplace_back(para.get());
            func.call({}, args);
          }
        }
        TIMETASK_CATCH("setTimeout-Function");
      });
  std::unique_lock<std::shared_mutex> lock(locker);
  data.swap(timeTaskMap[tid]);
  return tid;
}

int NewTimeout(Local<String> func, int timeout) {
  int tid = ++timeTaskId;
  TimeTaskData data;

  data.code = func;
  data.engine = EngineScope::currentEngine();

  data.task = scheduler.add<ll::schedule::DelayTask>(
      ll::chrono::ticks(timeout / 50),
      [engine{EngineScope::currentEngine()}, id{tid}]() {
        try {
          if ((ll::getServerStatus() != ll::ServerStatus::Running))
            return;
          if (!EngineManager::isValid(engine))
            return;
          EngineScope scope(engine);
          TimeTaskData taskData;
          {
            std::unique_lock<std::shared_mutex> lock(locker);

            auto t = timeTaskMap.find(id);
            if (t == timeTaskMap.end())
              return;
            t->second.swap(taskData);
            timeTaskMap.erase(id);
          }

          if (taskData.code.isEmpty())
            return;
          auto code = taskData.code.get().toString();
          engine->eval(code);
        }
        TIMETASK_CATCH("setTimeout-String");
      });

  std::unique_lock<std::shared_mutex> lock(locker);
  data.swap(timeTaskMap[tid]);
  return tid;
}

int NewInterval(Local<Function> func, vector<Local<Value>> paras, int timeout) {
  int tid = ++timeTaskId;
  TimeTaskData data;

  data.func = func;
  data.engine = EngineScope::currentEngine();
  for (auto &para : paras)
    data.paras.emplace_back(std::move(para));

  data.task = scheduler.add<ll::schedule::RepeatTask>(
      ll::chrono::ticks(timeout / 50),
      [engine{EngineScope::currentEngine()}, id{tid}]() {
        try {
          if ((ll::getServerStatus() != ll::ServerStatus::Running))
            return;
          if (!EngineManager::isValid(engine)) {
            ClearTimeTask(id);
            return;
          }
          EngineScope scope(engine);
          Local<Value> func = Local<Value>();
          vector<Local<Value>> args;
          {
            std::unique_lock<std::shared_mutex> lock(locker);

            auto t = timeTaskMap.find(id);
            if (t == timeTaskMap.end())
              return;

            TimeTaskData &taskData = t->second;

            if (taskData.func.isEmpty())
              return;
            func = taskData.func.get();
            if (!taskData.paras.empty()) {
              vector<Local<Value>> args;
              for (auto &para : taskData.paras)
                if (para.isEmpty())
                  return;
                else
                  args.emplace_back(para.get());
            }
          }
          if (!func.isFunction())
            return;
          if (args.size() > 0)
            func.asFunction().call({}, args);
          else
            func.asFunction().call();
        }
        TIMETASK_CATCH("setInterval-Function");
      });

  std::unique_lock<std::shared_mutex> lock(locker);
  data.swap(timeTaskMap[tid]);
  return tid;
}

int NewInterval(Local<String> func, int timeout) {
  int tid = ++timeTaskId;
  TimeTaskData data;

  data.code = func;
  data.engine = EngineScope::currentEngine();

  data.task = scheduler.add<ll::schedule::RepeatTask>(
      ll::chrono::ticks(timeout / 50),
      [engine{EngineScope::currentEngine()}, id{tid}]() {
        try {
          if ((ll::getServerStatus() != ll::ServerStatus::Running))
            return;
          if (!EngineManager::isValid(engine)) {
            ClearTimeTask(id);
            return;
          }
          EngineScope scope(engine);
          std::string code;
          {
            std::unique_lock<std::shared_mutex> lock(locker);

            auto t = timeTaskMap.find(id);
            if (t == timeTaskMap.end())
              return;
            TimeTaskData &taskData = t->second;

            if (taskData.code.isEmpty())
              return;
            code = taskData.code.get().toString();
          }
          if (!code.empty())
            engine->eval(code);
        }
        TIMETASK_CATCH("setInterval-String");
      });

  std::unique_lock<std::shared_mutex> lock(locker);
  data.swap(timeTaskMap[tid]);
  return tid;
}

bool ClearTimeTask(int id) {
  assert(EngineScope::currentEngine() != nullptr);
  TimeTaskData data;
  try {
    std::unique_lock<std::shared_mutex> lock(locker);
    auto it = timeTaskMap.find(id);
    if (it != timeTaskMap.end()) {
      data.swap(timeTaskMap[id]);
      timeTaskMap.erase(id);
    }
  } catch (...) {
    logger.error("Fail in ClearTimeTask");
  }
  return true;
}

///////////////////////// Func /////////////////////////

void LLSERemoveTimeTaskData(ScriptEngine *engine) {
  // enter scope to prevent script::Global::~Global() from crashing
  EngineScope enter(engine);
  std::unordered_map<int, TimeTaskData> tmpMap;
  try {
    std::unique_lock<std::shared_mutex> lock(locker);
    for (auto it = timeTaskMap.begin(); it != timeTaskMap.end();) {
      if (it->second.engine == engine) {
        it->second.swap(tmpMap[it->first]);
        it = timeTaskMap.erase(it);
      } else
        ++it;
    }
  } catch (...) {
    logger.info("Fail in LLSERemoveTimeTaskData");
  }
  tmpMap.clear();
}
