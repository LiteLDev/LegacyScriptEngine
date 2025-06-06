# 脚本引擎 - GUI表单界面文档

> 这里API使你可以在游戏中创建、修改或者影响 **GUI界面**

很明显，清晰直观的UI界面有助于玩家快速上手插件的使用，提供良好的用户体验。

## 📊 表单相关 API

对于某个特定的玩家对象`pl`，有以下这些表单接口可用

### 向玩家发送模式表单

模式表单包含一个标题、一个文本显示框以及两个按钮

`pl.sendModalForm(title,content,confirmButton,cancelButton,callback[,forUpdating])`

- 参数：
    - title : `String`  
      表单标题
    - content : `String`  
      表单内容
    - confirmButton : `String`  
      按钮1文本的字符串
    - cancelButton : `String`  
      按钮2文本的字符串
    - callback : `Function`  
      玩家点击按钮之后被调用的回调函数。
    - forUpdating : `Boolean`
      （可选参数，在0.11.0中加入）是否为表单更新，默认值为false，即非表单更新
- 返回值：发送的表单ID
- 返回值类型：`Integer`
    - 如果返回值为`Null`，则代表发送失败

参数`callback`的回调函数原型：`function(player,result,reason)`

- player : `Player`  
  与表单互动的玩家对象
- result : `Boolean`    
  玩家点击**确定**按钮为`true`，**取消**按钮为`false`  
  如果id为`Null`，则代表玩家取消了表单
- reason : `Integer`(在0.9.0中加入)
  表单被取消的原因
  0 = UserClosed, 1 = UserBusy
  reason可能会是`null`.

### 向玩家发送普通表单

普通表单包含一个标题、一个文本显示框以及若干按钮，可以设置按钮上显示的图标  
由于按钮的内容设置相对复杂，建议使用下一节的表单构建器API更好地完成这项任务。

`pl.sendSimpleForm(title,content,buttons,images,callback[,forUpdating])`

- 参数：
    - title : `String`  
      表单标题
    - content : `String`  
      表单内容
    - buttons : `Array<String,String,...>`  
      各个按钮文本的字符串数组
    - images : `Array<String,String,...>`  
      各个按钮对应的图片路径
    - callback : `Function`  
      玩家点击按钮之后被调用的回调函数。
    - forUpdating : `Boolean`
      （可选参数，在0.11.0中加入）是否为表单更新，默认值为false，即非表单更新
- 返回值：发送的表单ID
- 返回值类型：`Integer`
    - 如果返回值为`Null`，则代表发送失败

参数`callback`的回调函数原型：`function(player,id,reason)`

- player : `Player`  
  与表单互动的玩家对象
- id : `Integer`    
  玩家点击的表单按钮的序号，从0开始编号  
  如果id为`Null`，则代表玩家取消了表单
- reason : `Integer`(在0.9.0中加入)
  表单被取消的原因
  0 = UserClosed, 1 = UserBusy
  reason可能会是`null`.

图片路径参数 `images` 使用材质包路径或者URL来标识按钮对应的图标。  
对于表单上的每个按钮，如下设置对应的图标

1. 如果使用材质包路径，图片路径应该形如 `textures/items/apple`
2. 如果使用URL路径，那么在这里放入完整的URL即可，形如 `https://www.baidu.com/img/flexible/logo/pc/result.png`
3. 如果这个按钮你不需要显示图片，那将对应的图片路径设置为空字符串即可

### 向玩家发送自定义表单（JSON格式）

自定义表单可以包含丰富的自定义控件。  
由于相关JSON定义格式相对复杂，建议使用下一节的表单构建器API更好地完成这项任务。

`pl.sendCustomForm(json,callback[,forUpdating])`

- 参数：
    - json : `String`  
      自定义表单json字符串
    - callback : `Function`  
      玩家提交表单之后被调用的回调函数。
    - forUpdating : `Boolean`
      （可选参数，在0.11.0中加入）是否为表单更新，默认值为false，即非表单更新
- 返回值：发送的表单ID
- 返回值类型：`Integer`
    - 如果返回值为`Null`，则代表发送失败

参数`callback`的回调函数原型：`function(player,data,reason)`

- player : `Player`  
  与表单互动的玩家对象
- data : `Array<...>`    
  返回的表单内容数组  
  数组中，第一项一定为`Null`，从第二项开始，按表单上的控件顺序储存了每一个控件的内容  
  如果data只为`Null`，则代表玩家取消了表单
- reason : `Integer`(在0.9.0中加入)
  表单被取消的原因
  0 = UserClosed, 1 = UserBusy
  reason可能会是`null`.

### 关闭玩家正在打开的表单

`pl.closeForm()`

- 返回值：请求是否成功
- 返回值类型：`Boolean`
