# 📦 方块对象 API

在脚本引擎中，使用「方块对象」来操作和获取某一类方块的相关信息。

### 获取一个方块对象

#### 从事件或API获取

通过注册**事件监听**函数，或者调用某些**返回方块对象**的函数，获取到BDS给出的与相关事件有关的方块对象  
详见 [事件监听文档 - EventAPI](../EventAPI/Listen.zh.md)  

#### 通过方块坐标获取

通过此函数来手动生成对象，注意，你要获取的方块必须处于已被加载的范围中，否则会出现问题

`mc.getBlock(pos)`  
`mc.getBlock(x,y,z,dimid)`

- 参数：
  - pos : `IntPos `  
    方块所在坐标（或者使用x, y, z, dimid来确定方块位置）
- 返回值：生成的方块对象 
- 返回值类型：`Block`
  - 如返回值为 `Null` 则表示获取方块失败

> 注意：不要**长期保存**一个方块对象  
> 当方块对象对应的方块被销毁时，对应的方块对象将同时释放。因此，如果有长期操作某个方块的需要，请通过上述途径获取实时的方块对象




### 方块对象 - 属性

每一个方块对象都包含一些固定的对象属性。对于某个特定的方块对象`bl`，有以下这些属性

| 属性                    | 含义                 | 类型      |
| ----------------------- | -------------------- | --------- |
| bl.name                 | 游戏内显示的方块名称 | `String`  |
| bl.type                 | 方块标准类型名       | `String`  |
| bl.id                   | 方块的游戏内id       | `Integer` |
| bl.pos                  | 方块所在坐标         | `IntPos`  |
| bl.tileData             | 方块数据值           | `Integer` |
| bl.variant              | The block variant    | `Integer` |
| bl.translucency         | 方块透明度           | `Integer` |
| bl.thickness            | 方块厚度             | `Integer` |
| bl.isAir                | 方块是否为空气       | `Boolean` |
| bl.isBounceBlock        | 是否为可弹跳方块     | `Boolean` |
| bl.isButtonBlock        | 是否为按钮方块       | `Boolean` |
| bl.isCropBlock          | 是否为农作物方块     | `Boolean` |
| bl.isDoorBlock          | 是否为门方块         | `Boolean` |
| bl.isFenceBlock         | 是否为栅栏方块       | `Boolean` |
| bl.isFenceGateBlock     | 是否为栅栏门方块     | `Boolean` |
| bl.isThinFenceBlock     | 是否为细栅栏方块     | `Boolean` |
| bl.isHeavyBlock         | 是否为重的方块       | `Boolean` |
| bl.isStemBlock          | 是否为干方块         | `Boolean` |
| bl.isSlabBlock          | 是否为半转方块       | `Boolean` |
| bl.isUnbreakable        | 方块是否为不可破坏   | `Boolean` |
| bl.isWaterBlockingBlock | 方块是否可阻挡水     | `Boolean` |

这些对象属性都是只读的，无法被修改



### 方块对象 - 函数

每一个方块对象都包含一些可以执行的成员函数（成员方法）。对于某个特定的方块对象`bl`，可以通过以下这些函数对这个方块进行一些操作

#### 破坏方块

`bl.destroy(drop)`

- 参数：
  - drop : `Boolen`  
    是否生成掉落物
- 返回值：是否成功破坏
- 返回值类型：`Boolen`



#### 获取方块对应的NBT对象

`bl.getNbt()`

- 返回值：方块的NBT对象
- 返回值类型：`NbtCompound`



#### 写入方块对应的NBT对象

`bl.setNbt(nbt)`

- 参数：
  - nbt : `NbtCompound`  
    NBT对象
- 返回值：是否成功写入
- 返回值类型：`Boolean`

关于NBT对象的更多使用，请参考 [NBT接口文档](../NbtAPI/NBT.zh.md)
注意：慎重使用此api，请考虑使用 `mc.setBlock()` 代替



#### 获取方块的BlockState

`bl.getBlockState()`

- 返回值：方块的`BlockState`
- 返回值类型：`Object`

方便函数，协助解析方块`BlockState`并转换为`Object`，方便读取与解析  
等价于脚本执行`block.getNbt().getTag("states").toObject()`



#### 判断方块是否拥有容器

`bl.hasContainer()`

- 返回值：这个方块是否拥有容器
- 返回值类型：`Boolean`

如箱子、桶等容器，他们各自拥有一个属于自己的容器对象



#### 获取方块所拥有的容器对象

`bl.getContainer()`

- 返回值：这个方块所拥有的容器对象
- 返回值类型：`Container`

关于容器对象的更多使用，请参考 [容器对象 API文档](./Container.zh.md)



#### 判断方块是否拥有方块实体

`bl.hasBlockEntity()`

- 返回值：这个方块是否拥有方块实体
- 返回值类型：`Boolean`



#### 获取方块所拥有的方块实体

`bl.getBlockEntity()`

- 返回值：这个方块所拥有的方块实体
- 返回值类型：`BlockEntity`



#### 删除方块所拥有的方块实体

`bl.removeBlockEntity()`

- 返回值：是否成功删除
- 返回值类型：`Boolean`

关于方块实体对象的更多使用，请参考 [方块实体对象 API文档](../GameAPI/BlockEntity.zh.md)



### 其他方块函数 API

下面这些API提供了与游戏中指定位置方块互动的API

#### 设置指定位置的方块

`mc.setBlock(pos, blockObject)`  
`mc.setBlock(pos, blockString, tileData)`  
`mc.setBlock(x, y, z, dimId, blockObject)`  
`mc.setBlock(x, y, z, dimId, blockString, tileData)`

- 参数：
  - pos： `IntPos` 或 `FloatPos`  
    目标方块位置（或者使用 x、y、z、dimId 来确定方块位置）

  - blockObject： `Block` 或 `NBTCompound`  
    要设置成的方块对象或方块 NBT 数据

  - blockString：`String`  
    方块标准类型名（如`minecraft:stone`）

  - tileData：`Integer`  
    方块状态值，同原版 /setBlock 指令的 tileData，默认为0，仅通过方块类型名放置方块时有效

- 返回值：是否成功设置

- 返回值类型：`Boolean`

通过此函数，将一个坐标对应的方块设置成另一个，类似于命令 `/setblock`



#### 在指定位置生成粒子效果

`mc.spawnParticle(pos,type)`  
`mc.spawnParticle(x,y,z,dimid,type)`

- 参数：
  - pos : `IntPos` / `FloatPos`  
    目标生成位置（或者使用x, y, z, dimid来确定方块位置）
  - type : `String`  
    要生成的粒子效果名称（可查阅wiki得知）
- 返回值：是否成功生成
- 返回值类型：`Boolean`

粒子效果名称可以查阅[Minecraft Wiki](https://zh.minecraft.wiki/w/粒子#类型)得知，在传入参数的时候不要忘记命名空间前缀。类似于 `minecraft:heart_particle`
