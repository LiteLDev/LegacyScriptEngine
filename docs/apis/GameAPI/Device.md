#  📱 Device Information Object API

In LSE, the "device information object" is used to manipulate and obtain information about the game device used by a player.

### Get a Device Info Object

#### Acquired by the Player

Get the **device information object** corresponding to a player through the `.getDevice` member function of the player object
See [Player Object API](Player.md)      

> Note: Do not save a device information object **long-term.**
> When the player corresponding to the device exits the game, the corresponding object will become invalid. Therefore, if there is a need to operate an object for a long time, please obtain the real-time device information object through the above methods.




### Device Information Object - Properties

Each device information object contains some fixed object properties. for a specific entity object `dv`, has the following properties:

| Attributes        | Meaning                                       | Data Types |
| ----------------- | --------------------------------------------- | ---------- |
| dv.ip             | The IP address of the player's device         | `String`   |
| dv.avgPing        | Average network latency for players (ms)      | `Integer`  |
| dv.avgPacketLoss  | Player's average network packet loss rate (%) | `Float`    |
| dv.lastPing       | Network latency for players (ms)              | `Integer`  |
| dv.lastPacketLoss | Player's network packet loss rate (%)         | `Float`    |
| dv.os             | The OS type of the player's device            | `String`   |
| dv.inputMode      | Player's input mode                           | `Integer`  |
| dv.serverAddress  | The player's connection address               | `String`   |
| dv.clientId       | ID of the player client                       | `String`   |

These object properties are read-only and cannot be modified 

Among them, the operating system type attribute returns a string that records the operating system of the player's device. The possible return values ​​are as follows:

| dv.os                    | Device Operating System                       |
| ------------------------ | --------------------------------------------- |
| `Google`                 | Google Android (Mobile phone)                 |
| `IOS`                    | Apple iOS (Mobile phone) or iPadOS            |
| `Osx`                    | Apple macOS (Computer)                        |
| `Amazon`                 | Amazon FireOS                                 |
| `GearVRDeprecated`       | Samsung GearVR (deprecated)                   |
| `UwpDeprecated`          | Microsoft Windows, UWP (Computer, deprecated) |
| `Win32`                  | Microsoft Windows (Computer)                  |
| `Dedicated`              | Dedicated server                              |
| `TvOSDeprecated`         | Apple tvOS (deprecated)                       |
| `Sony`                   | Sony PlayStation Host                         |
| `Nintendo`               | Nintendo Switch Host                          |
| `Xbox`                   | Microsoft Xbox Host                           |
| `WindowsPhoneDeprecated` | Windows Mobile (deprecated)                   |
| `Linux`                  | Linux                                         |
| `Unknown`                | Unknown System                                |

| Input mode ENUM              |
| ---------------------------- |
| `InputMode.Mouse`            |
| `InputMode.Touch`            |
| `InputMode.GamePad`          |
| `InputMode.MotionController` |
