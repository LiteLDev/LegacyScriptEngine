## 💰 Economic System API

In many servers, the economic system is a very critical part.  
In order to solve various problems of traditional use of the scoreboard economic system, LLSE provides an interface to connect to the LLMoney economic system, which can communicate with other series of plugins. 

In addition to the capabilities of the traditional economic system, LLMoney also has additional capabilities such as querying the history of changes in the amount and operating the economy of offline players.  
LiteLoader is installed with the LLMoney plugin, so you can use this interface directly without additional installation. 

### Set the Player’s Deposit Amount

`Player.setMoney(value)`

`money.set(xuid,value)`

- Parameters: 
  - xuid : `String`  
    The XUID identifier of the player.
  - value : `Integer`  
    Amount of money being set.  
- Return value: Whether the setting is successful.
- Return value type: `Boolean`

<br>

### Get the Player’s Deposit Amount

`Player.getMoney()`

`money.get(xuid)`

- Parameter: 
  - xuid : `String`  
    The XUID identifier of the player to read.
- Return value: Player's bank value.
- Return value type: `Integer`

<br>

### Increase Player’s Deposit

`Player.addMoney(value)`

`money.add(xuid,value)`

- Parameters: 
  - xuid : `String`  
    The XUID identifier of the player.
  - value : `Integer`  
    The amount of money to add to the player's bank.  
- Return value: Whether the setting is successful.
- Return value type: `Boolean`

<br>

### Decrease the Player’s Deposit

`Player.reduceMoney(value)`

`money.reduce(xuid,money)`

- Parameters: 
  - xuid : `String`  
    The XUID identifier of the player.
  - money : `Integer`  
    The amount of money to take from the player.  
- Return value: Whether the setting is successful.
- Return value type: `Boolean`

<br>

### Make a Transfer

`Player.transMoney(target,money[,note])`

`money.trans(xuid1,xuid2,money[,note])`

- Parameters: 
  - xuid1 : `String`  
    The XUID identifier of the paying player.
    
  - xuid2 : `String`  
    The XUID identifier of the player who will receive the payment.
    
    If you are using `Player.transMoney`, target can be `Player`.
    
  - money : `Integer`  
    The amount of money being transferred.  
  
  - note : `String`  
    (Optional) Add some text to this transfer.
  
- Return value: Whether the transfer is successful.

- Return value type: `Boolean`

<br>

### Query Historical Payments

`Player.getMoneyHistory(time)`

`money.getHistory(xuid,time)`

- Parameters: 
  - xuid : `String`  
    The XUID identifier of the player.
  - time : `Integer`  
    Query all records within the last `time` seconds.
- Return value: An array of query result objects.
- Return value type: `Array<Object>`

Where the result is an array of record objects. for each `record` object record, with the following keys and corresponding values:

| Key             | Meaning of Value         | Data Type  |
| -------------- | -------------------------- | --------- |
| `record.from`  | XUID of money sender   | `String`  |
| `record.to`    | XUID of money receiver   | `String`  |
| `record.money` | Amount of money             | `Integer` |
| `record.time`  | Time when this transaction occurred | `String`  |
| `record.note`  | Additional notes for this transaction.       | `String`  |

The format of the time string is: YYYY-mm-dd hh:mm:ss

<br>

### Delete Billing History

`money.clearHistory(time)`

- Parameter: 
  - time : `Integer`  
    Delete all records within the last `time` seconds.
- Return value: Whether the deletion is successful.
- Return value type: `Boolean`

<br>