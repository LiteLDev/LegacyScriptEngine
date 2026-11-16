// LegacyScriptEngine documentation — built with levimc-kit (see kit/README.md).
// Ported 1:1 from the former mkdocs.yml (MkDocs Material + mkdocs-static-i18n).
import { defineDocsConfig } from './kit/docs/node/config.ts';

export default defineDocsConfig({
  base: '/LegacyScriptEngine/',
  title: 'LegacyScriptEngine',
  logo: 'img/logo.png',
  favicon: 'img/logo.png',
  project: 'LegacyScriptEngine',
  repo: 'https://github.com/LiteLDev/LegacyScriptEngine',
  editLink: 'https://github.com/LiteLDev/LegacyScriptEngine/edit/develop/docs/:path',
  contentDir: 'docs',
  hostname: 'https://levimc.org',
  sitemap: { hostname: 'https://levimc.org' },
  lastUpdated: true,
  i18n: {
    mode: 'suffix',
    defaultLocale: 'en',
    locales: [
      { code: 'en', label: 'English', lang: 'en' },
      // Simplified Chinese at zh-Hans/ (sources stay *.zh.md); the former zh/ URLs redirect to it (kit 2.21)
      { code: 'zh', label: '简体中文', lang: 'zh-Hans', prefix: 'zh-Hans/', formerPrefixes: ['zh/'] },
      // Traditional Chinese, generated from the zh pages (levimc-kit convertFrom)
      { code: 'zh-Hant', label: '繁體中文', lang: 'zh-Hant', convertFrom: 'zh' },
    ],
  },
  urlStyle: 'directory',
  slug: 'pymdownx',
  markdown: { compat: 'python-markdown' },
  // MkDocs Material lists h2–h6 in "Table of contents"; the API entries of File, Basic, Command, DataBase
  // and LanguageSupport are h5 headings, so keep the full depth instead of the kit default [2, 4].
  outline: [2, 6],
  sidebar: 'tabs',
  // The LeviMC navigation model (kit/README.md "Navigation model"): every top-level entry is a section (a header tab
  // and its sidebar tree); the brand links the home page, which introduces the project. Tutorials (installing LSE,
  // your first plugin) live at https://levimc.org/tutorials/. The API Reference section has more than 24 links, so
  // only the group holding the current page starts open.
  nav: [
    { Guide: [{ Help: ['faq.md'] }] },
    {
      'API Reference': [
        'apis/README.md',
        'apis/LanguageSupport.md',
        {
          DataAPI: [
            'apis/DataAPI/ConfigFile.md',
            'apis/DataAPI/DataBase.md',
            'apis/DataAPI/Economy.md',
            'apis/DataAPI/OtherData.md',
            'apis/DataAPI/PlayerData.md',
          ],
        },
        {
          EventAPI: [
            'apis/EventAPI/Listen.md',
            'apis/EventAPI/BlockEvents.md',
            'apis/EventAPI/EconomicEvents.md',
            'apis/EventAPI/EntityEvents.md',
            'apis/EventAPI/OtherEvents.md',
            'apis/EventAPI/PlayerEvents.md',
          ],
        },
        {
          GameAPI: [
            'apis/GameAPI/Basic.md',
            'apis/GameAPI/Block.md',
            'apis/GameAPI/BlockEntity.md',
            'apis/GameAPI/Command.md',
            'apis/GameAPI/Container.md',
            'apis/GameAPI/Device.md',
            'apis/GameAPI/Entity.md',
            'apis/GameAPI/GameUtils.md',
            'apis/GameAPI/Item.md',
            'apis/GameAPI/Packet.md',
            'apis/GameAPI/Particle.md',
            'apis/GameAPI/Player.md',
            'apis/GameAPI/ScoreBoard.md',
            'apis/GameAPI/Server.md',
          ],
        },
        { GuiAPI: ['apis/GuiAPI/Form.md', 'apis/GuiAPI/FormBuilder.md'] },
        {
          NbtAPI: [
            'apis/NbtAPI/NBT.md',
            'apis/NbtAPI/NBTCompound.md',
            'apis/NbtAPI/NBTList.md',
            'apis/NbtAPI/NBTValue.md',
          ],
        },
        {
          ScriptAPI: [
            'apis/ScriptAPI/Logger.md',
            'apis/ScriptAPI/ScriptHelp.md',
            'apis/ScriptAPI/i18n.md',
            'apis/ScriptAPI/Ll.md',
            'apis/ScriptAPI/VanillaI18n.md',
          ],
        },
        {
          SystemAPI: [
            'apis/SystemAPI/File.md',
            'apis/SystemAPI/FileSystem.md',
            'apis/SystemAPI/Network.md',
            'apis/SystemAPI/SystemCall.md',
            'apis/SystemAPI/SystemInfo.md',
          ],
        },
      ],
    },
  ],
  // The tutorial moved to levimc.org/tutorials/: every URL it had (English, zh-Hans/, the former zh/, zh-Hant/) goes to
  // the tutorial in the matching language.
  redirects: {
    'tutorials/create_your_first_plugin/': 'https://levimc.org/tutorials/develop/first-script-plugin/',
    'zh-Hans/tutorials/create_your_first_plugin/': 'https://levimc.org/tutorials/zh-Hans/develop/first-script-plugin/',
    'zh/tutorials/create_your_first_plugin/': 'https://levimc.org/tutorials/zh-Hans/develop/first-script-plugin/',
    'zh-Hant/tutorials/create_your_first_plugin/': 'https://levimc.org/tutorials/zh-Hant/develop/first-script-plugin/',
  },
  navTranslations: {
    zh: { Guide: '指南', Help: '帮助', 'API Reference': 'API 参考' },
  },
});
