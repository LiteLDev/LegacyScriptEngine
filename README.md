# LegacyScriptEngine

English | [简体中文](README.zh.md)  
A plugin engine for running LLSE plugins on LeviLamina

## Installation

Follow the tutorial [Install mods on your server](https://levimc.org/tutorials/server/install-mods/). To write a
plugin, start with [Your first script plugin](https://levimc.org/tutorials/develop/first-script-plugin/).

Each language has its own engine, installed with [lip](https://levimc.org/lip/) as a variant of the package:
`lip install LegacyScriptEngine#lua` (or `#quickjs`, `#nodejs`, `#python`). Without a variant, lip installs the QuickJS
and Lua engines together.

## Usage

To access plugin development API hints and scaffolding toolkits, visit the [legacy-script-engine-api](https://github.com/LiteLDev/legacy-script-engine-api) repository.

1. Put LLSE plugins directly in `plugins/` (since 0.23.0 a link to `mods/`, created on startup)
2. Run the server, then the plugins will be migrated automatically and get a LeviLamina mod manifest (manifest.json)
3. To load them, you need to restart the server

For more information, please refer to [the documentation](https://levimc.org/LegacyScriptEngine/)

## Contributing

If you have any questions, please open an issue to discuss it  
Help us improve translation [here](https://crowdin.com/project/legacyscriptengine)  
PRs are welcome: see [CONTRIBUTING.md](CONTRIBUTING.md)

## License

GPL-3.0-or-later © LeviMC
