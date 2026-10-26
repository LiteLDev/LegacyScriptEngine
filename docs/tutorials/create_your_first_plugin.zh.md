# 创造你的首个脚本插件

> 本指南旨在展示创建你的第一个插件的非常简单和直接的过程--以及在考虑做什么和如何做时的一些最佳做法。建议有使用JavaScript的经验，但不是必须的。JavaScript是一种对初学者非常友好的语言，所以不要被淹没了!

## 先决条件

在开发你的第一个插件之前，我们需要设置你的开发环境。你用来编程的软件由你自己选择，但建议使用可信和可靠的软件。

- [Atom](https://github.blog/news-insights/product-news/sunsetting-atom/) 是轻量级的编辑器，在本教程中很好用。
- [VSCode](https://code.visualstudio.com/) 也是一个广泛使用的编辑器，具有很多强大的功能。

你还需要建立一个干净的LeviLamina安装，关于如何安装LeviLamina的细节可以在 [这里](https://docs.levimc.org/LeviLamina/zh/user_guides/install/) 找到。这个服务器将被用来测试你的插件。

有了你的开发环境，并完成了服务器的安装，你就可以开始了!

## 我现在该做什么？

开发一个 LSE 插件，首先要创建你的插件文件夹和文件。在服务器安装目录的 `plugins/` 文件夹中创建一个以插件命名的文件夹，例如
`MyPlugin`，并在其中创建你的脚本文件，例如 `main.js`。有些开发环境会允许你创建一个新文件并选择一个位置，而其他开发环境则允许你在点击"另存为"后才选择。

接下来，在同一文件夹中创建 `manifest.json` 文件。这个文件用于向 LeviLamina 注册你的插件：`entry` 是要运行的脚本文件，`name`
是插件名称，`type` 指定由哪个引擎运行插件（JavaScript 为 `lse-quickjs`，其他引擎为 `lse-nodejs`、`lse-lua` 和
`lse-python`），`dependencies` 指定需要先加载的引擎（`legacy-script-engine-quickjs`、`legacy-script-engine-nodejs`、
`legacy-script-engine-lua` 或 `legacy-script-engine-python`）。

```json
{
    "entry": "main.js",
    "name": "MyPlugin",
    "type": "lse-quickjs",
    "dependencies": [
        {
            "name": "legacy-script-engine-quickjs"
        }
    ]
}
```

如果你直接把单个旧版脚本文件（例如 `MyPlugin.js`）放进 `plugins/`，LSE 会在服务器启动时自动为它创建上述文件夹和
`manifest.json`；重启服务器后，迁移后的插件就会被加载。

你可能会注意到，下面的代码直接使用了 `mc`、`log` 等对象，却没有定义或导入它们。它们由 LegacyScriptEngine
提供，会在脚本运行时自动可用。这与你看到的任何其他没有定义的变量/类的引用是一样的。

现在我们已经创建了脚本文件和它的 `manifest.json`，接下来要做的就是创建一个事件监听器。我们通过使用`mc`来实现。

```js
mc.listen("onJoin", (player) => {
    log(`${player.name}已经加入服务器。`);
});
```

> 参考资料: [事件监听文档](../apis/EventAPI/Listen.zh.md)

为了测试你的插件，只需启动服务器，服务器应该能够识别你的插件并成功加载它。LeviLamina控制台将记录您创建的任何日志，以及您的插件或API失败时的任何错误。开发时的迭代很重要。经常测试，每一步都要确保当问题出现时，你清楚地知道你改变了什么，并能想出解决方案来解决它。

你可以引用`mc`类，以及其他特殊的类和构造函数。`mc`类是你的插件的面包和黄油，将允许你做很多很酷的事情。游戏内容接口有所有的方法和属性供你使用。
> 参考：[游戏元素接口文档](../apis/GameAPI/Basic.zh.md)

例如，我们可以使用玩家对象并直接对其采取行动，以发送信息/操纵玩家。

```js
mc.listen("onJoin", (player) => {
    log(`${player.name}已经加入服务器。`);
    player.sendToast('欢迎！', '感谢您游玩本服！');
});
```

我们可以引用玩家对象的属性，并使用它来执行其他动作。

```js
mc.listen("onJoin", (player) => {
    log(`${player.name}已经加入服务器。`);
    player.sendToast('欢迎！', '感谢您游玩本服！');
    let loginReward = mc.newItem('minecraft:diamond', 1);
    mc.spawnItem(loginReward, player.pos);
});
```

这给我们带来了最后的考虑。在制作插件时，尽量想一些简单的、自我封闭的东西。每个开发者都想建立一个具有大量功能的大型插件，但这样的项目很容易被放弃，因为它们从未真正完成。做一系列有特定目的的小插件。为这些插件添加功能，以实现配置和定制。找到你希望游戏拥有的功能或事物，并使用LeviLamina中的方法来实现它们。使用LeviLamina的API确实有无限可能。

如果您在开发过程中遇到任何问题，可以通过加入[LeviLamina](https://github.com/LiteLDev/LeviLamina)
的相关交流群、在LegacyScriptEngine的GitHub仓库上开立一个问题，来回答您的问题、意见或担忧。
