---
layout: home
hero:
  name: LegacyScriptEngine
  tagline: 一个用于在 LeviLamina 上运行 LLSE 插件的插件引擎
  image:
    src: img/logo.png
    alt: LegacyScriptEngine
  actions:
    - theme: brand
      text: 写你的第一个脚本插件
      link: https://levimc.org/tutorials/zh-Hans/develop/first-script-plugin/
    - theme: alt
      text: 在服务器上安装
      link: https://levimc.org/tutorials/zh-Hans/server/install-mods/
    - theme: alt
      text: API 参考
      link: apis/README.md
features:
  - title: 四种脚本引擎
    details: Lua、QuickJS、Node.js 和 Python，每种引擎都通过 lip 作为独立的包变体安装。
    link: apis/LanguageSupport.md
  - title: 运行 LLSE 插件
    details: 把 LLSE 插件放进 plugins/ 并启动服务器，插件会被自动迁移并获得 LeviLamina 模组清单。
  - title: LLSE API
    details: 事件、玩家、方块、物品、NBT、表单、文件、网络等。
    link: apis/README.md
---

## LegacyScriptEngine 是什么

LegacyScriptEngine（LSE）是一个 [LeviLamina](https://levimc.org/LeviLamina/) 模组，用来运行以脚本语言而不是 C++
编写的插件。它延续了 LiteLoaderBDS 的脚本引擎（LLSE）：为 LLSE 编写的插件几乎无需修改即可在 LeviLamina 上运行，
新插件也使用同一套 API，本站即是它的文档。

LSE 可运行在基岩版专用服务器和 Windows 上的 Minecraft 客户端中。与其他 LeviLamina 模组一样，服务器上用
[lip](https://levimc.org/lip/) 安装，客户端上用 [LeviLauncher](https://levimc.org/LeviLauncher/) 安装。

## 引擎

每种语言都有自己的引擎，作为 `LegacyScriptEngine` 包的一个变体安装。所有引擎共用一套 API，因此 API 参考适用于每种语言
（差异见[语言支持](apis/LanguageSupport.md)）。

| 变体 | 语言 | 引擎 |
|---|---|---|
| `#lua` | Lua | CLua |
| `#quickjs` | JavaScript | QuickJS，支持 ES 模块 |
| `#nodejs` | JavaScript | Node.js，支持 npm 包 |
| `#python` | Python | CPython，支持 pip 包 |

Python 引擎需要在运行它的机器上安装 Python：

| LSE 版本 | Python 版本 |
|---|---|
| >=0.16.2 | 3.12.10 |
| >=0.9.0 <0.16.2 | 3.12.8 |
| <0.9.0 | 3.10.11 |

在客户端上，0.23.1 及更早的版本把变体命名为 `#client_lua`、`#client_quickjs`、`#client_nodejs` 和 `#client_python`；
LeviLauncher 会为你选用正确的版本。

## 从这里开始

- [给服务器安装模组](https://levimc.org/tutorials/zh-Hans/server/install-mods/)：用 lip 在 LeviLamina 服务器上安装
  LegacyScriptEngine（以及其他模组）。
- [写你的第一个脚本插件](https://levimc.org/tutorials/zh-Hans/develop/first-script-plugin/)：编写一个插件，加载它并看到它运行。

已经有 LLSE 插件？把它们放进 `plugins/` 并启动服务器：插件会被自动迁移，并生成 LeviLamina 模组清单
（`manifest.json`）。重启服务器后，插件就会被加载。

## 本站内容

- **API 参考**：LSE 的[所有 API](apis/README.md)：事件、玩家、方块、物品、NBT、表单、文件、网络等。编辑器提示和项目脚手架见
  [legacy-script-engine-api](https://github.com/LiteLDev/legacy-script-engine-api)。
- **指南**：[常见问题](faq.md)。

LegacyScriptEngine 是开源项目：参见它的[仓库](https://github.com/LiteLDev/LegacyScriptEngine)和
[如何参与贡献](https://github.com/LiteLDev/LegacyScriptEngine/blob/develop/CONTRIBUTING.md)（issue、翻译、PR）。

## 许可

GPL-3.0-or-later © LeviMC
