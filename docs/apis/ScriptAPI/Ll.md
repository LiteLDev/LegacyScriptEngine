# 💡 Plugin loading related API

Some interfaces related to loader operations are provided here. 

### Properties

| Property                 | Type      | Description                                                |
| ------------------------ | --------- | ---------------------------------------------------------- |
| `ll.language`            | `String`  | The language LeviLamina used.(such as `zh`, `en`, `ru_RU`) |
| `ll.major`               | `Integer` | Major Version Number (ex:  the **2** in **2**.7.1)         |
| `ll.minor`               | `Integer` | Minor Version Number (ex: the **7** in 2.**7**.1)          |
| `ll.revision`            | `Integer` | Revision Number: (ex: the **1** in 2.7.**1**)              |
| `ll.status`              | `Integer` | Status (`0` is Dev, `1` is Beta, `2` is Release)           |
| `ll.scriptEngineVersion` | `String`  | LeviLamina Script Engine Version                           |
| `ll.isWine`              | `Boolean` | Whether the LeviLamina started from Wine                   |
| `ll.isDebugMode`         | `Boolean` | Whether the LeviLamina in debug mode                       |
| `ll.isBeta`              | `Boolean` | Whether the current version is a beta version              |
| `ll.isDev`               | `Boolean` | Whether the current version is a dev version               |
| `ll.isRelease`           | `Boolean` | Whether the current version is a release version           |

### Get LeviLamina loader version string

`ll.versionString()`

- Return value: loader version
- Return value type:  `String`

### Get information about Plugin

`ll.getPluginInfo(name)`

- Parameter:
  - name: `String`
  Plugin name
- Return value: Plugin Object
- Return value type:  `Plugin`
  - For a returned plugin object, there are the following members:  

  | Property          | Description             | Type                             |
  | ----------------- | ----------------------- | -------------------------------- |
  | plugin.name       | Plugin name             | `String`                         |
  | plugin.desc       | Plugin description      | `String`                         |
  | plugin.type       | Plugin type             | `String`                         |
  | plugin.version    | Plugin version (array)  | `Array<Integer,Integer,Integer>` |
  | plugin.versionStr | Plugin version (string) | `String`                         |
  | plugin.filePath   | Path to plugin          | `String`                         |
  | plugin.others     | Other information       | `Object`                         |

### List all loaded plugins

`ll.listPlugins()`

- Return value: A list containing the names of all loaded plugin
- Return value type:  `Array<String,String,...>`

### List all loaded plugins with information

`ll.getAllPluginInfo()`

- Return value: A list containing the plugin objects of all loaded plugin
- Return value type:  `Array<Plugin,Plugin,...>`

### Remote Function Call

In order to allow the pre-plug-ins developed by developers to provide interfaces and services for other plug-ins, the remote function call function is provided here, so that one LLSE plug-in can call the existing functions in another plug-in. 

#### Export Function

In order to allow the pre-plug-ins developed by developers to provide interfaces and services for other plug-ins, the remote function call function is provided here, so that an LL or LLSE plug-in can call the existing functions in another plug-in.

`ll.exports(func,namespace,name)`

- Parameter: 
  - func : `Function`  
    Function to be exported
  - namespace : `String`  
    The namespace name of the function, which is only convenient for distinguishing the API exported by different plugins.
  - name : `String`  
    The export name of the function. Other plugins call this function based on the export name.
- Return value: Whether the export was successful.
- Return value type:  `Boolean`

Note: If the namespace and name of the exported function are exactly the same as another already exported function, the export will fail. Please select the namespace and export name appropriately when exporting.

#### Import Function

After you have learned that there is a plug-in exporting function, in order to use the function exported by him, you first need to import this function into your own scripting system.
LLSE provides the interface import to import functions already exported by other plugins.

`ll.imports(namespace,name)`

- Parameter: 
  - namespace : `String`  
    The namespace name used by the function that is being imported.
  - name : `String`  
    The name of the function that is being imported.
- Return value: The imported function
- Return value type:  `Function`

The return value of `ll.import` is a function. When you call this function, the cross-plugin call process will be done automatically in the background. The parameters of the calling function will be wrapped and passed to the remote function, and the return value of this function is the return value returned by the remote function after it has been executed.

#### Example of Remote Calling Function 

For example, there is a plug-in that exports a function using the namespace AAA, and the name of the exported function is Welcome
You can execute `welcome = ll.import("AAA", "Welcome"); ` to import this function. After the import is complete, you can execute directly below:

`welcome("hello",2,true);`   

The parameters of the function will be automatically forwarded to the corresponding target function for execution, and the return value of the corresponding target function will be returned after execution. The whole process is automatically completed. 

Notice! When calling a function, you need to ensure that the number and types of parameters you pass in and the parameters accepted by the target function are correct and in one-to-one correspondence. Otherwise, an error will occur. 

### Determine if a remote function has been exported

`ll.hasExported(namespace,name)`

- Parameter：
  - namespace : `String`  
    Namespace name used by the function
  - name : `String`  
    Export name used by the function
- Return value：Whether the function has been exported
- Return value type： `Boolean`

### Execute a String as a Script

`ll.eval(str)`

- Parameter: 
  - str : `String`  
    String to execute as a Script
- Return value: Execution result
- Return value type:  `Any Type`
