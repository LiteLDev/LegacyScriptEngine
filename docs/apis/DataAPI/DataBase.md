# 📦 Database API

Database, generally used for plugins to persistently store data generated and processed by certain plugins.  
Unlike configuration files, databases generally have no readability requirements, but have considerable considerations for performance and stability.  
LLSE provides a consolidated database interface to accomplish this task.    
In terms of specific implementation, the engine provides two different database formats: NoSQL database in key-value pair format, and SQL database in tabular format. You can use either as needed.



### 🔑 Key-Value NoSQL Database 

Key-value databases are suitable for storing data in key-value form, such as `name:apple`, `value:5` and many more. 
This is accomplished with `leveldb`.

#### Create/Open a Key-Value Database

Before using the database, you need to give a database path, the interface will open/create the specified database and return a database object.  
A leveldb database is composed of multiple files, so you need to pass in the path of a folder where the database files will be stored.  
If this directory already contains a database, it will be opened, otherwise a new one will be created.

[JavaScript] `new KVDatabase(dir)`  
[Lua] `KVDatabase(dir)`

- Parameters: 
  - dir : `String`  
    The storage directory path of the database, based on the BDS root directory.
- Return value: Open/created database objects
- Return value type: `KVDatabase`
  - If the return value is `Null`, it means the creation/opening failed 

When the given directory does not exist, it will try to automatically create the corresponding directory path layer by layer.

After successfully opening the database, you can use the following interfaces to perform related operations.  
For a database object `db`, with the following functions:



#### Write Data Item

`db.set(name,data)`

- Parameters: 
  - name : `String`  
    Data item name
  - data : `Any type`  
    Data to write. The allowed data types are:    
    `Integer` `Float` `String` `Boolean` `Array` `Object `  
    The above elements can only be nested inside an `Array` or an `Object`.
- Return value: Whether the write is successful.
- Return value type: `Boolean`



#### Read Data Item

`db.get(name)`

- Parameters: 
  - name : `String`  
    Data item name
- Return value: The data of this item stored in the database.
- Return value type: `Any type`, depending on the specific type of data stored.
  - If the return value is `Null` it means that the data does not exist.



#### Delete Data Item

`db.delete(name)`

- Parameters: 
  - name : `String`  
    Data item name
- Return value: Whether the deletion was successfu.
- Return value type: `Boolean`



#### Get All Data Item Names 

`db.listKey()`

- Return value: An array of all data item names.
- Return value type: `Array`



#### Close the Database

`db.close()`

- Return value: Whether the shutdown was successful
- Return value type: `Boolean`

After the database is closed, do not continue to use it!

------

## 📋 SQL Database

SQL databases are suitable for handling large amounts of relational data using SQL statements. The underlying interface is implemented using a cross-database framework, supporting most common SQL databases on the market.

**Note:** Unless otherwise specified, the following APIs may throw exceptions. It is recommended to use exception handling blocks. For **JavaScript**, use `try ... catch` blocks; for **Lua**, use `pcall`. Generally, if no error is thrown, the call is successful.

> If you are a JavaScript plugin developer, you can also try using the [Yoyo](https://www.google.com/search?q=https://gitee.com/Y_oyo) LLDB Chained Operation Library (primarily for beginners who are unfamiliar with SQL syntax). Details: [Click here](https://gitee.com/Y_oyo/yoyo-mcbe-lite-xloader-item/blob/master/sql/yoyoSqlite.js%202.0.0.md)

### Open a SQL Database Session

To achieve cross-database compatibility, connecting to a database requires passing an object containing connection parameters or a connection string.

**[JavaScript]** `new DBSession(type, params)`

**[Lua]** `DBSession(type, params)`

* **Parameters:**
* `type`: `String`
  The database type, supports `"sqlite3"` and `"mysql"`.
* `params`: `Object`
  [Connection Parameters](https://www.google.com/search?q=%23connection-parameters)


* **Returns:** The opened database session object.
* **Return Type:** `DBSession`
* Returns `Null` if the connection fails.



**[JavaScript]** `new DBSession(str)`

**[Lua]** `DBSession(str)`

* **Parameters:**
* `str`: `String`
  Connection string in formats like `file:///mydb.db?k=v` or `mysql://root:password@localhost:3306/db`.


* **Returns:** The opened database session object.
* **Return Type:** `DBSession`
* Returns `Null` if the connection fails.



#### Connection Parameters

| Key | Purpose | Supported DB | Example | Default |
| --- | --- | --- | --- | --- |
| `path` | Path to the database file | `SQLite` | `plugins/test.db` | - |
| `create` | Auto-create if file doesn't exist | `SQLite` | `true`/`false` | `true` |
| `readonly` | Open in read-only mode | `SQLite` | `true`/`false` | `false` |
| `readwrite` | Open in read-write mode | `SQLite` | `true`/`false` | `true` |

---

### Execute SQL and Get Result Set

`session.query(sql)`

* **Parameters:**
* `sql`: `String`
  The SQL statement to query.


* **Returns:** Query results (result set).
* **Return Type:** `Array<Array>`
  The first row (`result[0]`) contains the table headers (column names); subsequent rows contain the data.

**Example Result:**
If the query returns:
| a | b |
| :--- | :--- |
| ll | 233 |
| h | 114 |

The `query` method returns:

```json
[
  ["a",  "b"],
  ["ll", 233],
  ["h",  114]
]

```

---

### Execute SQL Without Returning Results

`session.exec(sql)`

`session.execute(sql)`

* **Parameters:**
* `sql`: `String`
  The SQL statement to execute.


* **Returns:** The processed session object (for chaining).
* **Return Type:** `DBSession`

---

### Check Session Status

`session.isOpen()`

* **Returns:** Whether the session is currently open.
* **Return Type:** `Boolean`

---

### Close Database Session

`session.close()`

* **Returns:** Whether the close operation was successful.
* **Return Type:** `Boolean`

---

### SQL Prepared Statements

> **Prepared Statements** are a crucial part of SQL. They work by sending a SQL statement with placeholders to the server to be compiled first, then binding parameters to it before execution. Different SQL engines may have different syntax for placeholders; please refer to the official documentation for the specific database you are using.
> The main purpose of prepared statements is to prevent **SQL Injection**—a common and dangerous attack. Directly using unvalidated user input (like BDS does xD) can lead to password bypass or data loss (e.g., executing `DROP TABLE`). It is highly recommended to use prepared statements for user input. Additionally, they improve performance by compiling the statement once for multiple executions with different values.

#### Prepare a Statement

`session.prepare(sql)`

* **Parameters:**
* `sql`: `String`
  The SQL statement to prepare.


* **Returns:** A prepared statement object; throws an error on failure.
* **Return Type:** `DBStmt`

##### Placeholder Syntax Examples

**SQLite:**

```sql
-- Single '?' for positional parameters
SELECT * FROM table WHERE id = ?;
-- '?X' and '?Y' (where X/Y are names) for named binding
INSERT INTO table VALUES (?X, ?Y);
-- '$X', ':Z', and '@V' are also supported named parameters
INSERT INTO table VALUES ($X, ?Y, :Z, @V);

```

**MySQL:**

```sql
-- Single '?' for positional parameters
SELECT * FROM table WHERE id = ?;
-- Native MySQL does not support named parameters; LLDB implements simple parsing for compatibility, but use with caution.
INSERT INTO table VALUES (?X, ?Y);
INSERT INTO table VALUES ($X, ?Y, :Z);

```

#### Prepared Statement - Properties

| Property | Description | Type | See Also |
| --- | --- | --- | --- |
| `stmt.affectedRows` | Rows affected by the last execution (INSERT, UPDATE, DELETE, etc.) | `Integer` | [SQLite](https://www.sqlite.org/c3ref/changes.html) / [MySQL](https://dev.mysql.com/doc/c-api/8.0/en/mysql-affected-rows.html) |
| `stmt.insertId` | The last inserted row ID (see official docs for row ID behavior) | `Integer` | [SQLite](https://www.sqlite.org/c3ref/last_insert_rowid.html) / [MySQL](https://dev.mysql.com/doc/c-api/8.0/en/mysql-stmt-insert-id.html) |

*These properties are read-only and only available after the statement is executed.*

---

#### Bind Parameters to a Statement

`stmt.bind(val)`

Binds the value to the first unbound parameter.

`stmt.bind(obj)`

Binds an object; equivalent to iterating through the object and calling `bind(val, key)`.

`stmt.bind(arr)`

Binds an array; equivalent to iterating through the array and calling `bind(val)`.

`stmt.bind(val, index)`

Binds the value to a specific index (starting from `0`).

`stmt.bind(val, name)`

Binds the value to a named parameter.

* **Returns:** The statement object (for chaining).
* **Return Type:** `DBStmt`

*After binding, you must call `stmt.execute()` to run the query.*

##### Binding Example:

```js
let stmt = session.prepare("INSERT INTO table VALUES ($a, $b, $c, $d, $e, $f, $g, $h)");
let values = {
  c: "have you",
  d: "finished",
  e: "your",
  f: "homework?"
};
stmt.bind(values); // Binds c, d, e, f
stmt.bind("LLSE"); // Binds to a
stmt.bind(["****", "mojang"]); // Binds to b and g
stmt.bind(114514, 7);  // Binds to h (index 7)

```

---

#### Execute the Current Statement

`stmt.execute()`

* **Returns:** The statement object (for chaining).
* **Return Type:** `DBStmt`

#### Step to Next Result Row

`stmt.step()` or `stmt.next()`

* **Returns:** Whether the step was successful (true if a row exists).
* **Return Type:** `Boolean`
* **Note:** The cursor is positioned at the **first row** immediately after execution. Use a `do...while` loop to iterate, otherwise the first row will be skipped.

#### Fetch Current Row

`stmt.fetch()`

* **Returns:** The current result row as an object, e.g., `{col1: "value", col2: 2333}`.
* **Return Type:** `Object`

#### Fetch All Result Rows

`stmt.fetchAll()`

* **Returns:** The full result set. (See [Execute SQL and Get Result Set](https://www.google.com/search?q=%23execute-sql-and-get-result-set)).
* **Return Type:** `Array<Array>`

`stmt.fetchAll(callback)`

* **Parameters:**
* `callback`: `Function<bool(Object)>`
  Function to iterate through rows. Returning `false` inside the callback stops the iteration.


* **Returns:** The statement object.
* **Return Type:** `DBStmt`

---

#### Reset Statement State

`stmt.reset()`

* **Returns:** The statement object.
* **Note:** This resets the statement to a "pending" state but **does not** clear bound parameters.

#### Re-execute Statement

`stmt.reexec()`

* **Note:** A convenience function equivalent to calling `stmt.reset()` and then `stmt.execute()`.

#### Clear Bound Parameters

`stmt.clear()`

* **Returns:** The statement object.

---

### Code Example

```js
let dat = {};
let modified = {};
let session = null;

function initdb() {
  if (!file.exists("plugins/MyPlugin")) file.mkdir("plugins/MyPlugin");
  session = new DBSession("sqlite", {path: "./plugins/MyPlugin/dat.db"});
  session.exec(`
    CREATE TABLE IF NOT EXISTS "test" (
      player  CHAR(100) NOT NULL,
      coins   INTEGER   NOT NULL
    );`); 
    // SQLite automatically adds a hidden ROWID column. 
    // insertId refers to this ROWID value.
}

// Using do...while loop
function loadData() {
  let stmt = session.prepare("SELECT * FROM test");
  // After prepare/execute, the cursor is already on the first row
  do { 
    let row = stmt.fetch();
    dat[row.player] = row.coins;
  } while (stmt.step()); 
}

// Using callback function
function loadData2() {
  session.prepare("SELECT * FROM test")
    .execute()
    .fetchAll((row) => {
      dat[row.player] = row.coins;
    });
}

function writeData() {
  let keys = Object.keys(modified);
  let stmt = session.prepare("UPDATE test SET coins = ? WHERE player = ?");
  for (let i = 0; i < keys.length; i++) {
    let v = modified[keys[i]];
    // Automatically executes after binding if logic requires
    stmt.bind([v, keys[i]]).execute(); 
    stmt.clear(); // Clear values for next iteration
  }
}

mc.regPlayerCmd("getcoin", "Get a coin!", (pl, args)=>{
  dat[pl.realName]++;
  modified[pl.realName]++;
});
```