# LLSE - Generic Scripting Interface Documentation

> Here are some commonly used **auxiliary functions** , such as plugin registration, output information and asynchronous
> interfaces, etc.

They make it easier and more natural for you to develop scripts and avoid a lot of unnecessary details.

## 💼 Script Assist API

The following APIs add necessary auxiliary interfaces to scripts.

### Output Information To The Console

`log(data1,data2,...)`

- Parameter:.,
    - Variable or data to be output  
      Can be of any type, and the number of parameters can be any number.
- Return value: none

### Output Color Text

This is an upgraded version of the above function; it supports color output.

`colorLog(color,data1,data2,...)`

- Parameter:
    - color : `String`  
      The color output by this line (code example and effect are as follows)
    - data... :
      Variable or data to be output  
      Can be of any type, and the number of parameters can be any number.
- Return value: none

#### Show results:

![ColorLogExample](/img/ColorLog.png)

### Asynchronous Output

This function returns immediately after the output request is sent, avoiding the blocking time caused by synchronous
reading and writing.
The bottom layer has lock protection, different `fastLog` There will be no string phenomenon between.

`fastLog(data1,data2,...)`

- Parameter:
    - data... :
      Variable or data to be output
      Can be of any type, and the number of parameters can be any number
- Return value: none

### Delay the execution of a function for a period of time

`setTimeout(func,msec)`

- Parameter:

    - func : `Function`  
      The function to be executed.

    - msec : `Integer`  
      Delay execution time (milliseconds)
- Return value: this task id.
- Return value type: `Integer`
    - If it returns `Null`, the task failed.

### Delay the execution of a code segment for a period of time (eval)

`setTimeout(code,msec)`

- Parameter:

    - code : `String`  
      The code segment to be executed.

    - msec : `Integer`  
      Delay execution time (milliseconds)
- Return value: this task id
- Return value type: `Integer`
    - If it returns `Null`, the task creation failed.

### Set period execution function

`setInterval(func,msec)`

- Parameter:
    - func : `Function`  
      The function to be executed

    - msec : `Integer`  
      Execution interval period (ms)
- Return value: this task id
- Return value type:  `Integer`

### Set period execution code segment (eval)

`setInterval(code,msec)`

- Parameter:
    - code : `String`  
      The code to be executed.

    - msec : `Integer`  
      Execution interval period (ms)
- Return value: this task id
- Return value type:  `Integer`
    - If it returns `Null`, the task creation failed.

### Cancel Delay/Period Execution Item

`clearInterval(taskid)`

- Parameter:
    - timerid : `Integer`  
      The task ID returned by the first few functions
- Return value: whether the cancellation was successful.
- Return value type:  `Boolean`
    - If it returns `Null`, the cancellation of the task failed.


