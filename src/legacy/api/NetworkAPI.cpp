#include "api/NetworkAPI.h"

#include "api/APIHelp.h"
#include "engine/EngineManager.h"
#include "engine/TimeTaskSystem.h"
#include "httplib.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/service/ServerInfo.h"
#include "ll/api/thread/ThreadPoolExecutor.h"
#include "ll/api/utils/ErrorUtils.h"
#include "main/SafeGuardRecord.h"

#include <string>
#include <vector>

using namespace cyanray;
using namespace ll::coro;

// Some script::Exception have a problem which can crash the server, and I have no idea, so not output message &
// stacktrace
#define CATCH_CALLBACK(LOG)                                                                                            \
    catch (const Exception& e) {                                                                                       \
        EngineScope enter(engine);                                                                                     \
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(LOG);                                       \
        ll::error_utils::printException(e, lse::LegacyScriptEngine::getInstance().getSelf().getLogger());              \
        return;                                                                                                        \
    }                                                                                                                  \
    catch (...) {                                                                                                      \
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(LOG);                                       \
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());          \
        EngineScope enter(engine);                                                                                     \
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__);                                                                      \
        return;                                                                                                        \
    }

#define CATCH_CALLBACK_IN_CORO(LOG)                                                                                    \
    catch (const Exception& e) {                                                                                       \
        EngineScope enterCoro(engine);                                                                                 \
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(LOG);                                       \
        ll::error_utils::printException(e, lse::LegacyScriptEngine::getInstance().getSelf().getLogger());              \
        co_return;                                                                                                     \
    }                                                                                                                  \
    catch (...) {                                                                                                      \
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(LOG);                                       \
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());          \
        EngineScope enterCoro(engine);                                                                                 \
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__);                                                                      \
        co_return;                                                                                                     \
    }
//////////////////// Classes ////////////////////

ClassDefine<void> NetworkClassBuilder = defineClass("network")
                                            .function("httpGet", &NetworkClass::httpGet)
                                            .function("httpPost", &NetworkClass::httpPost)
                                            .function("httpGetSync", &NetworkClass::httpGetSync)

                                            // For compatibility
                                            .function("newWebSocket", &NetworkClass::newWebSocket)
                                            .build();

ClassDefine<WSClientClass> WSClientClassBuilder =
    defineClass<WSClientClass>("WSClient")
        .constructor(&WSClientClass::constructor)
        .instanceProperty("status", &WSClientClass::getStatus)
        .instanceFunction("connect", &WSClientClass::connect)
        .instanceFunction("connectAsync", &WSClientClass::connectAsync)
        .instanceFunction("send", &WSClientClass::send)
        .instanceFunction("listen", &WSClientClass::listen)
        .instanceFunction("close", &WSClientClass::close)
        .instanceFunction("shutdown", &WSClientClass::shutdown)
        .instanceFunction("errorCode", &WSClientClass::errorCode)

        .property("Open", [] { return Number::newNumber((int)WebSocketClient::Status::Open); })
        .property("Closing", [] { return Number::newNumber((int)WebSocketClient::Status::Closing); })
        .property("Closed", [] { return Number::newNumber((int)WebSocketClient::Status::Closed); })
        .build();

ClassDefine<HttpServerClass> HttpServerClassBuilder =
    defineClass<HttpServerClass>("HttpServer")
        .constructor(&HttpServerClass::constructor)
        .instanceFunction("onGet", &HttpServerClass::onGet)
        .instanceFunction("onPut", &HttpServerClass::onPut)
        .instanceFunction("onPost", &HttpServerClass::onPost)
        .instanceFunction("onPatch", &HttpServerClass::onPatch)
        .instanceFunction("onDelete", &HttpServerClass::onDelete)
        .instanceFunction("onOptions", &HttpServerClass::onOptions)

        .instanceFunction("onPreRouting", &HttpServerClass::onPreRouting)
        .instanceFunction("onPostRouting", &HttpServerClass::onPostRouting)
        .instanceFunction("onError", &HttpServerClass::onError)
        .instanceFunction("onException", &HttpServerClass::onException)

        .instanceFunction("listen", &HttpServerClass::listen)
        .instanceFunction("startAt", &HttpServerClass::listen)
        .instanceFunction("stop", &HttpServerClass::stop)
        .instanceFunction("close", &HttpServerClass::stop)
        .instanceFunction("isRunning", &HttpServerClass::isRunning)
        .build();

ClassDefine<HttpRequestClass> HttpRequestClassBuilder =
    defineClass<HttpRequestClass>("HttpRequest")
        .constructor(nullptr)
        .instanceFunction("getHeader", &HttpRequestClass::getHeader)

        .instanceProperty("headers", &HttpRequestClass::getHeaders)
        .instanceProperty("method", &HttpRequestClass::getMethod)
        .instanceProperty("body", &HttpRequestClass::getBody)
        .instanceProperty("path", &HttpRequestClass::getPath)
        .instanceProperty("params", &HttpRequestClass::getParams)
        .instanceProperty("query", &HttpRequestClass::getParams)
        .instanceProperty("remoteAddr", &HttpRequestClass::getRemoteAddr)
        .instanceProperty("remotePort", &HttpRequestClass::getRemotePort)
        .instanceProperty("version", &HttpRequestClass::getVersion)
        .instanceProperty("matches", &HttpRequestClass::getRegexMatches)
        .build();

ClassDefine<HttpResponseClass> HttpResponseClassBuilder =
    defineClass<HttpResponseClass>("HttpResponse")
        .constructor(nullptr)
        .instanceFunction("getHeader", &HttpResponseClass::getHeader)
        .instanceFunction("setHeader", &HttpResponseClass::setHeader)
        .instanceFunction("write", &HttpResponseClass::write)

        .instanceProperty("headers", &HttpResponseClass::getHeaders, &HttpResponseClass::setHeaders)
        .instanceProperty("status", &HttpResponseClass::getStatus, &HttpResponseClass::setStatus)
        .instanceProperty("body", &HttpResponseClass::getBody, &HttpResponseClass::setBody)
        .instanceProperty("version", &HttpResponseClass::getVersion, &HttpResponseClass::setVersion)
        .instanceProperty("reason", &HttpResponseClass::getReason, &HttpResponseClass::setReason)
        .build();

// 生成函数
WSClientClass::WSClientClass(const Local<Object>& scriptObj)
: ScriptClass(scriptObj),
  ws(std::make_shared<WebSocketClient>()) {
    initListeners();
}

WSClientClass::WSClientClass()
: ScriptClass(ScriptClass::ConstructFromCpp<WSClientClass>{}),
  ws(std::make_shared<WebSocketClient>()) {
    initListeners();
}

void WSClientClass::initListeners() {
    ws->OnTextReceived([nowList{&listeners[int(WSClientEvents::onTextReceived)]}](WebSocketClient&, string msg) {
        if (!nowList->empty())
            for (auto& listener : *nowList) {
                if (!EngineManager::isValid(listener.engine)) return;
                EngineScope enter(listener.engine);
                // dangerous
                NewTimeout(listener.func.get(), {String::newString(msg)}, 1);
            }
    });

    ws->OnBinaryReceived(
        [nowList{&listeners[int(WSClientEvents::onBinaryReceived)]}](WebSocketClient&, vector<uint8_t> data) {
        if (!nowList->empty())
            for (auto& listener : *nowList) {
                if (!EngineManager::isValid(listener.engine)) return;
                EngineScope enter(listener.engine);
                NewTimeout(listener.func.get(), {ByteBuffer::newByteBuffer(data.data(), data.size())}, 1);
            }
        }
    );

    ws->OnError([nowList{&listeners[int(WSClientEvents::onError)]}](WebSocketClient&, string msg) {
        if (!nowList->empty())
            for (auto& listener : *nowList) {
                if (!EngineManager::isValid(listener.engine)) return;
                EngineScope enter(listener.engine);
                NewTimeout(listener.func.get(), {String::newString(msg)}, 1);
            }
    });

    ws->OnLostConnection([nowList{&listeners[int(WSClientEvents::onLostConnection)]}](WebSocketClient&, int code) {
        if (!nowList->empty())
            for (auto& listener : *nowList) {
                if (!EngineManager::isValid(listener.engine)) return;
                EngineScope enter(listener.engine);
                NewTimeout(listener.func.get(), {Number::newNumber(code)}, 1);
            }
    });
}

void WSClientClass::initListeners_s() {
    ws->OnTextReceived([nowList{&listeners[int(WSClientEvents::onTextReceived)]},
                        engine = EngineScope::currentEngine()](WebSocketClient&, std::string msg) {
        ll::coro::keepThis([nowList, engine, msg = std::move(msg)]() -> ll::coro::CoroTask<> {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                || engine->isDestroying())
                co_return;

            co_await ll::chrono::ticks(1);
            try {
                if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                    || engine->isDestroying())
                    co_return;

                EngineScope enter(engine);
                for (auto& listener : *nowList) {
                    listener.func.get().call({}, {String::newString(msg)});
                }
            }
            CATCH_CALLBACK_IN_CORO("Fail in OnTextReceived")
        }).launch(ll::thread::ThreadPoolExecutor::getDefault());
    });

    ws->OnBinaryReceived([nowList{&listeners[int(WSClientEvents::onBinaryReceived)]},
                          engine = EngineScope::currentEngine()](WebSocketClient&, std::vector<uint8_t> data) {
        ll::coro::keepThis([nowList, engine, data = std::move(data)]() mutable -> ll::coro::CoroTask<> {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                || engine->isDestroying())
                co_return;

            co_await ll::chrono::ticks(1);
            try {
                if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                    || engine->isDestroying())
                    co_return;

                EngineScope enter(engine);
                for (auto& listener : *nowList) {
                    listener.func.get().call({}, {ByteBuffer::newByteBuffer(data.data(), data.size())});
                }
            }
            CATCH_CALLBACK_IN_CORO("Fail in OnBinaryReceived")
        }).launch(ll::thread::ThreadPoolExecutor::getDefault());
    });

    ws->OnError([nowList{&listeners[int(WSClientEvents::onError)]},
                 engine = EngineScope::currentEngine()](WebSocketClient&, std::string msg) {
        ll::coro::keepThis([nowList, engine, msg = std::move(msg)]() -> ll::coro::CoroTask<> {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                || engine->isDestroying())
                co_return;

            co_await ll::chrono::ticks(1);
            try {
                if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                    || engine->isDestroying())
                    co_return;

                EngineScope enter(engine);
                for (auto& listener : *nowList) {
                    listener.func.get().call({}, {String::newString(msg)});
                }
            }
            CATCH_CALLBACK_IN_CORO("Fail in OnError")
        }).launch(ll::thread::ThreadPoolExecutor::getDefault());
    });

    ws->OnLostConnection([nowList{&listeners[int(WSClientEvents::onLostConnection)]},
                          engine = EngineScope::currentEngine()](WebSocketClient&, int code) {
        ll::coro::keepThis([nowList, engine, code]() -> ll::coro::CoroTask<> {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                || engine->isDestroying())
                co_return;

            co_await ll::chrono::ticks(1);
            try {
                if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                    || engine->isDestroying())
                    co_return;

                EngineScope enter(engine);
                for (auto& listener : *nowList) {
                    listener.func.get().call({}, {Number::newNumber(code)});
                }
            }
            CATCH_CALLBACK_IN_CORO("Fail in OnLostConnection")
        }).launch(ll::thread::ThreadPoolExecutor::getDefault());
    });
}

void WSClientClass::clearListeners() {
    ws->OnTextReceived([](WebSocketClient&, std::string) {});
    ws->OnBinaryReceived([](WebSocketClient&, std::vector<uint8_t>) {});
    ws->OnError([](WebSocketClient&, std::string) {});
    ws->OnLostConnection([](WebSocketClient&, int) {});
}

WSClientClass* WSClientClass::constructor(const Arguments& args) { return new WSClientClass(args.thiz()); }

// 成员函数
void WSClientClass::addListener(const string& event, Local<Function> func) {
    if (event == "onTextReceived")
        listeners[(int)WSClientEvents::onTextReceived].push_back(
            {EngineScope::currentEngine(), script::Global<Function>(func)}
        );
    else if (event == "onBinaryReceived")
        listeners[(int)WSClientEvents::onBinaryReceived].push_back(
            {EngineScope::currentEngine(), script::Global<Function>(func)}
        );
    else if (event == "onError")
        listeners[(int)WSClientEvents::onError].push_back({EngineScope::currentEngine(), script::Global<Function>(func)}
        );
    else if (event == "onLostConnection")
        listeners[(int)WSClientEvents::onLostConnection].push_back(
            {EngineScope::currentEngine(), script::Global<Function>(func)}
        );
    else {
        LOG_ERROR_WITH_SCRIPT_INFO(__FUNCTION__, "WSClient Event \"" + event + "\" No Found!\n");
    }
}

Local<Value> WSClientClass::getStatus() {
    try {
        return Number::newNumber((int)ws->GetStatus());
    } catch (const std::runtime_error&) {
        return Local<Value>();
    }
    CATCH("Fail in getStatus!");
}

Local<Value> WSClientClass::connect(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    // if (args.size() > 1 && args[1].isFunction())
    //     return connectAsync(args);
    try {

        string target = args[0].asString().toString();
        RecordOperation(getEngineOwnData()->pluginName, "ConnectToWebsocketServer", target);
        ws->Connect(target);
        return Boolean::newBoolean(true);
    } catch (const std::runtime_error&) {
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in connect!");
}

// 异步连接ws客户端
Local<Value> WSClientClass::connectAsync(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        string target = args[0].asString().toString();
        RecordOperation(getEngineOwnData()->pluginName, "ConnectToWebsocketServer", target);

        script::Global<Function> callbackFunc{args[1].asFunction()};
        std::thread([ws{this->ws},
                     target,
                     callback{std::move(callbackFunc)},
                     engine{EngineScope::currentEngine()},
                     pluginName{getEngineOwnData()->pluginName}]() mutable {
            try {
                bool result = false;
                try {
                    ws->Connect(target);
                    result = true;
                } catch (const std::runtime_error&) {
                    result = false;
                }
                if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                    || engine->isDestroying())
                    return;
                EngineScope enter(engine);
                // fix get on empty Global
                if (callback.isEmpty()) return;
                NewTimeout(callback.get(), {Boolean::newBoolean(result)}, 0);
            } catch (...) {
                lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                    "WSClientClass::connectAsync Failed!"
                );
                ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
                lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("In Plugin: " + pluginName);
            }
        }).detach();
        return Boolean::newBoolean(true);
    } catch (const std::runtime_error&) {
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in connectAsync!");
}

Local<Value> WSClientClass::send(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        if (args[0].isString()) ws->SendText(args[0].asString().toString());
        else if (args[0].isByteBuffer())
            ws->SendBinary((char*)args[0].asByteBuffer().getRawBytes(), args[0].asByteBuffer().byteLength());
        else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        return Boolean::newBoolean(true);
    } catch (const std::runtime_error&) {
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in send!");
}

Local<Value> WSClientClass::listen(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        addListener(args[0].asString().toString(), args[1].asFunction());
        return Boolean::newBoolean(true);
    } catch (const std::runtime_error&) {
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in listen!");
}

Local<Value> WSClientClass::close(const Arguments&) {
    try {
        ws->Close();
        return Boolean::newBoolean(true);
    } catch (const std::runtime_error&) {
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in close!");
}

Local<Value> WSClientClass::shutdown(const Arguments&) {
    try {
        ws->Shutdown();
        return Boolean::newBoolean(true);
    } catch (const std::runtime_error&) {
        return Boolean::newBoolean(false);
    }
    CATCH("Fail in shutdown!");
}

Local<Value> WSClientClass::errorCode(const Arguments&) {
    try {
        return Number::newNumber(WSAGetLastError());
    } catch (const std::runtime_error&) {
        return Local<Value>();
    }
    CATCH("Fail in errorCode!");
}

//////////////////// Class HttpServer ////////////////////

using namespace httplib;

// ll::thread::TickSyncTaskPool taskPool;

void ADD_CALLBACK(
    std::shared_ptr<httplib::Server>                svr,
    std::multimap<std::string, HttpServerCallback>& callbacks,
    HttpRequestType const&                          method,
    std::string const&                              path,
    Local<Function> const&                          func
) {
    callbacks.emplace(
        make_pair(path, HttpServerCallback{EngineScope::currentEngine(), script::Global<Function>{func}, method, path})
    );
    auto receiveMethod =
        [engine = EngineScope::currentEngine(), method, callbacks](const Request& req, Response& resp) {
            if ((ll::getGamingStatus() == ll::GamingStatus::Stopping) || !EngineManager::isValid(engine)
                || engine->isDestroying())
                return;
            ll::coro::keepThis([engine, req, &resp, method, callbacks]() -> ll::coro::CoroTask<> {
                if ((ll::getGamingStatus() == ll::GamingStatus::Stopping) || !EngineManager::isValid(engine)
                    || engine->isDestroying())
                    co_return;

                EngineScope enter(engine);
                try {
                    for (auto& [k, v] : callbacks) {
                        if (v.type != method) continue;
                        std::regex  rgx(k);
                        std::smatch matches;
                        if (std::regex_match(req.path, matches, rgx)) {
                            if (matches == req.matches) {
                                auto reqObj  = new HttpRequestClass(req);
                                auto respObj = new HttpResponseClass(resp);
                                v.func.get().call({}, reqObj, respObj);
                                resp = *respObj->get();
                                break;
                            }
                        }
                    }
                }
                CATCH_CALLBACK_IN_CORO("Fail in NetworkAPI callback")
            }).syncLaunch(ll::thread::ThreadPoolExecutor::getDefault());
        };
    switch (method) {
    case HttpRequestType::Get:
        svr->Get(path.c_str(), receiveMethod);
        break;
    case HttpRequestType::Post:
        svr->Post(path.c_str(), receiveMethod);
        break;
    case HttpRequestType::Put:
        svr->Put(path.c_str(), receiveMethod);
        break;
    case HttpRequestType::Delete:
        svr->Delete(path.c_str(), receiveMethod);
        break;
    case HttpRequestType::Options:
        svr->Options(path.c_str(), receiveMethod);
        break;
    case HttpRequestType::Patch:
        svr->Patch(path.c_str(), receiveMethod);
        break;
    default:
        break;
    }
}

HttpServerClass::HttpServerClass(const Local<Object>& scriptObj) : ScriptClass(scriptObj), svr(new Server) {}
HttpServerClass::HttpServerClass() : ScriptClass(ScriptClass::ConstructFromCpp<HttpServerClass>{}), svr(new Server) {}

HttpServerClass::~HttpServerClass() { svr->stop(); }

HttpServerClass* HttpServerClass::constructor(const Arguments& args) { return new HttpServerClass(args.thiz()); }

Local<Value> HttpServerClass::onGet(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        auto path = args[0].asString().toString();
        auto func = args[1].asFunction();
        ADD_CALLBACK(svr, callbacks, HttpRequestType::Get, path, func);
        return this->getScriptObject();
    }
    CATCH("Fail in onGet")
}

Local<Value> HttpServerClass::onPut(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        auto path = args[0].asString().toString();
        auto func = args[1].asFunction();
        ADD_CALLBACK(svr, callbacks, HttpRequestType::Put, path, func);
        return this->getScriptObject();
    }
    CATCH("Fail in onPut!");
}

Local<Value> HttpServerClass::onPost(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        auto path = args[0].asString().toString();
        auto func = args[1].asFunction();
        ADD_CALLBACK(svr, callbacks, HttpRequestType::Post, path, func);
        return this->getScriptObject();
    }
    CATCH("Fail in onPost!");
}

Local<Value> HttpServerClass::onPatch(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        auto path = args[0].asString().toString();
        auto func = args[1].asFunction();
        ADD_CALLBACK(svr, callbacks, HttpRequestType::Patch, path, func);
        return this->getScriptObject();
    }
    CATCH("Fail in onPatch!");
}

Local<Value> HttpServerClass::onDelete(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        auto path = args[0].asString().toString();
        auto func = args[1].asFunction();
        ADD_CALLBACK(svr, callbacks, HttpRequestType::Delete, path, func);
        return this->getScriptObject();
    }
    CATCH("Fail in onDelete!");
}

Local<Value> HttpServerClass::onOptions(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);

    try {
        auto path = args[0].asString().toString();
        auto func = args[1].asFunction();
        ADD_CALLBACK(svr, callbacks, HttpRequestType::Options, path, func);
        return this->getScriptObject();
    }
    CATCH("Fail in onOptions!");
}

Local<Value> HttpServerClass::onPreRouting(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kFunction);

    try {
        preRoutingCallback = {EngineScope::currentEngine(), script::Global{args[0].asFunction()}};
        svr->set_pre_routing_handler([this, engine = EngineScope::currentEngine()](const Request& req, Response& resp) {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                || engine->isDestroying())
                return Server::HandlerResponse::Unhandled;
            bool handled = false;
            ll::coro::keepThis([this, engine, req, &resp, &handled]() -> ll::coro::CoroTask<> {
                if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                    || engine->isDestroying())
                    co_return;

                try {
                    EngineScope enter(engine);
                    auto        reqObj  = new HttpRequestClass(req);
                    auto        respObj = new HttpResponseClass(resp);

                    auto res = this->preRoutingCallback.func.get().call({}, reqObj, respObj);
                    if (res.isBoolean() && res.asBoolean().value() == false) {
                        handled = true;
                    }
                    resp = *respObj->get();
                }
                CATCH_CALLBACK_IN_CORO("Fail in onPreRouting");
            }).syncLaunch(ll::thread::ThreadPoolExecutor::getDefault());

            return handled ? Server::HandlerResponse::Handled : Server::HandlerResponse::Unhandled;
        });
        return this->getScriptObject();
    }
    CATCH("Fail in onPreRouting!");
}

Local<Value> HttpServerClass::onPostRouting(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kFunction);

    try {
        postRoutingCallback = {EngineScope::currentEngine(), script::Global{args[0].asFunction()}};
        svr->set_post_routing_handler([this,
                                       engine = EngineScope::currentEngine()](const Request& req, Response& resp) {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                || engine->isDestroying())
                return;

            ll::coro::keepThis([this, engine, req, &resp]() -> ll::coro::CoroTask<> {
                if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                    || engine->isDestroying())
                    co_return;

                EngineScope enter(engine);
                try {
                    auto reqObj  = new HttpRequestClass(req);
                    auto respObj = new HttpResponseClass(resp);
                    this->postRoutingCallback.func.get().call({}, reqObj, respObj);
                    resp = *respObj->get();
                }
                CATCH_CALLBACK_IN_CORO("Fail in onPostRouting");
            }).syncLaunch(ll::thread::ThreadPoolExecutor::getDefault());
        });
        return this->getScriptObject();
    }
    CATCH("Fail in onPostRouting!");
}

Local<Value> HttpServerClass::onError(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kFunction);

    try {
        errorCallback = {EngineScope::currentEngine(), script::Global{args[0].asFunction()}};
        svr->set_error_handler([this, engine = EngineScope::currentEngine()](const Request& req, Response& resp) {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                || engine->isDestroying())
                return;

            ll::coro::keepThis([this, engine, req, &resp]() -> ll::coro::CoroTask<> {
                if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                    || engine->isDestroying())
                    co_return;

                EngineScope enter(engine);
                try {
                    auto reqObj  = new HttpRequestClass(req);
                    auto respObj = new HttpResponseClass(resp);
                    this->errorCallback.func.get().call({}, reqObj, respObj);
                    resp = *respObj->get();
                }
                CATCH_CALLBACK_IN_CORO("Fail in onError");
            }).syncLaunch(ll::thread::ThreadPoolExecutor::getDefault());
        });
        return this->getScriptObject();
    }
    CATCH("Fail in onError!");
}

Local<Value> HttpServerClass::onException(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kFunction);

    try {
        exceptionCallback = {EngineScope::currentEngine(), script::Global{args[0].asFunction()}};
        svr->set_exception_handler([this,
                                    engine = EngineScope::currentEngine(
                                    )](const Request& req, Response& resp, std::exception_ptr e) {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                || engine->isDestroying())
                return;

            ll::coro::keepThis([this, engine, req, &resp, e]() -> ll::coro::CoroTask<> {
                if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                    || engine->isDestroying())
                    co_return;

                EngineScope enter(engine);
                try {
                    auto reqObj  = new HttpRequestClass(req);
                    auto respObj = new HttpResponseClass(resp);
                    if (e) {
                        try {
                            std::rethrow_exception(e);
                        } catch (const std::exception& exp) {
                            this->exceptionCallback.func.get().call({}, reqObj, respObj, String::newString(exp.what()));
                        }
                    }
                    resp = *respObj->get();
                }
                CATCH_CALLBACK_IN_CORO("Fail in onException");
            }).syncLaunch(ll::thread::ThreadPoolExecutor::getDefault());
        });
        return this->getScriptObject();
    }
    CATCH("Fail in onException!");
}

Local<Value> HttpServerClass::listen(const Arguments& args) {
    if (args.size() == 1) {
        CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    } else if (args.size() == 2) {
        CHECK_ARG_TYPE(args[0], ValueKind::kString);
        CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
    } else {
        LOG_WRONG_ARG_TYPE(__FUNCTION__);
        return Local<Value>();
    }

    try {
        std::string addr = "127.0.0.1";
        int         port = 80;
        if (args.size() == 2) {
            addr = args[0].asString().toString();
            port = args[1].asNumber().toInt32();
        } else {
            port = args[0].asNumber().toInt32();
        }
        if (port < 0 || port > 65535) {
            throw script::Exception("Invalid port number! (0~65535)");
        }
        RecordOperation(getEngineOwnData()->pluginName, "StartHttpServer", fmt::format("on {}:{}", addr, port));
        std::thread([this, addr, port]() {
            try {
                svr->stop();
                svr->listen(addr, port);
            } catch (...) {
                ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
            }
        }).detach();
        return this->getScriptObject(); // return self
    }
    CATCH("Fail in listen!");
}

Local<Value> HttpServerClass::stop(const Arguments& args) {
    try {
        RecordOperation(getEngineOwnData()->pluginName, "StopHttpServer", "");
        svr->stop();
        return Local<Value>();
    }
    CATCH("Fail in stop!");
}

Local<Value> HttpServerClass::isRunning(const Arguments& args) {
    try {
        return Boolean::newBoolean(svr->is_running());
    }
    CATCH("Fail in isRunning!");
}

Local<Object> Headers2Object(const Headers& headers) {
    auto obj = Object::newObject();
    for (auto& header : headers) {
        obj.set(header.first, Array::newArray());
    }
    for (auto& header : headers) {
        auto arr = obj.get(header.first).asArray();
        arr.add(String::newString(header.second));
        obj.set(header.first, arr);
    }
    return obj;
}

Local<Object> Params2Object(const Params& params) {
    auto obj = Object::newObject();
    for (auto& param : params) {
        if (params.count(param.first) == 1) obj.set(param.first, param.second);
        else {
            if (!obj.has(param.first)) {
                obj.set(param.first, Array::newArray());
            }
            auto arr = obj.get(param.first).asArray();
            arr.add(String::newString(param.second));
            obj.set(param.first, arr);
        }
    }
    return obj;
}

HttpRequestClass::HttpRequestClass(const Local<Object>& scriptObj, const Request& req)
: ScriptClass(scriptObj),
  req(new Request(req)) {}
HttpRequestClass::HttpRequestClass(const Request& req)
: ScriptClass(ScriptClass::ConstructFromCpp<HttpRequestClass>{}),
  req(new Request(req)) {}

std::shared_ptr<Request> HttpRequestClass::get() { return req; }

Local<Value> HttpRequestClass::getHeaders() {
    try {
        return Headers2Object(req->headers);
    }
    CATCH("Fail in getHeaders!");
}

Local<Value> HttpRequestClass::getHeader(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto key = args[0].asString().toString();
        if (req->headers.count(key) == 0) return Array::newArray();
        auto value = req->headers.equal_range(key);
        auto arr   = Array::newArray();
        for (auto it = value.first; it != value.second; ++it) {
            arr.add(String::newString(it->second));
        }
        return arr;
    }
    CATCH("Fail in getHeader!");
}

Local<Value> HttpRequestClass::getBody() {
    try {
        return String::newString(req->body);
    }
    CATCH("Fail in getBody!");
}

Local<Value> HttpRequestClass::getMethod() {
    try {
        return String::newString(req->method);
    }
    CATCH("Fail in getMethod!");
}

Local<Value> HttpRequestClass::getPath() {
    try {
        return String::newString(req->path);
    }
    CATCH("Fail in getPath!");
}

Local<Value> HttpRequestClass::getParams() {
    try {
        return Params2Object(req->params);
    }
    CATCH("Fail in getParams!");
}

Local<Value> HttpRequestClass::getRemoteAddr() {
    try {
        return String::newString(req->remote_addr);
    }
    CATCH("Fail in getRemoteAddr!");
}

Local<Value> HttpRequestClass::getRemotePort() {
    try {
        return Number::newNumber(req->remote_port);
    }
    CATCH("Fail in getRemotePort!");
}

Local<Value> HttpRequestClass::getVersion() {
    try {
        return String::newString(req->version);
    }
    CATCH("Fail in getVersion!");
}

Local<Value> HttpRequestClass::getRegexMatches() {
    try {
        auto smt = req->matches;
        auto arr = Array::newArray();
        for (auto& match : smt) {
            arr.add(String::newString(match));
        }
        return arr;
    }
    CATCH("Fail in getRegexMatches!");
}

HttpResponseClass::HttpResponseClass(const Local<Object>& scriptObj, const Response& resp)
: ScriptClass(scriptObj),
  resp(new Response(resp)) {}
HttpResponseClass::HttpResponseClass(const Response& resp)
: ScriptClass(ScriptClass::ConstructFromCpp<HttpResponseClass>{}),
  resp(new Response(resp)) {}

std::shared_ptr<Response> HttpResponseClass::get() { return resp; }

Local<Value> HttpResponseClass::setHeader(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto   key = args[0].asString().toString();
        string value;
        if (args[1].isString()) value = args[1].asString().toString();
        else value = ValueToString(args[1]);
        resp->headers.insert(make_pair(key, value));
        return this->getScriptObject(); // return self
    }
    CATCH("Fail in setHeader!");
}

Local<Value> HttpResponseClass::getHeader(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto key = args[0].asString().toString();
        if (resp->headers.count(key) == 0) return Array::newArray();
        auto value = resp->headers.equal_range(key);
        auto arr   = Array::newArray();
        for (auto it = value.first; it != value.second; ++it) {
            arr.add(String::newString(it->second));
        }
        return arr;
    }
    CATCH("Fail in getHeader!");
}

Local<Value> HttpResponseClass::write(const Arguments& args) {
    try {
        for (size_t i = 0ULL, mEnd = args.size(); i < mEnd; ++i) {
            resp->body += ValueToString(args[i]);
        }
        return this->getScriptObject();
    }
    CATCH("Fail in write!");
}

void HttpResponseClass::setHeaders(const Local<Value>& headers) {
    CHECK_ARG_TYPE_S(headers, ValueKind::kObject);

    try {
        auto keys = headers.asObject().getKeys();
        for (auto& key : keys) {
            auto key_str = key.toString();
            auto value   = headers.asObject().get(key);
            if (value.isArray()) {
                for (size_t i = 0ULL; i < value.asArray().size(); ++i) {
                    resp->headers.insert(make_pair(key_str, ValueToString(value.asArray().get(i))));
                }
                return;
            }
            auto val_str = ValueToString(value);
            resp->headers.insert(make_pair(key_str, val_str));
        }
    }
    CATCH_S("Fail in setHeaders!");
}

void HttpResponseClass::setStatus(const Local<Value>& status) {
    CHECK_ARG_TYPE_S(status, ValueKind::kNumber);

    try {
        resp->status = status.asNumber().toInt32();
    }
    CATCH_S("Fail in setStatus!");
}

void HttpResponseClass::setBody(const Local<Value>& body) {
    CHECK_ARG_TYPE_S(body, ValueKind::kString);

    try {
        resp->body = body.asString().toString();
    }
    CATCH_S("Fail in setBody!");
}

void HttpResponseClass::setReason(const Local<Value>& reason) {
    CHECK_ARG_TYPE_S(reason, ValueKind::kString);

    try {
        resp->reason = reason.asString().toString();
    }
    CATCH_S("Fail in setReason!");
}

void HttpResponseClass::setVersion(const Local<Value>& version) {
    CHECK_ARG_TYPE_S(version, ValueKind::kString);

    try {
        resp->version = version.asString().toString();
    }
    CATCH_S("Fail in setVersion!");
}

Local<Value> HttpResponseClass::getHeaders() {
    try {
        return Headers2Object(resp->headers);
    }
    CATCH("Fail in getHeaders!");
}

Local<Value> HttpResponseClass::getStatus() {
    try {
        return Number::newNumber(resp->status);
    }
    CATCH("Fail in getStatus!");
}

Local<Value> HttpResponseClass::getBody() {
    try {
        return String::newString(resp->body);
    }
    CATCH("Fail in getBody!");
}

Local<Value> HttpResponseClass::getReason() {
    try {
        return String::newString(resp->reason);
    }
    CATCH("Fail in getReason!");
}

Local<Value> HttpResponseClass::getVersion() {
    try {
        return String::newString(resp->version);
    }
    CATCH("Fail in getVersion!");
}

//////////////////// Ported from LiteLoaderBDS ////////////////////

void SplitHttpUrl(const std::string& url, string& host, string& path) {
    host = url;

    bool foundProtocol = host.find('/') != string::npos;

    auto splitPos = host.find('/', foundProtocol ? host.find('/') + 2 : 0); // 查找协议后的第一个/分割host与路径
    if (splitPos == string::npos) {
        path = "/";
    } else {
        path = host.substr(splitPos);
        host = host.substr(0, splitPos);
    }
}
bool HttpGet(
    const std::string&                           url,
    const httplib::Headers&                      headers,
    const std::function<void(int, std::string)>& callback,
    int                                          timeout = -1
) {
    string host, path;
    SplitHttpUrl(url, host, path);

    auto* cli = new httplib::Client(host.c_str());
    if (!cli->is_valid()) {
        delete cli;
        return false;
    }
    if (timeout > 0) cli->set_connection_timeout(timeout, 0);

    std::thread([cli, headers, callback, path{std::move(path)}]() {
        try {
            auto response = cli->Get(path.c_str(), headers);
            delete cli;

            if (!response) callback(-1, "");
            else callback(response->status, response->body);
        } catch (...) {}
    }).detach();

    return true;
}

bool HttpGet(const std::string& url, const std::function<void(int, std::string)>& callback, int timeout = -1) {
    return HttpGet(url, {}, callback, timeout);
}

bool HttpPost(
    const std::string&                           url,
    const httplib::Headers&                      headers,
    const std::string&                           data,
    const std::string&                           type,
    const std::function<void(int, std::string)>& callback,
    int                                          timeout = -1
) {
    std::string host, path;
    SplitHttpUrl(url, host, path);
    auto* cli = new httplib::Client(host.c_str());
    if (!cli->is_valid()) {
        delete cli;
        return false;
    }
    if (timeout > 0) cli->set_connection_timeout(timeout, 0);

    std::thread([cli, headers, data, type, callback, path{std::move(path)}]() {
        try {
            auto response = cli->Post(path.c_str(), headers, data, type.c_str());
            delete cli;
            if (!response) callback(-1, "");
            else callback(response->status, response->body);
        } catch (...) {}
    }).detach();
    return true;
}

bool HttpPost(
    const string&                           url,
    const string&                           data,
    const string&                           type,
    const std::function<void(int, string)>& callback,
    int                                     timeout = -1
) {
    return HttpPost(url, {}, data, type, callback, timeout);
}

bool HttpGetSync(const std::string& url, int* statusRtn, std::string* dataRtn, int timeout = -1) {
    string host, path;
    SplitHttpUrl(url, host, path);

    httplib::Client cli(host.c_str());
    if (!cli.is_valid()) {
        return false;
    }
    if (timeout > 0) cli.set_connection_timeout(timeout, 0);

    auto response = cli.Get(path.c_str());

    if (!response) return false;
    else {
        if (statusRtn) *statusRtn = response->status;
        if (dataRtn) *dataRtn = response->body;
    }
    return true;
}

//////////////////// APIs ////////////////////

Local<Value> NetworkClass::httpGet(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    if (args.size() > 2) {
        CHECK_ARG_TYPE(args[1], ValueKind::kObject);
        CHECK_ARG_TYPE(args[2], ValueKind::kFunction);
    } else {
        CHECK_ARG_TYPE(args[1], ValueKind::kFunction);
    }

    try {
        string target = args[0].asString().toString();
        RecordOperation(getEngineOwnData()->pluginName, "HttpGet", target);
        script::Global<Function> callbackFunc{args[args.size() - 1].asFunction()};

        auto lambda = [callback{std::move(callbackFunc)},
                       engine{EngineScope::currentEngine()}](int status, string body) {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                || engine->isDestroying())
                return;

            EngineScope scope(engine);
            try {
                NewTimeout(callback.get(), {Number::newNumber(status), String::newString(body)}, 1);
                // callback.get().call({}, {Number::newNumber(status),
                // String::newString(body)});
            }
            CATCH_IN_CALLBACK("HttpGet")
        };
        if (args.size() > 2) {
            httplib::Headers maps;
            auto             obj  = args[1].asObject();
            auto             keys = obj.getKeyNames();
            if (keys.size() > 0) {
                for (size_t i = 0ULL, mEnd = keys.size(); i < mEnd; ++i) {
                    maps.insert({keys[i], obj.get(keys[i]).asString().toString()});
                }
            }
            return Boolean::newBoolean(HttpGet(target, maps, lambda));
        }
        return Boolean::newBoolean(HttpGet(target, lambda));
    }
    CATCH("Fail in HttpGet");
}

Local<Value> NetworkClass::httpPost(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 4);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[2], ValueKind::kString);
    if (args.size() > 4) {
        CHECK_ARG_TYPE(args[1], ValueKind::kObject);
        CHECK_ARG_TYPE(args[3], ValueKind::kString);
        CHECK_ARG_TYPE(args[4], ValueKind::kFunction);
    } else {
        CHECK_ARG_TYPE(args[1], ValueKind::kString);
        CHECK_ARG_TYPE(args[3], ValueKind::kFunction);
    }

    try {
        string target = args[0].asString().toString();
        RecordOperation(getEngineOwnData()->pluginName, "HttpPost", target);
        script::Global<Function> callbackFunc{args[args.size() - 1].asFunction()};

        auto lambda = [callback{std::move(callbackFunc)},
                       engine{EngineScope::currentEngine()}](int status, string body) {
            if ((ll::getGamingStatus() != ll::GamingStatus::Running) || !EngineManager::isValid(engine)
                || engine->isDestroying())
                return;

            EngineScope scope(engine);
            try {
                NewTimeout(callback.get(), {Number::newNumber(status), String::newString(body)}, 1);
                // callback.get().call({}, {Number::newNumber(status),
                // String::newString(body)});
            }
            CATCH_IN_CALLBACK("HttpPost")
        };
        if (args.size() > 4) {
            httplib::Headers maps;
            auto             obj  = args[1].asObject();
            auto             keys = obj.getKeyNames();
            if (keys.size() > 0) {
                for (size_t i = 0ULL, mEnd = keys.size(); i < mEnd; ++i) {
                    maps.insert({keys[i], obj.get(keys[i]).asString().toString()});
                }
            }

            return Boolean::newBoolean(
                HttpPost(target, maps, args[2].asString().toString(), args[3].asString().toString(), lambda)
            );
        }
        return Boolean::newBoolean(
            HttpPost(target, args[1].asString().toString(), args[2].asString().toString(), lambda)
        );
    }
    CATCH("Fail in HttpPost");
}

Local<Value> NetworkClass::httpGetSync(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        string target = args[0].asString().toString();
        RecordOperation(getEngineOwnData()->pluginName, "HttpGetSync", target);

        int    status;
        string result;
        HttpGetSync(target, &status, &result);
        Local<Object> res = Object::newObject();
        res.set("status", status);
        res.set("data", result);
        return res;
    }
    CATCH("Fail in HttpGetSync");
}

// For compatibility
Local<Value> NetworkClass::newWebSocket(const Arguments&) {
    auto newp = new WSClientClass();
    return newp->getScriptObject();
}

Local<Object> WSClientClass::newWSClient() {
    auto newp = new WSClientClass();
    return newp->getScriptObject();
}
