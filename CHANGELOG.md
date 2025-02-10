# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.9.7] - 2025-02-10

### Added

- Added more block for onRedStoneUpdate [#212]

### Fixed

- Fixed debug command output
- Fixed Player::talkAs & talkTo
- Fixed onMobHurt [#236]
- Fixed onEffectRemoved not triggered when effects are removed manually

## [0.9.6] - 2025-02-07

### Added

- Added `ll.onUnload` [#227]
- Added onNpcCmd [#226]
- Added onEffectAdded/Updated/Removed
- Added onEntityTransformation

### Changed

- Make version related api show LSE version instead of LeviLamina version

### Fixed

- Fixed onPlayerInteractEntity [#231]
- Fixed onEntityExplode [#225]

## [0.9.5] - 2025-02-04

### Added

- Added some api and event [#220]

### Changed

- Updated translation
- Updated lightwebsocketclient [#221]

### Fixed

- Fixed onAte event [#222] (#223)

## [0.9.4] - 2025-02-02

### Changed

- Disabled LLSECallEventsOnHotUnload when server stops

### Fixed

- Fixed setTimeout can't be cancelled [#219]
- Fixed crash on unload

## [0.9.3] - 2025-01-30

### Fixed

- Fixed Player::clearItem [#216]

## [0.9.2] - 2025-01-29

### Fixed

- Removed redundant endl in ColorLog
- Add missing destroy engine when load failed
- Fixed Player/Entity::getBlockFromViewVector [#214]
- Fixed Player/Entity::isOnHotBlock
- Fixed Player::setAbility [#213]
- Fixed Logger::setLogLevel & ColorLog
- Fixed debug engine's logger
- Fixed npm executing [#204]

## [0.9.1] - 2025-01-26

### Fixed

- Fix issue of preRelease in registerPlugin

## [0.9.0] - 2025-01-25

### Added

- Implemented Logger::setTitle
- Added mysql support except for Node.js engine
- Add FormCancelReason for GuiAPI
- Implemented registerPlugin for old plugins

### Fixed

- Fix setAbility
- Fix runtime command warning
- Add missing stacktrace in catch
- Add missing LLSECallEventsOnHotUnload and fix unload

## [0.9.0-rc.5] - 2025-01-15

### Fixed

- Add missing condition for onContainerChange
- Fix onFarmLandDecay cancellation
- Fix processConsolePipCmd [#208]
- Fix NodeJs engine disabling

## [0.9.0-rc.4] - 2025-01-13

### Changed

- Find modules in python engine's directory

### Fixed

- Fix playerInfo not launch [#206]
- Fix plugin reload [#207]

## [0.9.0-rc.3] - 2025-01-12

### Changed

- Support LeviLamina 1.0.0-rc.3

## [0.9.0-rc.2] - 2025-01-10

### Fixed

- Fix command compatibility
- Fix System::cmd and System::newProcess [#203]
- Fix npm & pip command

## [0.9.0-rc.1] - 2025-01-09

### Changed

- Support LeviLamina 1.0.0-rc.2
- Replace QuickJs with QuickJs-Ng v0.8.0
- Upgrade Lua to v5.4.7
- Upgrade NodeJs to v22.12.0
- Upgrade Python to v3.12.8
- Refactor command api

## [0.8.20] - 2024-12-01

### Fixed

- Fix setGameMode
- Fix clearItem [#186]

## [0.8.19] - 2024-08-23

### Changed

- Remove NativeAPI completely

### Fixed

- Fix LLSECallEventsOnHotLoad's position
- Fix Entity::teleport with 2 argument

## [0.8.18] - 2024-08-12

### Fixed

- Fix onMobHurt [#157]

## [0.8.17] - 2024-08-10

### Fixed

- Add valid check for Entity

## [0.8.16] - 2024-08-10

### Fixed

- Fix EntityAPI completely [#157]

## [0.8.15] - 2024-08-09

### Changed

- Refactoring PlayerAPI & EntityAPI & DeviceAPI
- Refactoring ItemAPI

## [0.8.14] - 2024-08-08

### Fixed

- Fix NbtAPI constructor [#160]

## [0.8.13] - 2024-08-08

### Added

- Add uuid support for `mc.getPlayer()`
- Add new PlayerInfo API

### Fixed

- Fix simulateLookAt [#146]

## [0.8.12] - 2024-08-07

### Fixed

- Fix bugs related to Nbt ownership

## [0.8.11] - 2024-08-07

### Changed

- Refactoring DeviceAPI to prevent crash

### Fixed

- Add missing Logger::setPlayer

## [0.8.10] - 2024-08-06

### Fixed

- Fix Entity::toPlayer

## [0.8.9] - 2024-08-05

### Changed

- Refactoring PlayerAPI

### Fixed

- Fix onMobHurt crash
- Fix DefaultDataLoadHelper

## [0.8.8] - 2024-08-05

### Fixed

- Fix setNbt bug
- Refactor MoreGlobal
- Fix a critical issue in 0.8.7

## [0.8.7] - 2024-08-04

### Changed

- Adapt to LeviLamina 0.13.5
- Remove useless output
- Remove useless translation
- Refactor translation

## [0.8.6] - 2024-08-03

### Fixed

- Fix onMobHurt exception [#157]

## [0.8.5] - 2024-07-29

### Changed

- Downgrade cpp-httplib
- Refactoring some confusing code
- Remove useless package for python engine

### Fixed

- Fix exception message in NbtCompoundClass::constructor
- Fix onMobHurt [#154]
- Fix Entity::hurt [#153]

## [0.8.4] - 2024-07-24

### Changed

- Adapt to LeviLamina 0.13.4

## [0.8.3] - 2024-07-14

### Fixed

- Fix Entity::hurt [#152]
- Fix trident consumption for onSpawnProjectile [#143]

## [0.8.2] - 2024-07-12

### Fixed

- Fix onLiquidFlow's behavior
- Fix player attribute setter (#149)

## [0.8.1] - 2024-06-23

### Fixed

- Fix reduceExperience [#145]
- Fix ScoreboardAPI [#84]

## [0.8.0] - 2024-06-18

### Added

- Adapt to LeviLamina 0.13.x

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
[#84]: https://github.com/LiteLDev/LegacyScriptEngine/issues/84
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
[#143]: https://github.com/LiteLDev/LegacyScriptEngine/issues/143
[#145]: https://github.com/LiteLDev/LegacyScriptEngine/issues/145
[#146]: https://github.com/LiteLDev/LegacyScriptEngine/issues/146
[#152]: https://github.com/LiteLDev/LegacyScriptEngine/issues/152
[#153]: https://github.com/LiteLDev/LegacyScriptEngine/issues/153
[#154]: https://github.com/LiteLDev/LegacyScriptEngine/issues/154
[#157]: https://github.com/LiteLDev/LegacyScriptEngine/issues/157
[#160]: https://github.com/LiteLDev/LegacyScriptEngine/issues/160
[#186]: https://github.com/LiteLDev/LegacyScriptEngine/issues/186
[#203]: https://github.com/LiteLDev/LegacyScriptEngine/issues/203
[#204]: https://github.com/LiteLDev/LegacyScriptEngine/issues/204
[#206]: https://github.com/LiteLDev/LegacyScriptEngine/issues/206
[#207]: https://github.com/LiteLDev/LegacyScriptEngine/issues/207
[#208]: https://github.com/LiteLDev/LegacyScriptEngine/issues/208
[#212]: https://github.com/LiteLDev/LegacyScriptEngine/issues/212
[#213]: https://github.com/LiteLDev/LegacyScriptEngine/issues/213
[#214]: https://github.com/LiteLDev/LegacyScriptEngine/issues/214
[#216]: https://github.com/LiteLDev/LegacyScriptEngine/issues/216
[#219]: https://github.com/LiteLDev/LegacyScriptEngine/issues/219
[#220]: https://github.com/LiteLDev/LegacyScriptEngine/issues/220
[#221]: https://github.com/LiteLDev/LegacyScriptEngine/issues/221
[#222]: https://github.com/LiteLDev/LegacyScriptEngine/issues/222
[#225]: https://github.com/LiteLDev/LegacyScriptEngine/issues/225
[#226]: https://github.com/LiteLDev/LegacyScriptEngine/issues/226
[#227]: https://github.com/LiteLDev/LegacyScriptEngine/issues/227
[#231]: https://github.com/LiteLDev/LegacyScriptEngine/issues/231
[#236]: https://github.com/LiteLDev/LegacyScriptEngine/issues/236

[Unreleased]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.9.7...HEAD
[0.9.7]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.9.6...v0.9.7
[0.9.6]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.9.5...v0.9.6
[0.9.5]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.9.4...v0.9.5
[0.9.4]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.9.3...v0.9.4
[0.9.3]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.9.2...v0.9.3
[0.9.2]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.9.1...v0.9.2
[0.9.1]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.9.0...v0.9.1
[0.9.0]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.9.0-rc.5...v0.9.0
[0.9.0-rc.5]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.9.0-rc.4...v0.9.0-rc.5
[0.9.0-rc.4]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.9.0-rc.3...v0.9.0-rc.4
[0.9.0-rc.3]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.9.0-rc.2...v0.9.0-rc.3
[0.9.0-rc.2]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.9.0-rc.1...v0.9.0-rc.2
[0.9.0-rc.1]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.20...v0.9.0-rc.1
[0.8.20]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.19...v0.8.20
[0.8.19]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.18...v0.8.19
[0.8.18]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.17...v0.8.18
[0.8.17]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.16...v0.8.17
[0.8.16]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.15...v0.8.16
[0.8.15]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.14...v0.8.15
[0.8.14]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.13...v0.8.14
[0.8.13]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.12...v0.8.13
[0.8.12]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.11...v0.8.12
[0.8.11]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.10...v0.8.11
[0.8.10]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.9...v0.8.10
[0.8.9]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.8...v0.8.9
[0.8.8]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.7...v0.8.8
[0.8.7]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.6...v0.8.7
[0.8.6]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.5...v0.8.6
[0.8.5]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.4...v0.8.5
[0.8.4]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.3...v0.8.4
[0.8.3]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.2...v0.8.3
[0.8.2]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.1...v0.8.2
[0.8.1]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.8.0...v0.8.1
[0.8.0]: https://github.com/LiteLDev/LegacyScriptEngine/compare/v0.7.12...v0.8.0
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
