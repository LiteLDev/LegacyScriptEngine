# 🎈 实体相关事件

#### `"onMobDie"` - 生物死亡

- 监听函数原型
  `function(mob,source,cause)`
- 参数：
    - mob : `Entity`  
      死亡的实体对象
    - source : `Entity`  
      伤害来源的实体对象（可能为`Null`）
    - cause : `Integer`  
      死亡原因

- 拦截事件：不可以拦截

注意，当玩家死亡时，除了触发`onPlayerDie`事件，这个事件同样也会被触发一次

#### `"onMobHurt"` - 生物受伤（包括玩家）

- 监听函数原型
  `function(mob,source,damage,cause)`
- 参数：
    - mob : `Entity`  
      受伤的实体对象
    - source : `Entity`  
      伤害来源的实体对象（可能为`Null`）
    - damage : `Float`  
      受到的伤害数值
    - cause : `ActorDamageCause`  
      受伤原因
- 拦截事件：函数返回`false`

ActorDamageCause 为伤害原因枚举，枚举值如下，有问号的待验证，？？？待补充

| `ActorDamageCause` 枚举            | 值   | 解释                                     |
| ---------------------------------- | --- | ---------------------------------------- |
| `ActorDamageCause.None`            | -1  | 其他                                     |
| `ActorDamageCause.Override`        | 0 | 非正常方式（如脚本直接设置血量为0），这种方式的伤害不会被盔甲与buff吸收 |
| `ActorDamageCause.Contact`         | 1 | 接触伤害（如仙人掌）                       |
| `ActorDamageCause.EntityAttack`    | 2 | 实体攻击                                 |
| `ActorDamageCause.Projectile`      | 3 | 抛射物攻击                               |
| `ActorDamageCause.Suffocation`     | 4 | 窒息（密封空间）                          |
| `ActorDamageCause.Fall`            | 5 | 掉落                                     |
| `ActorDamageCause.Fire`            | 6 | 燃烧                                     |
| `ActorDamageCause.FireTick`        | 7 | 点燃                                     |
| `ActorDamageCause.Lava`            | 8 | 熔岩                                     |
| `ActorDamageCause.Drowning`        | 9 | 溺水                                     |
| `ActorDamageCause.BlockExplosion`  | 10 | 方块爆炸                                 |
| `ActorDamageCause.EntityExplosion` | 11 | 实体爆炸                                 |
| `ActorDamageCause.Void`            | 12 | 虚空                                     |
| `ActorDamageCause.SelfDestruct`    | 13 | 自杀                                     |
| `ActorDamageCause.Magic`           | 14 | 尖牙对生物造成的伤害、守卫者对生物造成的魔法伤害和药水伤害等         |
| `ActorDamageCause.Wither`          | 15 | 凋零效果                                   |
| `ActorDamageCause.Starve`          | 16 | 饥饿                                     |
| `ActorDamageCause.Anvil`           | 17 | 下落的铁砧                               |
| `ActorDamageCause.Thorns`          | 18 | 荆棘                                     |
| `ActorDamageCause.FallingBlock`    | 19 | 下落的方块                                |
| `ActorDamageCause.Piston`          | 20 | 活塞                                     |
| `ActorDamageCause.FlyIntoWall`     | 21 | 动能（滑翔撞墙）                          |
| `ActorDamageCause.Magma`           | 22 | 岩浆块                                    |
| `ActorDamageCause.Fireworks`       | 23 | 烟花                                     |
| `ActorDamageCause.Lightning`       | 24 | 闪电                                     |
| `ActorDamageCause.Charging`        | 25 | 充能（？）                               |
| `ActorDamageCause.Temperature`     | 26 | 温度 （雪人？）                          |
| `ActorDamageCause.Freezing`        | 27 | 冰冻                                     |
| `ActorDamageCause.Stalactite`      | 28 | 被钟乳石砸到                              |
| `ActorDamageCause.Stalagmite`      | 29 | 掉落到石笋上                              |
| `ActorDamageCause.RamAttack`       | 30 | 山羊冲撞                                  |
| `ActorDamageCause.SonicBoom`       | 31 | 坚守着音爆攻击                            |
| `ActorDamageCause.Campfire`        | 32 | 营火                                     |
| `ActorDamageCause.SoulCampfire`    | 33 | 灵魂营火                                 |
| `ActorDamageCause.MaceSmash`       | 34 | 下落攻击                                 |
| `ActorDamageCause.All`             | 35 | 所有                                     |

#### `"onEntityExplode"` - 发生由实体引起的爆炸

- 监听函数原型
  `function(source,pos,radius,maxResistance,isDestroy,isFire)`
- 参数：
    - source : `Entity`  
      爆炸来源的实体对象
    - pos : `FloatPos`  
      爆炸发生的坐标
    - radius : `Float`
      爆炸波及的半径
    - maxResistance : `Float`  
      爆炸可破坏的方块爆炸抗性上限
    - isDestroy : `Boolean`  
      爆炸是否破坏方块
    - isFire : `Boolean`  
      爆炸是否产生火焰

- 拦截事件：函数返回`false`

#### `"onMobTrySpawn"` - 发生于实体尝试自然生成

- 监听函数原型
  `function(typeName,pos)`
- 参数：
    - typeName : `string`  
      生成实体名称
    - pos : `FloatPos`  
      生成的坐标

- 拦截事件：函数返回`false`

#### `"onMobSpawned"` - 发生于实体自然生成完成

- 监听函数原型
  `function(entity,pos)`
- 参数：
    - entity : `Entity`  
      生成的实体对象
    - pos : `FloatPos`  
      生成的坐标

- 拦截事件：不可拦截

此事件为实体成功生成后触发，不可直接拦截，如需拦截请使用entity.despawn()或entity.remove()

#### `"onProjectileHitEntity"` - 实体被弹射物击中

- 监听函数原型
  `function(entity,source)`
- 参数：
    - entity: `Entity`  
      被击中的实体对象
    - source : `Entity`  
      发射的弹射物实体（如箭）
- 拦截事件：不可以拦截

#### `"onWitherBossDestroy"` - 凋灵破坏方块

- 监听函数原型
  `function(witherBoss,AAbb,aaBB)`
- 参数：
    - witherBoss: `Entity`  
      凋灵的实体对象

    - AAbb: `IntPos`

      凋灵将破坏的区域（长方体），对角点A坐标

    - aaBB: `IntPos`

      凋灵将破坏的区域（长方体），对角点B坐标

- 拦截事件：函数返回`false`

注意，此事件不包括凋灵爆炸的破坏。

#### `"onRide"` - 生物骑乘

- 监听函数原型
  `function(entity1,entity2)`
- 参数：
    - entity1 : `Entity`  
      尝试骑乘的实体对象
    - entity2 : `Entity`  
      被骑乘的实体对象
- 拦截事件：函数返回`false`

注：骑乘包括坐矿车、坐船、骑马、骑猪等。

#### `"onStepOnPressurePlate"` - 生物踩压力板

- 监听函数原型
  `function(entity,pressurePlate)`
- 参数：
    - entity : `Entity`  
      踩压力板的实体对象
    - pressurePlate : `Block`  
      被踩的压力板方块对象
- 拦截事件：函数返回`false`

注：生物踩压力板时，将会反复多次触发此事件。

#### `"onSpawnProjectile"` - 弹射物创建

- 监听函数原型
  `function(shooter,type)`
- 参数：

    - shooter : `Entity`  
      发射弹射物的的实体对象

    - type : `String`  
      弹射物标准类型名

- 拦截事件：函数返回 `false`

注：已知可拦截的弹射物有鸡蛋、末影珍珠、雪球、三叉戟、箭、钓竿（鱼钩）。

#### `"onProjectileCreated"` - 弹射物创建完毕

- 监听函数原型
  `function(shooter,entity)`
- 参数：

    - shooter : `Entity`  
      创建此弹射物的的实体对象

    - entity : `Entity`  
      被创建的弹射物实体对象

- 拦截事件：不可以拦截

#### `"onNpcCmd"` - NPC执行命令

!!! warning
    此事件仅在0.9.6及更高版本中可用。

- 监听函数原型
  `function(npc,pl,cmd)`
- 参数：
    - npc : `Entity`  
      执行命令的NPC实体对象
    - pl : `Player`  
      触发NPC命令执行的玩家对象
    - cmd : `String`  
      NPC执行的命令，如果有多条命令，以`;`分隔
- 拦截事件：函数返回`false`

#### `"onChangeArmorStand"` - 操作盔甲架

- 监听函数原型
  `function(as,pl,slot)`

- 参数：

    - as: `Entity`  
      被操作的盔甲架实体对象

    - pl : `Player`  
      操作盔甲架的玩家对象

    - slot : `Number`

      装备栏编号

- 拦截事件：函数返回`false`

#### `"onEntityTransformation"` - 实体转变

- 监听函数原型
  `function(uniqueId,entity)`
- 参数：

    - uniqueId: `String`
      转变前的实体的唯一标识符
    - entity : `Entity`
      转换完成的实体
- 拦截事件：不可以拦截

注：此事件为 `Addons` 中实体的 `TransformationComponent` 激活时触发，多用于引擎与Addon交互。由于转变前的实体指针很快被销毁，因此只提供
`UniqueId`。


#### `"onEndermanTakeBlock"` - 末影人搬运方块

!!! warning
    此事件仅在0.11.3及更高版本中可用。

- 监听函数原型
  `function(entity, block, pos)`
- 参数：
    - entity : `Entity`  
      搬运方块的末影人
    - block : `Block`  
      被搬运的方块
    - pos : `BlockPos`  
      被搬运的方块坐标
- 拦截事件：函数返回`false`