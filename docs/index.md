# LegacyScriptEngine

A plugin engine for running LLSE plugins on LeviLamina

## Installation

### Server

!!! warning
    Before installing the Python engine, you need to install Python first. Here is a list of LSE versions and their required Python versions.

| LSE Version     | Python Version |
|-----------------|----------------|
| >=0.16.2        | 3.12.10        |
| >=0.9.0 <0.16.2 | 3.12.8         |
| <0.9.0          | 3.10.11        |

To install a specific engine, you can use the following command:
```shell
lip install github.com/LiteLDev/LegacyScriptEngine#lua
lip install github.com/LiteLDev/LegacyScriptEngine#quickjs
lip install github.com/LiteLDev/LegacyScriptEngine#nodejs
lip install github.com/LiteLDev/LegacyScriptEngine#python
```
For version older than 0.10.0, you can use the following command:
```shell
lip install git.levimc.org/LiteLDev/legacy-script-engine-lua@version
lip install git.levimc.org/LiteLDev/legacy-script-engine-quickjs@version
lip install git.levimc.org/LiteLDev/legacy-script-engine-nodejs@version
lip install git.levimc.org/LiteLDev/legacy-script-engine-python@version
```
Version numbers can be found in [releases](https://github.com/LiteLDev/LegacyScriptEngine/releases).

### Client

To install a specific engine, you can use the following command:
```shell
lip install github.com/LiteLDev/LegacyScriptEngine#client_lua
lip install github.com/LiteLDev/LegacyScriptEngine#client_quickjs
lip install github.com/LiteLDev/LegacyScriptEngine#client_nodejs
lip install github.com/LiteLDev/LegacyScriptEngine#client_python
```
To install a specific version, you can add`@version` at the end of installation command, such as:
```shell
lip install github.com/LiteLDev/LegacyScriptEngine#client_lua@0.17.0-rc.2
lip install github.com/LiteLDev/LegacyScriptEngine#client_quickjs@0.17.0-rc.2
lip install github.com/LiteLDev/LegacyScriptEngine#client_nodejs@0.17.0-rc.2
lip install github.com/LiteLDev/LegacyScriptEngine#client_python@0.17.0-rc.2
```


## Usage

To access plugin development API hints and scaffolding toolkits, visit the [legacy-script-engine-api](https://github.com/LiteLDev/legacy-script-engine-api) repository.

1. Put LLSE plugins directly in `plugins/`
2. Run the server, then the plugins will be migrated to LeviLamina plugin manifest automatically
3. To load them, you need to restart the server

## Contributing

If you have any questions, please open an issue to discuss it  
Help us improve translation [here](https://crowdin.com/project/legacyscriptengine)  
PRs are welcome

## License

GPL-3.0-or-later © LiteLDev
