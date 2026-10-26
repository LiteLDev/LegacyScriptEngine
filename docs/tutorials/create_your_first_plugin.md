# Create Your First Plugin

> This guide serves to demonstrate the very simple and straight forward process to creating your first plugin - as well
> as some best practices when thinking about what to make and how to do it. Experience using JavaScript previously is
> recommended, but not required. JavaScript is a very beginner friendly language, so don't get overwhelmed!

## Prerequisites

Before developing your first plugin, we will need to set up your developer environment (IDE). The choice of software you
use to program is yours, but trusted and reliable software is recommended. [Atom](https://github.blog/news-insights/product-news/sunsetting-atom/) is lightweight
editor that works well for this tutorial, otherwise [VSCode](https://code.visualstudio.com/) is also a widely used
editor with a lot of powerful features.

You will also need to set up a clean LeviLamina installation, details on how to install LeviLamina can be
found [here](https://docs.levimc.org/LeviLamina/user_guides/install/). This server will be used to test your plugin.

With your IDE in hand, and the server installation complete, you are ready to begin!

## What Do I Do Now?

Developing an LSE plugin begins with creating your plugin's folder and files. Create a folder named after your plugin,
for example `MyPlugin`, in the `plugins/` folder of your server installation, and create your script file, for example
`main.js`, in it. Some IDEs will allow you to create a new file and choose a location, while others will allow you to
choose only after hitting "Save As".

Next, create a `manifest.json` file in the same folder. This file registers your plugin with LeviLamina: `entry` is the
script file to run, `name` is the name of your plugin, `type` tells LeviLamina which engine runs it (`lse-quickjs` for
JavaScript; `lse-nodejs`, `lse-lua` and `lse-python` for the other engines), and `dependencies` names the engine to load
first (`legacy-script-engine-quickjs`, `legacy-script-engine-nodejs`, `legacy-script-engine-lua` or
`legacy-script-engine-python`).

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

If you put a single legacy script file (for example `MyPlugin.js`) directly in `plugins/` instead, LSE creates this
folder and `manifest.json` for you when the server starts; restart the server to load the migrated plugin.

You may notice that the code below uses objects such as `mc` and `log` without defining or importing them. They are
provided by LegacyScriptEngine and are automatically available while your script runs. This is the same with any other
variables/classes you see referenced without a definition.

Now that we have created our script file and its `manifest.json`, all we have to do from here is create an event
listener. We do that by utilizing `mc`.

```js
mc.listen("onJoin", (player) => {
    log(`${player.name} has joined the server.`);
});
```

> Reference: [Event Listening Documentation](../apis/EventAPI/Listen.md)

In order to test your plugin, simply start the server and the server should be able to identify your plugin and
successfully load it. LeviLamina console will log any logs you create, as well as any errors if your plugin, or the API
fails. Iteration while developing is important. Test frequently and each step of the way to ensure that when an issue
does pop up, you know exactly what you changed and can come up with a solution to fix it.

You can reference the `mc` class as well as other special classes and constructors. The `mc` class is the bread and
butter of your plugin and will allow you to do a lot of cool things. The Game Content interface has all the methods and
properties available to you.
> Reference: [Game Element Interface Documentation](../apis/GameAPI/Basic.md)

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

This brings us to the final considerations. When making plugins, try to think of something simple and self-enclosing.
Every developer wants to build a massive plugin with a ton of features, but such projects are prone to abandonment as
they never truly get finished. Make a series of small plugins that have specific purpose. Add features to those plugins
to enable configuration and customization. Find features or things you wish the game had and use the methods available
in LeviLamina to make them happen. There are truly unlimited possibilities with LeviLamina's API.

If you run into any issues while developing, questions, comments or concerns can be answered by
joining [LeviLamina](https://github.com/LiteLDev/LeviLamina)‘s Groups or by opening an issue on the LegacyScriptEngine
GitHub Repo.
