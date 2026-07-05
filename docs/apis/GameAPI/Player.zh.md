# 🏃‍♂️ 玩家对象 API

在脚本引擎中，使用「玩家对象」来操作和获取某一个玩家的相关信息。

### 获取一个玩家对象

#### 从事件或API获取

通过注册**事件监听**函数，获取到BDS给出的与相关事件有关的玩家对象  
详见 [事件监听文档 - EventAPI](../EventAPI/Listen.zh.md)

#### 从现有玩家获取

通过**玩家信息**手动生成玩家对象  
通过此函数来手动生成对象，注意，你要获取的玩家必须是在线状态，否则会生成失败

`mc.getPlayer(info)`

!!! warning
    0.8.13之前的版本无法使用UUID获取玩家  
    0.9.5之前的版本无法使用RuntimeId获取玩家

- 参数：
    - info : `String`  
      玩家的名字、XUID、UniqueId、RuntimeId或者UUID
- 返回值：生成的玩家对象
- 返回值类型：`Player`
    - 如返回值为 `Null` 则表示获取玩家失败

#### 获取所有在线玩家

此函数会返回一个玩家对象的数组，其中每个对象都对应了一个服务器中的玩家

`mc.getOnlinePlayers()`

- 返回值：在线的玩家对象列表
- 返回值类型：`Array<Player,Player,...>`

> 注意：不要**长期保存**一个玩家对象  
> 当玩家退出服务器时，对应的玩家对象将同时释放。因此，如果有长期操作某个玩家的需要，请通过上述途径获取实时的玩家对象

### 玩家对象 - 属性

每一个玩家对象都包含一些固定的对象属性。对于某个特定的玩家对象`pl`，有以下这些属性

| 属性                       | 含义                          | 类型               |
|--------------------------|-----------------------------|------------------|
| pl.name                  | 玩家名                         | `String`         |
| pl.pos                   | 玩家所在坐标                      | `FloatPos`       |
| pl.feetPos               | 玩家腿部所在坐标                    | `FloatPos`       |
| pl.blockPos              | 玩家所在的方块坐标                   | `IntPos`         |
| pl.lastDeathPos          | 玩家上次死亡的坐标                   | `IntPos`         |
| pl.realName              | 玩家的真实名字                     | `String`         |
| pl.xuid                  | 玩家 XUID 字符串                 | `String`         |
| pl.uuid                  | 玩家 Uuid 字符串                 | `String`         |
| pl.permLevel             | 玩家的操作权限等级（0 - 4）            | `Integer`        |
| pl.gameMode              | 玩家的游戏模式（0 - 2, 6）           | `Integer`        |
| pl.canFly                | 玩家是否可以飞行                    | `Boolean`        |
| pl.canSleep              | 玩家是否可以睡觉                    | `Boolean`        |
| pl.canBeSeenOnMap        | 玩家是否可以在地图上看到                | `Boolean`        |
| pl.canFreeze             | 玩家是否可以冻结                    | `Boolean`        |
| pl.canSeeDaylight        | 玩家是否能看到日光                   | `Boolean`        |
| pl.canShowNameTag        | 玩家是否可以显示姓名标签                | `Boolean`        |
| pl.canStartSleepInBed    | 玩家是否可以开始在床上睡觉               | `Boolean`        |
| pl.canPickupItems        | 玩家是否可以拾取物品                  | `Boolean`        |
| pl.maxHealth             | 玩家最大生命值                     | `Integer`        |
| pl.health                | 玩家当前生命值                     | `Integer`        |
| pl.inAir                 | 玩家当前是否悬空                    | `Boolean`        |
| pl.inWater               | 玩家当前是否在水中                   | `Boolean`        |
| pl.inLava                | 玩家是否在熔岩中                    | `Boolean`        |
| pl.inRain                | 玩家是否下雨                      | `Boolean`        |
| pl.inSnow                | 玩家是否在雪中                     | `Boolean`        |
| pl.inWall                | 玩家是否在墙上                     | `Boolean`        |
| pl.inWaterOrRain         | 玩家是否在水中或雨中                  | `Boolean`        |
| pl.inWorld               | 玩家是否在世界                     | `Boolean`        |
| pl.inClouds              | 玩家是否在云端                     | `Boolean`        |
| pl.speed                 | 玩家当前速度                      | `Float`          |
| pl.direction             | 玩家当前朝向                      | `DirectionAngle` |
| pl.uniqueId              | 玩家（实体的）唯一标识符                | `String`         |
| pl.runtimeId             | 玩家（实体的）运行时标识符(在 0.9.5 时被加入) | `String`         |
| pl.langCode              | 玩家设置的语言的标识符(形如 zh_CN)       | `String`         |
| pl.isLoading             | 玩家是否正在加载                    | `Boolean`        |
| pl.isInvisible           | 玩家是否隐身中                     | `Boolean`        |
| pl.isInsidePortal        | 玩家在传送门中                     | `Boolean`        |
| pl.isHurt                | 玩家是否受伤                      | `Boolean`        |
| pl.isTrusting            | 未知                          | `Boolean`        |
| pl.isTouchingDamageBlock | 玩家是否在能造成伤害的方块上              | `Boolean`        |
| pl.isHungry              | 玩家是否饿了                      | `Boolean`        |
| pl.isOnFire              | 玩家是否着火                      | `Boolean`        |
| pl.isOnGround            | 玩家是否在地上                     | `Boolean`        |
| pl.isOnHotBlock          | 玩家是否在高温方块上（岩浆等）             | `Boolean`        |
| pl.isTrading             | 玩家在交易                       | `Boolean`        |
| pl.isAdventure           | 玩家是否是冒险模式                   | `Boolean`        |
| pl.isGliding             | 玩家在滑行                       | `Boolean`        |
| pl.isSurvival            | 玩家是否是生存模式                   | `Boolean`        |
| pl.isSpectator           | 玩家是否是观众模式                   | `Boolean`        |
| pl.isRiding              | 玩家是否在骑行                     | `Boolean`        |
| pl.isDancing             | 玩家在跳舞？                      | `Boolean`        |
| pl.isCreative            | 玩家是否是创造模式                   | `Boolean`        |
| pl.isFlying              | 玩家是否在飞行                     | `Boolean`        |
| pl.isSleeping            | 玩家是否正在睡觉                    | `Boolean`        |
| pl.isMoving              | 玩家是否正在移动                    | `Boolean`        |
| pl.isSneaking            | 玩家是否正在潜行                    | `Boolean`        |
| pl.isSwimming            | 玩家是否正在游泳（在 0.19.1 时被加入）         | `Boolean`        |
| pl.isCrawling            | 玩家是否正在爬行（在 0.19.1 时被加入）         | `Boolean`        |

这些对象属性都是只读的，无法被修改。其中：

- **坐标** 和 **腿部坐标**：玩家为两格高，`pos`为玩家视角高度的坐标，`feetPos`为游戏内显示的方块坐标
- **玩家游戏模式** 属性的取值为：`0` 代表生存模式，`1` 代表创造模式，`2` 代表冒险模式，`3` 代表旁观者模式
- **玩家真实名字** 属性储存的字符串可以被认为是可靠的，他们不会被改名而变动
- **玩家设备IP地址** 属性储存了玩家的设备IP以及端口号，格式类似`12.34.567.89:1111`
- **玩家当前朝向** 属性的详细解释见  [基础游戏接口文档](./Basic.zh.md)
- **操作权限等级** 属性的对照表如下：

| 操作权限等级 | 对应操作权限     |
|--------|------------|
| 0      | 普通成员权限     |
| 1      | OP权限       |
| 4      | OP + 控制台权限 |

### 玩家对象 - 函数

每一个玩家对象都包含一些可以执行的成员函数（成员方法）。对于某个特定的玩家对象`pl`，可以通过以下这些函数对这个玩家进行一些操作

#### 判断玩家是否为OP

`pl.isOP()`

- 返回值：玩家是否为OP
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      var open = pl.isOP();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      open = pl:isOP()
      ```

#### 断开玩家连接

`pl.kick([msg])`  
`pl.disconnect([msg])`

- 参数：
    - msg : `String`  
      （可选参数）被踢出玩家出显示的断开原因。  
      如果不传入，默认为“正在从服务器断开连接”
- 返回值：是否成功断开连接
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.kick();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:kick()
      ```

#### 发送一个文本消息给玩家

`pl.tell(msg[,type])`  
`pl.sendText(msg[,type])`

- 参数：

    - msg : `String`  
      待发送的文本

    - type : `Integer`  
      （可选参数）发送的文本消息类型，默认为0

| type参数 | 消息类型           |
|--------|----------------|
| 0      | 普通消息（Raw）      |
| 1      | 聊天消息（Chat）     |
| 4      | 音乐盒消息（Popup）   |
| 5      | 物品栏上方的消息（Tip）  |
| 9      | JSON格式消息（JSON） |

- 返回值：是否成功发送

- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      //对于一个玩家对象pl
      pl.tell("Welcome back ~ ", 5);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:tell("Welcome back ~ ", 5)
      ```

#### 设置玩家显示标题

`pl.setTitle(content[,type[,fadeInTime,stayTime,fadeOutTime]])`

- 参数：

    - content : `String`  
      欲设置标题内容

    - type : `Integer`  
      （可选参数）设置的标题类型，默认为2
    
    | type参数 | 消息类型                                |
    |--------|-------------------------------------|
    | 0      | 清空（Clear）                           |
    | 1      | 重设（Reset）                           |
    | 2      | 设置主标题（SetTitle）                     |
    | 3      | 设置副标题（SetSubTitle）                  |
    | 4      | 设置Actionbar（SetActionBar）           |
    | 5      | 设置显示时间（SetDurations）                |
    | 6      | Json型主标题（TitleTextObject）           |
    | 7      | Json型副标题（SubtitleTextObject）        |
    | 8      | Json型Actionbar（ActionbarTextObject） |

    - fadeInTime : `Integer`  
      （可选参数）淡入时间，单位为 `Tick` ，默认为10

    - stayTime: `Integer`

      （可选参数）停留时间，单位为 `Tick` ，默认为70

    - fadeOutTime:`Integer`

      （可选参数）淡出时间，单位为 `Tick`，默认为20

- 返回值：是否成功发送

- 返回值类型：`Boolean`

#### 广播一个文本消息给所有玩家

`mc.broadcast(msg[,type])`

- 参数：

    - msg : `String`  
      待发送的文本

    - type : `Integer`  
      （可选参数）发送的文本消息类型，默认为0

| type参数 | 消息类型           |
|--------|----------------|
| 0      | 普通消息（Raw）      |
| 1      | 聊天消息（Chat）     |
| 5      | 物品栏上方的消息（Tip）  |
| 9      | JSON格式消息（JSON） |

- 返回值：是否成功发送

- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      mc.broadcast("Hello everyone ~ ");
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      mc.broadcast("Hello everyone ~ ")
      ```

#### 在屏幕上方显示消息(类似于成就完成)

`pl.sendToast(title,message)`

- 参数：

    - title : `String`  
      待发送的标题

    - message : `String`  
      待发送的文本

- 返回值：是否成功发送

- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.sendToast("Hello", "everyone ~");
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:sendToast("Hello", "everyone ~")
      ```

#### 以某个玩家身份执行一条命令

`pl.runcmd(cmd)`

- 参数：
    - cmd : `String`  
      待执行的命令
- 返回值：是否执行成功
- 返回值类型： `Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.runcmd("tp ~ ~50 ~");
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:runcmd("tp ~ ~50 ~")
      ```

#### 以某个玩家身份说话

`pl.talkAs(text)`

- 参数：
    - text : `String`  
      模拟说话内容
- 返回值：是否执行成功
- 返回值类型： `Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.talkAs("Hello everyone ~ ");
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:talkAs("Hello everyone ~ ")
      ```

#### 获取玩家到坐标的距离

`pl.distanceTo(pos)`
`pl.distanceToSqr(pos)`

- 参数:
    - pos : `Entity` / `Player` / `IntPos` / `FloatPos`
      目标位置
- 返回值: 到坐标的距离(方块)
- 返回值类型:  `Number`

> **注意** 若玩家的坐标与目标的坐标不在同一维度，将返回整数最大值。

#### 以某个玩家身份向某玩家说话

`pl.talkTo(text,target)`

- 参数：
    - text : `String`  
      模拟说话内容
    - target : `Player`  
      模拟说话对象
- 返回值：是否执行成功
- 返回值类型： `Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.talkAs(anotherpl, "Hello ~ ");
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:talkAs(anotherpl, "Hello everyone ~ ")
      ```

#### 传送玩家至指定位置

`pl.teleport(pos[,rot])`
`pl.teleport(x,y,z,dimid[,rot])`

- 参数：
    - pos: `IntPos `/ `FloatPos`
      目标位置坐标 （或者使用x, y, z, dimid来确定玩家位置）

    - rot: `DirectionAngle`

      （可选参数）传送后玩家的朝向，若缺省则与传送前朝向相同

- 返回值：是否成功传送

- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      //对于一个玩家对象pl
      pl.teleport(pos);
      ```
    - Lua
      ```lua
      pl:teleport(pos)
      ```

#### 杀死玩家

`pl.kill()`

- 返回值：是否成功执行
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.kill();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:kill()
      ```

#### 对玩家造成伤害

`pl.hurt(damage,type,source)`

- 参数：
    - damage : `Float`  
      对玩家造成的伤害数值
    - type : `Integer`  
      伤害类型
    - source : `Entity`
      伤害来源
- 返回值：是否造成伤害
- 返回值类型：`Boolean`

注意，此处造成的伤害为真实伤害，无法被盔甲等保护装备减免

| `ActorDamageCause` 枚举            | 值 |
| ---------------------------------- | --- |
| `ActorDamageCause.None`            | -1 |
| `ActorDamageCause.Override`        | 0 |
| `ActorDamageCause.Contact`         | 1 |
| `ActorDamageCause.EntityAttack`    | 2 |
| `ActorDamageCause.Projectile`      | 3 |
| `ActorDamageCause.Suffocation`     | 4 |
| `ActorDamageCause.Fall`            | 5 |
| `ActorDamageCause.Fire`            | 6 |
| `ActorDamageCause.FireTick`        | 7 |
| `ActorDamageCause.Lava`            | 8 |
| `ActorDamageCause.Drowning`        | 9 |
| `ActorDamageCause.BlockExplosion`  | 10 |
| `ActorDamageCause.EntityExplosion` | 11 |
| `ActorDamageCause.Void`            | 12 |
| `ActorDamageCause.SelfDestruct`    | 13 |
| `ActorDamageCause.Magic`           | 14 |
| `ActorDamageCause.Wither`          | 15 |
| `ActorDamageCause.Starve`          | 16 |
| `ActorDamageCause.Anvil`           | 17 |
| `ActorDamageCause.Thorns`          | 18 |
| `ActorDamageCause.FallingBlock`    | 19 |
| `ActorDamageCause.Piston`          | 20 |
| `ActorDamageCause.FlyIntoWall`     | 21 |
| `ActorDamageCause.Magma`           | 22 |
| `ActorDamageCause.Fireworks`       | 23 |
| `ActorDamageCause.Lightning`       | 24 |
| `ActorDamageCause.Charging`        | 25 |
| `ActorDamageCause.Temperature`     | 26 |
| `ActorDamageCause.Freezing`        | 27 |
| `ActorDamageCause.Stalactite`      | 28 |
| `ActorDamageCause.Stalagmite`      | 29 |
| `ActorDamageCause.RamAttack`       | 30 |
| `ActorDamageCause.SonicBoom`       | 31 |
| `ActorDamageCause.Campfire`        | 32 |
| `ActorDamageCause.SoulCampfire`    | 33 |
| `ActorDamageCause.MaceSmash`       | 34 |
| `ActorDamageCause.All`             | 35 |

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.hurt(20);
      // 使用枚举
      pl.hurt(20, ActorDamageCause.EntityAttack);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:hurt(20)
      ```

#### 治疗玩家

`pl.heal(health)`

- 参数:
    - int : `Integer`  
      治疗的心数
- 返回值: 治疗是否成功
- 返回值类型: `Boolean`

#### 设置玩家的生命值

`pl.setHealth(health)`

- 参数:
    - health : `Integer`  
      生命值数
- 返回值: 是否成功
- 返回值类型: `Boolean`

#### 为玩家设置伤害吸收属性

`pl.setAbsorption(value)`

- 参数:
    - value : `Integer`  
      新的值
- 返回值: 为玩家设置属性值是否成功
- 返回值类型: `Boolean`

#### 为玩家设置攻击伤害属性

`pl.setAttackDamage(value)`

- 参数:
    - value : `Integer`  
      新的值
- 返回值: 为玩家设置属性值是否成功
- 返回值类型: `Boolean`

#### 为玩家设置最大攻击伤害属性

`pl.setMaxAttackDamage(value)`

- 参数:
    - value : `Integer`  
      新的值
- 返回值: 为玩家设置属性值是否成功
- 返回值类型: `Boolean`

#### 为玩家设置跟随范围

`pl.setFollowRange(value)`

- 参数:
    - value : `Integer`  
      新的值
- 返回值: 为玩家设置属性值是否成功
- 返回值类型: `Boolean`

#### 为玩家设置击退抵抗属性

`pl.setKnockbackResistance(value)`

- 参数:
    - value : `Integer`  
      新的值 (0 or 1)
- 返回值: 为玩家设置属性值是否成功
- 返回值类型: `Boolean`

#### 为玩家设置幸运属性

`pl.setLuck(value)`

- 参数:
    - value : `Integer`  
      新的值
- 返回值: 为玩家设置属性值是否成功
- 返回值类型: `Boolean`

#### 为玩家设置移动速度属性

`pl.setMovementSpeed(value)`

- 参数:
    - value : `Integer`  
      新的值
- 返回值: 为玩家设置属性值是否成功
- 返回值类型: `Boolean`

#### 为玩家设置水下移动速度属性

`pl.setUnderwaterMovementSpeed(value)`

- 参数:
    - value : `Integer`  
      新的值
- 返回值: 为玩家设置属性值是否成功
- 返回值类型: `Boolean`

#### 为玩家设置岩浆上移动速度属性

`pl.setLavaMovementSpeed(value)`

- 参数:
    - value : `Integer`  
      新的值
- 返回值: 为玩家设置属性值是否成功
- 返回值类型: `Boolean`

#### 设置玩家最大生命值

`pl.setMaxHealth(health)`

- 参数:
    - health : `Integer`  
      生命值数
- 返回值: 是否成功
- 返回值类型: `Boolean`

#### 设置玩家饥饿值

`pl.setHungry(hunger)`

- 参数:
    - hunger : `Integer`  
      饥饿值数
- 返回值: 是否成功
- 返回值类型: `Boolean`

#### 使指定玩家着火

`pl.setFire(time,isEffect)`

- 参数：
    - time : `Integer`  
      着火时长，单位秒
    - isEffect : `Boolean`
      会不会有火的效果
- 返回值：是否成功着火
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.setFire(20,true);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:setFire(20,true)
      ```

#### 熄灭玩家

`pl.stopFire()`

- 返回值: 是否已被熄灭
- 返回值类型: `Boolean`

#### 缩放玩家

`pl.setScale(scale)`

- 参数:
    - scale : `Float`  
      新的玩家体积
- 返回值: 玩家是否成功地被缩放
- 返回值类型: `Boolean`

#### 重命名玩家

`pl.rename(newname)`

- 参数：
    - newname : `String`  
      玩家的新名字
- 返回值：是否重命名成功
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.rename("Steve");
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:rename("Steve")
      ```

#### 获取玩家当前站立所在的方块

`pl.getBlockStandingOn()`

- 返回值：当前站立在的方块对象
- 返回值类型：`Block`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      var bl = pl.getBlockStandingOn();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      bl = pl:getBlockStandingOn()
      ```

#### 获取玩家对应的设备信息对象

`pl.getDevice()`

- 返回值：玩家对应的设备信息对象
- 返回值类型：`Device`

设备信息对象储存了与玩家设备有关的某些信息，如设备IP地址、设备类型、网络延迟等信息。  
关于设备信息对象的其他信息请参考 [设备信息对象 API](./Device.zh.md)

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      var dv = pl.getDevice();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      dv = pl:getDevice()
      ```

#### 获取玩家主手中的物品对象

`pl.getHand()`

- 返回值：玩家主手中的物品对象
- 返回值类型：`Item`

此处获取的物品对象为引用。也就是说，修改此处返回的物品对象，或使用其API，就相当于直接操作玩家主手中对应的物品

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      var it = pl.getHand();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      it = pl:getHand()
      ```

#### 获取玩家副手的物品对象

`pl.getOffHand()`

- 返回值：玩家副手中的物品对象
- 返回值类型：`Item`

此处获取的物品对象为引用。也就是说，修改此处返回的物品对象，或使用其API，就相当于直接操作玩家副手中对应的物品

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      var it = pl.getOffHand();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      it = pl:getOffHand()
      ```

#### 获取玩家物品栏的容器对象

`pl.getInventory()`

- 返回值：玩家物品栏对应的容器对象
- 返回值类型：`Container`

关于容器对象的更多使用，请参考 [容器对象 API文档](./Container.zh.md)

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      var ct = pl.getInventory();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      ct = pl:getInventory()
      ```

#### 获取玩家盔甲栏的容器对象

`pl.getArmor()`

- 返回值：玩家盔甲栏对应的容器对象
- 返回值类型：`Container`

关于容器对象的更多使用，请参考 [容器对象 API文档](./Container.zh.md)

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      var ct = pl.getArmor();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      ct = pl:getArmor()
      ```

#### 获取玩家末影箱的容器对象

`pl.getEnderChest()`

- 返回值：玩家末影箱对应的容器对象
- 返回值类型：`Container`

关于容器对象的更多使用，请参考 [容器对象 API文档](./Container.zh.md)

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      var ct = pl.getEnderChest();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      ct = pl:getEnderChest()
      ```

#### 获取玩家的重生坐标

`pl.getRespawnPosition()`

- 返回值：重生点坐标
- 返回值类型：`IntPos`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      var pos = pl.getRespawnPosition();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pos = pl:getRespawnPosition()
      ```

#### 修改玩家的重生坐标

`pl.setRespawnPosition(pos)`  
`pl.setRespawnPosition(x,y,z,dimid)`

- 参数：
    - pos : `IntPos`或`FloatPos`  
      重生坐标（或者使用x, y, z, dimid来确定重生位置）
- 返回值：是否成功修改
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      pl.setRespawnPosition(pos);
      ```
    - Lua
      ```lua
      pl.setRespawnPosition(pos)
      ```

#### 给予玩家一个物品

`pl.giveItem(item[, amount])`

- 参数：
    - item : `Item`  
      给予的物品对象

    - amount: `Integer`

      （可选参数）给予物品对象的数量，若提供此参数则物品对象自身的Count属性将被忽略
- 返回值：是否成功给予
- 返回值类型：`Boolean`

如果玩家物品栏已满，将抛出多余物品

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.giveItem(item);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:giveItem(item)
      ```

#### 清除玩家背包中所有指定类型的物品

`pl.clearItem(type[, count])`

- 参数：
    - type : `String`  
      要清除的物品对象类型名
    - count : `Integer`  
      （可选参数）要清除的物品数量
- 返回值：清除的物品个数
- 返回值类型：`Integer`

将玩家物品栏、主手、副手、盔甲栏中所有物品的type属性与此字符串进行比较  
如果相等，则清除此物品

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.clearItem("minecraft:dirt");
      pl.clearItem("minecraft:dirt", 114514);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:clearItem("minecraft:dirt")
      pl:clearItem("minecraft:dirt", 1919)
      ```

#### 刷新玩家物品栏、盔甲栏

`pl.refreshItems()`

- 返回值：是否成功刷新
- 返回值类型：`Boolean`

在修改玩家物品之后，为了促使客户端生效，需要刷新玩家所有的物品

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.refreshItems();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:refreshItems()
      ```

#### 刷新玩家加载的所有区块

`pl.refreshChunks()`

- 返回值：是否成功刷新
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.refreshChunks();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:refreshChunks()
      ```

#### 修改玩家操作权限

`pl.setPermLevel(level)`

- 参数：

    - level : `Integer`  
      目标操作权限等级

| 操作权限等级 | 对应操作权限     |
|--------|------------|
| 0      | 普通成员权限     |
| 1      | OP权限       |
| 4      | OP + 控制台权限 |

- 返回值：是否成功修改

- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.setPermLevel(0);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:setPermLevel(0)
      ```

#### 修改玩家游戏模式

`pl.setGameMode(mode)`

- 参数：

    - mode : `Integer`  
      目标游戏模式，0为生存模式，1为创造模式，2为冒险模式, 6为观察者模式

- 返回值：是否成功修改

- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.setGameMode(0);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:setGameMode(0)
      ```

#### 提高玩家经验等级

`pl.addLevel(count)`

- 参数：
    - count : `Integer`  
      要提高的经验等级
- 返回值：是否设置成功
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.addLevel(1);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:addLevel(1)
      ```

#### 降低玩家经验等级

`pl.reduceLevel(count)`

- 参数：
    - count : `Integer`  
      要降低的经验等级
- 返回值：是否设置成功
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.reduceLevel(1);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:reduceLevel(1)
      ```

#### 获取玩家经验等级

`pl.getLevel()`

- 返回值：玩家的经验等级
- 返回值类型：`Integer`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      var lv = pl.getLevel();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      lv = pl:getLevel()
      ```

#### 设置玩家经验等级

`pl.setLevel(count)`

- 参数：
    - count : `Integer`  
      要设置的经验等级
- 返回值：是否设置成功
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.setLevel(1);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:setLevel(1)
      ```

#### 重置玩家经验

`pl.resetLevel()`

- 返回值：是否设置成功
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.resetLevel();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:resetLevel()
      ```

#### 获取玩家当前经验值

`pl.getCurrentExperience()`

- 返回值：玩家当前经验值
- 返回值类型：`Integer`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.getCurrentExperience();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:getCurrentExperience()
      ```

#### 设置玩家当前经验值

`pl.setCurrentExperience(count)`

- 参数：
    - count : `Integer`  
      要设置的经验值
- 返回值：是否设置成功
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.setCurrentExperience(1);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:setCurrentExperience(1)
      ```

#### 获取玩家总经验值

`pl.getTotalExperience()`

- 返回值：玩家总经验值
- 返回值类型：`Integer`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      var xp = pl.getTotalExperience();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      xp = pl:getTotalExperience()
      ```

#### 设置玩家总经验值

`pl.setTotalExperience(count)`

- 参数：
    - count : `Integer`  
      要设置的经验值
- 返回值：是否设置成功
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.setTotalExperience(1);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:setTotalExperience(1)
      ```

#### 提高玩家经验值

`pl.addExperience(count)`

- 参数：
    - count : `Integer`
      要提高的经验值
- 返回值：是否设置成功
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.addExperience(1);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:addExperience(1)
      ```

#### 降低玩家经验值

`pl.reduceExperience(count)`

- 参数：
    - count : `Integer`
      要降低的经验值
- 返回值：是否设置成功
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.reduceExperience(1);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:reduceExperience(1)
      ```

#### 获取玩家升级所需的经验值

`pl.getXpNeededForNextLevel()`

- 返回值：玩家升级所需的经验值
- 返回值类型：`Integer`

注意，此方法在计算时会忽略当前经验值

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      var ndxp = pl.getXpNeededForNextLevel();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      ndxp = pl:getXpNeededForNextLevel()
      ```

#### 传送玩家至指定服务器

`pl.transServer(server,port)`

- 参数：
    - server : `String`  
      目标服务器IP / 域名

    - port : `Integer`  
      目标服务器端口
- 返回值：是否成功传送
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
        // 对于一个玩家对象pl
        pl.transServer("123.45.67.89", 23333);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:transServer("123.45.67.89", 23333)
      ```

#### 使玩家客户端崩溃

`pl.crash()`

- 返回值：是否成功执行
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.crash();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:crash()
      ```

#### 设置玩家自定义侧边栏

`pl.setSidebar(title,data[,sortOrder])`

- 参数：

    - title : `String`  
      侧边栏标题
    - data : `Object<String-Integer>`  
      侧边栏对象内容对象  
      对象中的每个键 - 值对将被设置为侧边栏内容的一行
    - sortOrder : `Number`  
      （可选参数）侧边栏内容的排序顺序。`0`为按分数升序，`1`为按分数降序。默认值为`1`

- 返回值：是否成功设置

- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.setSidebar("title", {
          aaaa: 3,
          bbb: 12,
          cc: 7,
      });
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:setSidebar("title", {
          "aaaa" = 3,
          "bbb" = 12,
          "cc" = 7
      })
      ```

#### 移除玩家自定义侧边栏

`pl.removeSidebar()`

- 返回值：是否成功移除
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.removeSidebar();
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:removeSidebar()
      ```

#### 设置玩家看到的自定义Boss血条

`pl.setBossBar(uid,title,percent,colour)`

- 参数：
    - uid : `Number`   
      唯一标识符，不可冲突重复！一个uid对于一行bar
    - title : `String`  
      自定义血条标题
    - percent : `Integer`  
      血条中的血量百分比，有效范围为0~100。0为空血条，100为满
    - colour : `Integer`  
      血条颜色(默认值为2(RED))
- 返回值：是否成功设置
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.setBossBar(1145141919, "Hello ~ ", 80, 0);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:setBossBar(1145141919, "Hello ~ ", 80, 0)
      ```

#### 移除玩家的自定义的指定Boss血条

`pl.removeBossBar(uid)`

- 参数：
    - uid : `Number`   
      标识符，与setBossBar对应！

- 返回值：是否成功移除
- 返回值类型：`Boolean`

- 示例：
    - JavaScript
      ```js
      // 对于一个玩家对象pl
      pl.removeBossBar(1145141919);
      ```
    - Lua
      ```lua
      -- 对于一个玩家对象pl
      pl:removeBossBar(1145141919)
      ```

#### 获取在线玩家对应的NBT对象

`pl.getNbt()`

- 返回值：玩家的NBT对象
- 返回值类型：`NbtCompound`

#### 写入在线玩家对应的NBT对象

`pl.setNbt(nbt)`

- 参数：
    - nbt : `NbtCompound`  
      NBT对象
- 返回值：是否成功写入
- 返回值类型：`Boolean`

关于NBT对象的更多使用，请参考 [NBT接口文档](../NbtAPI/NBT.zh.md)

#### 获取玩家对应的NBT对象

`mc.getPlayerNbt(uuid)`

- 参数：
    - uuid : `String`  
      玩家的UUID
- 返回值：玩家的NBT对象
- 返回值类型：`NbtCompound`
    - 如果玩家不存在或没有存档数据，则返回值为 `Null`

#### 写入玩家对应的NBT对象

`mc.setPlayerNbt(uuid,nbt[,forceCreate,isOnlineMode])`

- 参数：
    - uuid : `String`  
      玩家的UUID
    - nbt : `NbtCompound`  
      NBT对象
    - forceCreate : `Boolean` 可选参数  
      当该 UUID 没有对应存档记录时，是否创建离线玩家数据。默认值为 `false`
    - isOnlineMode : `Boolean` 可选参数  
      仅在 `forceCreate` 为 `true` 时生效。决定新建玩家数据是否使用在线模式。默认值为 `true`
- 返回值：是否成功写入
- 返回值类型：`Boolean`

#### 覆盖玩家对应的NBT对象的特定NbtTag

`mc.setPlayerNbtTags(uuid,nbt,tags)`

- 参数：
    - uuid : `String`  
      玩家的UUID
    - nbt : `NbtCompound`  
      NBT对象
    - tags : `Array<String>`
      需要覆盖的NbtTag
- 返回值：是否成功覆盖对应的Tag
- 返回值类型：`Boolean`

#### 从存档中删除玩家对应的NBT对象的全部内容

`mc.deletePlayerNbt(uuid)`

- 参数：
    - uuid : `String`  
      玩家的UUID
- 返回值：是否删除成功
- 返回值类型：`Boolean`

#### 获取全部已保存玩家 UUID

`mc.getAllPlayerUuids([isOnlineMode])`

- 参数：
    - isOnlineMode : `Boolean` 可选参数  
      为 `true` 时返回登录了微软账号玩家的 UUID；为 `false` 时返回离线模式玩家的 UUID，默认为 `false`
- 返回值：已保存玩家 UUID 字符串列表
- 返回值类型：`Array<String,String,...>`

#### 为玩家增加一个Tag

`pl.addTag(tag)`

- 参数：
    - tag: `String`  
      要增加的tag字符串
- 返回值：是否设置成功
- 返回值类型：`Boolean`

#### 为玩家移除一个Tag

`pl.removeTag(tag)`

- 参数：
    - tag: `String`  
      要移除的tag字符串
- 返回值：是否移除成功
- 返回值类型：`Boolean`

#### 检查玩家是否拥有某个Tag

`pl.hasTag(tag)`

- 参数：
    - tag: `String`  
      要检查的tag字符串
- 返回值：是否拥有这个Tag
- 返回值类型：`Boolean`

#### 获取玩家拥有的所有Tag列表

`pl.getAllTags()`

- 返回值：玩家所有的 tag 字符串列表
- 返回值类型：`Array<String,String,...>`

#### 获取玩家的Abilities能力列表（来自玩家NBT）

`pl.getAbilities()`

- 返回值：玩家所有能力信息的键 - 值对列表对象
- 返回值类型：`object<String-任意类型>`

键 - 值对列表中的每一项形如：`"mayfly": 1`  等等

#### 获取玩家的Attributes属性列表（来自玩家NBT）

`pl.getAttributes()`

- 返回值：玩家所有属性对象的数组
- 返回值类型：`Array<Object,Object,...>`

数组中的每一项为一个键 - 值对列表对象`Object`，Attributes对象默认含有`Base` `Current` `DefaultMax` `DefaultMin` `Max`
`Min` `Name` 等几种内容 。其内容形如：

```json
{
  Base: 0,
  Current: 0,
  DefaultMax: 1024,
  DefaultMin: -1024,
  Max: 1024,
  Min: -1024,
  Name: "minecraft:luck"
}
```

#### 获取玩家疾跑状态

`pl.isSprinting()`

- 返回值：玩家疾跑状态
- 返回值类型：`Boolean`

#### 设置玩家疾跑状态

`pl.setSprinting(sprinting)`

- 参数：
    - sprinting : `Boolean`  
      是否为疾跑状态
- 返回值：是否设置成功
- 返回值类型：`Boolean`

#### 获取视线方向实体

`pl.getEntityFromViewVector([maxDistance])`

- 参数：
    - maxDistance : `Float`  
      查找最大距离
- 返回值：视线方向实体，如果获取失败，返回 `Null`
- 返回值类型：`Entity?`

#### 获取视线方向方块

`pl.getBlockFromViewVector([includeLiquid,solidOnly,maxDistance,fullOnly])`

- 参数：
    - includeLiquid : `Boolean`  
      是否包含液态方块
    - solidOnly : `Boolean`  
      是否仅允许 `Solid` 类型的方块
    - maxDistance : `Float`  
      查找最大距离
    - fullOnly : `Boolean`  
      是否仅允许完整方块
- 返回值：视线方向方块，如果获取失败，返回 `Null`
- 返回值类型：`Block?`

#### 向玩家发送数据包

`pl.sendPacket(packet)`

- 参数：
    - packet : `Packet`  
      数据包
- 返回值：是否成功，如果pl不存在，返回Null
- 返回值类型：`Bool`

#### 获取玩家所在群系ID

`pl.getBiomeId()`

- 返回值：群系ID
- 返回值类型：`Integer`

#### 获取玩家所在群系名称

`pl.getBiomeName()`

- 返回值：群系名称
- 返回值类型：`String`

#### 设置玩家Ability属性

`pl.setAbility(AbilityID,value)`

- 参数：
    - AbilityID : `Integer`  
      Ability的ID
    - value : `Boolean`  
      是否开启
- 返回值：无作用
- 返回值类型：`Boolean`

#### 获取玩家全部药水效果

`pl.getAllEffects()`

- 返回值：玩家所有的药水效果id（见下表）
- 返回值类型：`Array<number,number,...>`

#### 为玩家添加一个药水效果

`pl.addEffect(id, tick, level, showParticles)`

- 参数：
    - id : `Number`
      药水效果的id（见下表）
    - tick : `Number`
      持续时间
    - level : `Number`
      等级
    - showParticles : `Boolean`
      是否显示粒子
- 返回值：操作是否成功
- 返回值类型：`Boolean`

#### 为玩家移除一个药水效果

`pl.removeEffect(id)`

- 参数：
    - id : `Number`
      药水效果的id（见下表）
- 返回值：操作是否成功
- 返回值类型：`Boolean`

| 效果     | 名称              | 数字 id |
|--------|-----------------|-------|
| 迅捷     | speed           | 1     |
| 缓慢     | slowness        | 2     |
| 急迫     | haste           | 3     |
| 挖掘疲劳   | mining_fatigue  | 4     |
| 力量     | strength        | 5     |
| 瞬间治疗   | instant_health  | 6     |
| 瞬间伤害   | instant_damage  | 7     |
| 跳跃提升   | jump_boost      | 8     |
| 反胃     | nausea          | 9     |
| 生命恢复   | regeneration    | 10    |
| 抗性提升   | resistance      | 11    |
| 抗火     | fire_resistance | 12    |
| 水下呼吸   | water_breathing | 13    |
| 隐身     | invisibility    | 14    |
| 失明     | blindness       | 15    |
| 夜视     | night_vision    | 16    |
| 饥饿     | hunger          | 17    |
| 虚弱     | weakness        | 18    |
| 中毒     | poison          | 19    |
| 凋零     | wither          | 20    |
| 生命提升   | health_boost    | 21    |
| 伤害吸收   | absorption      | 22    |
| 饱和     | saturation      | 23    |
| 飘浮     | levitation      | 24    |
| 中毒（致命） | fatal_poison    | 25    |
| 潮涌能量   | conduit_power   | 26    |
| 缓降     | slow_falling    | 27    |
| 不祥之兆   | bad_omen        | 28    |
| 村庄英雄   | village_hero    | 29    |
| 黑暗     | darkness        | 30    |

#### 将玩家对象转换实体对象

`en.toEntity()`

- 返回值：转换成的实体对象
- 返回值类型：`Entity`
    - 如果转换失败，则返回 `Null`

#### 判断是否为模拟玩家

`pl.isSimulatedPlayer()`

- 返回值：是否为模拟玩家
- 返回值类型：`Boolean`

## 模拟玩家（由于与玩家API重合过多，未生成新的模拟玩家类）

### 创建一个模拟玩家

`mc.spawnSimulatedPlayer(name,pos)`  
`mc.spawnSimulatedPlayer(name,x,y,z,dimid)`

- 参数：
    - name : `String`  
      模拟玩家名称
    - pos : `IntPos `/ `FloatPos`  
      生成生物的位置的坐标对象（或者使用x, y, z, dimid来确定生成位置）
- 返回值：生成的（模拟）玩家对象
- 返回值类型：`Player`
    - 如返回值为 `Null` 则表示生成失败

### 模拟玩家 - 函数

每一个模拟玩家对象都包含一些可以执行的成员函数（成员方法）。对于某个特定的模拟玩家对象`sp`，可以通过以下这些函数对这个模拟玩家进行一些操作

#### 模拟重生

`sp.simulateRespawn()`

- 返回值：是否成功模拟操作
- 返回值类型：'Boolean'

参考：[mojang-gametest docs](https://learn.microsoft.com/zh-cn/minecraft/creator/scriptapi/minecraft/server-gametest/simulatedplayer?view=minecraft-bedrock-experimental#respawn)

#### 模拟攻击

`sp.simulateAttack([target])`

- 参数：

    - target : `Entity`  
      （可选参数）攻击目标，默认为视线方向上的实体

- 返回值：是否成功模拟操作
- 返回值类型：`Boolean`

参考：[mojang-gametest docs](https://docs.microsoft.com/zh-cn/minecraft/creator/scriptapi/mojang-gametest/simulatedplayer#attack)

#### 模拟破坏

`sp.simulateDestroy([pos,face])`
`sp.simulateDestroy([block,face])`

- 参数：

    - pos :`IntPos`  
      （可选参数）要破坏的方块的坐标，默认为视线方向上的方块
    - block :`Block`  
      （可选参数）要破坏的方块，默认为视线方向上的方块
    - face :`Integer`  
      （可选参数）从哪面破坏，

- 返回值：是否成功模拟操作
- 返回值类型：`Boolean`

参考：[mojang-gametest docs](https://docs.microsoft.com/zh-cn/minecraft/creator/scriptapi/mojang-gametest/simulatedplayer#breakblock)

#### 模拟断开连接

`sp.simulateDisconnect()`

- 返回值：是否成功模拟操作
- 返回值类型：`Boolean`

#### 模拟交互

`sp.simulateInteract([target])`
`sp.simulateInteract([pos,face])`
`sp.simulateInteract([block,face])`

- 参数：

    - target : `Entity`  
      （可选参数）模拟交互目标，默认为视线方向上的方块或实体
    - pos :`IntPos`  
      （可选参数）模拟交互目标，默认为视线方向上的方块或实体
    - block :`Block`  
      （可选参数）模拟交互目标，默认为视线方向上的方块或实体
    - face :`Number`  
      （可选参数）模拟交互目标方块的面

- 返回值：是否成功模拟操作
- 返回值类型：`Boolean`

参考：[mojang-gametest docs](https://docs.microsoft.com/zh-cn/minecraft/creator/scriptapi/mojang-gametest/simulatedplayer#interact)

#### 模拟跳跃

`sp.simulateJump()`

- 返回值：是否成功模拟操作
- 返回值类型：`Boolean`

参考：[mojang-gametest docs](https://docs.microsoft.com/zh-cn/minecraft/creator/scriptapi/mojang-gametest/simulatedplayer#jump)

#### 模拟看向某方块或实体

`sp.simulateLookAt(pos, [lookDuration])`
`sp.simulateLookAt(entity, [lookDuration])`
`sp.simulateLookAt(block, [lookDurration])`

!!! warning
    `lookDurration`仅在0.8.13以及之后的版本中可用

- 参数：

    - target : `Entity`  
      要看向的实体
    - pos :`IntPos` / `FloatPos`  
      要看向的坐标
    - block :`Block`  
      要看向的方块
    - lookDuration: `Int`  
      模拟玩家看向目标的持续时间  
      0 = 立刻, 1 = 持续, 2 = 不变直到移动

- 返回值：是否成功模拟操作
- 返回值类型：`Boolean`

参考：[mojang-gametest docs](https://docs.microsoft.com/zh-cn/minecraft/creator/scriptapi/mojang-gametest/simulatedplayer#lookatblock)

#### 模拟设置身体角度

`sp.simulateSetBodyRotation(rot)`

- 参数：

    - rot : `Number`  
      要设置的角度

- 返回值：是否成功模拟操作
- 返回值类型：`Boolean`

参考：[mojang-gametest docs](https://learn.microsoft.com/zh-cn/minecraft/creator/scriptapi/minecraft/server-gametest/simulatedplayer?view=minecraft-bedrock-experimental#rotatebody)

#### 相对玩家坐标系移动

`sp.simulateLocalMove()`

- 参数：
    - pos : `IntPos` / `FloatPos`  
      移动方向
    - speed : `Number`  
      （可选参数）移动速度，默认为1

- 返回值：是否请求移动成功
- 返回值类型：`Boolean`

#### 相对世界坐标系移动

`sp.simulateWorldMove()`

- 参数：
    - pos : `IntPos` / `FloatPos`  
      移动方向
    - speed : `Number`  
      （可选参数）移动速度，默认为1

- 返回值：是否请求移动成功
- 返回值类型：`Boolean`

#### 直线移动到坐标

`sp.simulateMoveTo()`

- 参数：
    - pos : `IntPos` / `FloatPos`  
      目标位置
    - speed : `Number`  
      （可选参数）移动速度，默认为1

- 返回值：是否请求移动成功
- 返回值类型：`Boolean`

参考：[mojang-gametest docs](https://docs.microsoft.com/zh-cn/minecraft/creator/scriptapi/mojang-gametest/simulatedplayer#movetolocation)  
注：如需自动寻路，请考虑使用 `模拟导航移动`

#### 模拟导航移动

`sp.simulateNavigateTo(entity[,speed)`
`sp.simulateNavigateTo(pos[,speed])`

- 参数：

    - entity : `Entity`  
      导航目标
    - pos : `IntPos` / `FloatPos`  
      导航目标
    - speed : `Number`  
      （可选参数）移动速度，默认为1

- 返回值：是否能到达指定位置以及导航路径，结构：{isFullPath:`Boolean`,path:`Number[3][]`}
- 返回值类型：`Object`

参考：[mojang-gametest docs](https://docs.microsoft.com/zh-cn/minecraft/creator/scriptapi/mojang-gametest/simulatedplayer#navigatetoblock)  
返回值示例：

```json
{
  isFullPath: false,
  path: [
    [
      -8,
      0,
      -3
    ],
    [
      -7,
      0,
      -2
    ],
    [
      -6,
      0,
      -2
    ],
    [
      -5,
      0,
      -2
    ],
    [
      -4,
      0,
      -1
    ],
    [
      -3,
      0,
      -1
    ],
    [
      -2,
      0,
      -1
    ],
    [
      -1,
      0,
      0
    ]
  ]
}
```

此数据的目标坐标为(0,2,0)，路径终点为(-1,0,0)，所以isFullPath为false，但由于路径不为空，所以模拟玩家将会移动至(-1,0,0)坐标

#### 模拟导航移动（多目标）

`sp.simulateNavigateTo(posArray[,speed])`

- 参数：

    - posArray : `IntPos[]` / `FloatPos[]`  
      导航目标
    - speed : `Number`  
      （可选参数）移动速度，默认为1

- 返回值：是否成功模拟操作
- 返回值类型：`Boolean`

参考：[mojang-gametest docs](https://docs.microsoft.com/zh-cn/minecraft/creator/scriptapi/mojang-gametest/simulatedplayer#navigatetolocations)

#### 模拟使用物品

`sp.simulateUseItem([slot,pos,face,relative])`
`sp.simulateUseItem([item,pos,face,relative])`

- 参数：

    - item : `Item`  
      （可选参数）要使用的物品，默认为选中物品
    - slot : `Number`  
      （可选参数）要使用的物品所在的槽，默认为选中物品
    - pos : `IntPos`  
      （可选参数）目标坐标，默认为朝向方块坐标
    - face : `Number`  
      （可选参数）目标方块的面，默认为0
    - relative : `FloatPos`  
      （可选参数）相对方块偏移坐标，默认为{0.5,0.5,0.5}

- 返回值：是否成功模拟操作
- 返回值类型：`Boolean`

参考：[mojang-gametest docs](https://docs.microsoft.com/zh-cn/minecraft/creator/scriptapi/mojang-gametest/simulatedplayer#useitem)

#### 模拟停止破坏方块

`sp.simulateStopDestroyingBlock()`

- 返回值：是否成功模拟操作
- 返回值类型：`Boolean`

参考：[mojang-gametest docs](https://docs.microsoft.com/zh-cn/minecraft/creator/scriptapi/mojang-gametest/simulatedplayer#stopbreakingblock)

#### 模拟停止交互

`sp.simulateStopInteracting()`

- 返回值：是否成功模拟操作
- 返回值类型：`Boolean`

参考：[mojang-gametest docs](https://docs.microsoft.com/zh-cn/minecraft/creator/scriptapi/mojang-gametest/simulatedplayer#stopinteracting)

#### 模拟停止移动

`sp.simulateStopMoving()`

- 返回值：是否成功模拟操作
- 返回值类型：`Boolean`

参考：[mojang-gametest docs](https://docs.microsoft.com/zh-cn/minecraft/creator/scriptapi/mojang-gametest/simulatedplayer#stopmoving)

#### 模拟停止使用物品

`sp.simulateStopUsingItem()`

- 返回值：是否成功模拟操作
- 返回值类型：`Boolean`

参考：[mojang-gametest docs](https://docs.microsoft.com/zh-cn/minecraft/creator/scriptapi/mojang-gametest/simulatedplayer#stopusingitem)

