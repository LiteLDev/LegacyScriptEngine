# 🏃‍♂️ Player Related Events

#### `"onPreJoin"` - Player Connection Event

- Listener function prototype 
  `function(player)`
- Parameters: 
  - player : `Player`  
    The player that is connecting to the server.
- Intercept events: function returns `false`

Note: Only some basic information of players can be obtained in this monitoring function, such as name, IP, XUID, etc. Because the player has not fully entered the server at this time.



#### `"onJoin"` - Player Join Event

- Listener function prototype 
  `function(player)`
- Parameters: 
  - player : `Player`  
    The player that enters the game.
- Intercept event: cannot be intercepted.



#### `"onLeft"` - Player Leave Event

- Listener function prototype 
  `function(player)`
- Parameters: 
  - player : `Player`  
    The player that left the game.

- Intercept event: cannot be intercepted.



#### `"onRespawn"` - Player Respawn Event

- Listener function prototype 
  `function(player)`
- Parameters: 
  - player : `Player`  
    The player being respawned.
- Intercept event: cannot be intercepted.



#### `"onPlayerDie"` - Player Death Event

- Listener function prototype 
  `function(player,source)`
- Parameters: 
  - player : `Player`  
    The player that died.
  - source : `Entity`  
    The entity that dealt the damage that killed the player (may be `Null`).
  
- Intercept event: cannot be intercepted.



#### `"onPlayerCmd"` - Player Command Execution Event

- Listener function prototype 
  `function(player,cmd)`
- Parameters: 
  - player : `Player`  
    The player that executed the command.
  - cmd : `String`  
    The command that is being executed.

- Intercept events: function returns `false`



#### `"onChat"` - Player Chat Event

- Listener function prototype 
  `function(player,msg)`
- Parameters: 
  - player : `Player`  
    The player that sent the message.
  - msg : `String`  
    The message that was sent.

- Intercept events: function returns `false`



#### `"onChangeDim"` - Player Dimension Switch Event

- Listener function prototype 
  `function(player,dimid)`
- Parameters: 
  - player : `Player`  
    The player that switched dimensions.
  - dimid : `Integer`  
    Go to the dimension ID of the dimension, 0 is the main world, 1 is the nether, and 2 is the end.
- Intercept event: cannot be intercepted.

Reminder: This event does not fire when the player returns to the Overworld from the End via a return portal.



#### `"onJump"` - Player Jump Event

- Listener function prototype 
  `function(player)`
- Parameters: 
  - player : `Player`  
    The player that jumped.

- Intercept event: cannot be intercepted.



#### `"onSneak"` - Player Sneak Event

- Listener function prototype 
  `function(player,isSneaking)`
- Parameters: 
  - player : `Player`  
    The player that toggled their sneak state.
  - isSneaking : `Boolean`  
    `True` indicates that the player is sneaking,`False` indicates that the player is no longer sneaking.

- Intercept event: cannot be intercepted.



#### `"onAttackEntity"` - Player Attack Other Event

- Listener function prototype 
  `function(player,entity,damage)`
- Parameters: 
  - player : `Player` 
    The player that attacked an entity.
  - entity : `Entity` 
    The entity that is being attacked.

- Intercept events: function returns `false`



#### `"onAttackBlock"` - Player Attack Block Event

- Listener function prototype 
  `function(player,block,item)`

- Parameters: 

  - player : `Player` 
    The player that attacked the block.
  - entity : `Block` 
    Attacked block.
  - item: `Item`
    Item used to attack the block.

- Intercept events: function returns `false`



#### `"onUseItem"` - Player Item Use Event 

- Listener function prototype 
  `function(player,item)`
- Parameters: 
  - player : `Player`  
    The player that used the item.
  - item : `Item`  
    The item that was used.
- Intercept events: function returns `false`



#### `"onUseItemOn"` - Player Use Item on Block Event (Right-Click)

- Listener function prototype 
  `function(player,item,block,side,pos)`
- Parameters: 
  - player : `Player`  
    The player that used the item.
  - item : `Item`  
    The item being used.
  - block : `Block`  
    The block that was right-clicked.
  - side : `Number`  
    The face of the object that was clicked.  
    The faces: `0`-Down `1`-Up `2`-North `3`-South `4`-West `5`-East
  - pos : `FloatPos`
    The position that was right-clicked.

- Intercept events: function returns `false`

Note: Win10 client right-clicking on the player will trigger this event on the server multiple times in a row.



#### `"onUseBucketPlace"` - Players use bucket to pour things out on Block Event

- Listener function prototype 
  `function(player,item,block,side,pos)`
- Parameters：
  - player : `Player`  
    The player that used the item.
  - item : `Item`  
    The item being used.
  - block : `Block`  
    The block that was right-clicked.
  - side : `Number`  
    The face of the object that was clicked.   
    The faces: `0`-Down `1`-Up `2`-North `3`-South `4`-West `5`-East
  - pos : `FloatPos`  
    The position that was right-clicked.

- Intercept events: function returns `false`

Note: The player may trigger this event on the server multiple times in a row.



#### `"onUseBucketTake"` - Players use bucket to pack in things Event

- Listener function prototype 
  `function(player,item,target,side,pos)`
- Parameters：
  - player : `Player`  
    The player that used the item.
  - item : `Item`  
    The item being used.
  - target : `Block` / `Entity`  
    The block or entity that was right-clicked.
  - side : `Number`  
    The face of the object that was clicked.   
    The faces: `0`-Down `1`-Up `2`-North `3`-South `4`-West `5`-East
  - pos : `FloatPos`  
    The position that was right-clicked.

- Intercept events: function returns `false`

Note: The player may trigger this event on the server multiple times in a row.



#### `"onTakeItem"` - Player Pickup Item Event

- Listener function prototype 
  `function(player,entity,item)`
- Parameters: 

  - player : `Player`  
    The player that picked up the item.
  - entity: `Entity`  
    The dropped entity of the item about to be picked up.
  - item : `Item`  
    The item about to be picked up.

- Intercept events: function returns `false`



#### `"onDropItem"` - Player Drop Item Event

- Listener function prototype 
  `function(player,item)`
- Parameters: 
  - player : `Player`  
    The player that dropped the item.
  - item : `Item`  
    The item being dropped.

- Intercept events: function returns `false`



#### `"onEat"` - Player Eating Event

- Listener function prototype 
  `function(player,item)`
- Parameters: 
  - player : `Player`  
    The player that is eating.
  - item : `Item`  
    The item being eaten.
  
- Intercept events: function returns `false`

**Food** here is a broad concept of items, including conventional food, potions, milk, medicines and other items that can be ingested.



#### `"onAte"` - Player Ate Event

- Listener function prototype 
  `function(player,item)`
- Parameters: 
  - player : `Player`  
    The player that has eaten.
  - item : `Item`  
    The item which has been eaten.
  
- Intercept events: function returns `false`

#### `"onConsumeTotem"` - Player Consume Totem Event

- Listener function prototype 
  `function(player)`
- Parameters: 
  - player : `Player`  
    The player that consumes the totem.
- Intercept events: function returns `false`
  - After intercepting here, the resurrection effect of the totem will still be triggered, but the totem will not be consumed.



#### `"onEffectAdded"` - Player Effect Added Event

- Listener function prototype 
  `function(player,effectName,amplifier,duration)`
- Parameters: 
  - player : `Player`  
    The player who gets the effect.
  - effectName : `String`  
    Obtained effect name: **minecraft:effect.EffectName**
  - amplifier : `Number` 
    Obtained effect amplifier (effect level -1)
  - duration : `Number` 
    Obtained effect duration (ticks)
  
- Intercept events: function returns `false`



#### `"onEffectRemoved"` - Player Effect Removed Event

- Listener function prototype 
  `function(player,effectName)`
- Parameters: 
  - player : `Player`  
    Player with the removed effect.
  - effectName : `String`   
    Removed effect name: **minecraft:effect.EffectName**
  
- Intercept events: function returns `false`



#### `"onEffectUpdated"` - Player Effect Updated Event

- Listener function prototype 
  `function(player,effectName,amplifier,duration)`
- Parameters: 
  - player : `Player`  
    The player that updated the effect.
  - effectName : `String`   
    Refreshed effect name: **minecraft:effect.EffectName**
  - amplifier : `Number` 
    Obtained effect amplifier (effect level -1)
  - duration : `Number` 
    Obtained effect duration (ticks)
  
- Intercept events: function returns `false`



#### `"onStartDestroyBlock"` - Player Start Breaking Block Event

- Listener function prototype 
  `function(player,block)`
- Parameters: 
  - player : `Player`  
    The player that is breaking the block.
  - block : `Block`  
    The block that is being destroyed.

- Intercept event: cannot be intercepted.



#### `"onDestroyBlock"` - Player Destroyed Block Event

- Listener function prototype 
  `function(player,block)`
- Parameters: 
  - player : `Player`  
    The player that broke the block.
  - block : `Block`  
    The broken block.

- Intercept events: function returns `false`



#### `"onPlaceBlock"` - Player Try Places Block Event

- Listener function prototype 
  `function(player,block,face)`
- Parameters: 
  - player : `Player`  
    The player that placed the block.
  - block : `Block`  
    The block that was placed on.
  - face : `Integer`  
    The face that was placed on.

- Intercept events: function returns `false`

> **ATTENTION** This event will always fire when the player tries to place a block.



#### `"afterPlaceBlock"` - Player Placed Block Event

- Listener function prototype 
  `function(player,block)`
- Parameters: 
  - player : `Player`  
    The player that placed the block.
  - block : `Block`  
    The block that was placed.

- Intercept events: function returns `false`



#### `"onOpenContainer"` - Player Opens Container Event

- Listener function prototype 
  `function(player,block)`
- Parameters: 
  - player : `Player`  
    The player that opened the container.
  - block : `Block`  
    The opened container block.
- Intercept events: function returns `false`

The **container** here is a broad concept of container, including boxes, buckets and other containers that can store items can trigger this event.



#### `"onCloseContainer"` - Player Closes Container Event

- Listener function prototype 
  `function(player,block)`
- Parameters: 
  - player : `Player`  
    The player that closes the container.
  - block : `Block`  
    The container that was closed.
- Intercept events: function returns `false`

Due to the limitation of the monitoring function, the containers that currently support monitoring and closing are: chests (`minecraft:chest`), and wooden barrels (`minecraft:barrel`).



#### `"onInventoryChange"` - Player Inventory Change Event

- Listener function prototype 
  `function(player,slotNum,oldItem,newItem)`
- Parameters: 
  - player : `Player`  
    The player whose inventory changed.
  - slotNum : `Integer`  
    The slot position of the inventory operation.
  - oldItem : `Item`  
    The original item in the grid.
  - newItem : `Item`  
    The new item in the grid.
- Intercept event: cannot be intercepted.

Explanation of callback parameters:  
There are many different combinations of old item objects and new item objects, indicating different changes in the grid.

- Put item: the old item object is empty, the new item object is not empty.
- Take out the item: the old item object is not empty, the new item object is empty.
- Item Increase Stack: Old Item Object's `type` == new item object's `type`, old item's `count` < new item's `count`.
- Item Reduce Stack: Old Item Object's `type` == new item object's `type`, old item's `count` > new item's `count`.
Replacement Item: Old Item Object's `type` does not equal the new item's `type`, and neither item stack is empty.



#### `"onChangeSprinting"` - Player Sprint State Change Event

- Listener function prototype 
  `function(player,sprinting)`
- Parameters: 
  - player : `Player`  
    The player that started or stopped sprinting.
  - sprinting : `Boolean`  
    Whether the player is now sprinting.
- Intercept event: cannot be intercepted.

Note: Player.setSprinting (false) can be executed in the next game tick to achieve the interception effect.



#### `"onSetArmor"` - Player Armor Change Event

- Listener function prototype 
  `function(player,slotNum,item)`
- Parameters: 
  - player : `Player`  
    Player object that changes armor.
  - slotNum : `Integer`  
    The armor column number, range from 0 to 3.
  - item : `Item`  
    The item in the armor slot.
- Intercept event: function returns `false`
- Warning: After interception, you will take off your original equipment when you enter the game.



#### `"onUseRespawnAnchor"` - Player Respawn Anchor Use Event

- Listener function prototype 
  `function(player,pos)`
- Parameters: 
  - player : `Player`  
    The player using the respawn anchor.
  - pos : `IntPos`  
    The position of the respawn anchor that was used.
- Intercept events: function returns `false`



#### `"onOpenContainerScreen"` - Player Opens Container GUI Event

- Listener function prototype 
  `function(player)`
- Parameters: 
  - player : `Player`  
    The player that opened the GUI.
- Intercept events: function returns `false`

Note: This event is so powerful that it can even intercept and open backpacks.



#### `"onExperienceAdd"` - Player Get Experience

- Listener function prototype 
  `function(player,exp)`
- Parameters：
  - player : `Player`  
    Players with experience.
  - exp : `Integer`
    Amount of experience gained by the player.
- Intercept events: function returns `false`



#### `"onPlayerPullFishingHook"` - Player Pull Closer Entity Using Fishing Hook 

- Listener function prototype
  `function(player,entity,item)`
- Parameters:
  - player : `Player`  
    Player using fishing hook.
  - entity : `Entity`  
    Entity that player pull closer
  - item : `Item`  
    Item that player pull closer（If this entity is not item entity, this parameter will be null）
- Intercept events: function returns `false`



#### `"onBedEnter"` - Player Enters Bed

- Listener function prototype 
  `function(player,pos)`
- Parameters：
  - player : `Player`  
    The player using the bed.
  - pos : `IntPos`  
    The position of the bed used.
- Intercept events: function returns `false`
