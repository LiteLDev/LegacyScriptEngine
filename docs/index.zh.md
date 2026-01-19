# LegacyScriptEngine

一个用于在 LeviLamina 上运行 LLSE 插件的插件引擎

## 安装

!!! warning
    在安装Python引擎之前，你需要先安装Python。下面是一份LSE版本和需要的Python版本的列表。

| LSE 版本          | Python 版本 |
|-----------------|-----------|
| >=0.16.2        | 3.12.10   |
| >=0.9.0 <0.16.2 | 3.12.8    |
| <0.9.0          | 3.10.11   |

### 服务端

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

### 客户端

要安装特定的引擎，您可以使用以下命令：
```shell
lip install github.com/LiteLDev/LegacyScriptEngine#client_lua
lip install github.com/LiteLDev/LegacyScriptEngine#client_quickjs
lip install github.com/LiteLDev/LegacyScriptEngine#client_nodejs
lip install github.com/LiteLDev/LegacyScriptEngine#client_python
```
要安装特定版本，需要在命令最后加上`@版本号`，例如：
```shell
lip install github.com/LiteLDev/LegacyScriptEngine#client_lua@0.17.0-rc.2
lip install github.com/LiteLDev/LegacyScriptEngine#client_quickjs@0.17.0-rc.2
lip install github.com/LiteLDev/LegacyScriptEngine#client_nodejs@0.17.0-rc.2
lip install github.com/LiteLDev/LegacyScriptEngine#client_python@0.17.0-rc.2
```

## 使用

如需获取插件开发 API 提示库和脚手架工具，请访问 [legacy-script-engine-api](https://github.com/LiteLDev/legacy-script-engine-api) 仓库

1. 直接将 LLSE 插件放在 `plugins/` 中
2. 运行服务器，然后插件将自动迁移到 LeviLamina 插件清单中
3. 重启服务器后，插件就会被加载

## 贡献

如果您有任何问题，请开启一个 issue 来讨论  
[在这里](https://crowdin.com/project/legacyscriptengine)帮助我们完善翻译  
欢迎 PR

## 许可

GPL-3.0-only © LiteLDev
