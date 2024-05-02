# LegacyScriptEngine

一个用于在 LeviLamina 上运行 LLSE 插件的插件引擎

## 安装

要一次性安装QuickJs和Lua引擎，您可以使用以下命令：

```sh
lip install github.com/LiteLDev/LegacyScriptEngine
```

要安装特定的引擎，您可以使用以下命令：

```shell
lip install gitea.litebds.com/LiteLDev/legacy-script-engine-lua
lip install gitea.litebds.com/LiteLDev/legacy-script-engine-quickjs
lip install gitea.litebds.com/LiteLDev/legacy-script-engine-nodejs
lip install gitea.litebds.com/LiteLDev/legacy-script-engine-python
```

要升级特定引擎，您可以使用以下命令:

```shell
lip install --upgrade gitea.litebds.com/LiteLDev/legacy-script-engine-lua
lip install --upgrade gitea.litebds.com/LiteLDev/legacy-script-engine-quickjs
lip install --upgrade gitea.litebds.com/LiteLDev/legacy-script-engine-nodejs
lip install --upgrade gitea.litebds.com/LiteLDev/legacy-script-engine-python
```

## 使用

1. 直接将 LLSE 插件放在 `/path/to/bedrock_dedicated_server/plugins/` 中。

2. 运行服务器，然后插件将自动迁移到 LeviLamina 插件清单中。

3. 重启服务器后，插件就会被加载。

## 贡献

如果您有任何问题，请开启一个 issue 来讨论。

欢迎 PR。

## 许可

GPL-3.0-only © LiteLDev
