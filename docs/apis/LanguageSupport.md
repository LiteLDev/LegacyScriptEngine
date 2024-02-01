# 📋 LLSE - Multi development Language Support

## 🌏 Current Status

With the support of the [ScriptX](https://github.com/Tencent/ScriptX) project, LLSE adapts to multiple development languages using the same set of source code.    
At the same time, the API remains consistent, allowing various languages to share the same development document. It greatly reducing maintenance difficulties.

Currently, LLSE supports writing plugins in the following languages：

| Language backend | Remarks                                                      |
| ---------------- | ------------------------------------------------------------ |
| `JavaScript`     | Using QuickJS engine, with support for ES Modules            |
| `Lua`            | Using CLua engine                                            |
| `Node.js`         | Modify Node.js to work with embedding, with support for npm package management |
| `Python`  | Using CPython engine, with support for pip package management |

> [!INFO]
>
> If you need to write plugins in compiled languages such as C++, Go, .NET, etc., please go to [Home](../zh-Hans) for other language documentation

## JavaScript language support description

- Support for simple JavaScript plugins using the QuickJS engine, a lightweight engine with a low resource footprint
- The current version of QuickJS supports up to ES2020, and natively supports ES Modules which allows developers to easily manage projects.
- Package management is not supported. If needed, you can use Node.js for plugin development and use npm for package management
- Use `jsdebug` command in the BDS console to enter and exit the QuickJs interactive command line environment. This feature facilitates some simple testing when writing plugins

<br>

## Lua language support description

- Use the CLua engine, support require
- Since the Rocks package management mechanism requires the introduction of a compiler, the implementation is not available at this time. If you need to depend on extensions, you can compile them manually and introduce them into your project (e.g. SQLite)
- Use `luadebug` command in the BDS console to enter and exit the Lua interactive command line environment. This feature facilitates some simple testing when writing plugins

<br>

## Node.js support description

- LLSE makes it possible to work in embedded mode by implementing the Node.js starter code itself, and isolates the execution environment for different plugins
- Created interface to implement programmic support for npm. Support installing third-party extension dependencies via package.json

#### ⭐ **Node.js Plugin Development**

1. First, install NodeJs
2. Create a new directory for plugin development. Launch the terminal in this directory and execute `npm init`. Follow npm to create a new project and fill in the information about the project.
3. Write the plugin code in the filled entry point file
4. If necessary, create multiple source files and bring them in via require. Properly splitting the source code into files can help improve the clarity of the project structure and facilitate further maintenance and expansion of the plugin.

#### ⭐ **Node.js Plugin Packaging & Deployment**

- After the plugin is finish, package `package.json` and all the plugin source code into a zip archive and **change the file name suffix to .llplugin**
- The `node_modules` directory should not be packed in the archive
- Distribute the **.llplugin** file as a plugin. When installing the plugin, just place this file directly into the plugins directory
- LLSE will automatically recognize the **.llplugin** file when BDS launch, extract it to the `plugins/nodejs/<PluginName>` directory, and automatically execute `npm install` in the directory to install the dependency packages. No manual intervention is needed for the whole process

<br>

## Python language support description

- Use CPython engine, Python version 3.10.9. Support for installing third-party extension dependencies for plugins using pip package management. Support for multi-file plugin development and import. Support for modern project management.
- Use `pydebug` command in the BDS console to enter and exit the Python interactive command line environment. This feature facilitates some simple testing when writing plugins

#### **Python Single-File Plugin Development **

- To help developers unfamiliar with Python to get started quickly, we provide a way to write and load single-file Python plugin.
- Single file plugins are similar to QuickJs and Lua plugins: Just write a single .py file as a plugin, place the plugin directly into the plugins directory, and it will be loaded and run when BDS is started.
- It has many disadvantages: does not support plugin metadata storage, does not support source code in multiple files, does not support pip third-party packages. Only for developers familiar with LLSE's Python environment, or to develop very simple plugins to use.

#### ⭐ **Python Multi-File Plugin Development**

- For formal Python plugins, this method is highly recommended. Multi-file plugins support all full Python features.
- LLSE uses the `pyproject.toml` project file for metadata storage (similar to `package.json` in Node.Js). This project file is recommended to be automatically generated using PDM package manager ([pdm-project/pdm](https://github.com/pdm-project/pdm)), which supports modern project features to facilitate plugin project creation and maintenance.

##### **The turtorial for Python plugins development: **
1. First, install Python 3.10.9
2. Run `pip install --user pdm` in terminal to install the pdm package management tool
3. Create a new directory for plugin development. Launch the terminal in this directory and execute `pdm init`. Follow the prompts of the pdm tool to create a new project and fill in the information about the project.
   - If you need to install dependency packages, execute `pdm add <dependency package name>` in the project directory
   - All project metadata and dependency data will be automatically stored in `pyproject.toml`. No need to write it manually. You can also open this file to change metadata information such as version number, description, etc.
   - In addition to using the `pdm add` command, you can also manually write the project dependencies directly to `requirements.txt`. When installing the plugin, the dependencies described in `pyproject.toml` and `requirements.txt` will be processed and installed automatically.

3. Next, create the `__init__.py` file and write the plugin code in it. When loading the plugin, the Python interpreter will read and execute this file.
4. If necessary, you can create multiple source files and use import to include them. Properly splitting the source code into files helps improve the clarity of the project structure and facilitates further maintenance and extension of the plugin.

#### ⭐ **Python Plugin Packaging & Deployment**

- After the plugin is finished, package `pyproject.toml` and all plugin source code into a zip archive and **change the filename suffix to .llplugin**
- Do not include the `__pycache__` and `__pypackages__` directories in the zip archive
- Distribute the **.llplugin** file as a plugin. When installing the plugin, place it directly into the `plugins` directory.
- The scripting engine will automatically recognize the **.llplugin** file and unzip it to the `plugins/python/plugin name` directory, and automatically execute `pip install` to install the dependencies. No manual intervention is needed for the whole process.

#### **Known issues with Python plugin development**

To be fair, the quality and maintenance of CPython's code is in a somewhat worrisome state. Given below are some problems to be aware of when developing Python plugins, many of which are caused by bugs in CPython itself:

1. Do not use `threading`, `asyncio` and other features for the time being
   - LLSE uses CPython's sub-interpreter as the core unit of engine scheduling, but CPython itself has long had bad support for the sub-interpreter, and there are many bugs that are hard to explain. The GIL api that is currently used for these mechanisms does not take into account the existence of sub-interpreters, so deadlocks and crashes will occur once they are used.
   - Developers with parallel computing needs can temporarily use `multiprocess` to parallelize multiple processes.
   - Python 3.12 is scheduled to have targeted fixes for sub-interpreter and GIL-related bugs, and LLSE will adapt CPython 3.12 to address this issue after its release.
2. `sys.stdin` is disabled for all Python engines
   - This is another CPython bug, see https://github.com/python/cpython/issues/83526 for details
   - In addition, even if the CPython engine is not loaded in the above case, there will be a problem of stdin grabbing, causing some tools that use the input redirection to fail
   - Therefore, we hit a patch to disable `sys.stdin` for all Python engines. We will not restore it until future versions of CPython to completely solve this series of bugs.
