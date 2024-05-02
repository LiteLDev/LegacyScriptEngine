# 🎈 实体对象 API

在脚本引擎中，使用「实体对象」来操作和获取某一个实体的相关信息。

### 获取一个实体对象

#### 从事件或API获取

通过注册**事件监听**函数，或者调用某些**返回实体对象**的函数，来获取到BDS给出的实体对象    
详见 [事件监听文档 - EventAPI](../EventAPI/Listen.zh.md)      

#### 获取当前所有已加载的实体

此函数会返回一个实体对象的数组，其中每个对象都对应了一个已加载的实体

`mc.getAllEntities()`

- 返回值：实体对象列表
- 返回值类型：`Array<Entity,Entity,...>`

#### 生成新生物并获取

通过此函数，在指定的位置生成一个新生物，并获取它对应的实体对象

`mc.spawnMob(name,pos)`  
`mc.spawnMob(name,x,y,z,dimid)`

- 参数：
  - name : `String`  
    生物的命名空间名称，如 `minectaft:creeper`
  - pos : `IntPos `/ `FloatPos`  
    生成生物的位置的坐标对象（或者使用x, y, z, dimid来确定生成位置）
- 返回值：生成的实体对象
- 返回值类型：`Entity`
  - 如返回值为 `Null` 则表示生成失败

> 注意：不要**长期保存**一个实体对象  
> 当实体对象对应的实体被销毁时，对应的实体对象将同时释放。因此，如果有长期操作某个实体的需要，请通过上述途径获取实时的实体对象



#### 复制生物并获取

通过此函数，在指定的位置复制另一个实体，并获取它对应的实体对象

`mc.cloneMob(entity,pos)`  
`mc.cloneMob(entity,x,y,z,dimid)`

- 参数：
  - entity : `Entity`  
    需要复制的实体对象
  - pos : `IntPos `/ `FloatPos`  
    生成生物的位置的坐标对象（或者使用x, y, z, dimid来确定生成位置）
- 返回值 复制的实体对象
- 返回值类型：`Entity`
  - 如返回值为 `Null` 则表示生成失败

> 注意：不要**长期保存**一个实体对象  
> 当实体对象对应的实体被销毁时，对应的实体对象将同时释放。因此，如果有长期操作某个实体的需要，请通过上述途径获取实时的实体对象




### 实体对象 - 属性

每一个实体对象都包含一些固定的对象属性。对于某个特定的实体对象`en`，有以下这些属性

| 属性                     | 含义                   | 类型             |
| ------------------------ | ---------------------- | ---------------- |
| en.name                  | 实体名称               | `String`         |
| en.type                  | 实体标准类型名         | `String`         |
| en.id                    | 实体的游戏内id         | `Integer`        |
| en.pos                   | 实体所在坐标           | `FloatPos`       |
| en.feetPos               | 实体腿部所在坐标       | `FloatPos`       |
| en.blockPos              | 实体所在的方块坐标     | `IntPos`         |
| en.maxHealth             | 实体最大生命值         | `Integer`        |
| en.health                | 实体当前生命值         | `Integer`        |
| en.canFly                | 实体是否能飞行         | `Boolean`        |
| en.canFreeze             | 实体是否能被冻结       | `Boolean`        |
| en.canSeeDaylight        | 实体是否能看到天空     | `Boolean`        |
| en.canPickupItems        | 实体是否能拾取物品     | `Boolean`        |
| en.inAir                 | 实体是否悬空           | `Boolean`        |
| en.inWater               | 实体是否在水中         | `Boolean`        |
| en.inLava                | 实体是否在岩浆中       | `Boolean`        |
| en.inRain                | 实体是否在雨中         | `Boolean`        |
| en.inSnow                | 实体是否在雪中         | `Boolean`        |
| en.inWall                | 实体是否在墙上         | `Boolean`        |
| en.inWaterOrRain         | 实体是否在水中或雨中   | `Boolean`        |
| en.inWorld               | 实体是否在世界中       | `Boolean`        |
| en.speed                 | 实体当前速度           | `Float`          |
| en.direction             | 实体当前朝向           | `DirectionAngle` |
| en.uniqueId              | 实体唯一标识符         | `String`         |
| en.isInvisible           | 实体是否不可见         | `Boolean`        |
| en.isInsidePortal        | 实体是否在门户内       | `Boolean`        |
| en.isTrusting            | 实体是否信任           | `Boolean`        |
| en.isTouchingDamageBlock | 实体是否接触到伤害方块 | `Boolean`        |
| en.isOnFire              | 实体是否着火           | `Boolean`        |
| en.isOnGround            | 实体是否在地面         | `Boolean`        |
| en.isOnHotBlock          | 实体是否在热块上       | `Boolean`        |
| en.isTrading             | 实体是否在交易         | `Boolean`        |
| en.isRiding              | 实体是否正在骑行       | `Boolean`        |
| en.isDancing             | 实体是否在跳舞         | `Boolean`        |
| en.isSleeping            | 实体是否在睡觉         | `Boolean`        |
| en.isAngry               | 实体是否生气           | `Boolean`        |
| en.isBaby                | 实体是否为幼体         | `Boolean`        |
| en.isMoving              | 实体是否移动           | `Boolean`        |


这些对象属性都是只读的，无法被修改

- **实体当前朝向** 属性的详细解释见  [基础游戏接口文档](./Basic.zh.md)
- **坐标** 和 **腿部坐标**：如果这个实体为两格高，则`pos`与`feetPos`不同，`pos`为实体视角高度的坐标，`feetPos`为腿部所在格子的方块坐标



### 实体对象 - 函数

每一个实体对象都包含一些可以执行的成员函数（成员方法）。对于某个特定的实体对象`en`，可以通过以下这些函数对这个实体进行一些操作

#### 传送实体至指定位置

`en.teleport(pos[,rot])`  
`en.teleport(x,y,z,dimid,[,rot])`

- 参数：
  - pos :`IntPos `/ `FloatPos`  
    目标位置坐标（或者使用x, y, z, dimid来确定实体位置）
    
  - rot: `DirectionAngle`
  
    （可选参数）传送后实体的朝向，若缺省则与传送前朝向相同
- 返回值：是否成功传送
- 返回值类型：`Boolean`



#### 杀死指定实体  

`en.kill()`

- 返回值：是否成功执行
- 返回值类型：`Boolean`



#### 使指定实体刷新消失  

`en.despawn()`

- 返回值：是否成功执行
- 返回值类型：`Boolean`



#### 移除指定实体  

`en.remove()`

- 返回值：是否成功执行
- 返回值类型：`Boolean`



#### 对实体造成伤害

`en.hurt(damage,type,source)`

- 参数：
  - damage : `Float`  
    对实体造成的伤害数值
  - type : `Integer`  
    伤害类型
  - source : `Entity`
    伤害来源
- 返回值：是否造成伤害
- 返回值类型：`Boolean`

注意，此处造成的伤害为真实伤害，无法被盔甲等保护装备减免

| 伤害类型枚举                       |
| ---------------------------------- |
| `ActorDamageCause.Override`        |
| `ActorDamageCause.Contact `        |
| `ActorDamageCause.EntityAttack`    |
| `ActorDamageCause.Projectile`      |
| `ActorDamageCause.Suffocation`     |
| `ActorDamageCause.All`             |
| `ActorDamageCause.Fire`            |
| `ActorDamageCause.FireTick`        |
| `ActorDamageCause.Lava`            |
| `ActorDamageCause.Drowning `       |
| `ActorDamageCause.BlockExplosion`  |
| `ActorDamageCause.EntityExplosion` |
| `ActorDamageCause.Void`            |
| `ActorDamageCause.Suicide`         |
| `ActorDamageCause.Magic`           |
| `ActorDamageCause.Wither`          |
| `ActorDamageCause.Starve`          |
| `ActorDamageCause.Anvil`           |
| `ActorDamageCause.Thorns`          |
| `ActorDamageCause.FallingBlock`    |
| `ActorDamageCause.Piston`          |
| `ActorDamageCause.FlyIntoWall`     |
| `ActorDamageCause.Magma`           |
| `ActorDamageCause.Fireworks`       |
| `ActorDamageCause.Lightning`       |
| `ActorDamageCause.Charging`        |
| `ActorDamageCause.Temperature`     |
| `ActorDamageCause.Freezing`        |
| `ActorDamageCause.Stalactite`      |
| `ActorDamageCause.Stalagmite`      |
| `ActorDamageCause.All`             |

#### 治疗实体

`en.heal(health)`

- 参数: 
  - int : `Integer`  
    治疗的心数
- 返回值: 是否治疗成功
- 返回值类型: `Boolean`



#### 设置实体的生命值

`en.setHealth(health)`

- 参数: 
  - health : `Integer`  
    生命值数
- 返回值: 是否成功
- 返回值类型: `Boolean`



#### 为实体设置伤害吸收属性

`en.setAbsorption(value)`

- 参数: 
  - value : `Integer`  
    新的值
- 返回值: 为实体设置属性值是否成功
- 返回值类型: `Boolean`



#### 为实体设置攻击伤害属性

`en.setAttackDamage(value)`

- 参数: 
  - value : `Integer`  
    新的值
- 返回值: 为实体设置属性值是否成功
- 返回值类型: `Boolean`



#### 为实体设置最大攻击伤害属性

`en.setMaxAttackDamage(value)`

- 参数: 
  - value : `Integer`  
    新的值
- 返回值: 为实体设置属性值是否成功
- 返回值类型: `Boolean`



#### 为实体设置跟随范围

`en.setFollowRange(value)`

- 参数: 
  - value : `Integer`  
    新的值
- 返回值: 为实体设置属性值是否成功
- 返回值类型: `Boolean`



#### 为实体设置击退抵抗属性

`en.setKnockbackResistance(value)`

- 参数: 
  - value : `Integer`  
    新的值 (0 or 1)
- 返回值: 为实体设置属性值是否成功
- 返回值类型: `Boolean`



#### 为实体设置幸运属性

`en.setLuck(value)`

- 参数: 
  - value : `Integer`  
    新的值
- 返回值: 为实体设置属性值是否成功
- 返回值类型: `Boolean`



#### 为实体设置移动速度属性

`en.setMovementSpeed(value)`

- 参数: 
  - value : `Integer`  
    新的值
- 返回值: 为实体设置属性值是否成功
- 返回值类型: `Boolean`



#### 为实体设置水下移动速度属性

`en.setUnderwaterMovementSpeed(value)`

- 参数: 
  - value : `Integer`  
    新的值
- 返回值: 为实体设置属性值是否成功
- 返回值类型: `Boolean`



#### 为实体设置岩浆上移动速度属性

`en.setLavaMovementSpeed(value)`

- 参数: 
  - value : `Integer`  
    新的值
- 返回值: 为实体设置属性值是否成功
- 返回值类型: `Boolean`



#### 设置实体的最大生命值

`en.setMaxHealth(health)`

- 参数: 
  - health : `Integer`  
    生命值数
- 返回值: 是否成功
- 返回值类型: `Boolean`



#### 设置特定实体为燃烧状态

`en.setFire(time, isEffect)`

- 参数: 
  - time : `Integer`  
    燃烧的时间，秒为单位
  - isEffect : `Boolean`  
    是否有火焰效果
- 返回值: 是否设置成功
- 返回值类型:  `Boolean`



#### 熄灭实体

`en.stopFire()`

- 返回值: 是否熄灭成功
- 返回值类型: `Boolean`



#### 缩放实体

`en.setScale(scale)`

- 参数: 
  - scale : `Float`  
    新的实体体积
- 返回值: 实体是否成功地被缩放
- 返回值类型: `Boolean`



#### 获取实体到坐标的距离

`en.distanceTo(pos)`
`en.distanceToSqr(pos)`

- 参数: 
  - pos : `Entity` / `Player` / `IntPos` / `FloatPos`
    目标位置
- 返回值: 到坐标的距离(方块)
- 返回值类型:  `Number`

> **注意** 若玩家的坐标与目标的坐标不在同一维度，将返回整数最大值。



#### 判断一个实体对象是不是玩家

`en.isPlayer()`

- 返回值：当前实体对象是不是玩家
- 返回值类型：`Boolean`



#### 将实体对象转换玩家对象

`en.toPlayer()`

- 返回值：转换成的玩家对象
- 返回值类型：`Player`
  - 如果此实体对象指向的不是某个玩家，或者转换失败，则返回 `Null`

如果当前实体对象指向的是一个玩家，可以使用此函数将实体对象转换为玩家对象，以使用更多的玩家相关 API



#### 判断一个实体对象是不是掉落物实体

`en.isItemEntity()`

- 返回值：当前实体对象是不是掉落物实体
- 返回值类型：`Boolean`



#### 获取掉落物实体中的物品对象

`en.toItem()`

- 返回值：获取到的物品对象
- 返回值类型：`Item`
  - 如果此实体对象不是掉落物实体，或者获取失败，则返回 `Null`

如果当前实体对象是一个掉落物实体，可以使用此函数获取掉落物实体中的物品对象，以使用更多的物品相关 API



#### 获取实体当前站立所在的方块

`en.getBlockStandingOn()`

- 返回值：当前站立在的方块对象
- 返回值类型：`Block`



#### 获取生物盔甲栏的容器对象  

`en.getArmor()`

- 返回值：此实体盔甲栏对应的容器对象
- 返回值类型：`Container`

关于容器对象的更多使用，请参考 [容器对象 API文档](./Container.zh.md)



#### 判断生物是否拥有容器（盔甲栏除外）

`en.hasContainer()`

- 返回值：这个生物实体是否拥有容器
- 返回值类型：`Boolean`

如羊驼身上的箱子等，他们各自拥有一个属于自己的容器对象



#### 获取生物所拥有的容器对象（盔甲栏除外）

`en.getContainer()`

- 返回值：这个生物实体所拥有的容器对象
- 返回值类型：`Container`

关于容器对象的更多使用，请参考 [容器对象 API文档](./Container.zh.md)



#### 刷新生物物品栏、盔甲栏

`en.refreshItems()`

- 返回值：是否成功刷新
- 返回值类型：`Boolean`

在修改生物物品之后，为了促使客户端生效，需要刷新生物所有的物品



#### 为实体增加一个Tag

`en.addTag(tag)`

- 参数：
  - tag: `String`  
    要增加的tag字符串
- 返回值：是否设置成功
- 返回值类型：`Boolean`



#### 为实体移除一个Tag

`en.removeTag(tag)`

- 参数：
  - tag: `String`  
    要移除的tag字符串
- 返回值：是否移除成功
- 返回值类型：`Boolean`



#### 检查实体是否拥有某个Tag

`en.hasTag(tag)`

- 参数：
  - tag: `String`  
    要检查的tag字符串
- 返回值：是否拥有这个Tag
- 返回值类型：`Boolean`



#### 返回实体拥有的所有Tag列表

`en.getAllTags()`

- 返回值：实体所有的 tag 字符串列表
- 返回值类型：`Array<String,String,...>`



#### 获取实体对应的NBT对象

`en.getNbt()`

- 返回值：实体的NBT对象
- 返回值类型：`NbtCompound`



#### 写入实体对应的NBT对象

`en.setNbt(nbt)`

- 参数：
  - nbt : `NbtCompound`  
    NBT对象
- 返回值：是否成功写入
- 返回值类型：`Boolean`

关于NBT对象的更多使用，请参考 [NBT接口文档](../NbtAPI/NBT.md)



#### 获取视线方向实体

`en.getEntityFromViewVector([maxDistance])`  

- 参数：
  - maxDistance : `Float`  
    查找最大距离  
- 返回值：视线方向实体，如果获取失败，返回 `Null`  
- 返回值类型：`Entity`  



#### 获取视线方向方块

`en.getBlockFromViewVector([includeLiquid,solidOnly,maxDistance,fullOnly])`  

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
- 返回值类型：`Block`  



#### 获取生物所在群系ID

`en.getBiomeId()`  

- 返回值：群系ID
- 返回值类型：`Integer`



#### 获取生物所在群系名称

`en.getBiomeName()`  

- 返回值：群系名称
- 返回值类型：`String`



#### 获取实体全部药水效果

`en.getAllEffects()`

- 返回值：实体所有的药水效果id（见下表）
- 返回值类型：`Array<number,number,...>`



#### 为实体添加一个药水效果

`en.addEffect(id, tick, level, showParticles)`
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



#### 为实体移除一个药水效果

`en.removeEffect(id)`
- 参数：
  - id : `Number`
    药水效果的id（见下表）
- 返回值：操作是否成功
- 返回值类型：`Boolean`

| 效果         | 名称            | 数字id |
| ------------ | --------------- | ------ |
| 迅捷         | speed           | 1      |
| 缓慢         | slowness        | 2      |
| 急迫         | haste           | 3      |
| 挖掘疲劳     | mining_fatigue  | 4      |
| 力量         | strength        | 5      |
| 瞬间治疗     | instant_health  | 6      |
| 瞬间伤害     | instant_damage  | 7      |
| 跳跃提升     | jump_boost      | 8      |
| 反胃         | nausea          | 9      |
| 生命恢复     | regeneration    | 10     |
| 抗性提升     | resistance      | 11     |
| 抗火         | fire_resistance | 12     |
| 水下呼吸     | water_breathing | 13     |
| 隐身         | invisibility    | 14     |
| 失明         | blindness       | 15     |
| 夜视         | night_vision    | 16     |
| 饥饿         | hunger          | 17     |
| 虚弱         | weakness        | 18     |
| 中毒         | poison          | 19     |
| 凋零         | wither          | 20     |
| 生命提升     | health_boost    | 21     |
| 伤害吸收     | absorption      | 22     |
| 饱和         | saturation      | 23     |
| 飘浮         | levitation      | 24     |
| 中毒（致命） | fatal_poison    | 25     |
| 潮涌能量     | conduit_power   | 26     |
| 缓降         | slow_falling    | 27     |
| 不祥之兆     | bad_omen        | 28     |
| 村庄英雄     | village_hero    | 29     |
| 黑暗         | darkness        | 30     |



### 其他实体函数 API

下面这些API提供了与游戏中指定位置实体互动的API

#### 在指定位置制造一次爆炸

`mc.explode(pos,source,maxResistance,radius,isDestroy,isFire)`  
`mc.explode(x,y,z,dimid,source,maxResistance,radius,isDestroy,isFire)`

- 参数：
  - pos : `IntPos `/ `FloatPos`  
    引发爆炸的位置坐标（或者使用x, y, z, dimid来确定实体位置）
  - source : `Entity`  
    设置爆炸来源的实体对象，可以为`Null`
  - maxResistance : `Float`  
    方块最大爆炸抗性，低于此值的方块会被破坏
  - radius : `Float`  
    爆炸的范围半径，影响爆炸的波及范围
  - isDestroy : `Boolean`  
    爆炸是否破坏方块
  - isFire : `Boolean`  
    爆炸结束后是否留下燃烧的火焰
- 返回值：是否成功制造爆炸
- 返回值类型：`Boolean`




#### 快速执行Molang表达式

`en.quickEvalMolangScript(str)`

- 参数：
  - str : `String`  
    Molang表达式
- 返回值：表达式执行结果
- 返回值类型：`Float`

关于Molang的详细使用方法，请参考 [MOLANG文档 bedrock.dev](https://bedrock.dev/zh/docs/stable/Molang)


