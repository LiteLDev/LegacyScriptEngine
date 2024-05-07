# 💡 插件加载相关 API

这里提供了一些与加载器操作相关的接口。

### Properties

| 属性                     | 类型      | 描述                                                 |
| ------------------------ | --------- | ---------------------------------------------------- |
| `ll.language`            | `String`  | LeviLamina使用的语言。(例如`zh_Hans`、`en`和`ru_RU`) |
| `ll.major`               | `Integer` | 主版本号（如 **2**.1.0 里的 **2**）                  |
| `ll.minor`               | `Integer` | 次版本号（如 2.**1**.0 里的 **1**）                  |
| `ll.revision`            | `Integer` | 修订版本号（如 2.1.**0** 里的 **0**）                |
| `ll.status`              | `Integer` | 版本状态 (`0`为Dev, `1`为Beta, `2`为Release)         |
| `ll.scriptEngineVersion` | `String`  | LeviLamina Script Engine版本                         |
| `ll.isWine`              | `Boolean` | 是否处于Wine环境下                                   |
| `ll.isDebugMode`         | `Boolean` | 是否处于debug模式                                    |
| `ll.isBeta`              | `Boolean` | 当前版本是否为测试版                                 |
| `ll.isDev`               | `Boolean` | 当前版本是否为开发版                                 |
| `ll.isRelease`           | `Boolean` | 当前版本是否为发布版本                               |


### 获取 LeviLamina 版本字符串

`ll.versionString()`

- 返回值：加载器版本
- 返回值类型： `String`

### 获取有关插件的信息

`ll.getPluginInfo(name)`

- 参数:
  - name: `String`
  插件名称
- 返回值: 插件对象
- 返回值类型:  `Plugin`
  
  - 对于返回的某个插件对象 plugin，有如下这些属性：  

  | 属性              | 描述                 | 类型                             |
  | ----------------- | -------------------- | -------------------------------- |
  | plugin.name       | 插件名称             | `String`                         |
  | plugin.desc       | 插件描述             | `String`                         |
  | plugin.type       | 插件类型             | `String`                         |
  | plugin.version    | 插件版本（数组形式） | `Array<Integer,Integer,Integer>` |
  | plugin.versionStr | 插件版本             | `String`                         |
  | plugin.filePath   | 插件路径             | `String`                         |
  | plugin.others     | 其他信息             | `Object`                         |

### 列出所有已加载的插件

`ll.listPlugins()`

- 返回值：已加载的所有的插件名字列表
- 返回值类型： `Array<String,String,...>`

### 列出所有加载的插件信息

`ll.getAllPluginInfo()`

- 返回值: 包含所有已加载插件的插件对象的列表
- 返回值类型:  `Array<Plugin,Plugin,...>`

### 远程函数调用

#### 导出函数

为了可以让开发者开发的前置插件能够为其他插件提供接口和服务，这里提供了远程函数调用功能，让一个 原生 或 脚本 插件可以调用另一个插件中已有的函数。

`ll.exports(func,namespace,name)`

- 参数：
  - func : `Function`  
    要导出的函数
  - namespace : `String`  
    函数的命名空间名，只是方便用于区分不同插件导出的API
  - name : `String`  
    函数的导出名称。其他插件根据导出名称来调用这个函数
- 返回值：是否成功导出
- 返回值类型： `Boolean`

注意：如果导出的函数的命名空间和名字与另一个已经导出的函数完全相同，将会导出失败。选定命名空间和导出名称时请适当选择。

#### 导入函数

当你已经得知有插件导出函数之后，为了可以使用他导出的函数，首先需要将这个函数导入到你自己的脚本系统中  
脚本引擎提供了接口 import 来导入其他插件已经导出的函数。

`ll.imports(namespace,name)`

- 参数：
  - namespace : `String`  
    要导入的函数使用的命名空间名称
  - name : `String`  
    要导入的函数使用的导出名称
- 返回值：导入的函数
- 返回值类型： `Function`

`ll.import` 的返回值是一个函数。当你调用这个函数时，跨插件调用的流程将在后台自动完成。调用函数的参数将被包装并传递给远程函数，此函数的返回值即是远程函数执行完毕之后返回的返回值。

#### 远程调用参数类型对照，其中Type可以为其他受支持的类型

| C++层类型                                     | 脚本引擎类型  | .NET托管类型                             | 内部类型（备注）                       |
| --------------------------------------------- | ------------- | ---------------------------------------- | -------------------------------------- |
| `std::nullptr_t`                              | `Null`        | `null` / `Nothing` / `nullptr`           | `std::nullptr_t`                       |
| `bool`                                        | `Boolean`     | `Boolean`                                | `bool`                                 |
| `__int64`, `double`...                        | `Number`      | `Int64`, `Double`...                     | `RemoteCall::NumberType`               |
| `std::string`                                 | `String`      | `String`                                 | `std::string`                          |
| `std::vector<Type>`                           | `Array`       | `List<Type>`                             | `std::vector<Type>`                    |
| `std::unordered_map<std::string,Type>`        | `Object`      | `Dictionary<String,Type>`                | `std::unordered_map<std::string,Type>` |
| `Actor*`                                      | `Entity`      | `MC.Actor`                               | `Actor*`                               |
| `Player*`                                     | `Player`      | `MC.Player`                              | `Player*`                              |
| `ItemStack*`, `std::unique_ptr<ItemStack>`    | `Item`        | `RemoteCall.ItemType`                    | `RemoteCall::ItemType`                 |
| `Block*`, `BlockInstance`                     | `Block`       | `RemoteCall.BlockType`                   | `RemoteCall::BlockType`                |
| `BlockActor*`                                 | `BlockActor`  | `MC.BlockActor`                          | `BlockActor*`                          |
| `Container*`                                  | `Container`   | `MC.Container`                           | `Container*`                           |
| `Vec3`,`std::pair<Vec3,int>`                  | `FloatPos`    | `MC.Vec3`, `RemoteCall.WorldPosType`     | `RemoteCall::WorldPosType`             |
| `BlockPos`,`std::pair<BlockPos, int>`         | `IntPos`      | `MC.BlockPos`, `RemoteCall.BlockPosType` | `RemoteCall::BlockPosType`             |
| `CompoundTag*`,`std::unique_ptr<CompoundTag>` | `NBTCompound` | `RemoteCall.NbtType`                     | `RemoteCall::NbtType`                  |

#### 远程调用函数举例说明

比如，有一个插件导出了某个函数，函数导出使用的命名空间为 AAA，导出函数名称为 Welcome  
当你使用 `welcome = ll.import("AAA","welcome"); ` 完成导入之后，你就可以直接在下面执行：

`welcome("hello",2,true);`     

函数的参数将被自动转发到对应的目标函数执行，执行完毕之后将返回回应的目标函数的返回值，整个过程都是自动完成的。  

注意！在调用函数的时候，需要保证你传入的参数和目标函数接受的参数数量和类型都是正确且一一对应的。否则，将会发生错误。

### 判断远程函数是否已导出
`ll.hasExported(namespace,name)`

- 参数：
  - namespace : `String`  
    函数使用的命名空间名称
  - name : `String`  
    函数使用的导出名称
- 返回值：函数是否已导出
- 返回值类型： `Boolean`

### 将字符串作为脚本代码执行

`ll.eval(str)`

- 参数：
  - str : `String`  
    要作为脚本代码执行的字符串
- 返回值：执行结果
- 返回值类型： `任意类型`

