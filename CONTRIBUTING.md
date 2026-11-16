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

The documentation site at <https://levimc.org/LegacyScriptEngine/> is built from this repository:

- Pages are in `docs/`, in English (`page.md`) and Simplified Chinese (`page.zh.md`). Traditional Chinese is generated
  from the Simplified pages. Change both languages in the same pull request when you can.
- New pages are registered in the `nav` of `docs.config.ts` (with the Chinese label of titled entries in
  `navTranslations.zh`). A moved or removed page gets an entry in `redirects` so its old URLs keep working.
- The site keeps the API reference and guides. Step-by-step tutorials are on
  [levimc.org/tutorials](https://levimc.org/tutorials/), in the
  [LiteLDev/liteldev.github.io](https://github.com/LiteLDev/liteldev.github.io) repository (`tutorials/`).
- Markdown follows MkDocs / Python-Markdown conventions: 4-space list nesting, `!!! note` admonitions,
  `=== "Tab"` tabs, and links to other pages as relative `.md` paths.

The site is built with levimc-kit, which `npm ci` downloads into `kit/` (git-ignored; never edit it). With Node.js 24,
from the repository root:

```sh
npm ci
npm run typecheck   # tsc over docs.config.ts and the kit
npm run build       # → dist/; prints unresolved links and missing anchors
npm run preview     # http://127.0.0.1:4173/LegacyScriptEngine/
npm run linkcheck   # internal links, anchors and redirects of dist/
npm run dev         # dev server with hot reload
```

CI (`.github/workflows/build_docs.yml`) runs the same typecheck, build and linkcheck on every pull request.

## License

By contributing, you agree that your contributions are licensed under GPL-3.0-or-later (see [COPYING](COPYING)).
