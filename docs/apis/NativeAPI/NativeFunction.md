# NativeFunction

This API set contains two types: `NativeFunction` and `NativeHook`, where `NativeHook` is a specialized implementation that only contains `call` and `address` properties.

### Symbol get function

Automatically parse the symbol and get a callable function. If the parsing fails, an exception is thrown.

`static NativeFunction.fromSymbol(symbol)`

- Parameters:
  - String: `symbol`
    The function to be parsed
- Return value: native function instance
- Return type: `NativeFunction`



### Describe get function

Describe the function type and get an uncallable function. If you need to call it, you also need to manually set the address property.

`static NativeFunction.fromDescription(ReturnValue: NativeTypes.Void, Params: [NativeType.Int......])`

- Parameters:
  - Enum-NativeTypes: `ReturnValue`
    Return type
  - Enum-NativeTypes...: `Params`
    Parameter types, passed directly from left to right
- Return value: native function instance
- Return type: `NativeFunction`



### Script get function

Describe the function type and get a function from the script, which is wrapped as a function that can be directly called in native code.

`static NativeFunction.fromScript(ReturnValue: NativeTypes.Void, Params: [NativeType.Int......], Callback: func(Params...){})`

- Parameters:
  - Enum-NativeTypes: `ReturnValue`
    Return type
  - Enum-NativeTypes...: `Params`
    Parameter types, passed directly from left to right
  - Function: `Callback`
    Callback function, which will be called after the native wrapper function is called
- Return value: native function instance
- Return type: `NativeFunction`



### Hook function hook

Rewrite the header of the specified address function, set the callback function, and call the callback function when the original function is called.
If you need to keep the original function, please remember to call the original function in the callback function.

`NativeFunction.hook(function)`

- Parameters:
  - Function: `function`
    Callback function, please note that the parameter type is consistent with the description of NativeFunction
- Return value: original function
- Return type: `NativeHook`



### call function

Call the corresponding function through the virtual object call.

`NativeFunction.call(params...)`

- Parameters:
  - Parameters: `params`
    The function parameters described by NativeFunction
- Return value: the return type described by NativeFunction
- Return type: `Value`



### address property

The pointer value of the function pointer

`NativeFunction.address`

- Setter:
  - NativePointer
  - int64
- Getter
  - int64



## NativeType parameter type

This Enum shows all the types that can be used for function parameters and returns

- Void
- Bool
- Char
- UnsignedChar
- Short
- UnsignedShort
- Int
- UnsignedInt
- Long
- UnsignedLong
- LongLong
- UnsignedLongLong
- Float
- Double
- Pointer
