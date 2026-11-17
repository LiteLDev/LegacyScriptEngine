# LegacyScriptEngine

[English](README.md) | 简体中文  
一个用于在 LeviLamina 上运行 LLSE 插件的插件引擎

## 安装

请跟随教程[给服务器安装模组](https://levimc.org/zh-Hans/tutorials/server/install-mods/)。想编写插件，请从
[写你的第一个脚本插件](https://levimc.org/zh-Hans/tutorials/develop/first-script-plugin/)开始。

每种语言都有自己的引擎，用 [lip](https://levimc.org/zh-Hans/lip/) 作为包的一个变体安装：
`lip install LegacyScriptEngine#lua`（或 `#quickjs`、`#nodejs`、`#python`）。不指定变体时，lip 会同时安装 QuickJS 和 Lua 引擎。

## 使用

如需获取插件开发 API 提示库和脚手架工具，请访问 [legacy-script-engine-api](https://github.com/LiteLDev/legacy-script-engine-api) 仓库

1. 直接将 LLSE 插件放在 `plugins/` 中（自 0.23.0 起，它是启动时创建的指向 `mods/` 的链接）
2. 运行服务器，然后插件会被自动迁移，并生成 LeviLamina 模组清单（manifest.json）
3. 重启服务器后，插件就会被加载

更多信息请参见[文档](https://levimc.org/zh-Hans/LegacyScriptEngine/)

## 贡献

如果您有任何问题，请开启一个 issue 来讨论  
[在这里](https://crowdin.com/project/legacyscriptengine)帮助我们完善翻译  
欢迎 PR：参见 [CONTRIBUTING.md](CONTRIBUTING.md)（英文）

## 许可

GPL-3.0-or-later © LeviMC
