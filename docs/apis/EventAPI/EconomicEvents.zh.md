## 💰 经济系统事件

此处为脚本引擎内置经济系统相关的经济变动事件

#### `"beforeMoneyAdd"` - 玩家金额增加前事件

- 监听函数原型
  `function(xuid,money)`
- 参数：
  - xuid : `String`  
    金额变动的玩家的XUID
  - money : `Integer`  
    增加的金额
- 拦截事件：函数返回`false`

<br>

#### `"onMoneyAdd"` - 玩家金额增加事件

- 监听函数原型
  `function(xuid,money)`
- 参数：
  - xuid : `String`  
    金额变动的玩家的XUID
  - money : `Integer`  
    增加的金额

<br>

#### `"beforeMoneyReduce"` - 玩家金额减少前事件

- 监听函数原型
  `function(xuid,money)`
- 参数：
  - xuid : `String`  
    金额变动的玩家的XUID
  - money : `Integer`  
    减少的金额
- 拦截事件：函数返回`false`

<br>

#### `"onMoneyReduce"` - 玩家金额减少事件

- 监听函数原型
  `function(xuid,money)`
- 参数：
  - xuid : `String`  
    金额变动的玩家的XUID
  - money : `Integer`  
    减少的金额

<br>

#### `"beforeMoneyTrans"` - 玩家转账前事件

- 监听函数原型
  `function(from,to,money)`
- 参数：
  - from : `String`  
    发起转账的玩家的XUID
  - to : `String`  
    接受转账的玩家的XUID
  - money : `Integer`  
    转账的金额
- 拦截事件：函数返回`false`

<br>

#### `"onMoneyTrans"` - 玩家转账事件

- 监听函数原型
  `function(from,to,money)`
- 参数：
  - from : `String`  
    发起转账的玩家的XUID
  - to : `String`  
    接受转账的玩家的XUID
  - money : `Integer`  
    转账的金额

**注意: 当 `onMoneyReduce` 或 `onMoneyAdd` 被触发时，该事件也会被触发**
<br>

#### `"beforeMoneySet"` - 设置玩家金额前事件

- 监听函数原型
  `function(xuid,money)`
- 参数：
  - xuid : `String`  
    金额变动的玩家的XUID
  - money : `Integer`  
    被设置的金额
- 拦截事件：函数返回`false`

<br>

#### `"onMoneySet"` - 直接设置玩家金额事件

- 监听函数原型
  `function(xuid,money)`
- 参数：
  - xuid : `String`  
    金额变动的玩家的XUID
  - money : `Integer`  
    被设置的金额

<br>