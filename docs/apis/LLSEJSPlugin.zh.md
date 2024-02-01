# 使用JavaScript创造你的首个脚本插件

> 本指南旨在展示创建你的第一个插件的非常简单和直接的过程--以及在考虑做什么和如何做时的一些最佳做法。建议有使用JavaScript的经验，但不是必须的。JavaScript是一种对初学者非常友好的语言，所以不要被淹没了!

## 先决条件

在开发你的第一个插件之前，我们需要设置你的开发环境。你用来编程的软件由你自己选择，但建议使用可信和可靠的软件。
- [Atom](https://atom.io/) 是轻量级的编辑器，在本教程中很好用。
- [VSCode](https://code.visualstudio.com/) 也是一个广泛使用的编辑器，具有很多强大的功能。

<br>

你还需要建立一个干净的LiteLoaderBDS安装，关于如何安装LiteLoaderBDS的细节可以在[这里](https://docs.litebds.com/zh-Hans/#/Usage)找到。这个服务器将被用来测试你的插件。

有了你的开发环境，并完成了服务器的安装，你就可以开始了!

## 我现在该做什么？

开发一个脚本插件，首先要创建你的插件文件。这个文件应该命名为 "LLMyPlugin.js"，将 "MyPlugin" 替换为你想要的插件名称。它应该被放在你的服务器安装的插件文件夹中。有些开发环境会允许你创建一个新文件并选择一个位置，而其他开发环境则允许你在点击 "另存为" 后才选择。

你在该文件中需要的第一行是下面这行。关于这个方法的参数信息，以及关于Script Assist API的信息，你可以去[这里](https://docs.litebds.com/zh-Hans/#/LLSEPluginDevelopment/ScriptAPI/ScriptHelp)。

`ll.registerPlugin(name, introduction, version, otherInformation)`。

这可能会使一些开发者感到困惑，因为`ll`应该是未定义的。然而，这个文件将被LiteLoaderBDS ScriptX引擎所利用。`ll`将在脚本运行时自动包含。这与你看到的任何其他没有定义的变量/类的引用是一样的。

现在我们已经创建了我们的.js文件，并注册了插件，我们要做的就是创建一个事件监听器。我们通过使用`mc`来实现。


```js
mc.listen("onJoin", (player) => log(`${player.name}已经加入服务器。`));
```
> 参考资料: https://docs.litebds.com/zh-Hans/#/LLSEPluginDevelopment/EventAPI/Listen

为了测试你的插件，只需启动服务器，服务器应该能够识别你的插件并成功加载它。LiteLoaderBDS控制台将记录您创建的任何日志，以及您的插件或API失败时的任何错误。开发时的迭代很重要。经常测试，每一步都要确保当问题出现时，你清楚地知道你改变了什么，并能想出解决方案来解决它。

你可以引用`mc`类，以及其他特殊的类和构造函数。`mc`类是你的插件的面包和黄油，将允许你做很多很酷的事情。游戏内容接口有所有的方法和属性供你使用。
>参考：https://docs.litebds.com/en/#/LLSEPluginDevelopment/GameAPI/Basic

例如，我们可以使用玩家对象并直接对其采取行动，以发送信息/操纵玩家。

```js
mc.listen("onJoin", (player) => {
    log(`${player.name}已经加入服务器。`)。
    player.sendToast('欢迎！', '感谢您游玩本服！');
});
```

我们可以引用玩家对象的属性，并使用它来执行其他动作。

```js
mc.listen("onJoin", (player) => {
    log(`${player.name}已经加入服务器。`)。
    player.sendToast('欢迎！', '感谢您游玩本服！');
    let loginReward = mc.newItem('minecraft:diamond', 1);
    mc.spawnItem(loginReward, player.pos)。
});
```

这给我们带来了最后的考虑。在制作插件时，尽量想一些简单的、自我封闭的东西。每个开发者都想建立一个具有大量功能的大型插件，但这样的项目很容易被放弃，因为它们从未真正完成。做一系列有特定目的的小插件。为这些插件添加功能，以实现配置和定制。找到你希望游戏拥有的功能或事物，并使用LiteLoaderBDS中的方法来实现它们。使用LiteLoaderBDS的API确实有无限可能。

如果您在开发过程中遇到任何问题，可以通过加入[Telegram](https://t.me/LiteLoaderBDSChs)/[Discord](https://discord.gg/5HU3cJsVZ5)--或在LiteLoaderBDS Github 仓库上开立一个问题，来回答您的问题、意见或担忧。