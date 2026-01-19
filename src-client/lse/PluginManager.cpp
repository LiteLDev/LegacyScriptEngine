#include "lse/PluginManager.h"

#include "legacy/engine/EngineManager.h"
#include "legacy/engine/EngineOwnData.h"
#include "ll/api/io/FileUtils.h" // IWYU pragma: keep
#include "ll/api/mod/Mod.h"
#include "ll/api/mod/ModManager.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/utils/ErrorUtils.h"
#include "ll/api/utils/StringUtils.h"
#include "lse/Entry.h"
#include "lse/Plugin.h"

#include <ScriptX/ScriptX.h>
#include <exception>
#include <filesystem>
#include <memory>

#ifdef LSE_BACKEND_LUA

constexpr auto BaseLibFileName   = "BaseLib.lua";
constexpr auto PluginManagerName = "lse-lua";

#endif

#ifdef LSE_BACKEND_QUICKJS

constexpr auto BaseLibFileName   = "BaseLib.js";
constexpr auto PluginManagerName = "lse-quickjs";

#endif

#ifdef LSE_BACKEND_PYTHON

#include "legacy/main/PythonHelper.h"
constexpr auto BaseLibFileName   = "BaseLib.py";
constexpr auto PluginManagerName = "lse-python";

#endif

#ifdef LSE_BACKEND_NODEJS

#include "legacy/main/NodeJsHelper.h"
constexpr auto PluginManagerName = "lse-nodejs";

#endif

// Do not use legacy headers directly, otherwise there will be tons of errors.
void BindAPIs(std::shared_ptr<ScriptEngine> engine);
void LLSERemoveTimeTaskData(std::shared_ptr<ScriptEngine> engine);
bool LLSERemoveAllEventListeners(std::shared_ptr<ScriptEngine> engine);
bool LLSERemoveCmdRegister(std::shared_ptr<ScriptEngine> engine);
bool LLSERemoveCmdCallback(std::shared_ptr<ScriptEngine> engine);
bool LLSERemoveAllExportedFuncs(std::shared_ptr<ScriptEngine> engine);
bool LLSECallEventsOnHotLoad(std::shared_ptr<ScriptEngine> engine);
bool LLSECallEventsOnUnload(std::shared_ptr<ScriptEngine> engine);

namespace lse {

PluginManager::PluginManager() : ll::mod::ModManager(PluginManagerName) {}
PluginManager::~PluginManager() = default;

ll::Expected<> PluginManager::load(ll::mod::Manifest manifest) {
    if (hasMod(manifest.name)) {
        return ll::makeStringError("Plugin has already loaded");
    }

    auto plugin = std::make_shared<Plugin>(manifest);

    return plugin->onLoad().transform([&, this] { addMod(manifest.name, plugin); });
}

ll::Expected<> PluginManager::enable(std::string_view name) {
#ifdef LSE_BACKEND_PYTHON
    auto&                 logger  = lse::LegacyScriptEngine::getLogger();
    std::filesystem::path dirPath = ll::mod::getModsRoot() / manifest.name; // Plugin path
    std::string           entryPath =
        PythonHelper::findEntryScript(ll::string_utils::u8str2str(dirPath.u8string())); // Plugin entry
    // if (entryPath.empty()) return false;
    // std::string pluginName = PythonHelper::getPluginPackageName(dirPath.string()); // Plugin name

    // Run "pip install" if needed
    auto realPackageInstallDir = (std::filesystem::path(dirPath) / "site-packages").make_preferred();
    if (!std::filesystem::exists(realPackageInstallDir)) {
        std::string dependTmpFilePath =
            PythonHelper::getPluginPackDependencyFilePath(ll::string_utils::u8str2str(dirPath.u8string()));
        if (!dependTmpFilePath.empty()) {
            int exitCode = 0;
            logger.info(
                "Executing \"pip install\" for plugin {name}..."_tr(
                    fmt::arg("name", ll::string_utils::u8str2str(dirPath.filename().u8string()))
                )
            );

            if ((exitCode = PythonHelper::executePipCommand(
                     "pip install -r \"" + dependTmpFilePath + "\" -t \""
                     + ll::string_utils::u8str2str(realPackageInstallDir.u8string()) + "\" --disable-pip-version-check "
                 ))
                == 0) {
                logger.info("Pip finished successfully."_tr());
            } else logger.error("Error occurred. Exit code: {code}"_tr(fmt::arg("code", exitCode)));

            // remove temp dependency file after installation
            std::error_code ec;
            std::filesystem::remove(std::filesystem::path(dependTmpFilePath), ec);
        }
    }
#endif
#ifdef LSE_BACKEND_NODEJS
    auto&                 logger  = lse::LegacyScriptEngine::getLogger();
    std::filesystem::path dirPath = ll::mod::getModsRoot() / manifest.name; // Plugin path
    // std::string           entryPath = NodeJsHelper::findEntryScript(dirPath.string()); // Plugin entry
    // if (entryPath.empty()) return false;
    // std::string pluginName = NodeJsHelper::getPluginPackageName(dirPath.string()); // Plugin name

    // Run "npm install" if needed
    if (NodeJsHelper::doesPluginPackHasDependency(ll::string_utils::u8str2str(dirPath.u8string()))
        && !std::filesystem::exists(std::filesystem::path(dirPath) / "node_modules")) {
        int exitCode = 0;
        logger.info(
            "Executing \"npm install\" for plugin {name}..."_tr(
                fmt::arg("name", ll::string_utils::u8str2str(dirPath.filename().u8string()))
            )
        );
        if ((exitCode = NodeJsHelper::executeNpmCommand(
                 {"install", "--omit=dev", "--no-fund"},
                 ll::string_utils::u8str2str(dirPath.u8string())
             ))
            != 0) {
            logger.error("Error occurred. Exit code: {code}"_tr(fmt::arg("code", exitCode)));
        }
    }
#endif
    auto plugin = std::static_pointer_cast<Plugin>(getMod(name));
    if (!plugin) {
        return ll::makeStringError("Plugin {0} not found"_tr(name));
    }
    auto manifest = plugin->getManifest();

    auto scriptEngine = EngineManager::newEngine(manifest.name);

    try {
        EngineScope engineScope(scriptEngine.get());

        // Init plugin logger
        getEngineOwnData()->logger = ll::io::LoggerRegistry::getInstance().getOrCreate(manifest.name);

#ifdef LSE_BACKEND_PYTHON
        scriptEngine->eval("import sys as _llse_py_sys_module");
        std::error_code ec;

        // add plugin-own site-packages to sys.path
        std::string pluginSitePackageFormatted = ll::string_utils::u8str2str(
            std::filesystem::canonical(realPackageInstallDir.make_preferred(), ec).u8string()
        );
        if (!ec) {
            scriptEngine->eval("_llse_py_sys_module.path.insert(0, r'" + pluginSitePackageFormatted + "')");
        }
        // add plugin source dir to sys.path
        std::string sourceDirFormatted =
            ll::string_utils::u8str2str(std::filesystem::canonical(dirPath.make_preferred()).u8string());
        scriptEngine->eval("_llse_py_sys_module.path.insert(0, r'" + sourceDirFormatted + "')");

        // set __file__ and __name__
        std::string entryPathFormatted = ll::string_utils::u8str2str(
            std::filesystem::canonical(std::filesystem::path(entryPath).make_preferred()).u8string()
        );
        scriptEngine->set("__file__", entryPathFormatted);
        // engine->set("__name__", String::newString("__main__"));
#endif

        BindAPIs(scriptEngine);

#ifndef LSE_BACKEND_NODEJS // NodeJs backend load depends code in another place
        auto& self = LegacyScriptEngine::getInstance().getSelf();
        // Load BaseLib.
        auto baseLibPath    = self.getModDir() / "baselib" / BaseLibFileName;
        auto baseLibContent = ll::file_utils::readFile(baseLibPath);
        if (!baseLibContent) {
            return ll::makeStringError("Failed to read BaseLib at {0}"_tr(baseLibPath.string()));
        }
        scriptEngine->eval(baseLibContent.value());
#endif
        // Load the plugin entry.
        auto entryPath             = plugin->getModDir() / manifest.entry;
        getEngineOwnData()->plugin = plugin;
#ifdef LSE_BACKEND_PYTHON
        if (!PythonHelper::loadPluginCode(
                scriptEngine,
                ll::string_utils::u8str2str(entryPath.u8string()),
                ll::string_utils::u8str2str(dirPath.u8string())
            )) {
            return ll::makeStringError("Failed to load plugin code"_tr());
        }
#endif
#ifdef LSE_BACKEND_NODEJS
        if (!NodeJsHelper::loadPluginCode(
                scriptEngine,
                ll::string_utils::u8str2str(entryPath.u8string()),
                ll::string_utils::u8str2str(dirPath.u8string()),
                NodeJsHelper::isESModulesSystem(ll::string_utils::u8str2str(dirPath.u8string()))
            )) {
            return ll::makeStringError("Failed to load plugin code"_tr());
        }
#endif
#if (defined LSE_BACKEND_QUICKJS) || (defined LSE_BACKEND_LUA)
        // Try loadFile
        try {
            scriptEngine->loadFile(entryPath.u8string());
        } catch (const Exception&) {
            // loadFile failed, try eval
            auto pluginEntryContent = ll::file_utils::readFile(entryPath);
            if (!pluginEntryContent) {
                return ll::makeStringError("Failed to read plugin entry at {0}"_tr(entryPath.string()));
            }
            scriptEngine->eval(pluginEntryContent.value(), entryPath.u8string());
        }
#endif
        if (ll::getGamingStatus() == ll::GamingStatus::Running) { // Is hot load
            LLSECallEventsOnHotLoad(scriptEngine);
        }
        ExitEngineScope exit;

        if (auto res = plugin->onEnable(); !res) {
            return res;
        }

        return {};
    } catch (const Exception& e) {
        if (scriptEngine) {
            auto error = [&] {
                EngineScope engineScope(scriptEngine.get());
                return ll::makeStringError(
                    "Failed to enable plugin {0}: {1}\n{2}"_tr(manifest.name, e.message(), e.stacktrace())
                );
            }();

#ifndef LSE_BACKEND_NODEJS
            LLSERemoveTimeTaskData(scriptEngine);
#endif
            LLSERemoveAllEventListeners(scriptEngine);
            LLSERemoveCmdRegister(scriptEngine);
            LLSERemoveCmdCallback(scriptEngine);
            LLSERemoveAllExportedFuncs(scriptEngine);

            EngineOwnData::clearEngineObjects(scriptEngine);
            EngineManager::unregisterEngine(scriptEngine);
#ifdef LSE_BACKEND_NODEJS
            NodeJsHelper::stopEngine(scriptEngine);
#endif
            return error;
        } else {
            return ll::makeStringError("Failed to enable plugin {0}: {1}"_tr(manifest.name, "ScriptEngine is nullptr"));
        }
    }
}

ll::Expected<> PluginManager::disable(std::string_view name) {
    try {
        auto scriptEngine = EngineManager::getEngine(std::string(name));

        if (!scriptEngine) {
            return ll::makeStringError("Plugin {0} not found"_tr(name));
        }

        {
            EngineScope scope(scriptEngine.get());
            LLSECallEventsOnUnload(scriptEngine);
#ifndef LSE_BACKEND_NODEJS
            LLSERemoveTimeTaskData(scriptEngine);
#endif
            LLSERemoveAllEventListeners(scriptEngine);
            LLSERemoveCmdRegister(scriptEngine);
            LLSERemoveCmdCallback(scriptEngine);
            LLSERemoveAllExportedFuncs(scriptEngine);
            EngineOwnData::clearEngineObjects(scriptEngine);
        }
        EngineManager::unregisterEngine(scriptEngine);
#ifdef LSE_BACKEND_NODEJS
        NodeJsHelper::stopEngine(scriptEngine);
#endif

        if (auto res = std::static_pointer_cast<Plugin>(getMod(name))->onDisable(); !res) {
            return res;
        }
        return {};
    } catch (const Exception&) {
        return ll::makeStringError("Failed to disable plugin {0}: {1}"_tr(name, "Unknown script exception"));
    } catch (const std::exception& e) {
        return ll::makeStringError("Failed to disable plugin {0}: {1}"_tr(name, e.what()));
    }
}

ll::Expected<> PluginManager::unload(std::string_view name) {
    if (auto res = std::static_pointer_cast<Plugin>(getMod(name))->onUnload(); !res) {
        return res;
    }
    eraseMod(name);
    return {};
}

} // namespace lse
