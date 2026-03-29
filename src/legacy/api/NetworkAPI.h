#pragma once
#include "WebSocketClient.h"
#include "api/APIHelp.h"
#include "httplib.h"

#include <list>

using namespace cyanray;

//////////////////// Types ////////////////////

enum class WSClientEvents : char { onTextReceived = 0, onBinaryReceived, onError, onLostConnection, EVENT_COUNT };

struct ListenerListType {
    ScriptEngine*            engine;
    script::Global<Function> func;
};

enum class HttpRequestType : char { Get = 0, Post, Put, Delete, Options, Patch, Head };

struct HttpServerCallback {
    ScriptEngine*            engine;
    script::Global<Function> func;
    HttpRequestType          type;
    std::string              path;
};

//////////////////// Network Static ////////////////////

class NetworkClass {
public:
    static Local<Value> httpGet(Arguments const& args);
    static Local<Value> httpPost(Arguments const& args);
    static Local<Value> httpGetSync(Arguments const& args);

    // For Compatibility
    static Local<Value> newWebSocket(Arguments const& args);
};
extern ClassDefine<void> NetworkClassBuilder;

//////////////////// Classes ////////////////////

class WSClientClass : public ScriptClass {
private:
    std::shared_ptr<WebSocketClient> ws = nullptr;
    std::list<ListenerListType>      listeners[static_cast<int>(WSClientEvents::EVENT_COUNT)];
    void                             addListener(string const& event, Local<Function> const& func);

public:
    explicit WSClientClass(Local<Object> const& scriptObj);
    explicit WSClientClass();
    void initListeners();
    void initListeners_s();
    void clearListeners() const;
    ~WSClientClass() override { ws->Shutdown(); }
    static WSClientClass* constructor(Arguments const& args);

    Local<Value> getStatus() const;

    Local<Value> connect(Arguments const& args) const;
    Local<Value> connectAsync(Arguments const& args);
    Local<Value> send(Arguments const& args) const;
    Local<Value> listen(Arguments const& args);
    Local<Value> close(Arguments const& args) const;
    Local<Value> shutdown(Arguments const& args) const;
    Local<Value> errorCode(Arguments const& args);

    // For Compatibility
    static Local<Object> newWSClient();
};
extern ClassDefine<WSClientClass> WSClientClassBuilder;

class HttpServerClass : public ScriptClass {

protected:
    std::shared_ptr<httplib::Server>               svr = nullptr;
    std::multimap<std::string, HttpServerCallback> callbacks;
    HttpServerCallback errorCallback, exceptionCallback, preRoutingCallback, postRoutingCallback;

public:
    HttpServerClass(Local<Object> const& scriptObj);
    HttpServerClass();
    ~HttpServerClass() override;
    static HttpServerClass* constructor(Arguments const& args);

    Local<Value> onGet(Arguments const& args);
    Local<Value> onPut(Arguments const& args);
    Local<Value> onPost(Arguments const& args);
    Local<Value> onPatch(Arguments const& args);
    Local<Value> onDelete(Arguments const& args);
    Local<Value> onOptions(Arguments const& args);

    Local<Value> onPreRouting(Arguments const& args);
    Local<Value> onPostRouting(Arguments const& args);
    Local<Value> onError(Arguments const& args);
    Local<Value> onException(Arguments const& args);

    Local<Value> listen(Arguments const& args) const;
    Local<Value> stop(Arguments const& args) const;
    Local<Value> isRunning(Arguments const& args) const;
};
extern ClassDefine<HttpServerClass> HttpServerClassBuilder;

class HttpRequestClass : public ScriptClass {
    std::shared_ptr<httplib::Request> req = nullptr;

public:
    HttpRequestClass(Local<Object> const& scriptObj, httplib::Request const& req = {});
    HttpRequestClass(httplib::Request const& req = {});
    // static HttpRequestClass* constructor(const Arguments& args);
    std::shared_ptr<httplib::Request> get();

    Local<Value> getHeaders() const;
    Local<Value> getHeader(Arguments const& args) const;
    Local<Value> getBody() const;
    Local<Value> getMethod() const;
    Local<Value> getPath() const;
    Local<Value> getParams() const;
    Local<Value> getRemoteAddr() const;
    Local<Value> getRemotePort() const;
    Local<Value> getVersion() const;
    Local<Value> getRegexMatches() const;
    // Local<Value> getMultiFormData();
};
extern ClassDefine<HttpRequestClass> HttpRequestClassBuilder;

class HttpResponseClass : public ScriptClass {
    std::shared_ptr<httplib::Response> resp = nullptr;

public:
    HttpResponseClass(Local<Object> const& scriptObj, httplib::Response const& resp = {});
    HttpResponseClass(httplib::Response const& resp);
    // static HttpResponseClass* constructor(const Arguments& args);
    std::shared_ptr<httplib::Response> get();

    Local<Value> setHeader(Arguments const& args) const;
    Local<Value> getHeader(Arguments const& args) const;
    Local<Value> write(Arguments const& args) const;

    void setHeaders(Local<Value> const& headers) const;
    void setStatus(Local<Value> const& status) const;
    void setBody(Local<Value> const& body) const;
    void setReason(Local<Value> const& reason) const;
    void setVersion(Local<Value> const& version) const;

    Local<Value> getHeaders() const;
    Local<Value> getStatus() const;
    Local<Value> getBody() const;
    Local<Value> getReason() const;
    Local<Value> getVersion() const;
};
extern ClassDefine<HttpResponseClass> HttpResponseClassBuilder;
