# LegacyScriptEngine

一个用于在 LeviLamina 上运行 LLSE 插件的插件引擎

## 安装

### 注意

在安装Python引擎之前，你需要先安装[Python 3.12.8](https://www.python.org/downloads/release/python-3128/)

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

1. 直接将 LLSE 插件放在 `plugins/` 中
2. 运行服务器，然后插件将自动迁移到 LeviLamina 插件清单中
3. 重启服务器后，插件就会被加载

## 一些对插件开发有帮助的项目

- [LiteLoaderSE-Aids](https://github.com/LiteLDev/LiteLoaderSE-Aids)
- [LiteLoaderSE-Aids-Magic-Revision](https://github.com/luoqing510/LiteLoaderSE-Aids-Magic-Revision)
- [HelperLib](https://github.com/LiteLDev/HelperLib)
- [llpy-helper-lib](https://github.com/LiteLDev/llpy-helper-lib)

## 贡献

如果您有任何问题，请开启一个 issue 来讨论  
[在这里](https://crowdin.com/project/legacyscriptengine)帮助我们完善翻译  
欢迎 PR

## 许可

GPL-3.0-only © LiteLDev
