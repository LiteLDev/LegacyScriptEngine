# NativePointer

This API is designed to help developers manipulate native pointers

> Due to the x64 calling convention, any object (size>8) is passed by reference. To learn more details, please refer to [msdoc: x64 calling convention](https://docs.microsoft.com/cpp/build/x64-calling-convention)

### Memory allocation

This function helps to allocate a block of memory of a specified size

`static NativePointer.malloc(size)`

- Parameters:
  - int64: `size`
    The size of the memory to be allocated
- Return value: a pointer to the new memory
- Return type: `NativePointer`



### Memory destruction

Destroy a specified block of memory

`static NativePointer.free(block)`

- Parameters:
  - NativePointer: `block`
    The block of memory to be destroyed



### Pointer offset

Get the address after offsetting relative to a pointer

`NativePointer.offset(offset)`

- Parameters:
  - int32: `offset`
    
- Return value: the pointer after offsetting
- Return type: `NativePointer`



### Get symbol address

Get an MCAPI symbol address, equivalent to `dlsym` in CPP

`NativePointer.fromSymbol(symbol)`

- Parameters:
  - String: `symbol`
    The symbol to be queried
- Return value: the query result, or a null pointer if the query fails
- Return type: `NativePointer`



### Get address instance

Get a pointer instance of a specified address

`NativePointer.fromAddress(address)`

- Parameters:
  - String/int64: `address`
    The address, expressed as a hexadecimal string or a number
- Return value: the pointer instance of the corresponding address
- Return type: `NativePointer`



### Get pointer address

Get the raw pointer address

`NativePointer.asRawAddress`
    
- Return value: the pointer address expressed as a number
- Return type: `int64`



### Get pointer address description

Get the raw pointer address (hexadecimal string)

`NativePointer.asHexAddress`
    
- Return value: the pointer address expressed as a hexadecimal string
- Return type: `String`



### Read and write pointer memory

The following virtual properties are set up to help access the pointer content. The available types are shown in the table below.

`NativePointer.type`

| Access name | Size  | Special notes                                                 |
| :---------- | :---: | :------------------------------------------------------------ |
| byte        |   1   | This property is read and written through hexadecimal strings |
| int8        |   1   |                                                               |
| uint8       |   1   |                                                               |
| int16       |   2   |                                                               |
| uint16      |   2   |                                                               |
| int32       |   3   |                                                               |
| uint32      |   4   |                                                               |
| long        |   4   |                                                               |
| ulong       |   4   |                                                               |
| int64       |   8   |                                                               |
| uint64      |   8   |                                                               |
| float       |   4   |                                                               |
| double      |   8   |                                                               |
| string      |  32   | This property represents std::string*                         |
| bool        |   1   |                                                               |
