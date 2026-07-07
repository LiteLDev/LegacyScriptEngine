# LLSE - NBT Documentation

> **NBT** (**N**amed **B**inary **T**ags) format is a storage format used in Minecraft to store data into files.
> The NBT format stores data in a tree structure with many *tags*. All tags have an independent ID and name.
>
> --- Minecraft Wiki

This provides scripts with the ability to manipulate the **NBT** data type. **NBT** interface support greatly improves the scalability of the game.

In the game, a node called **NBT tag** is used to identify an item of NBT data. A variety of data types such as ordinary data, List, Compound, etc. can be stored in NBT tags.

In LegacyScriptEngine, each NBT data type has its corresponding data type, and we collectively call them "NBT objects".
The comparison of the LLSE type with the NBT data type is as follows:

| NBT Data Type | Corresponding NBT Object Type | Type Description (Wiki)                         |
| ------------- | ----------------------------- | ----------------------------------------------- |
| `Byte`        | `NbtByte`                     | Signed byte or boolean (8 bits)                 |
| `Short`       | `NbtShort`                    | Signed Short (16-bit)                           |
| `Int`         | `NbtInt`                      | Signed Integer (32-bit)                         |
| `Long`        | `NbtLong`                     | Signed Long (64-bit)                            |
| `Float`       | `NbtFloat`                    | Single precision floating point number          |
| `Double`      | `NbtDouble`                   | Double precision floating point number          |
| `ByteArray`   | `NbtByteBuffer`               | Byte Array                                      |
| `String`      | `NbtString`                   | UTF-8 string                                    |
| `List`        | `NbtList`                     | NBT List (similar to array)                     |
| `Compound`    | `NbtCompound`                 | NBT Tags (Similar to a list of Key-Value pairs) |

Each data type may have a slightly different usage interface. They are introduced separately below:



## 🎈 NBT Object Generic Interface 

Each NBT object contains some executable member functions (member methods).  
For any kind of NBT object, there are the following general interfaces. named `nbt`. An example of an NBT object of:

#### Get the Data Type Stored by the NBT Object 

`nbt.getType()`

- Return value: The data type stored by this NBT object.
- Return value type: `Enum`

Possible return values are: `NBT.End` `NBT.Byte` `NBT.Short` `NBT.Int` `NBT.Long`   
`NBT.Float` `NBT.Double` `NBT.ByteArray` `NBT.String`  
`NBT.List` `NBT.Compound`



#### Convert NBT Object to JSON String 

`nbt.toString([space])`

- Parameter:
  - space : `Integer`  
    (Optional parameter) If you want to format the output string, pass this parameter.  
    Represents the number of spaces per indent, so the resulting string is more human-readable.  
    This parameter defaults to `-1`, i.e. the output string is not formatted.
- Return value: The JSON form of the NBT object.
- Return value type: `String`

Hint: If this NBT object stores the `List` or `Compoundtype`, they will expand recursively to `Array` or `Object`.
If this NBT object stores the `ByteArray` Type, the output byte string will be base64 encoded before output.

> The string output by the above function conforms to the JSON standard format, but cannot be deserialized.
> If there is a need for deserialization, please use the **SNBT** interface provided by the NBT tag class.


#### Convert NBT Object to SNBT String

`nbt.toSNBT([space[,format]])`

- Parameter:
  - space : `Integer`
    (Optional parameter) Indent width. Pass `-1` or omit it to disable pretty formatting.
  - format : `Enum`
    (Optional parameter) SNBT output format, using a value from the `SnbtFormat` enum.
- Return value: The SNBT form of the NBT object.
- Return value type: `String`

Calling `nbt.toSNBT()` uses `SnbtFormat.ForceQuote` by default.  
Calling `nbt.toSNBT(space)` with `space >= 0` uses
`SnbtFormat.PartialLineFeed`.  
If you need to specify another SNBT format explicitly, use
`nbt.toSNBT(space, format)`.


## 🎨 SnbtFormat Enum

`SnbtFormat` is the enum used by `nbt.toSNBT([space[,format]])`.

The following values are available:

| Enum Value                  | Numeric Value | Description |
|----------------------------|---------------|-------------|
| `SnbtFormat.Minimize`      | `0`           | Minimal output. |
| `SnbtFormat.CompoundLineFeed` | `1`        | Insert line feeds for compound members. |
| `SnbtFormat.ArrayLineFeed` | `2`           | Insert line feeds for arrays / lists. |
| `SnbtFormat.Colored`       | `4`           | Add color formatting to output. |
| `SnbtFormat.Console`       | `8`           | Use console-oriented formatting. |
| `SnbtFormat.ForceAscii`    | `16`          | Force ASCII output. |
| `SnbtFormat.ForceQuote`    | `32`          | Force quoted string output. |
| `SnbtFormat.CommentMarks`  | `64`          | Add comment-style marks. |
| `SnbtFormat.Jsonify`       | `96`          | Preset: `ForceQuote | CommentMarks`. |
| `SnbtFormat.PartialLineFeed` | `1`         | Alias of `CompoundLineFeed`. |
| `SnbtFormat.AlwaysLineFeed` | `3`          | Preset: `CompoundLineFeed | ArrayLineFeed`. |
| `SnbtFormat.PrettyFilePrint` | `1`         | Alias of `PartialLineFeed`. |
| `SnbtFormat.PrettyChatPrint` | `5`         | Preset: `PrettyFilePrint | Colored`. |
| `SnbtFormat.PrettyConsolePrint` | `13`    | Preset: `PrettyFilePrint | Colored | Console`. |

`CompoundLineFeed`, `ArrayLineFeed`, `Colored`, `Console`, `ForceAscii`, `ForceQuote`, and `CommentMarks` are bit flags.  
The other entries above are predefined combinations or aliases.


