## 🏃‍♂️ Player Object API

In LLSE, use "player objects" to manipulate and obtain information about a player.

### Get a Player Object

#### Get From Event or API

By registering the **event listener** function, get the player object related to the related event given by BDS.
For details, see [Event listener documentation - EventAPI](/LLSEPluginDevelopment/EventAPI/Listen.md)

#### Acquired From Existing Players

Manually generate player objects by **player information**
Use this function to manually generate objects. Note that the player you want to get must be online, otherwise the generation will fail.

`mc.getPlayer(info)`

- Parameters: 
  - info : `String`  
    Player's name or XUID or uniqueId.
- Return value: The generated player object. 
- Return value type: `Player`
  - If the return value is `Null`, it means that getting the player failed.

#### Get All Online Players

This function returns an array of player objects, each of which corresponds to a player on the server.

`mc.getOnlinePlayers()`

- Return value: List of online `Player` objects.
- Return value type: `Array<Player,Player,...>`

> Note: Do not save a `Player` object **long-term**.
> When the player exits the server, the corresponding player object will become invalid. Therefore, if there is a need to operate a player for a long time, please obtain the real-time player object through the above methods.

<br>

### Player Object - Properties

Each player object contains some fixed object properties. For a particular player object `pl`, there are these properties.

| Attributes               | Meaning                                                      | Data Type        |
| ------------------------ | ------------------------------------------------------------ | ---------------- |
| pl.name                  | Player's name                                                | `String`         |
| pl.pos                   | Player's coordinates                                         | `FloatPos`       |
| pl.feetPos               | The coordinates of the player's leg                          | `FloatPos`       |
| pl.blockPos              | The coordinates of the block that the player is standing on. | `IntPos`         |
| pl.lastDeathPos          | The coordinates of the block that the player last died.      | `IntPos`         |
| pl.realName              | Player's Real Name                                           | `String`         |
| pl.xuid                  | Player XUID String                                           | `String`         |
| pl.uuid                  | Player Uuid string                                           | `String`         |
| pl.permLevel             | Player's permission level (0 - 4)                            | `Integer`        |
| pl.gameMode              | Player's game mode (0 - 2, 6)                                | `Integer`        |
| pl.canSleep              | Whether the player can sleep                                 | `Boolean`        |
| pl.canFly                | Whether the player can fly                                   | `Boolean`        |
| pl.canBeSeenOnMap        | Whether the player can be seen on map                        | `Boolean`        |
| pl.canFreeze             | Whether the player can freeze                                | `Boolean`        |
| pl.canSeeDaylight        | Whether the player can see daylight                          | `Boolean`        |
| pl.canShowNameTag        | Whether the player can show name tag                         | `Boolean`        |
| pl.canStartSleepInBed    | Whether the player can start sleep in bed                    | `Boolean`        |
| pl.canPickupItems        | Whether the player can pickup items                          | `Boolean`        |
| pl.maxHealth             | Player's maximum health                                      | `Integer`        |
| pl.health                | Player's current health                                      | `Integer`        |
| pl.inAir                 | Whether the player is in the air                             | `Boolean`        |
| pl.inWater               | Whether the player is in water                               | `Boolean`        |
| pl.inLava                | Whether the player is in lava                                | `Boolean`        |
| pl.inRain                | Whether the player is in rain                                | `Boolean`        |
| pl.inSnow                | Whether the player is in snow                                | `Boolean`        |
| pl.inWall                | Whether the player is in wall                                | `Boolean`        |
| pl.inWaterOrRain         | Whether the player is in water or rain                       | `Boolean`        |
| pl.inWorld               | Whether the player is in world                               | `Boolean`        |
| pl.inClouds              | Whether the player is in clouds                              | `Boolean`        |
| pl.speed                 | Player's current speed                                       | `Float`          |
| pl.direction             | Player's current orientation                                 | `DirectionAngle` |
| pl.uniqueId              | Player's (entity's) unique identifier                        | `String`         |
| pl.isLoading             | Player is loading                                            | `Boolean`        |
| pl.isInvisible           | Player is invisible                                          | `Boolean`        |
| pl.isInsidePortal        | Player is inside portal                                      | `Boolean`        |
| pl.isHurt                | Player is hurt                                               | `Boolean`        |
| pl.isTrusting            | Player is trusting                                           | `Boolean`        |
| pl.isTouchingDamageBlock | Player is touching the damage block                          | `Boolean`        |
| pl.isHungry              | Player is hungry                                             | `Boolean`        |
| pl.isOnFire              | Player is on fire                                            | `Boolean`        |
| pl.isOnGround            | Player is on ground                                          | `Boolean`        |
| pl.isOnHotBlock          | Player is on hot block (magma and etc.)                      | `Boolean`        |
| pl.isTrading             | Player is trading                                            | `Boolean`        |
| pl.isAdventure           | Player is in Adventure Mode                                  | `Boolean`        |
| pl.isGliding             | Player is gliding                                            | `Boolean`        |
| pl.isSurvival            | Player is in Survival Mode                                   | `Boolean`        |
| pl.isSpectator           | Player is in Spectator Mode                                  | `Boolean`        |
| pl.isRiding              | Player is riding                                             | `Boolean`        |
| pl.isDancing             | Player is dancing                                            | `Boolean`        |
| pl.isCreative            | Player is in Creative Mode                                   | `Boolean`        |
| pl.isFlying              | Player is flying                                             | `Boolean`        |
| pl.isSleeping            | Player is sleeping                                           | `Boolean`        |
| pl.isMoving              | Player is moving                                             | `Boolean`        |
| pl.isSneaking            | Player is sneaking                                           | `Boolean`        |

These object properties are read-only and cannot be modified. in:

- **coordinates** and **leg coordinates**: player is two blocks high, and `pos` are the coordinates of the player's view's height, `feetPos` are the coordinates of the block position displayed in the game
- The value of the **Player Game Mode** attribute is: `0` for survival mode, `1` for creative mode, `2` for adventure mode, `3` for spectator mode
- **Player's real name** attribute stored strings can be considered reliable, they will not be changed by name changes
- **Player device IP address** attribute stores the player's device IP and port number, the format is similar to `12.34.567.89:1111`
- For a detailed explanation of the **player's current orientation** attribute, see [Basic Game Interface Documentation](/LLSEPluginDevelopment/GameAPI/Basic.md)
- The comparison table of **operation authority level** attributes is as follows:

| Permission Level | Corresponding Authority  |
| ---------------- | ------------------------ |
| 0                | Ordinary Member          |
| 1                | OP permissions           |
| 4                | OP + Console permissions |

<br>

### Player Object - Function

Each player object contains some member functions (member methods) that can be executed. For a specific player object `pl`, you can perform some operations on this player through the following functions.

#### Determine if the Player Is OP  

`pl.isOP()`

- Return value: Whether the player is an OP.
- Return value type: `Boolean`  

[JavaScript]
```js
// For a `Player` object pl
var open = pl.isOP();
```

#### Disconnect Player  

`pl.kick([msg])`  
`pl.disconnect([msg])`

- Parameters: 
  - msg : `String`  
    (Optional parameter) The reason for the disconnection displayed to the kicked player.
    Defaults to "disconnecting from server".  
- Return value: Whether the connection was successfully disconnected.
- Return value type: `Boolean`

[JavaScript]
```js
//For a `Player` object pl
pl.kick();
```
[Lua]
```lua
pl:kick()
```

#### Send a Text Message to the Player  

`pl.tell(msg[,type])`  
`pl.sendText(msg[,type])`

- Parameters: 

  - msg : `String`  
    The message to be sent.  

  - type : `Integer`  
    (Optional parameter) The type of text message to send, default is `0`.

    | Type Parameter | Message Type                      |
    | -------------- | --------------------------------- |
    | 0              | Normal Message (Raw)              |
    | 1              | Chat Message (Chat)               |
    | 4              | Music Box Message (Popup)         |
    | 5              | Message Above the Inventory (Tip) |
    | 9              | JSON format message (JSON)        |

- Return value: Whether the message was sent successfully.

- Return value type: `Boolean`

[JavaScript]
```js
//For a `Player` object pl
pl.tell("Welcome back ~ ",5);
```

#### Set Title Message to the Player 

`pl.setTitle(content[,type[,fadeInTime,stayTime,fadeOutTime]])`

- Parameter: 

  - content : `String`  
    The title content. 

  - type : `Integer`  
    (optional) The title type, default = 2.

    | Types | Description         |
    | ----- | ------------------- |
    | 0     | Clear               |
    | 1     | Reset               |
    | 2     | SetTitle            |
    | 3     | SetSubTitle         |
    | 4     | SetActionBar        |
    | 5     | SetDurations        |
    | 6     | TitleTextObject     |
    | 7     | SubtitleTextObject  |
    | 8     | ActionbarTextObject |

  - fadeInTime : `Integer`  
    (optional) Fade in time, in `Tick` , default is 10

  - stayTime: `Integer`

    (optional) Stay time, in `Tick` , default is 10

  - fadeOutTime:`Integer`

    (optional) Fade out time, in `Tick` , default is 10

- Return value: Is send successfully?

- Return value type: `Boolean`

#### Broadcast a Text Message to All Players  

`mc.broadcast(msg[,type])`

- Parameters: 

  - msg : `String`  
    The message to be sent.  

  - type : `Integer`  
    (Optional parameter) The type of text message to send, default is `0`.

    | Type Parameter | Message Type                      |
    | -------------- | --------------------------------- |
    | 0              | Normal Message (Raw)              |
    | 1              | Chat Message (Chat)               |
    | 4              | Music Box Message (Popup)         |
    | 5              | Message Above the Inventory (Tip) |
    | 9              | JSON format message (JSON)        |

- Return value: Whether the message was sent successfully.

- Return value type: `Boolean`

[JavaScript]
```js
mc.broadcast("Hello everyone ~ ");
```

#### Display a toast to the top of the screen

`pl.sendToast(title,message)`

- Parameters: 

  - title : `String`  
    The title of the toast.  

  - message : `string`  
    the message that the toast may contain alongside the title.

- Return value: Whether the message was sent successfully.
- Return value type: `Boolean`

[JavaScript]
```js
pl.sendToast("Hello","everyone ~");
```


#### Execute a Command as a Player 

`pl.runcmd(cmd)`

- Parameters: 
  - cmd : `String`  
    The command to be executed.  
- Return value: Whether the execution was successful.
- Return value type:  `Boolean`   

[JavaScript]
```js
//For a `Player` object pl
var open = pl.runcmd("tp ~ ~+50 ~");
```

#### Speak as a Player

`pl.talkAs(text)`

- Parameters: 
  - text : `String`  
    The text the player will be made to say. 
- Return value: Whether the execution was successful.
- Return value type:  `Boolean`   

<br>

#### Get Player Distance To Pos

`pl.distanceTo(pos)`
`pl.distanceToSqr(pos)`

- Parameters: 
  - pos : `Entity` / `Player` / `IntPos` / `FloatPos`
    The target position. 
- Return value: Distance to coordinates (in blocks).
- Return value type:  `Number`   

<br>

#### Speak to a Player as a Player

`pl.talkAs(target,text)`

- Parameters: 
  - target : `Player`  
    The player who will be spoken to. 
  - text : `String`  
    The text the player will be made to say. 
- Return value: Whether the execution was successful.
- Return value type:  `Boolean`   

<br>

#### Teleport the Player to the Specified Location  

`pl.teleport(pos[,rot])`  
`pl.teleport(x,y,z,dimid[,rot])`

- Parameters: 
  - pos: `IntPos `/ `FloatPos`  
    Target position coordinates (or use x, y, z, dimid to determine player position)
    
  - rot: `DirectionAngle`
  
    (Optional) The orientation of the player after teleport, or the same orientation as before teleport if default
  
- Return value: Whether the teleport was successful or not.

- Return value type: `Boolean`

[JavaScript]
```js
//For a `Player` object pl, a coordinate object pos
pl.teleport(pos);
```

#### Kill the Player  

`pl.kill()`

- Return value: Whether the execution was successful.
- Return value type: `Boolean`

[JavaScript]
```js
[JavaScript]
//For a `Player` object pl
pl.kill();
[Lua]

```

#### Damage the Player

`pl.hurt(damage)`

- Parameters: 
  - damage : `Integer`  
    The amount of damage dealt to the player .
- Return value: Whether damage was dealt.
- Return value type: `Boolean`

Note that the damage dealt here is real damage and cannot be reduced by protective equipment such as armor.

<br>

#### Heal the Player

`pl.heal(health)`

- Parameters: 
  - health : `Integer`  
    Number of hearts to heal.
- Return value: Whether heal was dealt.
- Return value type: `Boolean`

<br>

#### Set Health for Player

`pl.setHealth(health)`

- Parameters: 
  - health : `Integer`  
    Number of hearts.
- Return value: Whether set health for player was success.
- Return value type: `Boolean`

<br>

#### Set Absorption Attribute for Player

`pl.setAbsorption(value)`

- Parameters: 
  - value : `Integer`  
    New value
- Return value: Whether set attribute value for player was success.
- Return value type: `Boolean`

<br>

#### Set Attack Damage Attribute for Player

`pl.setAttackDamage(value)`

- Parameters: 
  - value : `Integer`  
    New value
- Return value: Whether set attribute value for player was success.
- Return value type: `Boolean`

<br>

#### Set Maximal Attack Damage Attribute for Player

`pl.setMaxAttackDamage(value)`

- Parameters: 
  - value : `Integer`  
    New value
- Return value: Whether set attribute value for player was success.
- Return value type: `Boolean`

<br>

#### Set Follow Range Attribute for Player

`pl.setFollowRange(value)`

- Parameters: 
  - value : `Integer`  
    New value
- Return value: Whether set attribute value for player was success.
- Return value type: `Boolean`

<br>

#### Set Knockback Resistance Attribute for Player

`pl.setKnockbackResistance(value)`

- Parameters: 
  - value : `Integer`  
    New value (0 or 1)
- Return value: Whether set attribute value for player was success.
- Return value type: `Boolean`

<br>

#### Set Luck Attribute for Player

`pl.setLuck(value)`

- Parameters: 
  - value : `Integer`  
    New value
- Return value: Whether set attribute value for player was success.
- Return value type: `Boolean`

<br>

#### Set Movement Speed for Player

`pl.setMovementSpeed(value)`

- Parameters: 
  - value : `Integer`  
    New value
- Return value: Whether set attribute value for player was success.
- Return value type: `Boolean`

<br>

#### Set Underwater Movement Speed for Player

`pl.setUnderwaterMovementSpeed(value)`

- Parameters: 
  - value : `Integer`  
    New value
- Return value: Whether set attribute value for player was success.
- Return value type: `Boolean`

<br>

#### Set Lava Movement Speed for Player

`pl.setLavaMovementSpeed(value)`

- Parameters: 
  - value : `Integer`  
    New value
- Return value: Whether set attribute value for player was success.
- Return value type: `Boolean`

<br>

#### Set Max Health for Player

`pl.setMaxHealth(health)`

- Parameters: 
  - health : `Integer`  
    Number of hearts.
- Return value: Whether set max health for player was success.
- Return value type: `Boolean`

<br>

#### Set Hunger for Player

`pl.setHungry(hunger)`

- Parameters: 
  - hunger : `Integer`  
    Number of hunger.
- Return value: Whether set hunger for player was success.
- Return value type: `Boolean`

<br>

#### Set the Specified Player on Fire

`pl.setFire(time,isEffect)`

- Parameters: 
  - time : `Integer`  
    Fire time, in seconds.
  - isEffect : `Boolean`
    Will there be a fire effect?
- Return value: Whether the player was set on fire.
- Return value type: `Boolean`

<br>

#### Put Out The Player

`pl.stopFire()`

- Return value: Has been extinguished.
- Return value type: `Boolean`

<br>

#### Scale Player

`pl.setScale(scale)`

- Parameters: 
  - scale : `Integer`  
    New player size
- Return value: Whether the player was scaled.
- Return value type: `Boolean`

<br>

#### Rename Player  

`pl.rename(newname)`

- Parameters: 
  - newname : `String`  
    Player's new name.  
- Return value: WHether the rename was successful.
- Return value type: `Boolean`

[JavaScript]
```js
//For a `Player` object pl
pl.rename("newname");
```

#### Get the Block the Player Is Currently Standing On

`pl.getBlockStandingOn()`

- Return value: The `Block` object currently standing on.
- Return value type: `Block`

<br>

#### Get the Player's Device Information Object

`pl.getDevice()`

- Return value: Device information object corresponding to the player
- Return value type: `Device`

The device information object stores certain information about the player's device, such as device IP address, device type, network latency, etc.  
For additional information about device information objects, please refer to [Device Information Objects API](/LLSEPluginDevelopment/GameAPI/Device.md)

<br>

#### Get the Item Object in the Player’s Main Hand

`pl.getHand()`

- Return value: The item object in the player's main hand.
- Return value type: `Item`

The item object obtained here is a reference. That is to say, modifying the item object returned here, or using its API, is equivalent to directly operating the corresponding item in the player's main hand.

<br>

#### Get the Item Object of the Player’s Off-Hand

`pl.getOffHand()`

- Return value: The item object in the player's off-hand
- Return value type: `Item`

The item object obtained here is a reference. That is to say, modifying the item object returned here, or using its API, is equivalent to directly operating the corresponding item in the player's off hand.

<br>

#### Get the Container Object of the Player’s Inventory  

`pl.getInventory()`

- Return value: The container object corresponding to the player's inventory
- Return value type: `Container`

For more usage of container objects, please refer to [Container Object API Documentation](/LLSEPluginDevelopment/GameAPI/Container.md)

<br>

#### Gets the Container Object for the Player’s Armor Bar  

`pl.getArmor()`

- Return value: The container object corresponding to the player's armor bar
- Return value type: `Container`

For more usage of container objects, please refer to [Container Object API Documentation](/LLSEPluginDevelopment/GameAPI/Container.md)

<br>

#### Get the Container Object of the Player’s Ender Chest  

`pl.getEnderChest()`

- Return value: The container object corresponding to the player's ender chest.
- Return value type: `Container`

For more usage of container objects, please refer to [Container Object API Documentation](/LLSEPluginDevelopment/GameAPI/Container.md)

<br>

#### Get the Player’s Respawn Coordinates  

`pl.getRespawnPosition()`

- Return value: Respawn point coordinates
- Return value type: `IntPos`

<br>

#### Modify the Player’s Respawn Coordinates  

`mc.setRespawnPosition(pos)`  
`mc.setRespawnPosition(x,y,z,dimid)`

- Parameters: 
  - pos : `IntPos`  
    Respawn coordinates (or use x, y, z, dimid to determine respawn position)
- Return value: Whether the modification was successful.
- Return value type: `Boolean`

<br>

#### Give the Player an Item

`pl.giveItem(item[, amount])`

- Parameters: 
  - item : `Item`  
    The item being given.
    
  - amount: `Integer`
  
    (Optional) The number of item given. If this parameter is provided, the Count property of the item object itself will be ignored.
- Return value: Whether the item was given.
- Return value type: `Boolean`

If the player's inventory is full, excess items will be drop.

<br>

#### Clears All Items of the Specified Type From the Player’s Backpack

`pl.clearItem(type[, count)`

- Parameters: 
  - type : `String`  
    Item object type name to clear
  - count : `Integer`  
    (Optional)Item count to be clear
- Return value: The number of items cleared
- Return value type: `Integer`

Compares the type attribute of all items in the player's inventory, main hand, off-hand, and armor to this string.
If found, clear this item.

<br>

#### Refresh Player Inventory, Armor Bar

`pl.refreshItems()`

- Return value: Whether the refresh was successful
- Return value type: `Boolean`

After modifying the player's items, in order for the client to take effect, it is necessary to refresh all the player's items.

<br>

#### Refresh All Chunks Loaded by the Player

`pl.refreshChunks()`

- Return value: Whether the refresh was successful.
- Return value type: `Boolean`

<br>

#### Modify Player Operation Permissions  

`pl.setPermLevel(level)`

- Parameters: 

  - level : `Integer`  
    Target operation authority level  

    | Player Permission Level | Corresponding Permission Authority |
    | ----------------------- | ---------------------------------- |
    | 0                       | Ordinary Member Permissions        |
    | 1                       | OP Permissions                     |
    | 4                       | OP + Console Permissions           |

- Return value: Whether the modification was successful.

- Return value type: `Boolean`

[JavaScript]
```js
//For a `Player` object pl
pl.setPermLevel(1);
```

#### Modify Player Game Mode

`pl.setGameMode(mode)`

- Parameters: 

  - mode : `Integer`  
    Target game mode, `0` is survival mode, `1` is creative mode, `2` is adventure mode. `6` is spectator mode.
- Return value: Whether the modification was successful.
- Return value type: `Boolean`

[JavaScript]
```js
//For a `Player` object pl
pl.setGameMode(1);
```

#### Increase Player Experience Level 

`pl.addLevel(count)`

- Parameters: 
  - count : `Integer`  
    The number of experience levels to add.
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

[JavaScript]
```js
//For a `Player` object pl
pl.addLevel(6);
```
#### Decreases Player Experience Level

`pl.reduceLevel(count)`

- Parameters: 
  - count : `Integer`  
    The number of experience levels to reduce.
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

<br>

#### Get Player Experience Level 

`pl.getLevel()`

- Return value: The player's experience level.
- Return value type: `Integer`

[JavaScript]
```js
//For a `Player` object pl
pl.getLevel();
```
[Lua]
```lua
--For a `Player` object pl
pl.getLevel()
```
#### Set Player Experience Level

`pl.setLevel(count)`

- Parameters
  - count : `Integer`  
    The number of experience levels to set.
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

<br>

#### Reset Player Experience

`pl.resetLevel()`

- Return value: Whether the setting was successful.
- Return value type: `Boolean`

[JavaScript]
```js
//For a `Player` object pl
pl.resetLevel();
```
[Lua]
```lua
--For a `Player` object pl
pl:resetLevel()
```

#### Get Player Current Experience Points 

`pl.getCurrentExperience()`

- Return value: The player's current experience points.
- Return value type: `Integer`

<br>

#### Set Player Current Experience Points

`pl.setCurrentExperience(count)`

- Parameters
  - count : `Integer`  
    The number of experience points to set.
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

<br>

#### Get Player Total Experience Points

`pl.getTotalExperience()`

- Return value: The player's total experience points.
- Return value type: `Integer`

<br>

#### Set Player Total Experience Points

`pl.getTotalExperience(count)`

- Parameters
  - count : `Integer`  
    The number of experience points to set.
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

<br>

#### Increase Player Experience Points

`pl.addExperience(count)`

- Parameters: 
  - count : `Integer`
    The amount of experience points to give to the player.
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

[JavaScript]
```js
//For a `Player` object pl
pl.addExperience(6);
```
[Lua]
```lua
--For a `Player` object pl
pl:addExperience(6)
```

#### Decreases Player Experience Points

`pl.reduceExperience(count)`

- Parameters: 
  - count : `Integer`  
    The number of experience points to reduce.
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

<br>

#### Get the Experience Points Needed for Players to Level Up

`pl.getXpNeededForNextLevel()`

- Return value: The amount of experience points required for the player to level up.
- Return value type: `Integer`

Note that this method ignores the experience value that exceeds the level when calculating.

[JavaScript]
```js
//For a `Player` object pl
pl.getXpNeededForNextLevel();
```
[Lua]
```lua
--For a `Player` object pl
pl.getXpNeededForNextLevel()
```

#### Send the Player to the Specified Server  

`pl.transServer(server,port)`

- Parameters: 
  - server : `String`  
    Target server IP / domain name

  - port : `Integer`  
    Target server port  
- Return value: Whether the transfer was successful or not.
- Return value type: `Boolean` 

[JavaScript]
```js
//For a `Player` object pl
pl.transServer("123.45.67.89",23333);
```

#### Crash the Player Client

`pl.crash()`

- Return value: Whether the execution was successful.
- Return value type: `Boolean` 

[JavaScript]
```js
//For a `Player` object pl
pl.crash();
```
[Lua]
```lua
--For a `Player` object pl
pl:crash()

```
#### Set Player Custom Sidebar

`pl.setSidebar(title,data[,sortOrder])`

- Parameters: 

  - title : `String`  
    Sidebar Title  
  - data : `Object<String-Integer>`  
    Sidebar Object Content Object  
    Each key-value pair in the object will be set as a row of the sidebar content.
  - sortOrder : `Number`  
    (Optional) Sort order for sidebar content. `0` In ascending order of scores, `1` in descending order by score. Default is `1`.

- Return value: Whether the setting was successful or not.

- Return value type: `Boolean`

[JavaScript]
```js
//For a `Player` object pl
pl.setSidebar("title",{"aaaa":3,"bbb":12,"cc":7});
```

#### Remove Player Customization Sidebar 

`pl.removeSidebar()`

- Return value: Whether the removal was successful.
- Return value type: `Boolean`

[JavaScript]
```js
//For a `Player` object pl
pl.removeSidebar();
```

#### Sets the Custom Boss Health Bar That the Player Sees  

`pl.setBossBar(uid,title,percent,colour)`

- Parameters: 
  - uid : `Number`   
    Unique identifier, no conflicting duplicates! One uid for one line of bar
  - title : `String`  
    Custom Health Bar Title  
  - percent : `Integer`  
    The percentage of health in the boss bar, the valid range is 0~100. `0` is empty boss bar, `100` is full.
  - colour : `Integer`
     Health bar color (default is 2 (RED))
- Return value: Whether the setting was successful or not.
- Return value type: `Boolean`

[JavaScript]
```js
//For a `Player` object pl
pl.setBossBar(1145141919,"Hello ~ ",80,0);
```

#### Remove the Player’s Custom Boss Health Bar  

`pl.removeBossBar(uid)`

- Parameters: 
  - uid : `Number`  
    Identifier, corresponding to setBossBar!
- Return value: Whether the removal was successful.
- Return value type: `Boolean`

[JavaScript]
```js
//For a `Player` object pl
pl.removeBossBar(1145141919);
```

#### Get an Online Player's NBT Object

`pl.getNbt()`

- Return value: Player's NBT object.
- Return value type: `NbtCompound`

<br>

#### Write to an Online Player's NBT Object

`pl.setNbt(nbt)`

- Parameters: 
  - nbt : `NbtCompound`  
    NBT objects
- Return value: Whether the write was successful or not.
- Return value type: `Boolean`

For more usage of NBT objects, please refer to [NBT Interface Documentation](/LLSEPluginDevelopment/NbtAPI/NBT.md)

<br>

#### Get an Player's NBT Object

`mc.getPlayerNbt(uuid)`

- Parameters: 
  - uuid : `String`  
    Player`s UUID
- Return value: Player's NBT object.
- Return value type: `NbtCompound`

Using this API, you can operate offline player`s nbt. 

<br>

#### Write to an Player's NBT Object

`mc.setPlayerNbt(uuid,nbt)`

- Parameters: 
  - uuid : `String`  
    Player`s UUID
  - nbt : `NbtCompound`  
    NBT objects
- Return value: Whether the write was successful or not.
- Return value type: `Boolean`

Using this API, you can operate offline player`s nbt. 

<br>

#### Write Data to Some Special Tags of an Player's NBT Object

`mc.setPlayerNbtTags(uuid,nbt,tags)`

- Parameters: 
  - uuid : `String`  
    Player`s UUID
  - nbt : `NbtCompound`  
    NBT objects
  - tags : `Array`  
    Tags need to write
- Return value: Whether the write was successful or not.
- Return value type: `Boolean`

Using this API, you can operate offline player`s nbt. 

<br>

#### Delete an Player's NBT Object

`mc.deletePlayerNbt(uuid)`

- Parameters: 
  - uuid : `String`  
    Player`s UUID
- Return value: Whether the delete was successful or not.
- Return value type: `Boolean`

Using this API, you can operate offline player`s nbt. 

<br>

#### Add a Tag for the Player

`pl.addTag(tag)`

- Parameters: 
  - tag: `String`  
    The tag string to be added.
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

<br>

#### Remove a Tag for a player

`pl.removeTag(tag)`

- Parameters: 
  - tag: `String`  
    The tag string to remove.
- Return value: Whether the removal was successful.
- Return value type: `Boolean`

<br>

#### Check if the Player Has a Tag 

`pl.hasTag(tag)`

- Parameters: 
  - tag: `String`  
    Tag string to check
- Return value: Whether the player has the tag.
- Return value type: `Boolean`

<br>

#### Get a List of All Tags Player the Player Has

`pl.getAllTags()`

- Return value: List of all tag strings of the player.
- Return value type: `Array<String,String,...>`

<br>

#### Get a List of the Player’s Abilities (From the Player’s NBT)

`pl.getAbilities()`

- Return value:  A list object of key-value pairs of all player ability information.
- Return value type: `object<String-Any Type>`

Each item in the list of key-value pairs looks like: `"mayfly": 1` etc.
<br>

#### Get a list of the player's Attributes (from the player's NBT)

`pl.getAttributes()`

- Return value: An array of all property objects of the player.
- Return value type: `Array<Object,Object,...>`

Each item in the array is a key-value pair list object `Object`, and the Attributes object contains several contents such as `Base` `Current` `DefaultMax` `DefaultMin` `Max` `Min` `Name` by default. Its content looks like:
```json
{
    "Base": 0, 
    "Current": 0, 
    "DefaultMax": 1024, 
    "DefaultMin": -1024, 
    "Max": 1024, 
    "Min": -1024, 
    "Name": "minecraft:luck"
}
```

(Here it's displayed visually using JSON format)

<br>

#### Get Player Sprint Status

`pl.isSprinting()`

- Return value: Player's sprint state
- Return value type: `Boolean`

<br>

#### Set Player Sprint State 

`pl.setSprinting(sprinting)`

- Parameters: 
  - sprinting : `Boolean`  
    Sprinting state.
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

<br>

#### Sending packets to the player

`pl.sendPacket(packet)`  

- Parameters:
  - packet : `Packet`  
    Packet
- Return value: Whether the setting was successful.
- Return value type: `Boolean`

<br>

#### Get the player's Biome ID

`pl.getBiomeId()`  

- Return value: Biome ID
- Return value type: `Integer`

<br>

#### Get the player's Biome Name

`pl.getBiomeName()`  

- Return value: Biome Name
- Return value type: `String`

<br>

#### Set Player Ability

`pl.setAbility(AbilityID,Value)`  
- Parameters:
  - AbilityID : `Integer`  
    Ability's ID   
  - Value : `Boolean`  
    Whether to turn on
- Return value: None
- Return value type: `Boolean`

#### Get player's effects

`pl.getAllEffects()`

- Return value: effect ID which is player owned
- Return type: `Array<number,number,...>`

<br>

#### Add an effect for player

`pl.addEffect(id, tick, level, showParticles)`
- Parameter: 
  - id : `Number`
    Effect ID
  - tick : `Number`
    Lasting time
  - level : `Number`
    Effect's level
  - showParticles : `Boolean`
    Whether to show particles
- Return value: Whether succeed
- Return type: `Boolean`

<br>

#### Remove an effect for player

`pl.removeEffect(id)`
- Parameter: 
  - id : `Number`
    Effect ID
- Return value: Whether succeed
- Return type: `Boolean`

| Name            | ID |
| --------------- | ------ |
| speed           | 1      |
| slowness        | 2      |
| haste           | 3      |
| mining_fatigue  | 4      |
| strength        | 5      |
| instant_health  | 6      |
| instant_damage  | 7      |
| jump_boost      | 8      |
| nausea          | 9      |
| regeneration    | 10     |
| resistance      | 11     |
| fire_resistance | 12     |
| water_breathing | 13     |
| invisibility    | 14     |
| blindness       | 15     |
| night_vision    | 16     |
| hunger          | 17     |
| weakness        | 18     |
| poison          | 19     |
| wither          | 20     |
| health_boost    | 21     |
| absorption      | 22     |
| saturation      | 23     |
| levitation      | 24     |
| fatal_poison    | 25     |
| conduit_power   | 26     |
| slow_falling    | 27     |
| bad_omen        | 28     |
| village_hero    | 29     |
| darkness        | 30     |