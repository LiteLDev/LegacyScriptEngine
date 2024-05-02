# LegacyScriptEngine

A plugin engine for running LLSE plugins on LeviLamina

## Installation

To install QuickJs and Lua engine, you can use the following command:

```shell
lip install github.com/LiteLDev/LegacyScriptEngine
```

To install a specific engine, you can use the following command:

```shell
lip install gitea.litebds.com/LiteLDev/legacy-script-engine-lua
lip install gitea.litebds.com/LiteLDev/legacy-script-engine-quickjs
lip install gitea.litebds.com/LiteLDev/legacy-script-engine-nodejs
lip install gitea.litebds.com/LiteLDev/legacy-script-engine-python
```

To upgrade engines, you can use the following command:

```shell
lip install --upgrade gitea.litebds.com/LiteLDev/legacy-script-engine-lua
lip install --upgrade gitea.litebds.com/LiteLDev/legacy-script-engine-quickjs
lip install --upgrade gitea.litebds.com/LiteLDev/legacy-script-engine-nodejs
lip install --upgrade gitea.litebds.com/LiteLDev/legacy-script-engine-python
```

## Usage

1. Put LLSE plugins directly in `/path/to/bedrock_dedicated_server/plugins/`。

2. Run the server, then the plugins will be migrated to LeviLamina plugin manifest automatically.

3. To load them, you need to restart the server.

For more information, please refer to [the documentation](https://lse.liteldev.com).

## Contributing

If you have any questions, please open an issue to discuss it.

PRs are welcome.

## License

GPL-3.0-or-later © LiteLDev
