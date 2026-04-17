#include "legacy/engine/MessageSystem.h"

#include "legacy/api/APIHelp.h"
#include "legacy/engine/EngineManager.h"
#include "legacy/engine/GlobalShareData.h"
#include "legacy/engine/LocalShareData.h"
#include "legacy/utils/IniHelper.h"
#include "legacy/utils/Utils.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/server/ServerStoppingEvent.h"
#include "ll/api/service/GamingStatus.h"

#include <atomic>
#include <processthreadsapi.h>

using namespace script;

inline ModuleMessage::MessageType GET_MESSAGE_TYPE(utils::Message& e) {
    return static_cast<ModuleMessage::MessageType>(e.data0);
}
inline ModuleMessage::MessageHeader* GET_MESSAGE_HEADER(utils::Message const& e) {
    return static_cast<ModuleMessage::MessageHeader*>(e.ptr0);
}
inline std::string* GET_MESSAGE_DATA_PTR(utils::Message const& e) { return static_cast<std::string*>(e.ptr1); }
#define MESSAGE_TYPE     data0
#define MESSAGE_HEADER   ptr0
#define MESSAGE_DATA_PTR ptr1

//////////////////// 消息处理注册 ////////////////////

#include "legacy/engine/RemoteCall.h"

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

utils::Message PackEngineMessage(
    string const&              toModuleType,
    ModuleMessage::MessageType type,
    string const&              data,
    int*                       messageId = nullptr
) {
    auto& callbacks = globalShareData->messageSystemHandlers[toModuleType];

    utils::Message engineMsg(callbacks.handler, callbacks.cleaner);
    engineMsg.MESSAGE_TYPE     = static_cast<int64_t>(type);
    engineMsg.MESSAGE_HEADER   = new ModuleMessage::MessageHeader();
    engineMsg.MESSAGE_DATA_PTR = new string(data);
    if (messageId) *messageId = (GET_MESSAGE_HEADER(engineMsg))->id;
    return engineMsg;
}

utils::Message
PackEngineMessage(string const& toModuleType, int messageId, ModuleMessage::MessageType type, string const& data) {
    auto& callbacks = globalShareData->messageSystemHandlers[toModuleType];

    utils::Message engineMsg(callbacks.handler, callbacks.cleaner);
    engineMsg.MESSAGE_TYPE              = static_cast<int64_t>(type);
    engineMsg.MESSAGE_HEADER            = new ModuleMessage::MessageHeader();
    (GET_MESSAGE_HEADER(engineMsg))->id = messageId;
    engineMsg.MESSAGE_DATA_PTR          = new string(data);
    return engineMsg;
}

/////////////////////////// Module Message ///////////////////////////

int ModuleMessage::getNextMessageId() {
    return InterlockedIncrement(reinterpret_cast<LONG*>(&(globalShareData->messageSystemNextId)));
}

ModuleMessageResult ModuleMessage::broadcastLocal(MessageType type, string const& data, int64_t delay) {
    std::list<std::shared_ptr<ScriptEngine>> engineList;
    int                                      msgId = -1;

    auto engines = EngineManager::getLocalEngines();
    for (auto& engine : engines) {
        try {
            engine->messageQueue()->postMessage(
                PackEngineMessage(LLSE_BACKEND_TYPE, type, data, &msgId),
                std::chrono::milliseconds(delay)
            );
            engineList.push_back(engine);
        } catch (Exception const& e) {
            EngineScope scope(engine.get());
            lse::LegacyScriptEngine::getLogger().error(
                "Fail to post message to plugin {}",
                getEngineData(engine)->pluginName
            );
            ll::error_utils::printException(e, lse::LegacyScriptEngine::getLogger());
        } catch (...) {
            EngineScope scope(engine.get());
            lse::LegacyScriptEngine::getLogger().error(
                "Fail to post message to plugin {}",
                getEngineData(engine)->pluginName
            );
            ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getLogger());
        }
    }
    return ModuleMessageResult(msgId, engineList);
}

ModuleMessageResult ModuleMessage::broadcastGlobal(MessageType type, string const& data, int64_t delay) {
    std::list<std::shared_ptr<ScriptEngine>> engineList;
    int                                      msgId = -1;

    auto engines = EngineManager::getGlobalEngines();
    for (auto& engine : engines) {
        try {
            engine->messageQueue()->postMessage(
                PackEngineMessage(EngineManager::getEngineType(engine), type, data, &msgId),
                std::chrono::milliseconds(delay)
            );
            engineList.push_back(engine);
        } catch (Exception const& e) {
            EngineScope scope(engine.get());
            lse::LegacyScriptEngine::getLogger().error(
                "Fail to post message to plugin {}",
                getEngineData(engine)->pluginName
            );
            ll::error_utils::printException(e, lse::LegacyScriptEngine::getLogger());
        } catch (...) {
            EngineScope scope(engine.get());
            lse::LegacyScriptEngine::getLogger().error(
                "Fail to post message to plugin {}",
                getEngineData(engine)->pluginName
            );
            ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getLogger());
        }
    }
    return ModuleMessageResult(msgId, engineList);
}

ModuleMessageResult
ModuleMessage::broadcastTo(std::string const& toModuleType, MessageType type, string const& data, int64_t delay) {
    std::list<std::shared_ptr<ScriptEngine>> engineList;
    int                                      msgId = -1;

    auto engines = EngineManager::getGlobalEngines();
    for (auto& engine : engines) {
        if (EngineManager::getEngineType(engine) == toModuleType) {
            try {
                engine->messageQueue()->postMessage(
                    PackEngineMessage(toModuleType, type, data, &msgId),
                    std::chrono::milliseconds(delay)
                );
                engineList.push_back(engine);
            } catch (Exception const& e) {
                EngineScope scope(engine.get());
                lse::LegacyScriptEngine::getLogger().error(
                    "Fail to post message to plugin {}",
                    getEngineData(engine)->pluginName
                );
                ll::error_utils::printException(e, lse::LegacyScriptEngine::getLogger());
            } catch (...) {
                EngineScope scope(engine.get());
                lse::LegacyScriptEngine::getLogger().error(
                    "Fail to post message to plugin {}",
                    getEngineData(engine)->pluginName
                );
                ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getLogger());
            }
        }
    }
    return ModuleMessageResult(msgId, engineList);
}

ModuleMessageResult
ModuleMessage::sendTo(std::shared_ptr<ScriptEngine> engine, MessageType type, std::string const& data, int64_t delay) {
    int    msgId        = -1;
    string toModuleType = LLSE_BACKEND_TYPE;

    try {
        engine->messageQueue()->postMessage(
            PackEngineMessage(EngineManager::getEngineType(engine), type, data, &msgId),
            std::chrono::milliseconds(delay)
        );
        return ModuleMessageResult(msgId, {engine});
    } catch (Exception const& e) {
        EngineScope scope(engine.get());
        lse::LegacyScriptEngine::getLogger().error(
            "Fail to post message to plugin {}",
            getEngineData(engine)->pluginName
        );
        ll::error_utils::printException(e, lse::LegacyScriptEngine::getLogger());
    } catch (...) {
        EngineScope scope(engine.get());
        lse::LegacyScriptEngine::getLogger().error(
            "Fail to post message to plugin {}",
            getEngineData(engine)->pluginName
        );
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getLogger());
    }
    return ModuleMessageResult(msgId, {});
}

ModuleMessageResult
ModuleMessage::sendToRandom(std::string const& toModuleType, MessageType type, std::string const& data, int64_t delay) {
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
            } catch (Exception const& e) {
                EngineScope scope(engine.get());
                lse::LegacyScriptEngine::getLogger().error(
                    "Fail to post message to plugin {}",
                    getEngineData(engine)->pluginName
                );
                ll::error_utils::printException(e, lse::LegacyScriptEngine::getLogger());
            } catch (...) {
                EngineScope scope(engine.get());
                lse::LegacyScriptEngine::getLogger().error(
                    "Fail to post message to plugin {}",
                    getEngineData(engine)->pluginName
                );
                ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getLogger());
            }
        }
    }
    return ModuleMessageResult(msgId, {});
}

bool ModuleMessage::sendResult(MessageType typ, std::string const& dat, int64_t delay) const {
    int           msgId  = header->id;
    ScriptEngine* engine = header->fromEngine;

    try {
        engine->messageQueue()->postMessage(
            PackEngineMessage(header->fromEngineModuleType, msgId, typ, dat),
            std::chrono::milliseconds(delay)
        );
        return true;
    } catch (...) {
        EngineScope scope(engine);
        lse::LegacyScriptEngine::getLogger().error(
            "Fail to post message to plugin {}",
            getEngineData(engine)->pluginName
        );
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getLogger());
    }
    return false;
}

/////////////////////////// Module Message Result ///////////////////////////

ModuleMessageResult::ModuleMessageResult(int messageId, std::list<std::shared_ptr<ScriptEngine>> const& engineList)
: msgId(messageId),
  resultCount(OperationCount::create(std::to_string(messageId))),
  engineList(engineList) {}

ModuleMessageResult::operator bool() const { return getSentCount() > 0; }

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

bool ModuleMessageResult::cancel() const {
    int id = msgId;
    for (auto& engine : engineList) {
        EngineScope scope(engine.get());
        engine->messageQueue()->removeMessageIf([id](utils::Message const& message) {
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
    auto snapshot = EngineManager::getGlobalEngines();
    for (auto& engine : snapshot) {
        if (EngineManager::isValid(engine) && EngineManager::getEngineType(engine) == LLSE_BACKEND_TYPE) {
            try {
                if (EngineScope::currentEngine() == engine.get())
                    engine->messageQueue()->loopQueue(script::utils::MessageQueue::LoopType::kLoopOnce);
                else {
                    EngineScope enter(engine.get());
                    engine->messageQueue()->loopQueue(script::utils::MessageQueue::LoopType::kLoopOnce);
                }
            } catch (Exception const& e) {
                EngineScope scope(engine.get());
                lse::LegacyScriptEngine::getLogger().error(
                    "Error occurred in Engine Message Loop! In plugin: {}",
                    getEngineOwnData()->pluginName
                );
                ll::error_utils::printException(e, lse::LegacyScriptEngine::getLogger());
            } catch (...) {
                lse::LegacyScriptEngine::getLogger().error("Error occurred in Engine Message Loop!");
                ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getLogger());
            }
        }
    }
    // messageLoopLock.unlock();
    // lse::LegacyScriptEngine::getLogger().debug("Engine-{} Message Loop.", LLSE_BACKEND_TYPE);
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
