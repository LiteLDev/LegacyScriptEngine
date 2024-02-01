## 🏃‍♂️ Player Binding Data

In actual development, there is often a need to associate certain data with a player in the server, and to maintain these data continuously during the work cycle of the plugin.  

To this end, LLSE designed the player binding data interface. The bound data interface stores data in the form of key-value pairs. 
After you bind data to a player, the player bound data will persist even if the player object goes out of scope and is destroyed, and even when the player exits the game. When you get the player's player object again, you can still read the previously stored binding data.  
All data will be destroyed uniformly only when the server is shut down.

As such, LLSE gives developers the ability to track data about a particular player throughout the plugin's lifecycle.  

<br>

For a specific player object `pl`, with the following interfaces:

#### Store Player Binding Data 

`pl.setExtraData(name,data)`

- Parameters: 
  - name : `String`  
    The name to store into the bound data.
  - data : `Any type`  
    The binding data you want to store, which can be `Null`

- Return value: Whether the save was successful or not.
- Return value type: `Boolean` 

<br>

#### Get Player Binding Data

`pl.getExtraData(name)`

- Parameters: 
  - name : `String`  
    The name of the bound data to read.
- Return value: Stored binding data
- Return value type: `Any type`, depending on the type of data stored.
  -  If the return value is `Null`, it means that the specified binding data is not obtained, or the data is empty.

<br>

#### Delete Player Binding Data

`pl.delExtraData(name)`

- Parameters: 
  - name : `String`  
    The name of the bound data to delete.
- Return value: Whether the deletion is successful.
- Return value type: `Boolean`

<br>

## 👨‍💻 XUID Database

The XUID database allows you to query the correspondence between player names and XUIDs even when players are offline.
When a player enters the server for the first time, his name and XUID are automatically recorded in the built-in XUID database. Use the following functions to make related queries.

#### Query XUID by Player Name

`data.name2xuid(name)`

- Parameters: 
  - name : `String`  
    The name of the player to query.
- Return value: Player's XUID.
- Return value type: `String`
  - If the return value is `Null`, it means the query failed.

<br>

#### Query Player Name Based on XUID

`data.xuid2name(xuid)`

- Parameters: 
  - xuid: `String`  
    Player's XUID to query.
- Return value: Player's name.
- Return value type: `String`
  - If the return value is `Null`, it means the query failed.

<br>

#### Query UUID by Player Name

`data.name2uuid(name)`

- Parameters: 
  - name : `String`  
    The name of the player to query.
- Return value: Player's UUID.
- Return value type: `String`
  - If the return value is `Null`, it means the query failed.

<br>

#### Query UUID Based on XUID

`data.xuid2uuid(xuid)`

- Parameters: 
  - xuid : `String`  
    The xuid of the player to query.
- Return value: Player's UUID.
- Return value type: `String`
  - If the return value is `Null`, it means the query failed.

<br>

#### Get all player information

`data.getAllPlayerInfo()`

- Return value: All player information.
- Return value type: `Array<Object>`
  - Each object contains the following properties:
    - `name`: Player's name.
    - `xuid`: Player's XUID.
    - `uuid`: Player's UUID.

<br>

Tip: The player name stored in the XUID database is named corresponding to the player object. `realName` field.
