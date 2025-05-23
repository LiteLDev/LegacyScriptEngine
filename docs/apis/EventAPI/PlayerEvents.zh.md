# 🏃‍♂️ 玩家相关事件

#### `"onPreJoin"` - 玩家开始连接服务器

- 监听函数原型
  `function(player)`
- 参数：
  - player : `Player`  
    正在连接服务器的玩家对象
- 拦截事件：函数返回`false`

注：在这个监听函数中只能获取一些玩家的基础信息，比如名字、IP、XUID等。因为此时玩家尚未完全进服



#### `"onJoin"` - 玩家进入游戏（加载世界完成）

- 监听函数原型
  `function(player)`
- 参数：
  - player : `Player`  
    进入游戏的玩家对象
- 拦截事件：不可以拦截



#### `"onLeft"` - 玩家离开游戏

- 监听函数原型
  `function(player)`
- 参数：
  - player : `Player`  
    离开游戏的玩家对象

- 拦截事件：不可以拦截



#### `"onRespawn"` - 玩家重生

- 监听函数原型
  `function(player)`
- 参数：
  - player : `Player`  
    重生的玩家对象
- 拦截事件：不可以拦截



#### `"onPlayerDie"` - 玩家死亡

- 监听函数原型
  `function(player,source)`
- 参数：
  - player : `Player`  
    死亡的玩家对象
  - source : `Entity`  
    伤害来源的实体对象（可能为`Null`）
  
- 拦截事件：不可以拦截



#### `"onPlayerCmd"` - 玩家执行命令

- 监听函数原型
  `function(player,cmd)`
- 参数：
  - player : `Player`  
    执行命令的玩家对象

  - cmd : `String`  
    执行的命令

- 拦截事件：函数返回`false`



#### `"onChat"` - 玩家发送聊天信息

- 监听函数原型
  `function(player,msg)`
- 参数：
  - player : `Player`  
    发送聊天信息的玩家对象

  - msg : `String`  
    发送的聊天消息

- 拦截事件：函数返回`false`



#### `"onChangeDim"` - 玩家切换维度

- 监听函数原型
  `function(player,dimid)`
- 参数：
  - player : `Player`  
    切换维度的玩家对象
  - dimid : `Integer`  
    前往维度的维度ID，0为主世界，1为下界，2为末地
- 拦截事件：不可以拦截

提醒：当玩家从末地通过返回传送门返回主世界时，不会触发此事件。



#### `"onJump"` - 玩家跳跃

- 监听函数原型
  `function(player)`
- 参数：
  - player : `Player`  
    跳跃的玩家对象

- 拦截事件：不可以拦截



#### `"onSneak"` - 玩家切换潜行状态

- 监听函数原型
  `function(player,isSneaking)`
- 参数：
  - player : `Player`  
    切换潜行状态的玩家对象
  - isSneaking : `Boolean`  
    `True`表示玩家进入潜行状态，`False`表示玩家退出潜行状态

- 拦截事件：不可以拦截



#### `"onAttackEntity"` - 玩家攻击实体

- 监听函数原型
  `function(player,entity)`
- 参数：
  - player : `Player` 
    攻击实体的玩家对象

  - entity : `Entity` 
    被攻击的实体对象

- 拦截事件：函数返回`false`



#### `"onAttackBlock"` - 玩家攻击方块

- 监听函数原型
  `function(player,block,item)`

- 参数：

  - player : `Player` 
    攻击方块的玩家对象

  - entity : `Block` 
    被攻击的方块对象

  - item：`Item`

    手持的物品对象

- 拦截事件：函数返回`false`



#### `"onUseItem"` - 玩家使用物品 

- 监听函数原型
  `function(player,item)`
- 参数：
  - player : `Player`  
    使用物品的玩家对象
  - item : `Item`  
    被使用的物品对象
- 拦截事件：函数返回`false`



#### `"onUseItemOn"` - 玩家对方块使用物品（点击右键）

- 监听函数原型
  `function(player,item,block,side,pos)`
- 参数：
  - player : `Player`  
    使用物品的玩家对象
  - item : `Item`  
    被使用的物品对象
  - block : `Block`  
    被点击到的方块对象
  - side : `Number`  
    方块被点击的面  
    依次为：`0`-下 `1`-上 `2`-北 `3`-南 `4`-西 `5`-东
  - pos : `FloatPos`  
    被点击的浮点位置

- 拦截事件：函数返回`false`

注：Win10客户端玩家右键会在服务端连续多次激发这个事件



#### `"onUseBucketPlace"` - 玩家使用桶倒出东西

- 监听函数原型
  `function(player,item,block,side,pos)`
- 参数：
  - player : `Player`  
    使用物品的玩家对象
  - item : `Item`  
    被使用的物品对象
  - block : `Block`  
    被点击到的方块对象
  - side : `Number`  
    方块被点击的面  
    依次为：`0`-下 `1`-上 `2`-北 `3`-南 `4`-西 `5`-东
  - pos : `FloatPos`  
    被点击的浮点位置

- 拦截事件：函数返回`false`

注：无论是手机还是Win10玩家，可能会连续多次激发这个事件



#### `"onUseBucketTake"` - 玩家使用桶装进东西

- 监听函数原型
  `function(player,item,target,side,pos)`
- 参数：
  - player : `Player`  
    使用物品的玩家对象
  - item : `Item`  
    被使用的物品对象
  - target : `Block` / `Entity`  
    被点击到的方块对象或者实体对象
  - side : `Number`  
    方块被点击的面  
    依次为：`0`-下 `1`-上 `2`-北 `3`-南 `4`-西 `5`-东
  - pos : `FloatPos`  
    被点击的浮点位置

- 拦截事件：函数返回`false`

注：无论是手机还是Win10玩家，可能会连续多次激发这个事件



#### `"onTakeItem"` - 玩家捡起物品

- 监听函数原型
  `function(player,entity,item)`
- 参数：

  - player : `Player`  
    捡起物品的玩家对象
  - entity: `Entity`  
    即将被捡起的物品的掉落物实体
  - item : `Item`  
    即将被捡起的物品对象

- 拦截事件：函数返回`false`



#### `"onDropItem"` - 玩家丢出物品

- 监听函数原型
  `function(player,item)`
- 参数：
  - player : `Player`  
    丢出物品的玩家对象

  - item : `Item`  
    被丢出的物品对象

- 拦截事件：函数返回`false`



#### `"onEat"` - 玩家正在吃食物

- 监听函数原型
  `function(player,item)`
- 参数：
  - player : `Player`  
    正在吃的玩家对象
  - item : `Item`  
    被吃的物品对象
  
- 拦截事件：函数返回`false`

此处的 **食物** 为宽泛物品的概念，包括常规食物、药水、牛奶、药品等多种可以被摄取的物品



#### `"onAte"` - 玩家吃下食物

- 监听函数原型
  `function(player,item)`
- 参数：
  - player : `Player`  
    正在吃的玩家对象
  - item : `Item`  
    被吃的物品对象
  
- 拦截事件：不可以拦截



#### `"onConsumeTotem"` - 玩家消耗图腾

- 监听函数原型
  `function(player)`
- 参数：
  - player : `Player`  
    消耗图腾的玩家对象
- 拦截事件：函数返回`false`
  - 此处拦截后,仍会触发图腾的复活效果,但不会消耗图腾



#### `"onEffectAdded"` - 玩家获得效果

- 监听函数原型
  `function(player,effectName,amplifier,duration)`
- 参数：
  - player : `Player`  
    获得效果的玩家对象
  - effectName : `String`  
    获得的效果名称 **minecraft:effect.效果**
  - amplifier : `Number`  
    获得的效果倍率 （效果等级 -1）
  - duration : `Number`  
    获得的效果时长 （单位：tick）
  
- 拦截事件：函数返回`false`



#### `"onEffectRemoved"` - 玩家移除效果

- 监听函数原型
  `function(player,effectName)`
- 参数：
  - player : `Player`  
    被移除效果的玩家对象
  - effectName : `String`   
    被移除的效果名称 **minecraft:effect.效果**
  
- 拦截事件：函数返回`false`



#### `"onEffectUpdated"` - 玩家刷新效果

- 监听函数原型
  `function(player,effectName,amplifier,duration)`
- 参数：
  - player : `Player`  
    刷新效果的玩家对象
  - effectName : `String`  
    获得的效果名称 **minecraft:effect.效果**
  - amplifier : `Number`  
    获得的效果倍率 （效果等级 -1）
  - duration : `Number`  
    获得的效果时长 （单位：tick）
  
- 拦截事件：函数返回`false`



#### `"onStartDestroyBlock"` - 玩家开始破坏方块  / 点击左键

- 监听函数原型
  `function(player,block)`
- 参数：
  - player : `Player`  
    正在破坏方块的玩家对象

  - block : `Block`  
    正在被破坏的方块对象

- 拦截事件：不可以拦截



#### `"onDestroyBlock"` - 玩家破坏方块完成

- 监听函数原型
  `function(player,block)`
- 参数：
  - player : `Player`  
    破坏方块的玩家对象

  - block : `Block`  
    被破坏的方块对象

- 拦截事件：函数返回`false`



#### `"onPlaceBlock"` - 玩家尝试放置方块

- 监听函数原型
  `function(player,block,face)`
- 参数：
  - player : `Player`  
    放置方块的玩家对象

  - block : `Block`  
    将要放置的方块对象

  - face : `Integer`  
    被放置的方块的朝向。

- 拦截事件：函数返回`false`

> **存在问题** 当玩家尝试放置方块时，该事件将持续被触发。



#### `"afterPlaceBlock"` - 玩家放置方块

- 监听函数原型
  `function(player,block)`
- 参数：
  - player : `Player`  
    放置方块的玩家对象

  - block : `Block`  
    被放置的方块对象

- 拦截事件：不可以拦截



#### `"onOpenContainer"` - 玩家打开容器方块

- 监听函数原型
  `function(player,block)`
- 参数：
  - player : `Player`  
    打开容器方块的玩家对象

  - block : `Block`  
    被打开的容器方块对象
- 拦截事件：函数返回`false`

此处的 **容器** 为宽泛容器的概念，包括箱子、桶等多种可以储存物品的容器都可以触发此事件



#### `"onCloseContainer"` - 玩家关闭容器方块

- 监听函数原型
  `function(player,block)`
- 参数：
  - player : `Player`  
    关闭容器方块的玩家对象

  - block : `Block`  
    被关闭的容器方块对象
- 拦截事件：函数返回`false`

由于监听函数下限制，目前支持监听关闭的容器有：箱子（`minecraft:chest`）、木桶（`minecraft:barrel`）



#### `"onInventoryChange"` - 玩家物品栏变化

- 监听函数原型
  `function(player,slotNum,oldItem,newItem)`
- 参数：
  - player : `Player`  
    操作物品栏的玩家对象
  - slotNum : `Integer`  
    操作物品栏的格子位置（第slotNum个格子）
  - oldItem : `Item`  
    格子中的原来旧物品对象
  - newItem : `Item`  
    格子中新的物品对象
- 拦截事件：不可以拦截

对回调参数的解释：  
旧物品对象与新物品对象有多种不同的组合情况，表示格子内不同的变化情况

- 放入物品：旧物品对象为空，新物品对象不为空
- 取出物品：旧物品对象不为空，新物品对象为空
- 物品增加堆叠：旧物品对象的`type` == 新物品对象的`type`，且旧物品对象的`count` < 新物品对象的`count`
- 物品减少堆叠：旧物品对象的`type` == 新物品对象的`type`，且旧物品对象的`count` > 新物品对象的`count`
- 替换物品：旧物品对象的`type` 不等于 新物品对象的`type`，且两物品对象均不为空



#### `"onChangeSprinting"` - 玩家改变疾跑状态

- 监听函数原型
  `function(player,sprinting)`
- 参数：
  - player : `Player`  
    要改变疾跑状态的玩家对象
  - sprinting : `Boolean`  
    要改变成的疾跑状态
- 拦截事件：不可以拦截

注：可在下一游戏刻执行player.setSprinting(false)达到拦截效果



#### `"onSetArmor"` - 玩家改变盔甲栏

- 监听函数原型
  `function(player,slotNum,item)`
- 参数：
  - player : `Player`  
    改变盔甲栏的玩家对象
  - slotNum : `Integer`  
    盔甲栏序号，范围0-3
  - item : `Item`  
    盔甲栏中的物品对象
- 拦截事件：函数返回`false`
- 注:拦截后，进入游戏时会脱下原先的装备



#### `"onUseRespawnAnchor"` - 玩家使用重生锚

- 监听函数原型
  `function(player,pos)`
- 参数：
  - player : `Player`  
    使用重生锚的玩家对象
  - pos : `IntPos`  
    被使用的重生锚的位置
- 拦截事件：函数返回`false`



#### `"onOpenContainerScreen"` - 玩家打开容器类GUI

- 监听函数原型
  `function(player)`
- 参数：
  - player : `Player`  
    尝试打开GUI的玩家对象
- 拦截事件：函数返回`false`

注：此事件非常强力，甚至可以拦截打开背包。



#### `"onExperienceAdd"` - 玩家获得经验

- 监听函数原型
  `function(player,exp)`
- 参数：
  - player : `Player`  
    获得经验的玩家对象
  - exp : `Integer`
    获得的经验值
- 拦截事件：函数返回`false`



#### `"onPlayerPullFishingHook"` - 玩家使用钓鱼竿钓起实体

- 监听函数原型
  `function(player,entity,item)`
- 参数：
  - player : `Player`  
    使用钓鱼竿的玩家对象
  - entity : `Entity`  
    钓起的实体（鱼钩拉起任意实体都会触发该事件，不一定是物品实体）
  - item : `Item`  
    钓起的物品对象（如果钓到的不是物品则返回null）
- 拦截事件：函数返回`false`



#### `"onBedEnter"` - 玩家上床

- 监听函数原型
  `function(player,pos)`
- 参数：
  - player : `Player`  
    上床的玩家对象
  - pos : `IntPos`  
    床的位置
- 拦截事件：函数返回`false`



#### `"onPlayerInteractEntity"` - 玩家交互实体

!!! warning
    此事件仅在0.9.5及以后版本可用

- 监听函数原型
  `function(player, entity, pos)`
- 参数：
  - player : `Player`  
    交互实体的玩家对象
  - entity : `Entity`  
    被交互的实体对象
  - pos : `FloatPos`  
    交互坐标
- 拦截事件：函数返回`false`
