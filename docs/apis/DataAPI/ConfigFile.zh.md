# 脚本引擎 - 配置与数据处理接口文档

> 这里为你提供了处理 **大量数据** 时的多种可选方案

在插件工作过程中，难免会碰到需要处理配置和大量游戏相关数据的场景。  
脚本引擎为脚本插件提供了大量**基础设施**，包括配置文件、数据库、经济系统等。  
方便开发者专注于业务代码实现，而非纠结于这些方面的技术细节。  

## 🔨 配置文件 API

配置文件，一般用于将插件的某些可供用户修改的设置项独立成文件，以方便修改某些设置。  
因此，配置文件的内容和格式一般需要有一定的可读性。  
脚本引擎提供 ConfigFile 配置文件接口来完成这个任务。  
当然，你也可以手动读写文件来协助相关配置文件的操作。  



### 说明：配置文件类型的选择

配置文件格式的选择，将影响你可以储存的配置数据类型。  

- `JSON`（**JS** **O**bject **N**otation） 格式可以储存**大多数**类型的数据，包括数组、对象等，表示的逻辑相对丰富  
- `Ini`（**Ini**tialization File） 格式相对**简单直观**，他人修改起来非常轻松，不过储存的数据类型受到一定限制

请按需做出选择。



### 📰 JSON 格式配置文件

#### 创建 / 打开一个 JSON 配置文件

[JavaScript] `new JsonConfigFile(path[,default])`  
[Lua] `JsonConfigFile(path[,default])`

- 参数：
  - path : `String`  
    配置文件所在路径，以BDS根目录为基准  
    如果配置文件路径中有目录尚不存在，脚本引擎会自动创建
  - default : `String`  
    （可选参数）配置文件的默认内容。  
    如果初始化时目标文件**不存在**，引擎将新建一个配置文件并将此处的默认内容写入文件中。  
    如果不传入此参数，新建时的配置文件将为空
- 返回值：打开 / 创建的配置文件对象
- 返回值类型：`JsonConfigFile`
  - 如果创建 / 打开失败，将抛出异常

我们建议你在 `BDS根目录/plugins/插件名字/` 目录下建立名为`config.json`的配置文件，以保持各插件的配置统一



对于一个 JSON 配置文件对象`conf`，你有这些读写接口可用

#### 初始化配置项（方便函数）

`conf.init(name,default)`

- 参数：
  - name : `String`  
    配置项名字
  - default : `任意类型`  
    配置项初始化时写入的值
- 返回值：指定配置项的数据
- 返回值类型：`任意类型`，以具体储存的数据类型为准

这里提供了一种简便的方法来初始化配置文件，避免了需要手写默认配置文件内容的麻烦  

如果`init`访问的配置项不存在，那么引擎将在配置文件中自动创建此项，并写入给出的默认值  
如果`init`访问的配置项已经存在，引擎将读取并返回配置文件中已有的值



#### 写入配置项

`conf.set(name,data)`

- 参数：
  - name : `String`  
    配置项名字
  - data : `指定类型`  
    要写入的配置数据。允许的数据类型有：  
    `Integer` `Float` `String` `Boolean` `Array` `Object`  
    其中，`Array` 和 `Object` 内部仅能嵌套上面这些元素

- 返回值：是否写入成功

- 返回值类型：`Boolean`



#### 读取配置项

`conf.get(name[,default])`

- 参数：
  - name : `String`  
    配置项名字
  - default : `任意类型`  
    （可选参数）当读取失败时返回的默认值  
    默认为`Null`
- 返回值：指定配置项的数据
- 返回值类型：`任意类型`，以具体储存的数据类型为准



#### 删除配置项

`conf.delete(name)`

- 参数：
  - name : `String`  
    配置项名字
- 返回值：是否删除成功
- 返回值类型：`Boolean`

如果这个配置项你不需要了，为了避免在他人修改配置文件时引起迷惑，你可以选择将它删除



### 📄 Ini 格式配置文件

#### 创建 / 打开一个 Ini 配置文件

[JavaScript] `new IniConfigFile(path[,default])`  
[Lua] `IniConfigFile(path[,default])`

- 参数：
  - path : `String`  
    配置文件所在路径，以BDS根目录为基准  
    如果配置文件路径中有目录尚不存在，脚本引擎会自动创建
  - default : `String`  
    （可选参数）配置文件的默认内容。  
    如果初始化时目标文件**不存在**，引擎将新建一个配置文件并将此处的默认内容写入文件中。  
    如果不传入此参数，新建时的配置文件将为空
- 返回值：打开 / 创建的配置文件对象
- 返回值类型：`IniConfigFile`
  - 如果创建 / 打开失败，将抛出异常

我们建议你在 `BDS根目录/plugins/插件名字/` 目录下建立名为`config.ini`的配置文件，以保持各插件的配置统一



对于一个 Ini 配置文件对象`conf`，你有这些读写接口可用

#### 初始化配置项（方便函数）

`conf.init(section,name,default)`

- 参数：
  - section : `String`  
    配置项键名
  - name : `String`  
    配置项名字
  - default : `指定类型`  
    配置项初始化时写入的值。允许的数据类型有：  
    `Integer` `Float` `String` `Boolean`
- 返回值：指定配置项的数据
- 返回值类型：`任意类型`，以具体储存的数据类型为准

这里提供了一种简便的方法来初始化配置文件，避免了需要手写默认配置文件内容的麻烦  

如果`init`访问的配置项不存在，那么引擎将在配置文件中自动创建此项，并写入给出的默认值  
如果`init`访问的配置项已经存在，引擎将读取并返回配置文件中已有的值



#### 写入配置项

`conf.set(section,name,data)`

- 参数：
  - section : `String`  
    配置项键名
  - name : `String`  
    配置项名字
  - data : `指定类型`  
    要写入的配置数据。允许的数据类型有：  
    `Integer` `Float` `String` `Boolean`

- 返回值：是否写入成功

- 返回值类型：`Boolean`

如果配置项不存在，接口会自动创建



#### 读取配置项

读取字符串 `conf.getStr(section,name[,default])`  
读取整数项 `conf.getInt(section,name[,default])`  
读取浮点数 `conf.getFloat(section,name[,default])`  
读取布尔值 `conf.getBool(section,name[,default])`  

- 参数：
  - section : `String`  
    配置项键名
  - name : `String`  
    配置项名字
  - default :  `String`/ `Integer`/ `Float`/ `Boolean`  
    （可选参数）当读取失败时返回的默认值  
    默认为`0`
- 返回值：指定配置项的数据
- 返回值类型：`String`/ `Integer`/ `Float`/ `Boolean`



#### 删除配置项

`conf.delete(section,name)`

- 参数：
  - section : `String`  
    配置项键名
  - name : `String`  
    配置项名字
- 返回值：是否删除成功
- 返回值类型：`Boolean`

如果这个配置项你不需要了，为了避免在他人修改配置文件时引起迷惑，你可以选择将它删除



### 💼 其他的通用接口函数

对于一个配置文件对象`conf`，你可以使用这些辅助用途的通用接口

#### 重新加载文件中的配置项

`conf.reload()`

- 返回值：是否成功加载

为了性能考虑，配置文件接口对读操作进行缓存，每次读取操作都从直接内存中读取，而写入才会直接写入磁盘文件。考虑到配置文件可能被用户修改，当你确认用户已经修改配置文件之后，需要使用此函数刷新配置文件的内存缓存数据。



#### 关闭配置文件

`conf.close()`

- 返回值：是否成功关闭

配置文件关闭之后，请勿继续使用！



#### 获取配置文件路径

`conf.getPath()`

- 返回值：当前配置文件的文件路径
- 返回值类型：`String`



#### 读取整个配置文件的内容

`conf.read()`

- 返回值：当前配置文件的所有内容
- 返回值类型：`String`



#### 写入整个配置文件的内容

`conf.write(content)`

- 参数：
  - content : `String`  
    写入的内容
- 返回值：是否写入成功
- 返回值类型：`Boolean`

