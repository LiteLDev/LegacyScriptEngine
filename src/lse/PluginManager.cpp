#include "PluginManager.h"

#include "Entry.h"
#include "Plugin.h"
#include "legacy/api/EventAPI.h"
#include "legacy/engine/EngineManager.h"
#include "legacy/engine/EngineOwnData.h"

#include <ScriptX/ScriptX.h>
#include <exception>
#include <fmt/format.h>
#include <ll/api/Logger.h>
#include <ll/api/io/FileUtils.h>
#include <ll/api/plugin/Plugin.h>
#include <ll/api/plugin/PluginManager.h>
#include <ll/api/service/ServerInfo.h>
#include <memory>
#include <stdexcept>

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_LUA

constexpr auto BaseLibFileName   = "BaseLib.lua";
constexpr auto PluginManagerName = "lse-lua";

#endif

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_QUICKJS

constexpr auto BaseLibFileName   = "BaseLib.js";
constexpr auto PluginManagerName = "lse-quickjs";

#endif

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON

#include "legacy/main/PythonHelper.h"
constexpr auto BaseLibFileName   = "BaseLib.py";
constexpr auto PluginManagerName = "lse-python";

#endif

// Do not use legacy headers directly, otherwise there will be tons of errors.
void BindAPIs(script::ScriptEngine* engine);
void LLSERemoveTimeTaskData(script::ScriptEngine* engine);
auto LLSERemoveAllEventListeners(script::ScriptEngine* engine) -> bool;
auto LLSERemoveCmdRegister(script::ScriptEngine* engine) -> bool;
auto LLSERemoveCmdCallback(script::ScriptEngine* engine) -> bool;
auto LLSERemoveAllExportedFuncs(script::ScriptEngine* engine) -> bool;

namespace lse {

PluginManager::PluginManager() : ll::plugin::PluginManager(PluginManagerName) {}

auto PluginManager::load(ll::plugin::Manifest manifest) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON
    // "dirPath" is public temp dir (LLSE_PLUGIN_PACKAGE_TEMP_DIR) or normal
    // plugin dir "packagePath" will point to plugin package path if
    // isUncompressedFirstTime == true
    // if (dirPath == LLSE_PLUGIN_PACKAGE_TEMP_DIR) {
    //     // Need to copy from temp dir to installed dir
    //     if (std::filesystem::exists(LLSE_PLUGIN_PACKAGE_TEMP_DIR "/pyproject.toml")) {
    //         auto pluginName = PythonHelper::getPluginPackageName(LLSE_PLUGIN_PACKAGE_TEMP_DIR);
    //         if (pluginName.empty()) {
    //             pluginName = ll::string_utils::u8str2str(
    //                 std::filesystem::path(packagePath).filename().replace_extension("").u8string()
    //             );
    //         }
    //         auto dest = std::filesystem::path(LLSE_PLUGINS_ROOT_DIR).append(pluginName);

    //         // copy files
    //         std::error_code ec;
    //         // if (filesystem::exists(dest))
    //         //     filesystem::remove_all(dest, ec);
    //         std::filesystem::copy(
    //             LLSE_PLUGIN_PACKAGE_TEMP_DIR "/",
    //             dest,
    //             std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive,
    //             ec
    //         );

    //         // reset dirPath
    //         dirPath = ll::string_utils::u8str2str(dest.u8string());
    //     }
    //     // remove temp dir
    //     std::error_code ec;
    //     std::filesystem::remove_all(LLSE_PLUGIN_PACKAGE_TEMP_DIR, ec);
    // }

    // std::string entryPath = PythonHelper::findEntryScript(dirPath);
    // if (entryPath.empty()) return false;
    // std::string pluginName = PythonHelper::getPluginPackageName(dirPath);

    // // Run "pip install" if needed
    // auto realPackageInstallDir = (std::filesystem::path(dirPath) / "site-packages").make_preferred();
    // if (!std::filesystem::exists(realPackageInstallDir)) {
    //     std::string dependTmpFilePath = PythonHelper::getPluginPackDependencyFilePath(dirPath);
    //     if (!dependTmpFilePath.empty()) {
    //         int exitCode = 0;
    //         lse::getSelfPluginInstance().getLogger().info("llse.loader.python.executePipInstall.start"_tr(
    //             fmt::arg("name", ll::string_utils::u8str2str(std::filesystem::path(dirPath).filename().u8string()))
    //         ));

    //         if ((exitCode = PythonHelper::executePipCommand(
    //                  "pip install -r \"" + dependTmpFilePath + "\" -t \""
    //                  + ll::string_utils::u8str2str(realPackageInstallDir.u8string()) + "\"
    //                  --disable-pip-version-check"
    //              ))
    //             == 0) {
    //             lse::getSelfPluginInstance().getLogger().info("llse.loader.python.executePipInstall.success"_tr());
    //         } else
    //             lse::getSelfPluginInstance().getLogger().error(
    //                 "llse.loader.python.executePipInstall.fail"_tr(fmt::arg("code", exitCode))
    //             );

    //         // remove temp dependency file after installation
    //         std::error_code ec;
    //         std::filesystem::remove(std::filesystem::path(dependTmpFilePath), ec);
    //     }
    // }
#endif

    try {

        logger.info("loading plugin {}", manifest.name);

        if (hasPlugin(manifest.name)) {
            throw std::runtime_error("plugin already loaded");
        }

        auto& scriptEngine = *EngineManager::newEngine(manifest.name);
        auto  plugin       = std::make_shared<Plugin>(manifest);

        try {
            script::EngineScope engineScope(scriptEngine);

            // Set plugins's logger title
            ENGINE_OWN_DATA()->logger.title = manifest.name;
            ENGINE_OWN_DATA()->pluginName   = manifest.name;

            BindAPIs(&scriptEngine);

            auto& self = getSelfPluginInstance();

            // Load BaseLib.
            auto baseLibPath    = self.getPluginDir() / "baselib" / BaseLibFileName;
            auto baseLibContent = ll::file_utils::readFile(baseLibPath);
            if (!baseLibContent) {
                throw std::runtime_error(fmt::format("failed to read BaseLib at {}", baseLibPath.string()));
            }
            scriptEngine.eval(baseLibContent.value());

            // Load the plugin entry.
            auto pluginDir = std::filesystem::canonical(ll::plugin::getPluginsRoot() / manifest.name);
            auto entryPath = pluginDir / manifest.entry;

            // Try loadFile
            try {
                scriptEngine.loadFile(entryPath.u8string());
            } catch (const script::Exception& e) {
                // loadFile failed, try eval
                auto pluginEntryContent = ll::file_utils::readFile(entryPath);
                if (!pluginEntryContent) {
                    throw std::runtime_error(fmt::format("failed to read plugin entry at {}", entryPath.string()));
                }
                scriptEngine.eval(pluginEntryContent.value());
            }
            if (ll::getServerStatus() == ll::ServerStatus::Running) { // Is hot load
                LLSECallEventsOnHotLoad(&scriptEngine);
            }
            plugin->onLoad([](ll::plugin::Plugin& plugin) { return true; });
            plugin->onUnload([](ll::plugin::Plugin& plugin) { return true; });
            plugin->onEnable([](ll::plugin::Plugin& plugin) { return true; });
            plugin->onDisable([](ll::plugin::Plugin& plugin) { return true; });
        } catch (const std::exception& e) {
            LLSERemoveTimeTaskData(&scriptEngine);
            LLSERemoveAllEventListeners(&scriptEngine);
            LLSERemoveCmdRegister(&scriptEngine);
            LLSERemoveCmdCallback(&scriptEngine);
            LLSERemoveAllExportedFuncs(&scriptEngine);

            scriptEngine.getData().reset();

            EngineManager::unregisterEngine(&scriptEngine);

            throw;
        }

        if (!addPlugin(manifest.name, plugin)) {
            throw std::runtime_error(fmt::format("failed to register plugin {}", manifest.name));
        }

        return true;

    } catch (const std::exception& e) {
        logger.error("failed to load plugin {}: {}", manifest.name, e.what());
        return false;
    }

    return true;
}

auto PluginManager::unload(std::string_view name) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    try {

        auto plugin = std::static_pointer_cast<Plugin>(getPlugin(name));

        logger.info("unloading plugin {}", name);

        auto& scriptEngine = *EngineManager::getEngine(std::string(name));

        LLSERemoveTimeTaskData(&scriptEngine);
        LLSERemoveAllEventListeners(&scriptEngine);
        LLSERemoveCmdRegister(&scriptEngine);
        LLSERemoveCmdCallback(&scriptEngine);
        LLSERemoveAllExportedFuncs(&scriptEngine);

        scriptEngine.getData().reset();

        EngineManager::unregisterEngine(&scriptEngine);

        scriptEngine.destroy(); // TODO: use unique_ptr to manage the engine.

        if (!erasePlugin(name)) {
            throw std::runtime_error(fmt::format("failed to unregister plugin {}", name));
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        logger.error("failed to unload plugin {}: {}", name, e.what());
        return false;
    }
}

} // namespace lse
