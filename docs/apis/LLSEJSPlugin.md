# LLSE - Using JavaScript to Create Your First Plugin

> This guide serves to demonstrate the very simple and straight forward process to creating your first plugin - as well as some best practices when thinking about what to make and how to do it. Experience using JavaScript previously is recommended, but not required. JavaScript is a very beginner friendly language, so don't get overwhelmed!

## Prerequisites

Before developing your first plugin, we will need to set up your developer environment (IDE). The choice of software you use to program is yours, but trusted and reliable software is recommended. [Atom](https://atom.io/) is lightweight editor that works well for this tutorial, otherwise [VSCode](https://code.visualstudio.com/) is also a widely used editor with a lot of powerful features.

You will also need to set up a clean LiteLoaderBDS installation, details on how to install LiteLoaderBDS can be found [here](https://docs.litebds.com/en/#/Usage). This server will be used to test your plugin.

With your IDE in hand, and the server installation complete, you are ready to begin!

## What Do I Do Now?

Developing a LLSE Plugin begins with creating your plugin's file. This file should be named "LLMyPlugin.js", replacing "MyPlugin" with what you would like to call your plugin. It should be placed in the plugins folder of your server installation. Some IDEs will allow you to create a new file and choose a location, while others will allow you to choose only after hitting "Save As".

The first line you will need in that file is the one below. For information on parameters for this method, and information on the Script Assist API, you can go [here](https://docs.litebds.com/en/#/LLSEPluginDevelopment/ScriptAPI/ScriptHelp).

`ll.registerPlugin(name, introduction, version, otherInformation)`

This might confuse some developers, as `ll` should be undefined. However, this file is going to be utilized by the LiteLoaderBDS ScriptX Engine. `ll` will be automatically included during the script's runtime. This is the same with any other variables/classes you see referenced without a definition.

Now that we have created our .js file, and registered the plugin, all we have to do from here is create an event listener. We do that by utilizing `mc`.


```js
mc.listen("onJoin", (player) => {
    log(`${player.name} has joined the server.`);
});
```
> Reference: https://docs.litebds.com/en/#/LLSEPluginDevelopment/EventAPI/Listen

In order to test your plugin, simply start the server and the server should be able to identify your plugin and successfully load it. LiteLoaderBDS console will log any logs you create, as well as any errors if your plugin, or the API fails. Iteration while developing is important. Test frequently and each step of the way to ensure that when an issue does pop up, you know exactly what you changed and can come up with a solution to fix it.

You can reference the `mc` class as well as other special classes and constructors. The `mc` class is the bread and butter of your plugin and will allow you to do a lot of cool things. The Game Content interface has all the methods and properties available to you.
> Reference: https://docs.litebds.com/en/#/LLSEPluginDevelopment/GameAPI/Basic

For example, we can use the player object and directly act on it to send information/manipulate a Player.

```js
mc.listen("onJoin", (player) => {
    log(`${player.name} has joined the server.`);
    player.sendToast('Welcome!', 'Thanks for joining the server!');
});
```

We can reference player object properties and use that to execute other actions.

```js
mc.listen("onJoin", (player) => {
    log(`${player.name} has joined the server.`);
    player.sendToast('Welcome!', 'Thanks for joining the server!');
    let loginReward = mc.newItem('minecraft:diamond', 1);
    mc.spawnItem(loginReward, player.pos);
});
```

This brings us to the final considerations. When making plugins, try to think of something simple and self-enclosing. Every developer wants to build a massive plugin with a ton of features, but such projects are prone to abandonment as they never truly get finished. Make a series of small plugins that have specific purpose. Add features to those plugins to enable configuration and customization. Find features or things you wish the game had and use the methods available in LiteLoaderBDS to make them happen. There are truly unlimited possibilities with LiteLoader's API.

If you run into any issues while developing, questions, comments or concerns can be answered by joining the [Discord](https://discord.gg/5HU3cJsVZ5) - or by opening an issue on the LiteLoaderBDS Github Repo.
