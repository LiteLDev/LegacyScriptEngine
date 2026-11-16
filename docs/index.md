---
layout: home
hero:
  name: LegacyScriptEngine
  tagline: A plugin engine for running LLSE plugins on LeviLamina
  image:
    src: img/logo.png
    alt: LegacyScriptEngine
  actions:
    - theme: brand
      text: Your first script plugin
      link: https://levimc.org/tutorials/develop/first-script-plugin/
    - theme: alt
      text: Install it on your server
      link: https://levimc.org/tutorials/server/install-mods/
    - theme: alt
      text: API Reference
      link: apis/README.md
features:
  - title: Four script engines
    details: Lua, QuickJS, Node.js and Python, each installed with lip as its own package variant.
    link: apis/LanguageSupport.md
  - title: Runs LLSE plugins
    details: Put LLSE plugins in plugins/ and start the server; they are migrated automatically and get a LeviLamina mod manifest.
  - title: The LLSE API
    details: Events, players, blocks, items, NBT, forms, files, network and more.
    link: apis/README.md
---

## What LegacyScriptEngine is

LegacyScriptEngine (LSE) is a [LeviLamina](https://levimc.org/LeviLamina/) mod that runs plugins written in a
script language instead of C++. It carries on the script engine of LiteLoaderBDS (LLSE): plugins written for LLSE run
on LeviLamina with little or no change, and new plugins use the same API, documented on this site.

LSE runs on Bedrock Dedicated Server and in the Minecraft client on Windows. Like every LeviLamina mod, it is
installed with [lip](https://levimc.org/lip/) on a server and with [LeviLauncher](https://levimc.org/LeviLauncher/)
on the client.

## Engines

Each language has its own engine, installed as a variant of the `LegacyScriptEngine` package. One API serves all of
them, so the reference applies to every language (see [Language Support](apis/LanguageSupport.md) for what differs).

| Variant | Language | Engine |
|---|---|---|
| `#lua` | Lua | CLua |
| `#quickjs` | JavaScript | QuickJS, with ES modules |
| `#nodejs` | JavaScript | Node.js, with npm packages |
| `#python` | Python | CPython, with pip packages |

The Python engine needs Python on the machine it runs on:

| LSE version | Python version |
|---|---|
| >=0.16.2 | 3.12.10 |
| >=0.9.0 <0.16.2 | 3.12.8 |
| <0.9.0 | 3.10.11 |

On the client, releases up to 0.23.1 name the variants `#client_lua`, `#client_quickjs`, `#client_nodejs` and
`#client_python`; LeviLauncher picks the right build for you.

## Start here

- [Install mods on your server](https://levimc.org/tutorials/server/install-mods/): install LegacyScriptEngine (and
  other mods) on a LeviLamina server with lip.
- [Your first script plugin](https://levimc.org/tutorials/develop/first-script-plugin/): write a plugin, load it
  and see it run.

Already have LLSE plugins? Put them in `plugins/` and start the server: they are migrated automatically and get a
LeviLamina mod manifest (`manifest.json`). Restart the server to load them.

## On this site

- **API Reference**: [every API](apis/README.md) of LSE: events, players, blocks, items, NBT, forms, files, network
  and more. For editor hints and project scaffolding, see
  [legacy-script-engine-api](https://github.com/LiteLDev/legacy-script-engine-api).
- **Guide**: the [FAQ](faq.md).

LegacyScriptEngine is open source: see its [repository](https://github.com/LiteLDev/LegacyScriptEngine) and
[how to contribute](https://github.com/LiteLDev/LegacyScriptEngine/blob/develop/CONTRIBUTING.md) (issues,
translations, pull requests).

## License

GPL-3.0-or-later © LeviMC
