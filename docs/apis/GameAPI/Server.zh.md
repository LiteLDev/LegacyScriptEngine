# 💻 服务端设置 API

下面这些API提供了自定义某些服务器设置的接口

### 获取服务器版本号

`mc.getBDSVersion()`

- 返回值：服务端版本号字符串，格式形如`v1.17.10`
- 返回值类型：`String`



### 获取服务器协议版本

`mc.getServerProtocolVersion()`

- 返回值：服务端协议版本
- 返回值类型：`Number`



### 获取服务器MOTD字符串

!!! warning
    此函数仅在0.19.1及以后版本可用

`mc.getMotd()`

- 返回值：当前服务器MOTD字符串
- 返回值类型：`String`



### 设置服务器MOTD字符串  

`mc.setMotd(motd)`

- 参数：
  - motd : `String`  
    目标MOTD字符串  
- 返回值：是否设置成功
- 返回值类型：`Boolean`



### 设置服务器最大玩家数  

`mc.setMaxPlayers(num)`

- 参数：
  - num : `Number`  
    最大玩家数  
- 返回值：是否设置成功
- 返回值类型：`Boolean`



### 获取在线玩家数量

!!! warning
    此函数仅在0.19.1及以后版本可用

`mc.getOnlinePlayerNum([ignoreSimulatedPlayer])`

- 参数：
  - ignoreSimulatedPlayer : `Boolean` = `false`  
    是否忽略模拟玩家
- 返回值：当前在线玩家数量
- 返回值类型：`Number`



### 获取服务器最大玩家数

!!! warning
    此函数仅在0.19.1及以后版本可用

`mc.getMaxNumPlayers()`

- 返回值：当前服务器最大玩家数
- 返回值类型：`Number`



### 获取服务器游戏时间  

`mc.getTime(TimeID)`

- 参数：
  - TimeID : `Integer`  
    想要查询的时间 (0 代表daytime，1 代表gametime，2 代表day)
- 返回值：获取到的时间
- 返回值类型：`Integer`

其中，daytime 代表自当天日出后流逝的游戏刻数，gametime 代表世界总共流逝的游戏刻数，day 代表已流逝的游戏天数。



### 设置服务器游戏时间  

`mc.setTime(tick)`

- 参数：
  - tick : `Integer`  
    想要设置的时间
- 返回值：是否设置成功
- 返回值类型：`Boolean`



### 获取服务器天气  

`mc.getWeather()`

- 返回值：当前天气 (0 代表晴天，1 代表雨天，2 代表雷暴)
- 返回值类型：`Integer`



### 设置服务器天气  

`mc.setWeather(WeatherID)`

- 参数：
  - WeatherID : `Integer`  
    想要设置的天气 (0 代表晴天，1 代表雨天，2 代表雷暴)
- 返回值：是否设置成功
- 返回值类型：`Boolean`



### 获取维度ID

!!! warning
    此函数仅在0.19.1及以后版本可用

`mc.getDimensionId(name)`

- 参数：
  - name : `String`  
    维度名称
- 返回值：对应维度名称的维度ID，若维度名称无效则返回`null`
- 返回值类型：`Number`



### 获取维度名称

!!! warning
    此函数仅在0.19.1及以后版本可用

`mc.getDimensionName(dimid)`

- 参数：
  - dimid : `Integer`  
    维度ID
- 返回值：对应维度ID的维度名称，若维度ID无效则返回`null`
- 返回值类型：`String`


