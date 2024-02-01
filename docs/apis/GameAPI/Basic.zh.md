# 🎨 脚本引擎 - 游戏元素接口文档

> 下列这些API，提供了对游戏内容进行 **修改** 和 **扩展** 的能力。  

显然，游戏元素接口处于整个插件体系中的核心位置，关乎到对游戏内容的控制和扩展能力。  
脚本引擎提供了丰富强大的游戏元素接口，为你发挥创意提供方便。

## 🔮 游戏元素对象

对于游戏元素的索引，脚本引擎使用专门类型的变量来跟踪每一个游戏元素，并称其为「xx对象」，如「玩家对象」或者「方块对象」。  
你可以将其理解为游戏元素的唯一标识符。

目前，脚本引擎拥有的游戏元素对象如下：

| 对象类型名       | 对象称呼        | 对象实际意义                               |
| ---------------- | --------------- | ------------------------------------------ |
| `IntPos`         | 整数 坐标对象   | 标识一个整数位置（如方块坐标）             |
| `FloatPos`       | 浮点数 坐标对象 | 标识一个浮点数位置（如实体坐标）           |
| `DirectionAngle` | 方向角对象      | 标识一个欧拉角角度信息（如玩家朝向）       |
| `Player`         | 玩家对象        | 标识一个玩家                               |
| `Entity`         | 实体对象        | 标识一个实体                               |
| `Block`          | 方块对象        | 标识一个具体的方块                         |
| `Item`           | 物品栏物品对象  | 标识一个物品栏中的物品                     |
| `Container`      | 容器对象        | 标识一个拥有格子、可以储存和放置物品的容器 |
| `BlockEntity`    | 方块实体对象    | 标识一个方块实体                           |
| `Objective`      | 记分项对象      | 标识一个记分板系统的记分项                 |

在后续的文档中，你会频繁地接触到他们。

## 🎯 坐标对象

在游戏中，数量众多的 API 都需要提供坐标。  
引擎采用 `IntPos` 和 `FloatPos` 类型的对象来标示坐标，称之为「坐标对象」。
坐标对象的各个成员都是**可读写**的。

1. `IntPos`对象
   它的成员均为**整数**，多用来表示**方块坐标**等用整数表示的位置  
   对于某个 `IntPos` 类型变量 pos，有如下这些成员：  

   | 成员      | 含义       | 类型      |
   | --------- | ---------- | --------- |
   | pos.x     | x 坐标     | `Integer` |
   | pos.y     | y 坐标     | `Integer` |
   | pos.z     | z 坐标     | `Integer` |
   | pos.dim   | 维度文字名 | `String`  |
   | pos.dimid | 维度ID     | `Integer` |

   其中，**维度ID** 属性的取值为：`0` 代表主世界，`1` 代表下界，`2` 代表末地
   **维度文字名** 属性的取值分别为："主世界"，"下界"，"末地"

   如果某种情况下维度无效，或者无法获取，你会发现`dimid`的值为-1

   <br>

2. `FloatPos`对象
   它的成员均为**浮点数**，多用来表示**实体坐标**等用无法用整数表示的位置  
   对于某个 `FloatPos` 类型变量 pos，有如下这些成员：  

   | 成员      | 含义       | 类型      |
   | --------- | ---------- | --------- |
   | pos.x     | x 坐标     | `Float`   |
   | pos.y     | y 坐标     | `Float`   |
   | pos.z     | z 坐标     | `Float`   |
   | pos.dim   | 维度文字名 | `String`  |
   | pos.dimid | 维度ID     | `Integer` |

   其中，**维度ID** 属性的取值为：`0` 代表主世界，`1` 代表下界，`2` 代表末地
   **维度文字名** 属性的取值分别为："主世界"，"下界"，"末地"

   如果某种情况下维度无效，或者无法获取，你会发现`dimid`的值为-1

### 坐标对象辅助接口

对于自然支持面向对象的脚本语言，你可以直接构造坐标对象，并传入x, y, z, dimid参数  
对于某些对面向对象支持一般的语言，脚本引擎也提供了辅助接口，帮助更方便地生成一个坐标对象

#### 生成一个整数坐标对象

[JavaScript] `new IntPos(x,y,z,dimid)`  
[Lua] `IntPos(x,y,z,dimid)`

- 参数：
  - x : `Integer`  
    x 坐标
  - y : `Integer`  
    y 坐标
  - z : `Integer`  
    z 坐标
  - dimid : `Integer`  
    维度ID：`0` 代表主世界，`1` 代表下界，`2` 代表末地  
- 返回值：一个整数坐标对象
- 返回值类型：`IntPos`

#### 生成一个浮点数坐标对象

[JavaScript] `new FloatPos(x,y,z,dimid)`  
[Lua] `FloatPos(x,y,z,dimid)`

- 参数：
  - x : `Float`  
    x 坐标
  - y : `Float`  
    y 坐标
  - z : `Float`  
    z 坐标
  - dimid : `Integer`  
    维度ID：`0` 代表主世界，`1` 代表下界，`2` 代表末地  
- 返回值：一个浮点数坐标对象
- 返回值类型：`FloatPos`

## 📐 方向角对象

引擎采用 `DirectionAngle` 对象来标示一个欧拉角，称之为「方向角对象」。  
它的两个成员均为**浮点数**，多用来表示实体的朝向等方向数据
方向角对象的各个成员都是**可读写**的。

对于某个 `DirectionAngle` 类型变量 ang，有如下这些成员：  

| 成员      | 含义       | 类型      |
| --------- | ---------- | --------- |
| ang.pitch  | 俯仰角（-90° ~ 90°） | `Float` |
| ang.yaw | 偏航角（旋转角） | `Float`   |

由于MC的实体系统不存在 自转 的概念，所以没有翻滚角相关数据

#### 创建偏航角

[JavaScript] `new DirectionAngle(pitch, yaw)`  
[Lua] `DirectionAngle(pitch, yaw)`

- 参数：
  - pitch : `Float`  
    俯仰角
  - yaw : `Float`  
    偏航角（旋转角）
- 返回值：一个方向角对象
- 返回值类型：`DirectionAngle`

#### 将偏航角转换为基本朝向

`ang.toFacing()`

- 返回值：当前方向角对象所指示的基本朝向
- 返回值类型：`Integer`

返回值为`0-3`，代表 **北东南西** 四个基本朝向。用于快速确定实体面向的大致方向

## 获取结构NBT

`mc.getStructure(pos1, pos2, ignoreBlocks = false, ignoreEntities = false)`

- 参数：

  - pos1 : `IntPos` 对角坐标1，填写方式与 [fill命令](https://minecraft.fandom.com/zh/wiki/%E5%91%BD%E4%BB%A4/fill?so=search#%E5%8F%82%E6%95%B0 "在维基百科中查看") 的 `from` 参数类似
  - pos2 : `IntPos` 对角坐标2，填写方式与 [fill命令](https://minecraft.fandom.com/zh/wiki/%E5%91%BD%E4%BB%A4/fill?so=search#%E5%8F%82%E6%95%B0 "在维基百科中查看") 的 `to` 参数类似
  - ignoreBlocks : `Boolean` 忽略方块
  - ignoreEntities : `Boolean` 忽略实体
- 返回值类型：`NbtCompound`

## 放置结构NBT

`mc.setStructure(nbt, pos, mirror = 0, rotation = 0)`

- 参数：

  - nbt : `NbtCompound`
  - pos : `IntPos` 放置坐标
  - mirror : `number` 镜像模式
    - `0: 不镜像` `1: X轴` `2: Z轴` `3: XZ轴`
  - rotation : `number` 旋转角度
    - `0: 不旋转` `1: 旋转90°` `2: 旋转180°` `3: 旋转270°`
- 返回值类型：`Boolean`
