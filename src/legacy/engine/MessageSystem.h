#pragma once
#include "legacy/api/APIHelp.h"
#include "legacy/engine/OperationCount.h"

#include <ScriptX/ScriptX.h>
#include <Windows.h>
#include <string>

using std::string;

//////////////////// Class ////////////////////

class ModuleMessageResult {
    int                                      msgId;
    OperationCount                           resultCount;
    std::list<std::shared_ptr<ScriptEngine>> engineList;

    friend class ModuleMessage;
    ModuleMessageResult(int messageId, std::list<std::shared_ptr<ScriptEngine>> const& engineList);

public:
    ModuleMessageResult() : msgId(0), resultCount("") {}
    ~ModuleMessageResult();
    operator bool() const;

    size_t getSentCount() const { return engineList.size(); }
    int    getMsgId() const { return msgId; }

    bool waitForAllResults(int maxWaitTime = -1);
    bool waitForOneResult(int maxWaitTime = -1);
    bool waitForResultCount(size_t targetCount, int maxWaitTime = -1);
    bool cancel() const;
};

class ModuleMessage {
    static int getNextMessageId();

public:
    enum class MessageType : UINT {
        MODULE_MESSAGE_REQUEST,
        RequireBroadcast,
        RemoteSyncCallRequest,
        // RemoteLoadRequest,
        MODULE_MESSAGE_RETURN,
        RemoteSyncCallReturn,
        // RemoteLoadReturn,
    };
    struct MessageHeader {
        MessageHeader() {
            id                   = getNextMessageId();
            fromEngine           = EngineScope::currentEngine();
            fromEngineModuleType = LLSE_BACKEND_TYPE;
        }
        int           id;
        ScriptEngine* fromEngine;
        string        fromEngineModuleType;
    };

    MessageType    type;
    MessageHeader* header;
    std::string*   data;

    unsigned    getId() const { return header->id; }
    MessageType getType() const { return type; }
    std::string getData() const { return *data; }

    static ModuleMessageResult broadcastLocal(MessageType type, std::string const& data, int64_t delay = 0);
    static ModuleMessageResult broadcastGlobal(MessageType type, std::string const& data, int64_t delay = 0);
    static ModuleMessageResult
    broadcastTo(std::string const& toModuleType, MessageType type, std::string const& data, int64_t delay = 0);
    static ModuleMessageResult
    sendTo(std::shared_ptr<ScriptEngine> engine, MessageType type, std::string const& data, int64_t delay = 0);
    static ModuleMessageResult
    sendToRandom(std::string const& toModuleType, MessageType type, std::string const& data, int64_t delay = 0);

    bool sendResult(MessageType type, std::string const& data, int64_t delay = 0) const;

    static void handle(script::utils::Message& engineMsg);
    static void cleanup(script::utils::Message& engineMsg);
};

///////////////////////////// Funcs /////////////////////////////
void InitMessageSystem();
void MessageSystemLoopOnce();
bool EndMessageSystemLoop();
