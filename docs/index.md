# LegacyScriptEngine

A plugin engine for running LLSE plugins on LeviLamina

## Installation

!!! warning
    Before installing the Python engine, you need to install Python first. Here is a list of LSE versions and their required Python versions.

| LSE Version     | Python Version |
|-----------------|----------------|
| >=0.16.2        | 3.12.10        |
| >=0.9.0 <0.16.2 | 3.12.8         |
| <0.9.0          | 3.10.11        |

Each engine is its own package variant. Version numbers can be found in
[releases](https://github.com/LiteLDev/LegacyScriptEngine/releases); add `@version` to install a specific one.

=== "Server"

    In the server directory, install the engine you need with lip:

    ```shell
    lip install github.com/LiteLDev/LegacyScriptEngine#lua
    lip install github.com/LiteLDev/LegacyScriptEngine#quickjs
    lip install github.com/LiteLDev/LegacyScriptEngine#nodejs
    lip install github.com/LiteLDev/LegacyScriptEngine#python
    ```

    For example, `lip install github.com/LiteLDev/LegacyScriptEngine#lua@0.17.0-rc.2`.

    For versions older than 0.10.0, you can only download them
    from [releases](https://github.com/LiteLDev/LegacyScriptEngine/releases) and decompress them in the `plugins/` folder.
    The Node.js engine then also needs **node-prebuilt.zip** from [LiteLDev/node](https://github.com/LiteLDev/node/releases),
    decompressed in `plugins/legacy-script-engine-nodejs/`. Installing through lip doesn't require these steps.

=== "Client"

    lip manages server directories only. On the client, install the engine into a LeviLauncher instance that has
    LeviLamina (see [Install client on Windows](https://docs.levimc.org/LeviLamina/user_guides/install_on_client/)):
    LeviLauncher installs the client variants of the package, `#client_lua`, `#client_quickjs`, `#client_nodejs` and
    `#client_python`, for example `github.com/LiteLDev/LegacyScriptEngine#client_lua@0.17.0-rc.2`. See LeviLauncher's
    [Mods & Integrations](https://docs.levimc.org/LeviLauncher/guide/mods-integrations?platform=windows).

## Usage

To access plugin development API hints and scaffolding toolkits, visit
the [legacy-script-engine-api](https://github.com/LiteLDev/legacy-script-engine-api) repository.

1. Put LLSE plugins directly in `plugins/`
2. Run the server, then the plugins will be migrated automatically and get a LeviLamina mod manifest (manifest.json)
3. To load them, you need to restart the server

## Contributing

If you have any questions, please open an issue to discuss it  
Help us improve translation [here](https://crowdin.com/project/legacyscriptengine)  
PRs are welcome

## License

GPL-3.0-or-later © LeviMC
