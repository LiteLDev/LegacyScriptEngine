# 脚本引擎 - 通用脚本接口文档

> 这里提供了一些常用的 **辅助功能** ，如插件注册、输出信息和异步接口等

他们让你开发脚本变得更加容易而自然，避免了很多无谓的细节问题的纠缠。

## 💼 脚本辅助 API

下面这些API为脚本增加了必要的辅助接口

### 输出信息到控制台

`log(data1,data2,...)`

- 参数：
    - 待输出的变量或者数据  
      可以是任意类型，参数数量可以是任意个
- 返回值：无

### 输出带颜色文本

这是楼上那位的升级版,没错,它支持彩色输出

`colorLog(color,data1,data2,...)`

- 参数：
    - color : `String`  
      此行输出的颜色(代码示例和效果如下图)
    - data... :
      待输出的变量或者数据  
      可以是任意类型，参数数量可以是任意个
- 返回值：无

#### 效果展示：

![ColorLogExample](/img/ColorLog.png)

### 异步输出

此函数将输出请求发出后即刻返回，避免同步读写造成的阻塞时间  
底层有锁保护，不同的`fastLog`之间不会出现串字符现象

`fastLog(data1,data2,...)`

- 参数：
    - data... :
      待输出的变量或者数据  
      可以是任意类型，参数数量可以是任意个
- 返回值：无

### 推迟一段时间执行函数

`setTimeout(func,msec)`

- 参数：

    - func : `Function`  
      待执行的函数

    - msec : `Integer`  
      推迟执行的时间（毫秒）
- 返回值：此任务ID
- 返回值类型：`Integer`
    - 如果返回`Null`，则代表创建任务失败

### 推迟一段时间执行代码段（eval）

`setTimeout(code,msec)`

- 参数：

    - code : `String`  
      待执行的代码段

    - msec : `Integer`  
      推迟执行的时间（毫秒）
- 返回值：此任务ID
- 返回值类型：`Integer`
    - 如果返回`Null`，则代表创建任务失败

### 设置周期执行函数

`setInterval(func,msec)`

- 参数：
    - func : `Function`  
      待执行的函数

    - msec : `Integer`  
      执行间隔周期（毫秒）
- 返回值：此任务ID
- 返回值类型： `Integer`

### 设置周期执行代码段（eval）

`setInterval(code,msec)`

- 参数：
    - code : `String`  
      待执行的代码段

    - msec : `Integer`  
      执行间隔周期（毫秒）
- 返回值：此任务ID
- 返回值类型： `Integer`
    - 如果返回`Null`，则代表创建任务失败

### 取消延时 / 周期执行项

`clearInterval(taskid)`

- 参数：
    - timerid : `Integer`  
      由前几个函数返回的任务ID
- 返回值：是否取消成功
- 返回值类型： `Boolean`
    - 如果返回`Null`，则代表取消任务失败


