# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
[#37]: https://github.com/LiteLDev/LegacyScriptEngine/issues/37
[#43]: https://github.com/LiteLDev/LegacyScriptEngine/issues/43
[#44]: https://github.com/LiteLDev/LegacyScriptEngine/issues/44
[#45]: https://github.com/LiteLDev/LegacyScriptEngine/issues/45
[#47]: https://github.com/LiteLDev/LegacyScriptEngine/issues/47
[#49]: https://github.com/LiteLDev/LegacyScriptEngine/issues/49
[#52]: https://github.com/LiteLDev/LegacyScriptEngine/issues/52
[#53]: https://github.com/LiteLDev/LegacyScriptEngine/issues/53
[#54]: https://github.com/LiteLDev/LegacyScriptEngine/issues/54

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
