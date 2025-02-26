# 🏃‍♂️ 玩家绑定数据

在实际开发中，经常有需要将某些数据和服务器中的某个玩家相关联，并在插件的工作周期中不断维护这些数据的需求。  

为此，脚本引擎设计了玩家绑定数据接口。绑定数据接口使用键 - 值对的形式储存数据。    
在你将数据绑定到某个玩家上以后，即使在玩家对象超出作用域被销毁，甚至当此玩家退出游戏时，玩家绑定数据将仍然存在。当你再次获得这个玩家的玩家对象的时候，仍然可以读取到之前储存的绑定数据。  
只有当服务端关闭的时候，所有的数据才会被统一销毁。

由此，脚本引擎给予开发者在插件的整个生命周期中跟踪某个特定玩家相关数据的能力。  

对于某个特定的玩家对象`pl`，有如下这些接口：

#### 储存玩家绑定数据

`pl.setExtraData(name,data)`

- 参数：
  - name : `String`  
    要储存到绑定数据的名字
  - data : `任意类型`  
    你要储存的绑定数据，可以是`Null`

- 返回值：是否成功储存
- 返回值类型：`Boolean` 

#### 获取玩家绑定数据

`pl.getExtraData(name)`

- 参数：
  - name : `String`  
    要读取的绑定数据的名字
- 返回值：储存的绑定数据
- 返回值类型：`任意类型`，取决于储存的数据类型
  -  如返回值为 `Null` 则表示未获取到指定的绑定数据，或者数据为空

#### 删除玩家绑定数据

`pl.delExtraData(name)`

- 参数：
  - name : `String`  
    要删除的绑定数据的名字
- 返回值：是否删除成功
- 返回值类型：`Boolean`

## 👨‍💻 XUID 数据库

XUID数据库让你可以即使在玩家离线的时候，也可以查询玩家名字与XUID的对应关系。  
当一个玩家第一次进服的时候，他的名字和XUID就会被自动记录在内置 XUID 数据库中。使用下面的函数来进行相关查询

#### 根据玩家名查询XUID

`data.name2xuid(name)`

- 参数：
  - name : `String`  
    要查询的玩家名
- 返回值：玩家的XUID
- 返回值类型：`String`
  - 如果返回值为`Null`，则代表查询失败

#### 根据XUID查询玩家名

`data.xuid2name(xuid)`

- 参数：
  - xuid: `String`  
    要查询的玩家XUID
- 返回值：玩家名
- 返回值类型：`String`
  - 如果返回值为`Null`，则代表查询失败

#### 根据玩家名查询UUID

`data.name2uuid(name)`

- 参数：
  - name : `String`  
    要查询的玩家名
- 返回值：玩家的UUID
- 返回值类型：`String`
  - 如果返回值为`Null`，则代表查询失败

#### 根据XUID查询玩家UUID

`data.xuid2uuid(xuid)`

- 参数：
  - xuid: `String`  
    要查询的玩家XUID
- 返回值：玩家的UUID
- 返回值类型：`String`
  - 如果返回值为`Null`，则代表查询失败

#### 获取所有的玩家信息

`data.getAllPlayerInfo()`

- 返回值: 所有的玩家信息
- 返回值类型: `Array<Object>`
  - 每个对象都含有以下属性:
    - `name`: 玩家名
    - `xuid`: 玩家XUID
    - `uuid`: 玩家UUID

提示：XUID数据库中储存的玩家名为玩家对象对应的`realName`字段

!!! warning
    以下API均为0.8.13新API，使用以下API将导致插件无法兼容旧版
#### 根据XUID查询玩家信息

`data.fromXuid(xuid)`

- 参数:
  - xuid: `String`
    要查询玩家的XUID
- 返回值： 玩家信息条目，例如 `{xuid:1145141919810,name:yjsp,uuid:2a30fa4a-3a63-3370-88a8-144a941101e2}`
- 返回值类型： `Object`
  - 如果返回值为`Null`，则代表查询失败

#### 根据UUID查询玩家信息

`data.fromUuid(uuid)`

- 参数:
  - uuid: `String`
    要查询玩家的UUID
- 返回值： 玩家信息条目，例如 `{xuid:1145141919810,name:yjsp,uuid:2a30fa4a-3a63-3370-88a8-144a941101e2}`
- 返回值类型： `Object`
  - 如果返回值为`Null`，则代表查询失败

#### 根据名字查询玩家信息

`data.fromName(name)`

- 参数:
  - name: `String`
    要查询玩家的名字
- 返回值： 玩家信息条目，例如 `{xuid:1145141919810,name:yjsp,uuid:2a30fa4a-3a63-3370-88a8-144a941101e2}`
- 返回值类型： `Object`
  - 如果返回值为`Null`，则代表查询失败
