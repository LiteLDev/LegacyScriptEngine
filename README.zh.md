# LegacyScriptEngine

[English](README.md) | 简体中文  
一个用于在 LeviLamina 上运行 LLSE 插件的插件引擎

## 安装

### 注意

在安装Python引擎之前，你需要先安装[Python 3.12.10](https://www.python.org/downloads/release/python-31210/)  
要安装特定的引擎，您可以使用以下命令：
```shell
lip install github.com/LiteLDev/LegacyScriptEngine#lua
lip install github.com/LiteLDev/LegacyScriptEngine#quickjs
lip install github.com/LiteLDev/LegacyScriptEngine#nodejs
lip install github.com/LiteLDev/LegacyScriptEngine#python
```
对于0.10.0之前的版本，可以使用以下命令：
```shell
lip install git.levimc.org/LiteLDev/legacy-script-engine-lua@版本
lip install git.levimc.org/LiteLDev/legacy-script-engine-quickjs@版本
lip install git.levimc.org/LiteLDev/legacy-script-engine-nodejs@版本
lip install git.levimc.org/LiteLDev/legacy-script-engine-python@版本
```
可以在[releases](https://github.com/LiteLDev/LegacyScriptEngine/releases)中找到版本号。

## 使用

如需获取插件开发 API 提示库和脚手架工具，请访问 [legacy-script-engine-api](https://github.com/LiteLDev/legacy-script-engine-api) 仓库

1. 直接将 LLSE 插件放在 `plugins/` 中
2. 运行服务器，然后插件将自动迁移到 LeviLamina 插件清单中
3. 重启服务器后，插件就会被加载

更多信息请参见[文档](https://lse.levimc.org)

## 贡献

如果您有任何问题，请开启一个 issue 来讨论  
[在这里](https://crowdin.com/project/legacyscriptengine)帮助我们完善翻译  
欢迎 PR

## 许可

GPL-3.0-only © LiteLDev
