## 💻 服务端设置 API

下面这些API提供了自定义某些服务器设置的接口

### 获取服务器版本号

`mc.getBDSVersion()`

- 返回值：服务端版本号字符串，格式形如`v1.17.10`
- 返回值类型：`String`

<br>

### 获取服务器协议版本

`mc.getServerProtocolVersion()`

- 返回值：服务端协议版本
- 返回值类型：`Number`

<br>

### 设置服务器MOTD字符串  

`mc.setMotd(motd)`

- 参数：
  - motd : `String`  
    目标MOTD字符串  
- 返回值：是否设置成功
- 返回值类型：`Boolean`

<br>

### 设置服务器最大玩家数  

`mc.setMaxPlayers(num)`

- 参数：
  - num : `Number`  
    最大玩家数  
- 返回值：是否设置成功
- 返回值类型：`Boolean`

<br>

### 获取服务器游戏时间  

`mc.getTime(TimeID)`

- 参数：
  - TimeID : `Integer`  
    想要查询的时间 (0 代表daytime，1 代表gametime，2 代表day)
- 返回值：获取到的时间
- 返回值类型：`Integer`

其中，daytime 代表自当天日出后流逝的游戏刻数，gametime 代表世界总共流逝的游戏刻数，day 代表已流逝的游戏天数。

<br>

### 设置服务器游戏时间  

`mc.setTime(tick)`

- 参数：
  - tick : `Integer`  
    想要设置的时间
- 返回值：是否设置成功
- 返回值类型：`Boolean`

<br>

### 获取服务器天气  

`mc.getWeather()`

- 返回值：当前天气 (0 代表晴天，1 代表雨天，2 代表雷暴)
- 返回值类型：`Integer`

<br>

### 设置服务器天气  

`mc.setWeather(WeatherID)`

- 参数：
  - WeatherID : `Integer`  
    想要设置的天气 (0 代表晴天，1 代表雨天，2 代表雷暴)
- 返回值：是否设置成功
- 返回值类型：`Boolean`

<br>
