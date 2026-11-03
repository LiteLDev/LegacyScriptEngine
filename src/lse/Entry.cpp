#include "lse/Entry.h"

#include "legacy/engine/EngineManager.h"
#include "legacy/engine/EngineOwnData.h"
#include "legacy/main/BindAPIs.h"
#include "legacy/main/EconomicSystem.h"
#include "legacy/utils/ScriptErrorPrinter.h"
#include "ll/api/Config.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/command/ServerCommandRegisterEvent.h"
#include "ll/api/event/server/ServerStartedEvent.h"
#include "ll/api/event/server/ServerStoppingEvent.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/io/FileUtils.h"
#include "ll/api/mod/ModManagerRegistry.h"
#include "ll/api/mod/NativeMod.h"
#include "ll/api/mod/RegisterHelper.h"
#include "ll/api/service/PlayerInfo.h"
#include "ll/api/utils/StringUtils.h"
#include "lse/PluginManager.h"
#include "lse/PluginMigration.h"

#include <ScriptX/ScriptX.h>
#ifdef _WIN32
#include <Windows.h>
#include <winioctl.h>
#endif
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <vector>

#ifdef LSE_BACKEND_LUA

constexpr auto BaseLibFileName = "BaseLib.lua";

#endif

#ifdef LSE_BACKEND_QUICKJS

constexpr auto BaseLibFileName = "BaseLib.js";

#endif

#ifdef LSE_BACKEND_PYTHON

#include "legacy/main/PythonHelper.h"
constexpr auto BaseLibFileName = "BaseLib.py";

#endif

#ifdef LSE_BACKEND_NODEJS

#include "legacy/main/NodeJsHelper.h"

#endif

using namespace ll::i18n_literals;

// Do not use legacy headers directly, otherwise there will be tons of errors.
void                                  InitBasicEventListeners();
void                                  InitGlobalShareData();
void                                  InitLocalShareData();
void                                  InitMessageSystem();
void                                  InitSafeGuardRecord();
void                                  RegisterDebugCommand();
bool                                  InConsoleDebugMode; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
std::shared_ptr<script::ScriptEngine> DebugEngine;        // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

namespace lse {

void ensurePluginsLink(ll::io::Logger& logger);
void loadConfig(ll::mod::NativeMod const& self, Config& cfg);
void loadDebugEngine(ll::mod::NativeMod const& self);
void registerPluginManager(std::shared_ptr<PluginManager> const& pm);

LegacyScriptEngine& LegacyScriptEngine::getInstance() {
    static LegacyScriptEngine instance;
    return instance;
}

bool LegacyScriptEngine::enable() {
    ll::service::PlayerInfo::getInstance();
#ifdef LL_PLAT_C
    using namespace ll::event;
    auto& bus = EventBus::getInstance();
    bus.emplaceListener<ServerStartedEvent>([](ServerStartedEvent&) { getInstance().getManager().enableAllPlugins(); });
    bus.emplaceListener<ServerStoppingEvent>([](ServerStoppingEvent&) {
        getInstance().getManager().disableAllPlugins();
    });
#endif
    return true;
}

void initializeLegacyStuff() {
    InitLocalShareData();
    InitGlobalShareData();
    InitSafeGuardRecord();
    EconomySystem::init();
#ifdef LSE_BACKEND_PYTHON
    PythonHelper::initPythonRuntime();
#endif

    InitBasicEventListeners();
    InitMessageSystem();
}

bool LegacyScriptEngine::load() {
    auto& logger = getSelf().getLogger();

    try {
        auto result = ll::i18n::getInstance().load(getSelf().getLangDir());

        config = Config();

        ensurePluginsLink(logger);

        pluginManager = std::make_shared<PluginManager>();

        loadConfig(getSelf(), config);

        if (config.migratePlugins) {
            migratePlugins(*pluginManager);
        }

        registerPluginManager(pluginManager);

        // Legacy stuff should be initialized before any possible call to legacy code.
        initializeLegacyStuff();

        loadDebugEngine(getSelf());

        using namespace ll::event;
        EventBus::getInstance().emplaceListener<ServerCommandRegisterEvent>([](ServerCommandRegisterEvent&) {
            RegisterDebugCommand();
        });
        return true;
    } catch (...) {
        logger.error("Failed to load: {0}"_tr(getSelf().getName()));
        ::legacy::script_error::printCurrentException(logger);
        return false;
    }
}

bool LegacyScriptEngine::unload() {
    DebugEngine.reset();
    return true;
}

Config const& LegacyScriptEngine::getConfig() const { return config; }

PluginManager& LegacyScriptEngine::getManager() const {
    if (!pluginManager) {
        throw std::runtime_error("pluginManager is null");
    }

    return *pluginManager;
}

namespace {

#ifdef _WIN32

struct JunctionReparseBuffer {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    USHORT SubstituteNameOffset;
    USHORT SubstituteNameLength;
    USHORT PrintNameOffset;
    USHORT PrintNameLength;
    WCHAR  PathBuffer[1];
};

std::error_code createJunction(std::filesystem::path const& link, std::filesystem::path const& target) {
    std::error_code ec;
    auto            targetPath = std::filesystem::absolute(target, ec).lexically_normal();
    if (ec) {
        return ec;
    }
    if (targetPath.filename().empty() && targetPath.has_parent_path()) {
        targetPath = targetPath.parent_path();
    }

    auto const targetNative = targetPath.native();
    auto       substitute =
        targetNative.starts_with(L"\\\\") ? L"\\??\\UNC\\" + targetNative.substr(2) : L"\\??\\" + targetNative;

    auto const substituteBytes = static_cast<DWORD>(substitute.size() * sizeof(wchar_t));
    auto const printBytes      = static_cast<DWORD>(targetNative.size() * sizeof(wchar_t));
    auto const pathBytes =
        substituteBytes + static_cast<DWORD>(sizeof(wchar_t)) + printBytes + static_cast<DWORD>(sizeof(wchar_t));

    std::vector<char> buffer(offsetof(JunctionReparseBuffer, PathBuffer) + pathBytes, 0);
    auto* const       reparse = reinterpret_cast<JunctionReparseBuffer*>(buffer.data());

    reparse->ReparseTag           = IO_REPARSE_TAG_MOUNT_POINT;
    reparse->ReparseDataLength    = static_cast<USHORT>(8 + pathBytes);
    reparse->Reserved             = 0;
    reparse->SubstituteNameOffset = 0;
    reparse->SubstituteNameLength = static_cast<USHORT>(substituteBytes);
    reparse->PrintNameOffset      = static_cast<USHORT>(substituteBytes + sizeof(wchar_t));
    reparse->PrintNameLength      = static_cast<USHORT>(printBytes);

    auto* const pathBuffer = reparse->PathBuffer;
    std::memcpy(pathBuffer, substitute.data(), substituteBytes);
    pathBuffer[substitute.size()] = L'\0';
    std::memcpy(pathBuffer + substitute.size() + 1, targetNative.data(), printBytes);
    pathBuffer[substitute.size() + 1 + targetNative.size()] = L'\0';

    if (!::CreateDirectoryW(link.c_str(), nullptr)) {
        return {static_cast<int>(::GetLastError()), std::system_category()};
    }

    auto const handle = ::CreateFileW(
        link.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr
    );
    if (handle == INVALID_HANDLE_VALUE) {
        ec = std::error_code(static_cast<int>(::GetLastError()), std::system_category());
        ::RemoveDirectoryW(link.c_str());
        return ec;
    }

    DWORD      returned  = 0;
    auto const succeeded = ::DeviceIoControl(
        handle,
        FSCTL_SET_REPARSE_POINT,
        buffer.data(),
        static_cast<DWORD>(buffer.size()),
        nullptr,
        0,
        &returned,
        nullptr
    );
    ec = succeeded ? std::error_code{} : std::error_code(static_cast<int>(::GetLastError()), std::system_category());

    ::CloseHandle(handle);
    if (!succeeded) {
        ::RemoveDirectoryW(link.c_str());
    }
    return ec;
}

#endif

std::error_code createDirectoryLink(std::filesystem::path const& link, std::filesystem::path const& target) {
#ifdef _WIN32
    if (auto ec = createJunction(link, target); !ec) {
        return {};
    }
#endif
    std::error_code ec;
    std::filesystem::create_directory_symlink(target, link, ec);
    return ec;
}

} // namespace

void ensurePluginsLink(ll::io::Logger& logger) {
    std::error_code ec;
    auto const      serverRoot = std::filesystem::current_path(ec);
    if (ec) {
        return;
    }

    auto const& modsRoot    = ll::mod::getModsRoot();
    auto const  pluginsPath = serverRoot / "plugins";

    if (std::filesystem::exists(pluginsPath, ec)) {
        if (std::filesystem::equivalent(pluginsPath, modsRoot, ec)) {
            return;
        }

        logger.error(
            "Failed to create a link at {0}: a file or directory with the same name already exists"_tr(
                ll::string_utils::u8str2str(pluginsPath.u8string())
            )
        );
        return;
    }
    ec.clear();

    if (auto linkError = createDirectoryLink(pluginsPath, modsRoot); linkError) {
        logger.error(
            "Failed to create a link at {0} pointing to {1}: {2}"_tr(
                ll::string_utils::u8str2str(pluginsPath.u8string()),
                ll::string_utils::u8str2str(modsRoot.u8string()),
                linkError.message()
            )
        );
    }
}

void loadConfig(ll::mod::NativeMod const& self, Config& cfg) {
    auto const& configFilePath = self.getConfigDir() / "config.json";
    if (!ll::config::loadConfig(cfg, configFilePath) && !ll::config::saveConfig(cfg, configFilePath)) {
        throw std::runtime_error("Cannot save default configurations to {0}"_tr(configFilePath));
    }
}

void loadDebugEngine(ll::mod::NativeMod const& self) {
#ifndef LSE_BACKEND_NODEJS // NodeJs backend didn't enable debug engine now
    auto scriptEngine = EngineManager::newEngine();

    script::EngineScope engineScope(scriptEngine.get());

    // Init plugin instance for debug engine to prevent something unexpected.
    ll::mod::Manifest manifest;
    manifest.name              = "DebugEngine";
    getEngineOwnData()->plugin = std::make_shared<lse::ScriptPlugin>(manifest);
    // Init logger
    getEngineOwnData()->logger = ll::io::LoggerRegistry::getInstance().getOrCreate("DebugEngine");

    BindAPIs(scriptEngine);

    // Load BaseLib.
    auto baseLibPath    = self.getModDir() / "baselib" / BaseLibFileName;
    auto baseLibContent = ll::file_utils::readFile(baseLibPath);
    if (!baseLibContent) {
        throw std::runtime_error("Failed to read BaseLib at {0}"_tr(baseLibPath.string()));
    }
    scriptEngine->eval(baseLibContent.value());

    DebugEngine = scriptEngine;
#endif
}

void registerPluginManager(std::shared_ptr<PluginManager> const& pm) {
    auto& pluginManagerRegistry = ll::mod::ModManagerRegistry::getInstance();

    if (!pluginManagerRegistry.addManager(pm)) {
        throw std::runtime_error("Failed to register plugin manager"_tr());
    }
}

} // namespace lse

LL_REGISTER_MOD(lse::LegacyScriptEngine, lse::LegacyScriptEngine::getInstance());
