# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.7.12] - 2024-05-22

### Fixed

- Fix onPistonTryPush & onPistonPush [#137]

## [0.7.11] - 2024-05-11

### Fixed

- Fix unload [#134]

## [0.7.10] - 2024-05-09

### Changed

- Allow nodejs engine unload

### Fixed

- Fix transMoney
- Fix nodejs engine in Non-English directory

## [0.7.9] - 2024-05-07

### Changed

- Not create manifest.json when migrating if it exists
- Add compability for mc.explode

### Fixed

- Fix block.name [#130]
- Fix getAllPluginInfo [#128]

## [0.7.8] - 2024-05-04

### Fixed

- Fix HttpServer
- Make FileSystemAPI supports utf-8

## [0.7.7] - 2024-05-03

### Fixed

- Fix [#123]

## [0.7.6] - 2024-05-02

### Changed

- Remove error log in some api

## [0.7.5] - 2024-05-02

### Added

- Add missing hopper events [#88]

### Changed

- Replace SimulatedPlayer::create

### Fixed

- Fix unexpected situation of onMobDie
- Fix some stupid exception process

## [0.7.4] - 2024-05-02

### Changed

- Make onPlaceBlock returning placing face

### Fixed

- Fix truePos pre calculation
- Fix onProjectileHitBlock & onProjectileHitEntity [#108]
- Fix mc.explode [#111]

## [0.7.3] - 2024-05-01

### Fixed

- Fix getRespawnPosition [#115]
- Fix onDropItem [#110]

## [0.7.2] - 2024-04-29

### Fixed

- Fix output of runcmdEx not integrity

## [0.7.1] - 2024-04-29

### Changed

- Disable unload in node engine

### Fixed

- Fix PluginManager's error

## [0.7.0] - 2024-04-28

### Changed

- Adapt to LeviLamina 0.12.x

## [0.6.4] - 2024-04-21

### Fixed

- Fix NetworkAPI's callback
- Enlarge NewProcess's buffer [#48]

## [0.6.3] - 2024-04-20

### Added

- Add onConsumeTotem [#104]
- Add onSetArmor [#95]
- Add onProjectileHitEntity & onProjectileHitBlock [#87]

### Changed

- Set snbt format to ForceQuote in toSNBT

### Fixed

- Fix NetworkPacket [#50]

## [0.6.2] - 2024-04-14

### Fixed

- Fix [#101] [#102]

## [0.6.1] - 2024-04-13

### Fixed

- Fix clearItem [#100]
- Fix getEnderChest [#99]

## [0.6.0] - 2024-04-13

### Fixed

- Support deferred command registration [#35]
- Fix `mc.getStructure` [#97]

## [0.5.4] - 2024-04-08

### Fixed

- Fix getPlayerNbt [#94]
- Fix offline setPlayerNbt

## [0.5.3] - 2024-04-05

### Fixed

- Add default value process into Entity::hurt [#91]
- Fix setScale [#92]

## [0.5.2] - 2024-03-28

### Fixed

- Fix [#79] [#80] [#81] [#82]

## [0.5.1] - 2024-03-27

### Changed

- Support LeviLamina 0.10.x
- Add judgement in getEnderChest

### Fixed

- Fix [#76]

## [0.4.15] - 2024-03-24

### Fixed

- Fix APIs about plugins [#72] [#73] [#74]

## [0.4.14] - 2024-03-18

### Changed

- Support LeviLamina 0.9.5

## [0.4.13] - 2024-03-15

### Fixed

- Fix economy related events
- Fix some events cancelled wrongly

## [0.4.12] - 2024-03-15

### Added

- Add onUseBucketPlace & onUseBucketTake [#70]

### Changed

- Make event's behavior match LLSE

### Fixed

- Fix setInterval [#71]

## [0.4.11] - 2024-03-14

### Fixed

- Fix some api about create directory

## [0.4.10] - 2024-03-14

### Changed

- Support LeviLamina 0.9.3

### Fixed

- Fix `setTimeout` dead lock completely

## [0.4.8] - 2024-03-10

### Fixed

- Fix `setTimeout` dead lock problem

## [0.4.7] - 2024-03-10

### Added

- Add onPlayerPullFishingHook
- Add onScoreChanged

## [0.4.6] - 2024-03-10

### Added

- Add python & nodejs support (#62)

### Fixed

- Fix some scoreboard api (#60)

## [0.4.3] - 2024-03-03

### Added

- Add seh translator

### Fixed

- Fix getAllTags (#56)
- Fix return value of runcmdEx [#54]
- Fix HttpServer exception [#53]
- Fix an error in Player::getAllItems
- Fix uncaught exception while loading plugins
- Fix Player::clearItem

## [0.4.2] - 2024-03-02

### Added

- Add Device::inputMode

### Fixed

- Fix Player::serverAddress [#52]

## [0.4.1] - 2024-02-25

### Fixed

- Fix getLastDeathPos [#45]
- Fix Player::talkAs [#47]
- Fix Player::setBossBar [#49]

## [0.4.0] - 2024-02-24

### Changed

- Support LeviLamina 0.9.x

### Fixed

- Fix [#43]
- Fix [#44]
- Fix [#45]
- Fix [#37]
- Fix [#31]

## [0.3.2] - 2024-02-23

### Changed

- Refactor plugin manager

## [0.3.1] - 2024-02-21

### Fixed

- Fix player score API (#36)

## [0.3.0] - 2024-02-13

### Added

- Support LeviLamina 0.8.x

### Fixed

- Fix `Player::talkAs`
- Fix Chinese path related problems
- Fix an error in `onMobSpawned`

## [0.2.4] - 2024-02-08

### Fixed

- Fix onCmdBlockExecute

## [0.2.2] - 2024-02-06

### Fixed

- Fix runcmdEx error when command has no output

## [0.2.1] - 2024-02-06

### Fixed

- Fix endless loop

## [0.2.0] - 2024-02-06

### Added

- Support LeviLamina 0.7.x

### Fixed

- Fix Device::getIP

## [0.1.6] - 2024-02-05

### Fixed

- Fix a logic error while migrating old plugins

## [0.1.5] - 2024-02-05

### Fixed

- Fix some error while migrating old plugins

## [0.1.4] - 2024-02-05

### Added

- First release.

[#31]: https://github.com/LiteLDev/LegacyScriptEngine/issues/31
[#35]: https://github.com/LiteLDev/LegacyScriptEngine/issues/35
[#37]: https://github.com/LiteLDev/LegacyScriptEngine/issues/37
[#43]: https://github.com/LiteLDev/LegacyScriptEngine/issues/43
[#44]: https://github.com/LiteLDev/LegacyScriptEngine/issues/44
[#45]: https://github.com/LiteLDev/LegacyScriptEngine/issues/45
[#47]: https://github.com/LiteLDev/LegacyScriptEngine/issues/47
[#48]: https://github.com/LiteLDev/LegacyScriptEngine/issues/48
[#49]: https://github.com/LiteLDev/LegacyScriptEngine/issues/49
[#50]: https://github.com/LiteLDev/LegacyScriptEngine/issues/50
[#52]: https://github.com/LiteLDev/LegacyScriptEngine/issues/52
[#53]: https://github.com/LiteLDev/LegacyScriptEngine/issues/53
[#54]: https://github.com/LiteLDev/LegacyScriptEngine/issues/54
[#70]: https://github.com/LiteLDev/LegacyScriptEngine/issues/70
[#71]: https://github.com/LiteLDev/LegacyScriptEngine/issues/71
[#72]: https://github.com/LiteLDev/LegacyScriptEngine/issues/72
[#73]: https://github.com/LiteLDev/LegacyScriptEngine/issues/73
[#74]: https://github.com/LiteLDev/LegacyScriptEngine/issues/74
[#76]: https://github.com/LiteLDev/LegacyScriptEngine/issues/76
[#79]: https://github.com/LiteLDev/LegacyScriptEngine/issues/79
[#80]: https://github.com/LiteLDev/LegacyScriptEngine/issues/80
[#81]: https://github.com/LiteLDev/LegacyScriptEngine/issues/81
[#82]: https://github.com/LiteLDev/LegacyScriptEngine/issues/82
[#87]: https://github.com/LiteLDev/LegacyScriptEngine/issues/87
[#88]: https://github.com/LiteLDev/LegacyScriptEngine/issues/88
[#91]: https://github.com/LiteLDev/LegacyScriptEngine/issues/91
[#92]: https://github.com/LiteLDev/LegacyScriptEngine/issues/92
[#94]: https://github.com/LiteLDev/LegacyScriptEngine/issues/94
[#95]: https://github.com/LiteLDev/LegacyScriptEngine/issues/95
[#97]: https://github.com/LiteLDev/LegacyScriptEngine/issues/97
[#99]: https://github.com/LiteLDev/LegacyScriptEngine/issues/99
[#100]: https://github.com/LiteLDev/LegacyScriptEngine/issues/100
[#101]: https://github.com/LiteLDev/LegacyScriptEngine/issues/101
[#102]: https://github.com/LiteLDev/LegacyScriptEngine/issues/102
[#104]: https://github.com/LiteLDev/LegacyScriptEngine/issues/104
[#108]: https://github.com/LiteLDev/LegacyScriptEngine/issues/108
[#110]: https://github.com/LiteLDev/LegacyScriptEngine/issues/110
[#111]: https://github.com/LiteLDev/LegacyScriptEngine/issues/111
[#115]: https://github.com/LiteLDev/LegacyScriptEngine/issues/115
[#123]: https://github.com/LiteLDev/LegacyScriptEngine/issues/123
[#128]: https://github.com/LiteLDev/LegacyScriptEngine/issues/128
[#130]: https://github.com/LiteLDev/LegacyScriptEngine/issues/130
[#134]: https://github.com/LiteLDev/LegacyScriptEngine/issues/134
[#137]: https://github.com/LiteLDev/LegacyScriptEngine/issues/137

[0.7.12]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.7.11...v0.7.12
[0.7.11]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.7.10...v0.7.11
[0.7.10]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.7.9...v0.7.10
[0.7.9]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.7.8...v0.7.9
[0.7.8]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.7.7...v0.7.8
[0.7.7]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.7.6...v0.7.7
[0.7.6]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.7.5...v0.7.6
[0.7.5]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.7.4...v0.7.5
[0.7.4]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.7.3...v0.7.4
[0.7.3]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.7.2...v0.7.3
[0.7.2]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.7.1...v0.7.2
[0.7.1]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.7.0...v0.7.1
[0.7.0]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.6.4...v0.7.0
[0.6.4]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.6.3...v0.6.4
[0.6.3]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.6.2...v0.6.3
[0.6.2]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.6.1...v0.6.2
[0.6.1]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.6.0...v0.6.1
[0.6.0]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.5.4...v0.6.0
[0.5.4]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.5.3...v0.5.4
[0.5.3]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.5.2...v0.5.3
[0.5.2]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.5.1...v0.5.2
[0.5.1]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.4.15...v0.5.1
[0.4.15]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.4.14...v0.4.15
[0.4.14]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.4.13...v0.4.14
[0.4.13]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.4.12...v0.4.13
[0.4.12]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.4.11...v0.4.12
[0.4.11]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.4.10...v0.4.11
[0.4.10]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.4.8...v0.4.10
[0.4.8]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.4.7...v0.4.8
[0.4.7]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.4.6...v0.4.7
[0.4.6]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.4.3...v0.4.6
[0.4.3]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.4.2...v0.4.3
[0.4.2]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.4.1...v0.4.2
[0.4.1]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.4.0...v0.4.1
[0.4.0]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.3.2...v0.4.0
[0.3.2]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.3.1...v0.3.2
[0.3.1]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.3.0...v0.3.1
[0.3.0]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.2.4...v0.3.0
[0.2.4]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.2.2...v0.2.4
[0.2.2]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.2.1...v0.2.2
[0.2.1]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.2.0...v0.2.1
[0.2.0]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.1.6...v0.2.0
[0.1.6]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.1.5...v0.1.6
[0.1.5]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.1.4...v0.1.5
[0.1.4]: https://github.com/LiteLDev/LegacyScriptEngine/releases/tag/v0.1.4
