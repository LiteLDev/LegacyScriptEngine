# Contributing to LegacyScriptEngine

Thank you for helping out! To *use* LegacyScriptEngine, see the [documentation](https://levimc.org/LegacyScriptEngine/)
and the tutorials [Install mods on your server](https://levimc.org/tutorials/server/install-mods/) and
[Your first script plugin](https://levimc.org/tutorials/develop/first-script-plugin/).

## Issues

Found a bug, or a plugin API that behaves differently from LLSE? Have a question or an idea?
[Open an issue](https://github.com/LiteLDev/LegacyScriptEngine/issues/new/choose) to discuss it. For a bug, include
the LSE version and engine (`#lua`, `#quickjs`, `#nodejs`, `#python`), the LeviLamina and game versions, the log and,
if you can, a small plugin that shows the problem.

## Translations

LSE's own messages (`src/lang/`) are translated on [Crowdin](https://crowdin.com/project/legacyscriptengine). Join the
project there; the translations come back to this repository as a pull request from the Crowdin workflow. Please
don't edit the translated files in `src/lang/` directly: only `en.json` is the source.

## Pull requests

Pull requests are welcome. Fork the repository, branch off `develop` and open the pull request against `develop`. For
a large change, open an issue first so we can agree on the approach.

LSE is built with [xmake](https://xmake.io) on Windows x64, one engine and one target at a time (as in
`.github/workflows/build.yml`):

```powershell
xmake repo -u
xmake f -a x64 -m release -p windows -y --target_type=server --backend=lua
xmake -y
```

`--target_type` is `server` (default) or `client`; `--backend` is `lua` (default), `quickjs`, `nodejs` or `python`.
Add user-visible changes to [CHANGELOG.md](CHANGELOG.md).

## Documentation

The documentation site at <https://levimc.org/LegacyScriptEngine/> (Simplified Chinese at
<https://levimc.org/zh-Hans/LegacyScriptEngine/>) is built from the
[LiteLDev/liteldev.github.io](https://github.com/LiteLDev/liteldev.github.io) repository, not from this one. Edit the
pages in [`docs/LegacyScriptEngine/`](https://github.com/LiteLDev/liteldev.github.io/tree/main/docs/LegacyScriptEngine)
there and open the pull request on that repository. Step-by-step tutorials are on
[levimc.org/tutorials](https://levimc.org/tutorials/), in the same repository (`tutorials/`).

## License

By contributing, you agree that your contributions are licensed under GPL-3.0-or-later (see [COPYING](COPYING)).
