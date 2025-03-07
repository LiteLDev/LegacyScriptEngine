# LegacyScriptEngine

A plugin engine for running LLSE plugins on LeviLamina

## Installation

### Attention

Before installing the Python engine, you need to
install [Python 3.12.8](https://www.python.org/downloads/release/python-3128/) first.

To install a specific engine, you can use the following command:

```shell
lip install gitea.litebds.com/LiteLDev/legacy-script-engine-lua@version
lip install gitea.litebds.com/LiteLDev/legacy-script-engine-quickjs@version
lip install gitea.litebds.com/LiteLDev/legacy-script-engine-nodejs@version
lip install gitea.litebds.com/LiteLDev/legacy-script-engine-python@version
```

Version numbers can be found in [releases](https://github.com/LiteLDev/LegacyScriptEngine/releases).

## Usage

1. Put LLSE plugins directly in `plugins/`
2. Run the server, then the plugins will be migrated to LeviLamina plugin manifest automatically
3. To load them, you need to restart the server

## Some helpful project for plugin development

- [LiteLoaderSE-Aids](https://github.com/LiteLDev/LiteLoaderSE-Aids)
- [LiteLoaderSE-Aids-Magic-Revision](https://github.com/luoqing510/LiteLoaderSE-Aids-Magic-Revision)
- [HelperLib](https://github.com/LiteLDev/HelperLib)
- [llpy-helper-lib](https://github.com/LiteLDev/llpy-helper-lib)

## Contributing

If you have any questions, please open an issue to discuss it  
Help us improve translation [here](https://crowdin.com/project/legacyscriptengine)  
PRs are welcome

## License

GPL-3.0-or-later © LiteLDev
