## 💻 Server Settings API

The following APIs provide interfaces for customizing some server settings:

### Get the Version Number of the BDS Server

`mc.getBDSVersion()`

- Return value: The server version number string, formatted like this: `v1.17.10`
- Return value type: `String`

<br>

### Get BDS Server Protocol Version 

`mc.getServerProtocolVersion()`

- Return value: Server protocol version 
- Return value type: `Number`

<br>

### Set Server Motd String  

`mc.setMotd(motd)`

- Parameters: 
  - motd : `String`  
    The desired Motd string.  
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

<br>

### Set the Maximum Number of Players on the Server  

`mc.setMaxPlayers(num)`

- Parameters: 
  - num : `Number`  
    The maximum number of players.  
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

<br>

### Get Sever time  

`mc.getTime(TimeID)`

- Parameters:
  - TimeID : `Integer`  
    Specifies the time to get. Must be 0, 1 or 2. (0 represents daytime, 1 represents gametime, 2 represents day)
- Return value: Current time
- Return value type: `Integer`

Among them, daytime is the number of game ticks since dawn, gametime is the age of the world in game ticks, day is the number of in-game days passed.

<br>

### Set Sever time   

`mc.setTime(tick)`

- Parameters:
  - tick : `Integer`  
    The time you want to set
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

<br>

### Get Sever Weather  

`mc.getWeather()`

- Return value: Current weather (0 represents Clear, 1 represents Rain, 2 represents Thunder)
- Return value type: `Integer`

<br>

### Set Sever Weather  

`mc.setWeather(WeatherID)`

- Parameters:
  - WeatherID : `Integer`  
    The weather you want to set (0 represents Clear, 1 represents Rain, 2 represents Thunder)
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

<br>
