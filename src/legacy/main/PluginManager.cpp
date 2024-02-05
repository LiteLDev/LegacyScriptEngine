#include "main/PluginManager.h"

#include "api/APIHelp.h"
#include "api/CommandAPI.h"
#include "api/EventAPI.h"
#include "engine/EngineManager.h"
#include "engine/LocalShareData.h"
#include "engine/RemoteCall.h"
#include "engine/TimeTaskSystem.h"
#include "legacyapi/utils/STLHelper.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/schedule/Task.h"
#include "ll/api/utils/CryptoUtils.h"
#include "ll/api/utils/StringUtils.h"
#include "main/Configs.h"
#include "main/Global.h"
#include "main/Loader.h"

#include <filesystem>
#include <utility>

#ifdef LLSE_BACKEND_NODEJS
#pragma warning(disable : 4251)
#include "main/NodeJsHelper.h"
#elif defined(LLSE_BACKEND_PYTHON)
#include "main/PythonHelper.h"
#endif

#define H(x) do_hash(x)

extern void BindAPIs(ScriptEngine* engine);

// Helper
string RemoveRealAllExtension(string fileName) {
    int pos = fileName.find(".");
    if (pos == string::npos) return fileName;
    else return fileName.substr(0, pos);
}

// Load plugin
// - This function must be called in correct backend
// - "filePath" can be a single-file plugin path, or a .llplugin compressed
// package path or a dir path which contains uncompressed plugin package
// * if mustBeCurrectModule == true and not-current-module plugin is found,
// will throw exception
using ll::string_utils::str2wstr;

bool PluginManager::loadPlugin(const std::string& fileOrDirPath, bool isHotLoad, bool mustBeCurrentModule) {
    if (fileOrDirPath == LLSE_DEBUG_ENGINE_NAME) return true;

    if (!std::filesystem::exists(ll::string_utils::str2wstr(fileOrDirPath))) {
        lse::getSelfPluginInstance().getLogger().error("Plugin not found! Check the path you input again.");
        return false;
    }

    // Get bacis information
    bool   isPluginPackage = std::filesystem::is_directory(fileOrDirPath);
    string backendType     = getPluginBackendType(fileOrDirPath);
    if (backendType.empty()) {
        lse::getSelfPluginInstance().getLogger().error(fileOrDirPath + " is not a valid plugin path!");
        return false;
    }
    std::filesystem::path p(ll::string_utils::str2wstr(fileOrDirPath));
    string pluginFileName = RemoveRealAllExtension(ll::string_utils::u8str2str(p.filename().stem().u8string()));

    // Uncompress plugin package if needed
    string realPath = fileOrDirPath;
    if (backendType == "PluginPackage") {
        // Get plugin package
        // Clean temp dir first
        std::error_code ec;
        std::filesystem::remove_all(LLSE_PLUGIN_PACKAGE_TEMP_DIR, ec);

        // Uncompress package to temp dir
        string uncompressToDir = LLSE_PLUGIN_PACKAGE_TEMP_DIR;
        auto [exitCode, output] =
            UncompressFile(fileOrDirPath, uncompressToDir, LLSE_PLUGIN_PACKAGE_UNCOMPRESS_TIMEOUT);
        if (exitCode != 0) {
            lse::getSelfPluginInstance().getLogger().error(
                "Fail to uncompress plugin package at " + fileOrDirPath + "!"
            );
            lse::getSelfPluginInstance().getLogger().debug(output);
            return false;
        }

        // Re-get backendType
        isPluginPackage = true;
        realPath        = uncompressToDir;
        backendType     = getPluginBackendType(realPath);
    }

    // Re-check backendType
    if (backendType.empty() || backendType == "PluginPackage") {
        lse::getSelfPluginInstance().getLogger().error(pluginFileName + " is not a valid plugin!");
        return false;
    } else if (backendType != LLSE_BACKEND_TYPE) {
        // Unmatched backend
        if (mustBeCurrentModule) throw Exception("Plugin of not matched backend given!");
        return false;
    }

    // Plugin package
    if (isPluginPackage) {
        bool isUncompressedFirstTime = (realPath != fileOrDirPath);
        return loadPluginPackage(realPath, fileOrDirPath, isHotLoad, isUncompressedFirstTime);
    }

    // Todo
    // Single file plugin
    // Check duplicated
    // if (PluginManager::getPlugin(pluginFileName)) {
    //     // lse::getSelfPluginInstance().getLogger().error("This plugin has been loaded by LeviLamina. You cannot load
    //     // it twice.");
    //     return false;
    // }

    ScriptEngine* engine = nullptr;
    try {
        // Create script engine
        engine = EngineManager::newEngine("", isHotLoad);
        EngineScope enter(engine);

        // setData
        ENGINE_OWN_DATA()->pluginName          = pluginFileName;
        ENGINE_OWN_DATA()->pluginFileOrDirPath = realPath;
        ENGINE_OWN_DATA()->logger.title        = pluginFileName;

        // Bind APIs
        try {
            BindAPIs(engine);
        } catch (const Exception& e) {
            lse::getSelfPluginInstance().getLogger().error("Fail in Binding APIs!\n");
            throw;
        }
        // Load depend libs
        try {
            for (auto& [path, content] : depends) {
                if (!content.empty()) engine->eval(content, path);
            }
        } catch (const Exception& e) {
            lse::getSelfPluginInstance().getLogger().error("Fail in Loading Dependence Lib!\n");
            throw;
        }

        // Load script
        try {
            // Try use loadFile
            engine->loadFile(realPath);
        } catch (const Exception& e1) {
            try {
                // loadFile failed. Try use eval instead
                auto scripts = ReadAllFile(realPath);
                if (!scripts) {
                    throw std::runtime_error("Fail to open plugin file!");
                }
                engine->eval(*scripts, ENGINE_OWN_DATA()->pluginFileOrDirPath);
            } catch (const Exception& e2) {
                // Fail
                lse::getSelfPluginInstance().getLogger().error("Fail in Loading Script Plugin!\n");
                throw e1; // throw the original exception out
            }
        }
        std::string const& pluginName = ENGINE_OWN_DATA()->pluginName;
        ExitEngineScope    exit;

        // Todo
        // If plugin itself doesn't register, help it to do so
        // if (!PluginManager::getPlugin(pluginName))
        //     PluginManager::registerPlugin(realPath, pluginName, pluginName, ll::data::Version(1, 0, 0), {});

        // Call necessary events when at hot load
        if (isHotLoad) LLSECallEventsOnHotLoad(engine);

        // Success
        lse::getSelfPluginInstance().getLogger().info(
            "llse.loader.loadMain.loadedPlugin"_tr(fmt::arg("type", backendType), fmt::arg("name", pluginName))
        );
        return true;
    } catch (const Exception& e) {
        lse::getSelfPluginInstance().getLogger().error("Fail to load " + realPath + "!");
        if (engine) {
            EngineScope enter(engine);
            lse::getSelfPluginInstance().getLogger().error("In Plugin: " + ENGINE_OWN_DATA()->pluginName);
            PrintException(e);
            ExitEngineScope exit;

            LLSERemoveTimeTaskData(engine);
            LLSERemoveAllEventListeners(engine);
            LLSERemoveCmdRegister(engine);
            LLSERemoveCmdCallback(engine);
            LLSERemoveAllExportedFuncs(engine);

            engine->getData().reset();
            EngineManager::unRegisterEngine(engine);
        }
        if (engine) {
            engine->destroy();
        }
    } catch (const std::exception& e) {
        lse::getSelfPluginInstance().getLogger().error("Fail to load " + realPath + "!");
        lse::getSelfPluginInstance().getLogger().error(ll::string_utils::tou8str(e.what()));
    } catch (...) {
        lse::getSelfPluginInstance().getLogger().error("Fail to load " + realPath + "!");
    }
    return false;
}

// Load plugin package
// This function must be called in correct backend
bool PluginManager::loadPluginPackage(
    const std::string& dirPath,
    const std::string& packagePath,
    bool               isHotLoad,
    bool               isUncompressedFirstTime
) {
    // "dirPath" is public temp dir (LLSE_PLUGIN_PACKAGE_TEMP_DIR) or normal
    // plugin dir "packagePath" will point to plugin package path if
    // isUncompressedFirstTime == true
    if (!std::filesystem::is_directory(dirPath)) return false;
    bool result = false;

#ifdef LLSE_BACKEND_NODEJS
    result = NodeJsHelper::loadNodeJsPlugin(dirPath, packagePath, isHotLoad);
#elif defined(LLSE_BACKEND_PYTHON)
    result = PythonHelper::loadPythonPlugin(dirPath, packagePath, isHotLoad);
#endif

    if (result && isUncompressedFirstTime) {
        // OK now. Delete installed plugin package
        std::error_code ec;
        std::filesystem::remove(packagePath, ec);
    }
    return result;
}

// Unload plugin
bool PluginManager::unloadPlugin(const std::string& name) {
    if (name == LLSE_DEBUG_ENGINE_NAME) return false;

    auto engine = EngineManager::getEngine(name, true);
    if (!engine) return false;
    string pluginName = ENGINE_GET_DATA(engine)->pluginName;

    // NodeJs use his own setTimeout, so no need to remove
#ifndef LLSE_BACKEND_NODEJS
    LLSERemoveTimeTaskData(engine);
#endif

    LLSECallEventsOnHotUnload(engine);
    LLSERemoveAllEventListeners(engine);
    LLSERemoveCmdRegister(engine);
    LLSERemoveCmdCallback(engine);
    LLSERemoveAllExportedFuncs(engine);

    EngineManager::unRegisterEngine(engine);
    engine->getData().reset();

    PluginManager::unRegisterPlugin(name);
    ll::schedule::DelayTask<ll::chrono::ServerClock> task(ll::chrono::ticks(1), [engine]() {
#ifdef LLSE_BACKEND_NODEJS
        NodeJsHelper::stopEngine(engine);
#else
    engine->destroy();
#endif
    });

    lse::getSelfPluginInstance().getLogger().info(pluginName + " unloaded.");
    return true;
}

// Reload plugin
bool PluginManager::reloadPlugin(const std::string& name) {
    if (name == LLSE_DEBUG_ENGINE_NAME) return true;

    auto plugin = PluginManager::getPlugin(name);
    if (!plugin) return false;

    std::string filePath = plugin->getManifest().entry;
    if (!PluginManager::unloadPlugin(name)) return false;
    try {
        return PluginManager::loadPlugin(filePath, true, true);
    } catch (...) {
        return false;
    }
}

// Reload all plugins
bool PluginManager::reloadAllPlugins() {
    auto pluginsList = PluginManager::getLocalPlugins();
    for (auto& plugin : pluginsList) reloadPlugin(plugin.second->getManifest().name);
    return true;
}

ll::plugin::Plugin* PluginManager::getPlugin(std::string name) { // Todo
    return {};
}

// Get all plugins of current language
std::unordered_map<std::string, ll::plugin::Plugin*> PluginManager::getLocalPlugins() {
    std::unordered_map<std::string, ll::plugin::Plugin*> res;

    auto engines = EngineManager::getLocalEngines();
    for (auto& engine : engines) {
        string name = ENGINE_GET_DATA(engine)->pluginName;
        if (name != LLSE_DEBUG_ENGINE_NAME) {
            ll::plugin::Plugin* plugin = PluginManager::getPlugin(name);
            if (plugin) res[plugin->getManifest().entry] = plugin;
        }
    }
    return res;
}

std::unordered_map<std::string, ll::plugin::Plugin*> PluginManager::getAllScriptPlugins() {
    // Todo
    // auto res = getAllPlugins();
    // erase_if(res, [](auto &item) {
    //   return item.second->type != ll::plugin::ScriptPlugin;
    // });
    // return res;
    return {};
}

// Get all plugins
std::unordered_map<std::string, ll::plugin::Plugin*> PluginManager::getAllPlugins() {
    // Todo
    // return ll::PluginManager::getAllPlugins();
    return {};
}

bool PluginManager::registerPlugin(
    std::string                        filePath,
    std::string                        name,
    std::string                        desc,
    ll::data::Version                  version,
    std::map<std::string, std::string> others
) {
    others["PluginType"]     = "Script Plugin";
    others["PluginFilePath"] = std::move(filePath);
    // Todo
    // return ll::PluginManager::registerPlugin(nullptr, std::move(name),
    //                                          std::move(desc), version, others);
    return true;
}

bool PluginManager::unRegisterPlugin(std::string name) {
    // Todo
    // return ll::PluginManager::unRegisterPlugin(std::move(name));
    return true;
}

// Get plugin backend type from its file path (single file plugin)
// or its unpressed dir path (plugin package)
std::string PluginManager::getPluginBackendType(const std::string& path) {
    std::filesystem::path filePath(ll::string_utils::str2wstr(path));
    if (!std::filesystem::exists(filePath)) return "";

    if (std::filesystem::is_directory(filePath)) {
        // Uncompressed plugin package
        auto           identifiers = LLSE_VALID_PLUGIN_PACKAGE_IDENTIFIER;
        vector<string> filesExts   = {};
        for (int i = 0; i < identifiers.size(); ++i)
            if (!identifiers.empty()) {
                string id = identifiers[i];
                if (id.empty()) continue;

                if (id.find('*') != std::string::npos) {
                    // match identifier like "*.py"
                    if (filesExts.empty()) {
                        // build filesExts list
                        std::filesystem::directory_iterator files(filePath);
                        for (auto& item : files) {
                            if (item.is_regular_file())
                                filesExts.emplace_back(ll::string_utils::u8str2str(item.path().extension().u8string()));
                        }
                    }
                    string compareExt = id.substr(id.find_last_of('.'));
                    if (std::find(filesExts.begin(), filesExts.end(), compareExt) != filesExts.end()) {
                        // match
                        return LLSE_VALID_BACKENDS[i];
                    }
                } else {
                    // match identifier like "package.json"
                    if (std::filesystem::exists(filePath / id)) {
                        // match
                        return LLSE_VALID_BACKENDS[i];
                    }
                }
            }
    } else {
        // Common plugin file
        string ext = ll::string_utils::u8str2str(filePath.extension().u8string());
        if (ext == LLSE_PLUGIN_PACKAGE_EXTENSION) {
            // Never consider .llplugin
            // Just uncompress it and then come to check
            return "PluginPackage";
        }
        auto validExts = LLSE_VALID_PLUGIN_EXTENSIONS;
        for (int i = 0; i < validExts.size(); ++i)
            if (!validExts[i].empty() && validExts[i] == ext) {
                // match
                return LLSE_VALID_BACKENDS[i];
            }
    }
    // none backend matched
    return "";
}
