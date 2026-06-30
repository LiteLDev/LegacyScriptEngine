# VanillaI18n API

此 API 用于修改 Minecraft 原版语言数据。

!!! warning
    此 API 仅在0.19.0及以后版本可用

此对象在脚本引擎内实际注册的类名为 `VaillanI18n`。

## 获取当前语言

`VaillanI18n.getCurrentLanguage()`

- 返回值：当前语言代码
- 返回值类型：`String`

## 设置当前语言

`VaillanI18n.setCurrentLanguage(language)`

- 参数：
  - language: `String`
    语言代码，如 `en_US`、`zh_CN`
- 返回值：是否成功
- 返回值类型：`Boolean`

## 获取支持的语言列表

`VaillanI18n.getSupportedLanguages()`

- 返回值：支持的语言代码列表
- 返回值类型：`Array<String>`

## 翻译原版文本

`VaillanI18n.translate(key[, params][, language])`

- 参数：
  - key: `String`
    翻译键名
  - params: `Array<String>`（可选参数）
    翻译参数
  - language: `String`（可选参数）
    目标语言。不传时使用当前语言
- 返回值：翻译后的文本
- 返回值类型：`String`

## 从对象加载语言数据

`VaillanI18n.loadLanguage(language, data)`

- 参数：
  - language: `String`
    语言代码
  - data: `Object`
    语言数据，其中每个键和值都必须为 `String`
- 返回值：是否成功
- 返回值类型：`Boolean`

## 从文件加载语言数据

`VaillanI18n.loadLanguageFromFile(language, path)`

- 参数：
  - language: `String`
    语言代码
  - path: `String`
    语言文件路径
- 返回值：是否成功
- 返回值类型：`Boolean`

## 从目录加载语言数据

`VaillanI18n.loadLanguagesFromDirectory(path)`

- 参数：
  - path: `String`
    目录路径
- 返回值：是否成功
- 返回值类型：`Boolean`

此函数会加载目录下所有 `.lang` 文件。  
文件名去掉 `.lang` 后缀后会作为语言代码使用。
