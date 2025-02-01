#include "engine/MessageSystem.h"

#include "api/APIHelp.h"
#include "engine/GlobalShareData.h"
#include "engine/LocalShareData.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/server/ServerStoppingEvent.h"
#include "ll/api/service/GamingStatus.h"
#include "utils/IniHelper.h"
#include "utils/Utils.h"

#include <exception>
#include <ll/api/event/EventBus.h>
#include <ll/api/service/ServerInfo.h>
#include <mutex>
#include <process.h>
#include <processthreadsapi.h>
#include <shared_mutex>

using namespace script;

inline ModuleMessage::MessageType    GET_MESSAGE_TYPE(utils::Message& e) { return (ModuleMessage::MessageType)e.data0; }
inline ModuleMessage::MessageHeader* GET_MESSAGE_HEADER(utils::Message& e) {
    return (ModuleMessage::MessageHeader*)e.ptr0;
}
inline std::string* GET_MESSAGE_DATA_PTR(utils::Message& e) { return (std::string*)e.ptr1; }
#define MESSAGE_TYPE     data0
#define MESSAGE_HEADER   ptr0
#define MESSAGE_DATA_PTR ptr1

//////////////////// 消息处理注册 ////////////////////

#include "engine/RemoteCall.h"

void ModuleMessage::handle(utils::Message& engineMsg) // Warning: Execute in another thread
{
    ModuleMessage msg;
    msg.type   = GET_MESSAGE_TYPE(engineMsg);
    msg.header = GET_MESSAGE_HEADER(engineMsg);
    msg.data   = GET_MESSAGE_DATA_PTR(engineMsg);

    switch (msg.getType()) {
    case ModuleMessage::MessageType::RemoteSyncCallRequest:
        RemoteSyncCallRequest(msg);
        break;
    case ModuleMessage::MessageType::RemoteSyncCallReturn:
        RemoteSyncCallReturn(msg);
        break;
    /*case ModuleMessage::MessageType::RemoteLoadRequest:
        RemoteLoadRequest(msg);
        break;
    case ModuleMessage::MessageType::RemoteLoadReturn:
        RemoteLoadReturn(msg);
        break;*/
    default:
        break;
    }
}

void ModuleMessage::cleanup(utils::Message& engineMsg) {
    delete GET_MESSAGE_HEADER(engineMsg);
    delete GET_MESSAGE_DATA_PTR(engineMsg);
}

/////////////////////////// Helper ///////////////////////////

utils::Message
PackEngineMessage(string toModuleType, ModuleMessage::MessageType type, string data, int* messageId = nullptr) {
    auto& callbacks = globalShareData->messageSystemHandlers[toModuleType];

    utils::Message engineMsg(callbacks.handler, callbacks.cleaner);
    engineMsg.MESSAGE_TYPE     = (int64_t)type;
    engineMsg.MESSAGE_HEADER   = new ModuleMessage::MessageHeader();
    engineMsg.MESSAGE_DATA_PTR = new string(data);
    if (messageId) *messageId = (GET_MESSAGE_HEADER(engineMsg))->id;
    return engineMsg;
}

utils::Message PackEngineMessage(string toModuleType, int messageId, ModuleMessage::MessageType type, string data) {
    auto& callbacks = globalShareData->messageSystemHandlers[toModuleType];

    utils::Message engineMsg(callbacks.handler, callbacks.cleaner);
    engineMsg.MESSAGE_TYPE              = (int64_t)type;
    engineMsg.MESSAGE_HEADER            = new ModuleMessage::MessageHeader();
    (GET_MESSAGE_HEADER(engineMsg))->id = messageId;
    engineMsg.MESSAGE_DATA_PTR          = new string(data);
    return engineMsg;
}

/////////////////////////// Module Message ///////////////////////////

int ModuleMessage::getNextMessageId() { return InterlockedIncrement((LONG*)&(globalShareData->messageSystemNextId)); }

ModuleMessageResult ModuleMessage::broadcastLocal(MessageType type, string data, int64_t delay) {
    std::vector<ScriptEngine*> engineList;
    int                        msgId = -1;

    auto engines = EngineManager::getLocalEngines();
    for (auto& engine : engines) {
        try {
            engine->messageQueue()->postMessage(
                PackEngineMessage(LLSE_BACKEND_TYPE, type, data, &msgId),
                std::chrono::milliseconds(delay)
            );
            engineList.push_back(engine);
        } catch (const Exception& e) {
            EngineScope scope(engine);
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                "Fail to post message to plugin {}",
                getEngineData(engine)->pluginName
            );
            ll::error_utils::printException(e, lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
        } catch (...) {
            EngineScope scope(engine);
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                "Fail to post message to plugin {}",
                getEngineData(engine)->pluginName
            );
            ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
        }
    }
    return ModuleMessageResult(msgId, engineList);
}

ModuleMessageResult ModuleMessage::broadcastGlobal(MessageType type, string data, int64_t delay) {
    std::vector<ScriptEngine*> engineList;
    int                        msgId = -1;

    auto engines = EngineManager::getGlobalEngines();
    for (auto& engine : engines) {
        try {
            engine->messageQueue()->postMessage(
                PackEngineMessage(EngineManager::getEngineType(engine), type, data, &msgId),
                std::chrono::milliseconds(delay)
            );
            engineList.push_back(engine);
        } catch (const Exception& e) {
            EngineScope scope(engine);
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                "Fail to post message to plugin {}",
                getEngineData(engine)->pluginName
            );
            ll::error_utils::printException(e, lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
        } catch (...) {
            EngineScope scope(engine);
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                "Fail to post message to plugin {}",
                getEngineData(engine)->pluginName
            );
            ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
        }
    }
    return ModuleMessageResult(msgId, engineList);
}

ModuleMessageResult ModuleMessage::broadcastTo(std::string toModuleType, MessageType type, string data, int64_t delay) {
    std::vector<ScriptEngine*> engineList;
    int                        msgId = -1;

    auto engines = EngineManager::getGlobalEngines();
    for (auto& engine : engines) {
        if (EngineManager::getEngineType(engine) == toModuleType) {
            try {
                engine->messageQueue()->postMessage(
                    PackEngineMessage(toModuleType, type, data, &msgId),
                    std::chrono::milliseconds(delay)
                );
                engineList.push_back(engine);
            } catch (const Exception& e) {
                EngineScope scope(engine);
                lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                    "Fail to post message to plugin {}",
                    getEngineData(engine)->pluginName
                );
                ll::error_utils::printException(e, lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
            } catch (...) {
                EngineScope scope(engine);
                lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                    "Fail to post message to plugin {}",
                    getEngineData(engine)->pluginName
                );
                ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
            }
        }
    }
    return ModuleMessageResult(msgId, engineList);
}

ModuleMessageResult ModuleMessage::sendTo(ScriptEngine* engine, MessageType type, std::string data, int64_t delay) {
    int    msgId        = -1;
    string toModuleType = LLSE_BACKEND_TYPE;

    try {
        engine->messageQueue()->postMessage(
            PackEngineMessage(EngineManager::getEngineType(engine), type, data, &msgId),
            std::chrono::milliseconds(delay)
        );
        return ModuleMessageResult(msgId, {engine});
    } catch (const Exception& e) {
        EngineScope scope(engine);
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to post message to plugin {}",
            getEngineData(engine)->pluginName
        );
        ll::error_utils::printException(e, lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
    } catch (...) {
        EngineScope scope(engine);
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to post message to plugin {}",
            getEngineData(engine)->pluginName
        );
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
    }
    return ModuleMessageResult(msgId, {});
}

ModuleMessageResult
ModuleMessage::sendToRandom(std::string toModuleType, MessageType type, std::string data, int64_t delay) {
    int msgId = -1;

    auto engines = EngineManager::getGlobalEngines();
    for (auto& engine : engines) {
        if (EngineManager::getEngineType(engine) == toModuleType) {
            try {
                engine->messageQueue()->postMessage(
                    PackEngineMessage(toModuleType, type, data, &msgId),
                    std::chrono::milliseconds(delay)
                );
                return ModuleMessageResult(msgId, {engine});
            } catch (const Exception& e) {
                EngineScope scope(engine);
                lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                    "Fail to post message to plugin {}",
                    getEngineData(engine)->pluginName
                );
                ll::error_utils::printException(e, lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
            } catch (...) {
                EngineScope scope(engine);
                lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                    "Fail to post message to plugin {}",
                    getEngineData(engine)->pluginName
                );
                ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
            }
        }
    }
    return ModuleMessageResult(msgId, {});
}

bool ModuleMessage::sendResult(MessageType typ, std::string dat, int64_t delay) {
    int           msgId  = header->id;
    ScriptEngine* engine = header->fromEngine;

    try {
        engine->messageQueue()->postMessage(
            PackEngineMessage(header->fromEngineModuleType, msgId, typ, dat),
            std::chrono::milliseconds(delay)
        );
        return true;
    } catch (const Exception& e) {
        EngineScope scope(engine);
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to post message to plugin {}",
            getEngineData(engine)->pluginName
        );
        ll::error_utils::printException(e, lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
    } catch (...) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Fail to post message to plugin {}",
            getEngineData(engine)->pluginName
        );
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
    }
    return false;
}

/////////////////////////// Module Message Result ///////////////////////////

ModuleMessageResult::ModuleMessageResult(int messageId, std::vector<ScriptEngine*> engineList)
: msgId(messageId),
  engineList(engineList),
  resultCount(OperationCount::create(std::to_string(messageId))) {}

ModuleMessageResult::operator bool() { return getSentCount() > 0; }

ModuleMessageResult::~ModuleMessageResult() { resultCount.remove(); }

bool ModuleMessageResult::waitForAllResults(int maxWaitTime) { return waitForResultCount(getSentCount(), maxWaitTime); }

bool ModuleMessageResult::waitForOneResult(int maxWaitTime) { return waitForResultCount(1, maxWaitTime); }

bool ModuleMessageResult::waitForResultCount(size_t targetCount, int maxWaitTime) {
    bool res      = false;
    auto fromTime = GetCurrentTimeStampMS();

    while (maxWaitTime < 0 ? true : GetCurrentTimeStampMS() - fromTime <= maxWaitTime) {
        Sleep(LLSE_MESSAGE_SYSTEM_WAIT_CHECK_INTERVAL);
        if (resultCount.hasReachCount(targetCount)) {
            res = true;
            break;
        }
        MessageSystemLoopOnce();
    }
    return res;
}

bool ModuleMessageResult::cancel() {
    int id = msgId;
    for (auto& engine : engineList) {
        EngineScope scope(engine);
        engine->messageQueue()->removeMessageIf([id](utils::Message& message) {
            return (GET_MESSAGE_HEADER(message))->id == id
                     ? utils::MessageQueue::RemoveMessagePredReturnType::kRemoveAndContinue
                     : utils::MessageQueue::RemoveMessagePredReturnType::kDontRemove;
        });
    }
    return true;
}

///////////////////////////// Funcs /////////////////////////////
void MessageSystemLoopOnce() {
    // if (!messageLoopLock.try_lock())
    //     return;
    std::list<ScriptEngine*> tmpList;
    {
        std::unique_lock<std::shared_mutex> lock(globalShareData->engineListLock);
        // low efficiency
        tmpList = globalShareData->globalEngineList;
    }
    for (auto engine : tmpList) {
        if (EngineManager::isValid(engine) && EngineManager::getEngineType(engine) == LLSE_BACKEND_TYPE) {
            try {
                if (EngineScope::currentEngine() == engine)
                    engine->messageQueue()->loopQueue(script::utils::MessageQueue::LoopType::kLoopOnce);
                else {
                    EngineScope enter(engine);
                    engine->messageQueue()->loopQueue(script::utils::MessageQueue::LoopType::kLoopOnce);
                }
            } catch (const Exception& e) {
                EngineScope scope(engine);
                lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                    "Error occurred in Engine Message Loop!"
                );
                ll::error_utils::printException(e, lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
                lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                    "In Plugin: " + getEngineOwnData()->pluginName
                );
            } catch (...) {
                lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                    "Error occurred in Engine Message Loop!"
                );
                ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
            }
        }
    }
    // messageLoopLock.unlock();
    // lse::LegacyScriptEngine::getInstance().getSelf().getLogger().debug("Engine-{} Message Loop.", LLSE_BACKEND_TYPE);
}

void InitMessageSystem() {
    globalShareData->messageSystemHandlers[LLSE_BACKEND_TYPE] = {ModuleMessage::handle, ModuleMessage::cleanup};

    ll::event::EventBus::getInstance().emplaceListener<ll::event::ServerStoppingEvent>(
        [](ll::event::ServerStoppingEvent&) { EndMessageSystemLoop(); }
    );

    // dangerous?
    std::thread([]() {
        globalShareData->messageThreads[LLSE_BACKEND_TYPE] = GetCurrentThread();
        while (true) {
            MessageSystemLoopOnce();
            if (ll::getGamingStatus() != ll::GamingStatus::Stopping) return;
            SleepEx(5, true);
            if (ll::getGamingStatus() != ll::GamingStatus::Stopping) return;
        }
    }).detach();
}

// Helper
void APCEmptyHelper(ULONG_PTR) { ; }

bool EndMessageSystemLoop() {
    auto res = globalShareData->messageThreads.find(LLSE_BACKEND_TYPE);
    if (res == globalShareData->messageThreads.end()) return false;

    QueueUserAPC(APCEmptyHelper, res->second, 0);
    globalShareData->messageThreads.erase(LLSE_BACKEND_TYPE);
    Sleep(1);
    return true;
}
