# 🌏 Web Interface API

The following APIs provide the basic network interface for scripts.
If there are more complex needs, the network library of the respective language platform can be used to complete the task.

## Contents
- 🔌 [WebSocket Client Object API](#-websocket-client-object-api)
- 📡 [Http Server Object API](#-http-server-object-api)
  - [Http Request Object API](#http-request-object-api)
  - [Http Response Object API](#http-response-object-api)

### Send an Asynchronous HTTP(s) Get Request  

`network.httpGet(url[,header],callback)`

- Parameters:
  - url : `String`  
    The target address of the request (including the parameters attached to the Get request).
  - header : `Object`  
    (Optional) Request headers.
  - callback : `Function`  
    The callback function to execute when the request returns, to return the HTTP(s) response result.
- Return value: Whether the request was successfully sent.
- Return value type: `Boolean`

Note: The prototype of the callback function of the parameter callback: `function(status,result)`  

- status : `Integer`    
  The returned HTTP(s) response code, such as 200 means the request was successful.
- result : `String`  
  The returned data.

If the request fails, the status value will be `-1`. 



### Send an Asynchronous HTTP(s) Post Request  

`network.httpPost(url[,header],data,type,callback)`

- Parameters:
  - url : `String`  
    The destination address of the request.
  - header : `Object`  
    (Optional) Request headers.
  - data : `String`  
    The data being sent.
  - type : `String`  
    The Post data type sent, in the form of `text/plain` `application/x-www-form-urlencoded`.
  - callback : `Function`  
    The callback function to execute when the request returns, to return the HTTP(s) response result.
- Return value: Whether the request was successfully sent.
- Return value type: `Boolean`

Note: The prototype of the callback function of the parameter callback: `function(status,result)`  

- status : `Integer`    
  The returned HTTP(s) response code, such as 200 means the request was successful.
- result : `String`  
  The returned data.

If the request fails, the status value will be `-1`.



## 🔌 WebSocket Client Object API

In LSE, use "WebSocket objects" to manipulate the connection and work of a WebSocket client.

### Create a New WebSocket Client Object 

[JavaScript] `new WSClient()`  
[Lua] `WSClient()`

- Return value: A new WebSocket client object.
- Return value type: `WSClient`



### WebSocket Client Object - Properties

Every WS client object contains some fixed object properties. for a specific client object `wsc`, has the following properties:

| Attributes | Meaning                   | Data Type |
| ---------- | ------------------------- | --------- |
| wsc.status | Current Connection Status | `Enum`    |

These object properties are read-only and cannot be modified.

Among them, the wsc.status enumeration has the following situations:

`wsc.Open` - In normal connection.  
`wsc.Closing` - Disconnecting.  
`wsc.Closed` - Not connected.



### WebSocket Client Object - Function

Every WS client object contains some member functions (member methods) that can be executed. for a specific client object `wsc`, you can perform some operations on this client through the following functions.

#### Create a Connection

`wsc.connect(target)`

- Parameter: 
  - target : `String`  
    The destination address to connect to, in the form of `ws://hostname[:port][/path/path][?query=value]`
- Return value: Whether the connection is successful.
- Return value type: `Boolean` 



#### Create a Connection Asynchronously

`wsc.connectAsync(target,callback)`

- Parameters: 
  - target : `String`  
    The destination address to connect to, in the form of `ws://hostname[:port][/path/path][?query=value]`
  - callback : `Function`
    A callback function to execute when the connection succeeds or fails.
- Return value: Whether the connection attempt was started successfully or not.
- Return value type: `Boolean` 

Note: The prototype of the callback function of the parameter callback: `function(success)`  

- success : `Boolean`    
  Whether the WebSocket connection is successful 



#### Send Text/Binary Messages

`wsc.send(msg)`

- Parameter: 
  - msg : `String` / `ByteBuffer`  
    Text/Binary data to send.
- Return value: Whether it was sent successfully.
- Return value type: `Boolean` 

If the parameter type passed in is `String`, will be sent as text, if it is `ByteBuffer` will be sent as binary data.



#### Listen for WebSocket Events 

In the process of WS working, when a message is received or an error occurs, the relevant information needs to be processed. Here is the interface for listening to events.

`wsc.listen(event,callback)`

- Parameters: 

  - event : `String`  
    The name of the event to listen for (see the list of listening events below).

  - callback : `Function`  
    Registered listener function (see below for function-related parameters)
    When the specified event occurs, LSE will call the listener function you gave and pass in the corresponding parameters.
- Return value: Whether the event was successfully monitored.
- Return value type: `Boolean` 



#### List of Listening Events

**`"onTextReceived"` - Listen for string messages.**

- Listener function prototype
  `function(msg)`
- Parameter: 
  - msg : `String`  
    Received string message.

**`"onBinaryReceived"` - Listen for binary messages.**

- Listener function prototype 
  `function(data)`
- Parameter: 
  - data : `ByteBuffer`  
    Received binary message.

**`"onError"` - Listen for errors.**

- Listener function prototype 
  `function(msg)`
- Parameter: 
  - msg : `String`  
    Error message.

**`"onLostConnection"` - Listen for lost connections.**

- Listener function prototype 
  `function(code)`
- Parameter: 
  - code : `Integer`  
    Error code.



#### Close the Connection

`wsc.close()`

- Return value: Whether the connection was successfully closed.
- Return value type: `Boolean` 

Do not continue to use this client object while it is closed!



#### Force Disconnect

`wsc.shutdown()`

- Return value: Whether the connection was successfully disconnected.
- Return value type: `Boolean` 

Do not continue to use this client object while it is disconnected!



#### Get Error Code

`wsc.errorCode()`

- Return value: The error code generated by the last error.
- Return value type: `Integer`

If you encounter a failure in the use of the above interface, you can get the last error code from here.

## 📡 Http Server Object API

LSE provides a simple Http server, which can be used for web services with **a small amount of data transfer**, such as webhooks.  
If you want to build a website, it is better to use well-established web server software such as Apache or Nginx.

### Create a New Server Object

[JavaScript]  `new HttpServer()`  
[Lua] `HttpServer()`

- Return value: A new server object.
- Return value type: `HttpServer`



### Listen for Get Requests

`svr.onGet(path, callback)`

- Parameters:
  - path : `String`  
    The request path. It starts with `/` and can contain regular expressions, such as `/test/(.+)`
  - callback : `Function<(HttpRequest, HttpResponse)>`  
    Callback function, called when a GET request matching path is received.
- Return value: The processed server object (for chaining other operations).
- Return value type: `HttpServer`



### Listen for Put Requests

`svr.onPut(path, callback)`

- Parameters:
  - path : `String`  
    The request path. It starts with `/` and can contain regular expressions, such as `/test/(.+)`
  - callback : `Function<(HttpRequest, HttpResponse)>`  
    Callback function, called when a PUT request matching path is received.
- Return value: The processed server object (for chaining other operations).
- Return value type: `HttpServer`



### Listen for Post Requests

`svr.onPost(path, callback)`

- Parameters:
  - path : `String`  
    The request path. It starts with `/` and can contain regular expressions, such as `/test/(.+)`
  - callback : `Function<(HttpRequest, HttpResponse)>`  
    Callback function, called when a POST request matching path is received.
- Return value: The processed server object (for chaining other operations).
- Return value type: `HttpServer`



### Listen for Patch Requests

`svr.onPatch(path, callback)`

- Parameters:
  - path : `String`  
    The request path. It starts with `/` and can contain regular expressions, such as `/test/(.+)`
  - callback : `Function<(HttpRequest, HttpResponse)>`  
    Callback function, called when a PATCH request matching path is received.
- Return value: The processed server object (for chaining other operations).
- Return value type: `HttpServer`



### Listen for Delete Requests

`svr.onDelete(path, callback)`

- Parameters:
  - path : `String`  
    The request path. It starts with `/` and can contain regular expressions, such as `/test/(.+)`
  - callback : `Function<(HttpRequest, HttpResponse)>`  
    Callback function, called when a DELETE request matching path is received.
- Return value: The processed server object (for chaining other operations).
- Return value type: `HttpServer`



### Listen for Options Requests

`svr.onOptions(path, callback)`

- Parameters:
  - path : `String`  
    The request path. It starts with `/` and can contain regular expressions, such as `/test/(.+)`
  - callback : `Function<(HttpRequest, HttpResponse)>`  
    Callback function, called when an OPTIONS request matching path is received.
- Return value: The processed server object (for chaining other operations).
- Return value type: `HttpServer`



### Listening for Requests - Notes

Please note that request listeners have priorities.  
If several paths match the regular expression at the same time, the path that was defined first is chosen.  
For example:  
```js
svr.onGet('/test/123', (req, res) => {
  res.write('test');
}).onGet('/test/(.+)', (req, res) => {
  res.write('get /test/');
}).onGet('/test/foo', (req, res) => {
  res.write("bar");
});
// A request to /test/123 calls the first callback; a request to /test/foo calls the second callback (even though the path of the third callback is a closer match)
```

### Listen for the PreRouting Event

`svr.onPreRouting(callback)`

- Parameters:
  - callback : `Function<bool(HttpRequest, HttpResponse)>`  
    Callback function, called when a request is received. The response can be modified in the callback. If it returns `false`, the request will not be routed to the callback of the corresponding path (but the PostRouting event is still triggered).
- Return value: The processed server object (for chaining other operations).
- Return value type: `HttpServer`



### Listen for the PostRouting Event

`svr.onPostRouting(callback)`

- Parameters:
  - callback : `Function<(HttpRequest, HttpResponse)>`  
    Callback function, called after the callback of the corresponding path (or the PreRouting event) has finished. The response can be modified in the callback.
- Return value: The processed server object (for chaining other operations).
- Return value type: `HttpServer`



### Listen for the Error Event

`svr.onError(callback)`

- Parameters:
  - callback : `Function<(HttpRequest, HttpResponse)>`  
    Callback function, called on errors (status code >= 400).
- Return value: The processed server object (for chaining other operations).
- Return value type: `HttpServer`



### Listen for the Exception Event

`svr.onException(callback)`

- Parameters:
  - callback : `Function<(HttpRequest, HttpResponse, String)>`  
    Callback function, called when an exception is thrown in a handler. The third parameter is the exception message.
- Return value: The processed server object (for chaining other operations).
- Return value type: `HttpServer`



### Listen on a Port and Start the Server

`svr.listen(addr, port)`
`svr.startAt(addr, port)`

- Parameters:
  - addr : `String`  
    The address to listen on. It can be an IP address or a domain name.
  - port : `Integer`  
    The port to listen on.
- Return value: The processed server object (for chaining other operations).
- Return value type: `HttpServer`



### Stop the Server

`svr.stop()`

- Return value: None



### Get Whether the Server Is Running

`svr.isRunning()`

- Return value: Whether the server is running.
- Return value type: `Boolean`



## Http Request Object API

### Http Request Object - Properties
| Attribute        | Meaning                                  | Data Type | Example                     |
| ---------------- | ---------------------------------------- | --------- | --------------------------- |
| `req.method`     | Request method                           | `String`  | `'GET'`                     |
| `req.path`       | Request path                             | `String`  | `'/233'`                    |
| `req.query`      | Request query parameters                 | `Object`  | `{a: 'b', c: ['d', 'e']}`   |
| `req.params`     | Request query parameters (same as above) | `Object`  | `{a: 'b', c: ['d', 'e']}`   |
| `req.headers`    | Request headers                          | `Object`  | `{a: ['b'], c: ['d', 'e']}` |
| `req.body`       | Request body                             | `String`  | `'awa'`                     |
| `req.remoteAddr` | Source address of the request            | `String`  | `'114.51.41.91'`            |
| `req.remotePort` | Source port of the request               | `Integer` | `11451`                     |
| `req.version`    | Request version                          | `String`  | `'HTTP/1.1'`                |
| `req.matches`    | Regex match results of the request path  | `Array`   | `['/test11', '11']`         |

These object properties are read-only and cannot be modified. They can only be used inside the request callback (copying them outside works in theory, but is not recommended).  

***Note:***
1. **`req.matches[0]` is the original text; the elements after it are the match results**
2. **`req.headers["Name"]` is an array, not a string**

***More examples:***
1. `?k=v1&k=v2&k1=v1` => `{k: ['v1', 'v2'], k1: 'v1'}`
2. `/test/(.+)/(.+)` => `/test/ll/233` => `['/test/ll/233', 'll', '233']`



### Get the Value of a Request Header

`req.getHeader(name)`

- Parameters:
  - name : `String`  
    The name of the request header.
- Return value: The array of values of the request header (an empty array `[]` if the request has no such header).
- Return value type: `Array<String>`



## Http Response Object API

### Http Response Object - Properties
| Attribute     | Meaning              | Data Type | Example                     |
| ------------- | -------------------- | --------- | --------------------------- |
| `res.status`  | Response status code | `Integer` | `200`                       |
| `res.headers` | Response headers     | `Object`  | `{a: ['b'], c: ['d', 'e']}` |
| `res.body`    | Response body        | `String`  | `'awa'`                     |
| `res.version` | Response version     | `String`  | `'HTTP/1.1'`                |
| `res.reason`  | Reason phrase        | `String`  | `'OK'`                      |

These properties can and should be modified, but changes only take effect inside the callback.



### Get the Value of a Response Header

`res.getHeader(name)`

- Parameters:
  - name : `String`  
    The name of the response header.
- Return value: The array of values of the response header (an empty array `[]` if the response has no such header).
- Return value type: `Array<String>`



### Set the Value of a Response Header

`res.setHeader(name, value)`

- Parameters:
  - name : `String`  
    The name of the response header.
  - value : `Any type`  
    The value of the response header.
- Return value: The processed response object.
- Return value type: `HttpResponse`



### Write the Response Body

`res.write(content1, content2, ...)`

- Parameters:
  - content : `Any type`  
    Response content.
- Return value: The processed response object.
- Return value type: `HttpResponse`
- Note: This function is currently equivalent to `res.body += content1 + content2 + ...`



## Http API Example

```js
let server = new HttpServer();
server.onGet("/hello(.+)", (req, resp) => {
	logger.info("http_server_test: run: onGet: Received a request from ", req.remoteAddr, 
				':', req.remotePort, " for ", req.path);
	resp.write("<html><body>")
		.write("<h1>It works!</h1>")
		.write("<br/>")
		.write("<p>Request Headers: ", JSON.stringify(req.headers, null, 4), "</p>")
		.write("<p>Request Body: ", req.body, "</p>")
		.write("<p>Your Address: ", req.remoteAddr, ':', req.remotePort, "</p>")
		.write("<p>HTTP Version: ", req.version, "</p>")
		.write("<p>Method: ", req.method, "</p>")
		.write("<p>Path: ", req.path, "</p>")
		.write("<p>Parsed Query: ", JSON.stringify(req.query, null, 4), "</p>")
		.write("<p>Path Regex Matches: ", JSON.stringify(req.matches), "</p>")
		.write("</body></html>")
	resp.status = 200;
	resp.reason = "OK";
	resp.setHeader("Content-Type", "text/html");
}).onGet("/404", (req, resp) => {
	resp.status = 404;
	resp.reason = "Not Found";
	resp.write("Not Found");
}).onGet("/test-redirect", (req, resp) => {
	// https://developer.mozilla.org/en-US/docs/Web/HTTP/Redirections
	resp.status = 301;
	resp.reason = "Moved Permanently";
	resp.setHeader("Location", "https://github.com");
}).onGet("/(.+)", (req, resp) => {
	resp.write("Hello World! ", req.matches);
}).onPreRouting((req, resp) => {
	logger.info("http_server_test: run: onPreRouting: resp.body.length: ", resp.body.length);
	if (req.path == "/test-prerouting") {
		resp.body = "Hello World!";
		resp.status = 200;
		resp.reason = "OK";
		return false;
	}
}).onPostRouting((req, resp) => {
	logger.info("http_server_test: run: onPostRouting: resp.body.length: ", resp.body.length);
}).onError((req, resp) => {
	resp.write("\nonError called");
}).listen(this.listen_address, this.listen_port);
logger.info("http_server_test: run: Server listening on " + this.listen_address + ":" + this.listen_port);
```
