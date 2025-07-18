#include "PluginMigration.h"

#include "Entry.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/mod/Manifest.h"
#include "ll/api/mod/Mod.h"
#include "ll/api/reflection/Serialization.h"

#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_set>

#if LSE_BACKEND_LUA

constexpr auto PluginExtName = ".lua";

#endif

#ifdef LSE_BACKEND_QUICKJS

constexpr auto PluginExtName = ".js";

#endif

#ifdef LSE_BACKEND_PYTHON

constexpr auto PluginExtName = ".py";

#endif

#ifdef LSE_BACKEND_NODEJS

#include "legacy/legacyapi/utils/FileHelper.h"
#include "main/NodeJsHelper.h"

constexpr auto PluginExtName = ".llplugin";

#endif

using namespace ll::i18n_literals;

namespace lse {

namespace {

auto migratePlugin(const PluginManager& pluginManager, const std::filesystem::path& path) -> void {
    auto& self = LegacyScriptEngine::getInstance().getSelf();

    auto& logger = self.getLogger();

    logger.info("Migrating legacy plugin at {0}"_tr(ll::string_utils::u8str2str(path.u8string())));

    const auto& pluginType = pluginManager.getType();

    const auto& pluginFileName     = path.filename();
    const auto& pluginFileBaseName = path.stem();
    const auto& pluginDir          = ll::mod::getModsRoot() / pluginFileBaseName;

    if (std::filesystem::exists(pluginDir / pluginFileName)) {
        throw std::runtime_error(
            "Failed to migrate legacy plugin at {0}: {1} already exists"_tr(
                ll::string_utils::u8str2str(path.u8string()),
                ll::string_utils::u8str2str(pluginDir.u8string())
            )
        );
    }
#ifndef LSE_BACKEND_NODEJS
    if (!std::filesystem::exists(pluginDir)) {
        if (!std::filesystem::create_directory(pluginDir)) {
            throw std::runtime_error(
                "Failed to create directory {0}"_tr(ll::string_utils::u8str2str(pluginDir.u8string()))
            );
        }
    }

    // Move plugin file.
    std::filesystem::rename(path, pluginDir / pluginFileName);

    ll::mod::Manifest manifest{
        .entry        = ll::string_utils::u8str2str(pluginFileName.u8string()),
        .name         = ll::string_utils::u8str2str(pluginFileBaseName.u8string()),
        .type         = pluginType,
        .dependencies = ll::SmallDenseSet<ll::mod::Dependency>{
                                                               ll::mod::Dependency{
                .name = self.getManifest().name,
            }, },
    };
#endif
#ifdef LSE_BACKEND_NODEJS
    lse::legacy::UncompressFile(
        ll::string_utils::u8str2str(path.u8string()),
        ll::string_utils::u8str2str(pluginDir.u8string()),
        30000
    );
    ll::mod::Manifest manifest{
        .entry        = NodeJsHelper::findEntryScript(ll::string_utils::u8str2str(path.u8string())),
        .name         = ll::string_utils::u8str2str(pluginFileBaseName.u8string()),
        .type         = pluginType,
        .dependencies = ll::SmallDenseSet<ll::mod::Dependency>{
                                                               ll::mod::Dependency{
                .name = self.getManifest().name,
            }, },
    };
    std::filesystem::remove(path);
#endif
    if (!std::filesystem::exists(pluginDir / "manifest.json")) {
        auto manifestJson = ll::reflection::serialize<nlohmann::ordered_json>(manifest).value();

        std::ofstream manifestFile{pluginDir / "manifest.json"};
        manifestFile << manifestJson.dump(4);
    }
}

} // namespace

auto migratePlugins(const PluginManager& pluginManager) -> void {
    auto& self = LegacyScriptEngine::getInstance().getSelf();

    auto& logger = self.getLogger();

    std::unordered_set<std::filesystem::path> pluginPaths;

    // Discover plugins.
    // logger.info("Discovering legacy plugins..."_tr());

    const auto& pluginBaseDir = ll::mod::getModsRoot();

    for (const auto& entry : std::filesystem::directory_iterator(pluginBaseDir)) {
        if (!entry.is_regular_file() || entry.path().extension() != PluginExtName) {
            continue;
        }

        pluginPaths.insert(entry.path());
    }

    if (pluginPaths.empty()) {
        // logger.info("No legacy plugin found, skipping migration"_tr());
        return;
    }

    // Migrate plugins.
    logger.info("Migrating legacy plugins..."_tr());

    for (const auto& path : pluginPaths) {
        migratePlugin(pluginManager, path);
    }

    logger.warn("Legacy plugins have been migrated, please restart the server to load them!"_tr());
}

} // namespace lse
