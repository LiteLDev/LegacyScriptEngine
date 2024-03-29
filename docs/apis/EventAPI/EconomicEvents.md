# 💰 Economic System Events

Here are the economic change events related to the LLSE built-in economic system:

#### `"beforeMoneyAdd"` - Pre-Money Adding Event

- Listener function prototype 
  `function(xuid,money)`
- Parameters: 
  - xuid : `String`  
    The XUID of the player whose amount of money changed.
  - money : `Integer`  
    The amount of money being given.
- Intercept events: Function returns `false`



#### `"onMoneyAdd"` - Money Adding Event

- Listener function prototype 
  `function(xuid,money)`
- Parameters: 
  - xuid : `String`  
    The XUID of the player whose amount of money changed.
  - money : `Integer`  
    The amount of money being given.



#### `"beforeMoneyReduce"` - Pre-Money Reduction Event

- Listener function prototype 
  `function(xuid,money)`
- Parameters: 
  - xuid : `String`  
    The XUID of the player whose amount of money changed.
  - money : `Integer`  
    The amount of money being taken.
- Intercept events: Function returns `false`



#### `"onMoneyReduce"` - Money Reduction Event

- Listener function prototype 
  `function(xuid,money)`
- Parameters: 
  - xuid : `String`  
    The XUID of the player whose amount of money changed.
  - money : `Integer`  
    The amount of money being taken.



#### `"beforeMoneyTrans"` - Pre-Player Money Transfer Event

- Listener function prototype 
  `function(from,to,money)`
- Parameters: 
  - from : `String`  
    The XUID of the player initating the transfer.
  - to : `String`  
    The XUID of the player accepting the transfer.
  - money : `Integer`  
    The amount of money being transferred.
- Intercept events: Function returns `false`



#### `"onMoneyTrans"` - Player Money Transfer Event

- Listener function prototype 
  `function(from,to,money)`
- Parameters: 
  - from : `String`  
    The XUID of the player initating the transfer.
  - to : `String`  
    The XUID of the player accepting the transfer.
  - money : `Integer`  
    The amount of money being transferred.

**Notice: When `onMoneyReduce` or `onMoneyAdd` was triggered, this event will be also triggered**


#### `"beforeMoneySet"` - Pre-Player Money Setting Event

- Listener function prototype 
  `function(xuid,money)`
- Parameters: 
  - xuid : `String`  
    The XUID of the player whose amount of money changed.
  - money : `Integer`  
    The amount of money being set.
- Intercept events: Function returns `false`



#### `"onMoneySet"` - Player Money Setting Event

- Listener function prototype 
  `function(xuid,money)`
- Parameters: 
  - xuid : `String`  
    The XUID of the player whose amount of money changed.
  - money : `Integer`  
    The amount of money being set.


