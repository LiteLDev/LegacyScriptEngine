# 🎓 数据包 API

下面这些对象与 API 为脚本提供了基本的 BDS 数据包接口。

温馨提示：此类 API 需要部分逆向基础，了解数据包结构（可通过参考 Nukkit，PokcetMine，BDS 逆向得知数据包结构）如出现客户端崩溃，为数据包结构错误，并非 BUG。

文档不列出数据包 ID 与其结构，请自行查询。

## 目录

- 🔉 [数据包对象 API](#-数据包对象-api)
- 🔌 [二进制流对象 API](#-二进制流对象-api)

## 🔉 数据包对象 API

在脚本引擎中，使用「数据包对象」来获取数据包的相关信息。

### 获取一个数据包对象

#### 从 API 获取

调用某些**返回数据包对象**的函数，来获取到 BDS 给出的数据包对象  
详见 [二进制流对象](#-二进制流对象-api)

### 数据包对象 - 函数

每一个数据包对象都包含一些可以执行的成员函数（成员方法）。对于某个特定的实体对象`pkt`，可以通过以下这些函数对这个数据包进行一些操作

#### 获取数据包名称

`pkt.getName()`

- 返回值：数据包名称
- 返回值类型： `String`

#### 获取数据包 ID

`pkt.getId()`

- 返回值：数据包 ID
- 返回值类型： `Integer`

#### 发送数据包到指定目标

!!! warning
此函数仅在 0.19.0 及以后版本可用

`pkt.sendTo(pos)`  
`pkt.sendTo(x,y,z,dimid)`  
`pkt.sendTo(target)`

- 参数：

  - pos : `IntPos` / `FloatPos`  
    数据包发送目标所在坐标（或者使用 x, y, z, dimid 来确定目标位置）
  - target : `Player` / `Entity`  
    数据包发送目标

- 返回值：是否成功
- 返回值类型： `Boolean`

如果 `target` 是 `Player`，则数据包会发送到指定玩家。  
如果 `target` 是 `Entity`，则数据包会发送到指定实体周围的玩家。

#### 发送数据包到所有客户端

!!! warning
此函数仅在 0.19.0 及以后版本可用

`pkt.sendToClients()`

- 返回值：是否成功
- 返回值类型： `Boolean`

#### 发送数据包到服务端

!!! warning
此函数仅在 0.19.0 及以后版本可用

`pkt.sendToServer()`

- 返回值：是否成功
- 返回值类型： `Boolean`

## 🔌 二进制流对象 API

### 创建一个二进制流对象

[JavaScript] `new BinaryStream()`

[Lua] `BinaryStream()`

- 返回值：二进制流对象
- 返回值类型： `BinaryStream`

### 二进制流对象 - 函数

每一个二进制流对象都包含一些可以执行的成员函数（成员方法）。对于某个特定的实体对象`bs`，可以通过以下这些函数对这个二进制流进行一些操作

#### 重置二进制流

`bs.reset()`

- 返回值：是否成功
- 返回值类型： `Boolean`

#### 获取二进制流读指针

!!! warning
此函数仅在 0.19.0 及以后版本可用

`bs.getReadPointer()`

- 返回值：当前读指针
- 返回值类型： `Integer`

#### 设置二进制流读指针

!!! warning
此函数仅在 0.19.0 及以后版本可用

`bs.setReadPointer(pos)`

- 参数：

  - pos : `Integer`  
    新的读指针

- 返回值：是否成功
- 返回值类型： `Boolean`

#### 获取二进制流数据

!!! warning
此函数的可选参数和 `ByteBuffer` 返回值仅在 0.19.0 及以后版本可用

`bs.getData([clear])`

- 参数：

  - clear : `Boolean`（可选参数）  
    获取后是否清空流数据。默认值为 `true`

- 返回值：二进制流数据
- 返回值类型： `ByteBuffer`

在 0.19.0 之前，此函数只能使用 `bs.getData()` 形式调用。  
当时它在获取后总是会清空流数据，且返回值类型为 `String`。

由于旧版返回的是 `String`，在 JavaScript 中可能会被强制按 UTF-8 编码处理，导致二进制数据损坏，拿到的数据不正确。

#### 设置二进制流数据

!!! warning
此函数仅在 0.19.0 及以后版本可用

`bs.setData(data)`

- 参数：

  - data : `ByteBuffer`  
    二进制流数据

- 返回值：是否成功
- 返回值类型： `Boolean`

#### 写入二进制流

`bs.writexxxx(value)`

- 参数：

  - value : `NULL`  
    参考下面表格
    部分数值写入函数也允许传入 `String`

- 返回值：是否成功
- 返回值类型： `Boolean`

| 可用函数                             | 参数类型             |
| ------------------------------------ | -------------------- |
| writeBool                            | `Boolean`            |
| writeByte                            | `Integer`            |
| writeBytes (0.19.0 时加入)           | `ByteBuffer`         |
| writeDouble                          | `Number` / `String`  |
| writeFloat                           | `Float` / `String`   |
| writeNormalizedFloat (0.19.0 时加入) | `Float` / `String`   |
| writeSignedBigEndianInt              | `Number` / `String`  |
| writeSignedInt                       | `Number` / `String`  |
| writeSignedInt64                     | `Number` / `String`  |
| writeSignedShort                     | `Integer` / `String` |
| writeString                          | `String`             |
| writeUnsignedChar                    | `Integer`            |
| writeUnsignedInt                     | `Number` / `String`  |
| writeUnsignedInt64                   | `Number` / `String`  |
| writeUnsignedShort                   | `Integer` / `String` |
| writeUnsignedVarInt                  | `Number` / `String`  |
| writeUnsignedVarInt64                | `Number` / `String`  |
| writeVarInt                          | `Number` / `String`  |
| writeVarInt64                        | `Number` / `String`  |
| writeVec3                            | `FloatPos`           |
| writeBlockPos (0.9.5 时加入)         | `BlockPos`           |
| writeCompoundTag                     | `NbtCompound`        |
| writeItem (0.9.5 时加入)             | `Item`               |
| writeUuid (0.19.0 时加入)            | `String`             |

#### 通过二进制流构建数据包

!!! warning
此函数的可选参数仅在 0.19.0 及以后版本可用

`bs.createPacket(pktid[,raw])`

- 参数：

  - pktid : `Integer`  
    数据包 ID
  - raw : `Boolean`（可选参数）  
    是否从当前二进制流数据创建原始网络数据包。默认值为 `false`

- 返回值：数据包对象
- 返回值类型： `Packet`

在 0.19.0 之前，此函数只能使用 `bs.createPacket(pktid)` 形式调用。

### 演示代码

向一个玩家发送 TextPacket 数据包

```js
mc.listen("onChat", (player, message) => {
  const text = "LLSE Packet Test";
  const bs = new BinaryStream();
  bs.reserve(text.length + 8);
  bs.writeBool(false);
  bs.writeByte(/* TextPacketPayload::mBody::MessageOnly (Variant Index) */0);
  bs.writeByte(/* TextPacketType::Raw (Enum) */0);
  bs.writeString(text);
  bs.writeString(""); // xuid
  bs.writeString(""); // platformId
  bs.writeString(""); // filtered message
  bs.createPacket(9).sendTo(player);
});
```
