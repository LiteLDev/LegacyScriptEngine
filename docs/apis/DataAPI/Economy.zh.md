# 💰 经济系统 API

在很多服务器中，经济系统是非常关键的一环。  
为了解决传统使用计分板经济系统的种种问题，脚本引擎提供了对接LLMoney经济系统的接口，可以与其他各系列插件数据互通。 

LLMoney除了拥有传统经济系统的能力之外，还有查询金额变动历史、操作离线玩家经济等额外能力。  
LeviLamina在安装时附带了LLMoney插件，因此无需额外安装，就可以直接使用此接口。 

### 设置玩家的存款金额

`Player.setMoney(value)`

`money.set(xuid,value)`

- 参数：
  - xuid : `String`  
    要操作的玩家的XUID标识符
  - value : `Integer`  
    要设置的金额  
- 返回值：是否设置成功
- 返回值类型：`Boolean`



### 获取玩家的存款金额

`Player.getMoney()`

`money.get(xuid)`

- 参数：
  - xuid : `String`  
    要读取的玩家的XUID标识符
- 返回值：玩家的资金数值
- 返回值类型：`Integer`



### 增加玩家的存款

`Player.addMoney(value)`

`money.add(xuid,value)`

- 参数：
  - xuid : `String`  
    要操作的玩家的XUID标识符
  - value : `Integer`  
    要增加的金额  
- 返回值：是否设置成功
- 返回值类型：`Boolean`



### 减少玩家的存款

`Player.reduceMoney(value)`

`money.reduce(xuid,value)`

- 参数：
  - xuid : `String`  
    要操作的玩家的XUID标识符
  - value : `Integer`  
    要减小的金额  
- 返回值：是否设置成功
- 返回值类型：`Boolean`



### 进行一笔转账

`Player.transMoney(target,money[,note])`

`money.trans(xuid1,xuid2,money[,note])`

- 参数：
  - xuid1 : `String`  
    付款的玩家的XUID标识符
    
  - xuid2 : `String`  
    收款的玩家的XUID标识符
  
    如果你使用 `Player.transMoney`，target可以是玩家对象
  
  - money : `Integer`  
    要支付的金额  
  
  - note : `String`  
    （可选参数）给这笔转账附加一些文字说明
  
- 返回值：是否转账成功

- 返回值类型：`Boolean`



### 查询历史账单

`Player.getHistory(time)`

`money.getHistory(xuid,time)`

- 参数：
  - xuid : `String`  
    要操作的玩家的XUID标识符
  - time : `Integer`  
    查询从现在开始往前time秒的记录
- 返回值：查询结果对象的数组
- 返回值类型：`Array<Object>`

其中，结果为一系列记录对象组成的数组。对于每个记录对象`record`，有如下的键和对应的值：

| 键             | 值的意义                   | 数据类型  |
| -------------- | -------------------------- | --------- |
| `record.from`  | 此项交易的发起者玩家XUID   | `String`  |
| `record.to`    | 此项交易的接收者玩家XUID   | `String`  |
| `record.money` | 此项交易的金额             | `Integer` |
| `record.time`  | 此项交易发生时的时间字符串 | `String`  |
| `record.note`  | 此交易的附加说明信息       | `String`  |

时间字符串的格式为：YYYY-mm-dd hh:mm:ss



### 删除账单历史记录

`money.clearHistory(time)`

- 参数：
  - time : `Integer`  
    删除从现在开始往前time秒的记录
- 返回值：是否删除成功
- 返回值类型：`Boolean`


