# 🎓 Packet API

The following objects and APIs provide the basic BDS packet interface for scripts.

(Please refer to Nukkit, PokcetMine, BDS Reverse to know the packet structure) If the client crashes, it is a packet structure error, not a bug.

The documentation does not list the packet ID and its structure, please check it yourself.

## 目录

- 🔉 [Packet Object API](#-packet-object-api)
- 🔌 [Binary stream object API](#-binary-stream-object-api)

## 🔉 Packet Object API

In LLSE, 「Packet Object」 is used to get information about packets.

### Get a packet object

#### Get from API

Call some **return packet object** function to get to the packet object given by BDS  
See [Binary Stream Objects](#-binary-stream-object-api) for details

### Packet Objects - Functions

Every packet object contains some member functions (member methods) that can be executed. For a particular entity object `pkt`, some operations can be performed on this packet by these functions

#### Get packet name

`pkt.getName()`

- Return value：packet name
- Return value type： `String`

#### Get packet ID

`pkt.getId()`

- Return value：packet id
- Return value type： `Integer`

#### Send packet to specified target

!!! warning
This function is only available in 0.19.0 and later.

`pkt.sendTo(pos)`  
`pkt.sendTo(x,y,z,dimid)`  
`pkt.sendTo(target)`

- Parameters：

  - pos : `IntPos` / `FloatPos`  
    The coordinates of the packet send target (or use x, y, z, dimid to determine the target position)
  - target : `Player` / `Entity`  
    Packet send target

- Return value：success or not
- Return value type： `Boolean`

If `target` is `Player`, the packet will be sent to the specified player.  
If `target` is `Entity`, the packet will be sent to players around the specified entity.

#### Send packet to all clients

!!! warning
This function is only available in 0.19.0 and later.

`pkt.sendToClients()`

- Return value：success or not
- Return value type： `Boolean`

#### Send packet to server

!!! warning
This function is only available in 0.19.0 and later.

`pkt.sendToServer()`

- Return value：success or not
- Return value type： `Boolean`

## 🔌 Binary Stream Object API

### Create a binary stream object

[JavaScript] `new BinaryStream()`

[Lua] `BinaryStream()`

- Return value: Binary stream object
- Return value type： `BinaryStream`

### Binary Stream Object - Functions

Every binary stream object contains some member functions (member methods) that can be executed. For a particular entity object `bs`, some operations can be performed on this binary stream by these functions

#### Resetting binary streams

`bs.reset()`

- Return value: success or not
- Return value type： `Boolean`

#### Get binary stream read pointer

!!! warning
This function is only available in 0.19.0 and later.

`bs.getReadPointer()`

- Return value: current read pointer
- Return value type： `Integer`

#### Set binary stream read pointer

!!! warning
This function is only available in 0.19.0 and later.

`bs.setReadPointer(pos)`

- Parameters：

  - pos : `Integer`  
    New read pointer

- Return value: success or not
- Return value type： `Boolean`

#### Get binary stream data

!!! warning
The optional parameter and `ByteBuffer` return value of this function are only available in 0.19.0 and later.

`bs.getData([clear])`

- Parameters：

  - clear : `Boolean` (Optional parameter)  
    Whether to clear stream data after reading. The default value is `true`

- Return value: binary stream data
- Return value type： `ByteBuffer`

Before 0.19.0, this function can only be used as `bs.getData()`.  
At that time, it always cleared the stream data after reading, and the return value type was `String`.

Because the old return value was `String`, in JavaScript it may be forcibly encoded as UTF-8, causing binary data corruption and incorrect data content.

#### Set binary stream data

!!! warning
This function is only available in 0.19.0 and later.

`bs.setData(data)`

- Parameters：

  - data : `ByteBuffer`  
    Binary stream data

- Return value: success or not
- Return value type： `Boolean`

#### Write to binary stream

`bs.writexxxx(value)`

- Parameters：

  - value : `NULL`  
    Refer to the table below
    For some numeric write functions, `String` is also accepted

- Return value：success or not
- Return value type： `Boolean`

| Available functions            | Parameter Type       |
| ------------------------------ | -------------------- |
| writeBool                      | `Boolean`/ `String`  |
| writeByte                      | `Integer` / `String` |
| writeBytes (Added in 0.19.0)   | `ByteBuffer`         |
| writeDouble                    | `Number` / `String`  |
| writeFloat                     | `Float` / `String`   |
| writeSignedBigEndianInt        | `Number` / `String`  |
| writeSignedInt                 | `Number` / `String`  |
| writeSignedInt64               | `Number` / `String`  |
| writeSignedShort               | `Integer` / `String` |
| writeString                    | `String`             |
| writeUnsignedChar              | `Integer` / `String` |
| writeUnsignedInt               | `Number` / `String`  |
| writeUnsignedInt64             | `Number` / `String`  |
| writeUnsignedShort             | `Integer` / `String` |
| writeUnsignedVarInt            | `Number` / `String`  |
| writeUnsignedVarInt64          | `Number` / `String`  |
| writeVarInt                    | `Number` / `String`  |
| writeVarInt64                  | `Number` / `String`  |
| writeVec3                      | `FloatPos`           |
| writeBlockPos (Added in 0.9.5) | `BlockPos`           |
| writeCompoundTag               | `NbtCompound`        |
| writeItem (Added in 0.9.5)     | `Item`               |
| writeUuid (Added in 0.19.0)    | `String`             |

#### Read from binary stream

!!! warning
This function group is only available in 0.19.0 and later.

`bs.readxxxx([asString])`  
`bs.readBytes(length)`

- Parameters:

  - asString : `Boolean` (Optional parameter)
    Only available for numeric read functions. When set to `true`, the return value is converted to `String`
  - length : `Integer`
    Only available for `readBytes`. Number of bytes to read. Must be greater than `0`

- Return value:
  - Read result

| Available functions    | Parameter Type | Return Type         |
| ---------------------- | -------------- | ------------------- |
| readBool               | None           | `Boolean`           |
| readByte               | `Boolean`      | `Number` / `String` |
| readBytes              | `Integer`      | `ByteBuffer`        |
| readUnsignedChar       | `Boolean`      | `Number` / `String` |
| readDouble             | `Boolean`      | `Number` / `String` |
| readFloat              | `Boolean`      | `Number` / `String` |
| readSignedBigEndianInt | `Boolean`      | `Number` / `String` |
| readSignedInt          | `Boolean`      | `Number` / `String` |
| readSignedInt64        | `Boolean`      | `Number` / `String` |
| readSignedShort        | `Boolean`      | `Number` / `String` |
| readString             | None           | `String`            |
| readUnsignedInt        | `Boolean`      | `Number` / `String` |
| readUnsignedInt64      | `Boolean`      | `Number` / `String` |
| readUnsignedShort      | `Boolean`      | `Number` / `String` |
| readUnsignedVarInt     | `Boolean`      | `Number` / `String` |
| readUnsignedVarInt64   | `Boolean`      | `Number` / `String` |
| readVarInt             | `Boolean`      | `Number` / `String` |
| readVarInt64           | `Boolean`      | `Number` / `String` |

When `asString` is `true`, numeric `read*` functions return `String`; otherwise they return `Number`.

#### Building packet from binary stream

!!! warning
The optional parameter of this function is only available in 0.19.0 and later.

`bs.createPacket(pktid[,raw])`

- Parameters：

  - pktid : `Integer`  
    Packet ID
  - raw : `Boolean` (Optional parameter)  
    Create raw network packet from current binary stream data. The default value is `false`

- Return value：Packet object
- Return value type： `Packet`

Before 0.19.0, this function can only be used as `bs.createPacket(pktid)`.

### Dome Code

Send TextPacket packets to a player

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
