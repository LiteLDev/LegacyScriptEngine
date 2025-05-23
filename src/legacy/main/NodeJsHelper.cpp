#include <filesystem>
#pragma warning(disable : 4251)

#include "api/EventAPI.h"
#include "engine/EngineManager.h"
#include "engine/EngineOwnData.h"
#include "engine/RemoteCall.h"
#include "fmt/format.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/io/FileUtils.h"
#include "ll/api/io/LogLevel.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/service/ServerInfo.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "ll/api/utils/StringUtils.h"
#include "main/Global.h"
#include "main/NodeJsHelper.h"
#include "uv/uv.h"
#include "v8/v8.h"

#include <functional>

using ll::chrono_literals::operator""_tick;

// pre-declare
extern void BindAPIs(ScriptEngine* engine);

namespace NodeJsHelper {

bool                     nodeJsInited = false;
std::vector<std::string> args;
std::vector<std::string> exec_args;

std::unique_ptr<node::MultiIsolatePlatform>                                               platform = nullptr;
std::unordered_map<script::ScriptEngine*, node::Environment*>                             environments;
std::unordered_map<script::ScriptEngine*, std::unique_ptr<node::CommonEnvironmentSetup>>* setups =
    new std::unordered_map<script::ScriptEngine*, std::unique_ptr<node::CommonEnvironmentSetup>>();
std::unordered_map<node::Environment*, bool> isRunning;
std::vector<node::Environment*>              uvLoopTask;

bool initNodeJs() {
    // Init NodeJs
    WCHAR buf[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, buf);
    auto  path  = ll::string_utils::wstr2str(buf) + "\\bedrock_server_mod.exe";
    char* cPath = (char*)path.c_str();
    uv_setup_args(1, &cPath);
    args        = {path};
    auto result = node::InitializeOncePerProcess(
        args,
        {node::ProcessInitializationFlags::kNoInitializeV8,
         node::ProcessInitializationFlags::kNoInitializeNodeV8Platform}
    );
    exec_args = result->exec_args();
    if (result->exit_code() != 0) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Failed to initialize node! NodeJs plugins won't be loaded"
        );
        return false;
    }

    // Init V8
    using namespace v8;
    platform = node::MultiIsolatePlatform::Create(std::thread::hardware_concurrency());
    V8::InitializePlatform(platform.get());
    V8::Initialize();

    nodeJsInited = true;
    return true;
}

void shutdownNodeJs() {
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    node::TearDownOncePerProcess();
}

script::ScriptEngine* newEngine() {
    if (!nodeJsInited && !initNodeJs()) {
        return nullptr;
    }
    std::vector<std::string>                      errors;
    std::unique_ptr<node::CommonEnvironmentSetup> setup = node::CommonEnvironmentSetup::Create(
        platform.get(),
        &errors,
        args,
        exec_args,
        node::EnvironmentFlags::kOwnsProcessState
    );
    // if kOwnsInspector set, inspector_agent.cc:681
    // CHECK_EQ(start_io_thread_async_initialized.exchange(true), false) fail!

    if (!setup) {
        for (const std::string& err : errors)
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                "CommonEnvironmentSetup Error: {}",
                err.c_str()
            );
        return nullptr;
    }
    v8::Isolate*       isolate = setup->isolate();
    node::Environment* env     = setup->env();

    v8::Locker         locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope    handle_scope(isolate);
    v8::Context::Scope context_scope(setup->context());

    script::ScriptEngine* engine = new script::ScriptEngineImpl({}, isolate, setup->context(), false);

    lse::LegacyScriptEngine::getInstance().getSelf().getLogger().debug(
        "Initialize ScriptEngine for node.js [{}]",
        (void*)engine
    );
    environments[engine] = env;
    (*setups)[engine]    = std::move(setup);
    isRunning[env]       = true;

    node::AddEnvironmentCleanupHook(
        isolate,
        [](void* arg) {
            static_cast<script::ScriptEngine*>(arg)->destroy();
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().debug(
                "Destroy ScriptEngine for node.js [{}]",
                arg
            );
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().debug("Destroy EnvironmentCleanupHook");
        },
        engine
    );
    return engine;
}

bool loadPluginCode(script::ScriptEngine* engine, std::string entryScriptPath, std::string pluginDirPath, bool esm) {
    // Process requireDir
    if (!pluginDirPath.ends_with('/')) pluginDirPath += "/";

    // check if entryScriptPath is not absolute path
    if (auto path = std::filesystem::path(entryScriptPath); !path.is_absolute()) {
        entryScriptPath = std::filesystem::absolute(path).string();
    }
    if (auto path = std::filesystem::path(pluginDirPath); !path.is_absolute()) {
        pluginDirPath = std::filesystem::absolute(path).string();
    }
    pluginDirPath   = ll::string_utils::replaceAll(pluginDirPath, "\\", "/");
    entryScriptPath = ll::string_utils::replaceAll(entryScriptPath, "\\", "/");

    // Find setup
    auto it = setups->find(engine);
    if (it == setups->end()) return false;

    auto env = it->second->env();

    try {
        using namespace v8;
        EngineScope enter(engine);

        string compiler = R"(
            ll.import=ll.imports;
            ll.export=ll.exports;)";

        if (esm) {
            compiler += fmt::format(
                R"(
                    Promise.all([import("url"), import("util")])
                        .then(([url, util]) => {{
                            const moduleUrl = url.pathToFileURL("{1}").href;
                            import(moduleUrl).catch((error) => {{
                                logger.error(`Failed to load ESM module: `, util.inspect(error));
                                process.exit(1);
                            }});
                        }})
                        .catch((error) => {{
                            console.error(`Failed to import "url" or "util" module:`, error);
                            process.exit(1);
                        }});
                )",
                pluginDirPath,
                entryScriptPath
            );
        } else {
            compiler += fmt::format(
                R"(
                    const __Path = require("path");
                    const __PluginPath = __Path.join("{0}");
                    const __PluginNodeModulesPath = __Path.join(__PluginPath, "node_modules");

                    __dirname = __PluginPath;
                    __filename = "{1}";
                    (function ReplaceRequire() {{
                        const PublicModule = require('module').Module;
                        const OriginalResolveLookupPaths = PublicModule._resolveLookupPaths;
                        PublicModule._resolveLookupPaths = function (request, parent) {{
                            let result = OriginalResolveLookupPaths.call(this, request, parent);
                            if (Array.isArray(result)) {{
                                result.push(__PluginNodeModulesPath);
                                result.push(__PluginPath);
                            }}
                            return result;
                        }};
                        require = PublicModule.createRequire(__PluginPath);
                    }})();
                    require("{1}");
                )",
                pluginDirPath,
                entryScriptPath
            );
        }

        // Set exit handler
        node::SetProcessExitHandler(env, [](node::Environment* env_, int exit_code) {
            auto engine = getEngine(env_);
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().log(
                exit_code == 0 ? ll::io::LogLevel::Debug : ll::io::LogLevel::Error,
                "NodeJs plugin {} exited with code {}.",
                getEngineData(engine)->pluginName,
                exit_code
            );
            stopEngine(engine);
        });

        // Load code
        MaybeLocal<v8::Value> loadenv_ret = node::LoadEnvironment(env, compiler);
        if (loadenv_ret.IsEmpty()) // There has been a JS exception.
        {
            node::Stop(env);
            uv_stop(it->second->event_loop());
            return false;
        }

        // Start libuv event loop
        uvLoopTask.push_back(env);
        ll::coro::keepThis(
            [engine, env, isRunningMap{&isRunning}, eventLoop{it->second->event_loop()}]() -> ll::coro::CoroTask<> {
                using namespace ll::chrono_literals;
                while (std::find(uvLoopTask.begin(), uvLoopTask.end(), env) != uvLoopTask.end()) {
                    co_await 2_tick;
                    if (!(ll::getGamingStatus() != ll::GamingStatus::Running) && (*isRunningMap)[env]) {
                        EngineScope enter(engine);
                        uv_run(eventLoop, UV_RUN_NOWAIT);
                    }
                    if ((ll::getGamingStatus() != ll::GamingStatus::Running)) {
                        uv_stop(eventLoop);
                        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().debug("Destroy ServerStopping");
                    }
                }
            }
        ).launch(ll::thread::ServerThreadExecutor::getDefault());

        return true;
    } catch (...) {
        return false;
    }
}

node::Environment* getEnvironmentOf(script::ScriptEngine* engine) {
    auto it = environments.find(engine);
    if (it == environments.end()) return nullptr;
    return it->second;
}

v8::Isolate* getIsolateOf(script::ScriptEngine* engine) {
    auto it = setups->find(engine);
    if (it == setups->end()) return nullptr;
    return it->second->isolate();
}

bool stopEngine(node::Environment* env) {
    if (!env) return false;
    try {
        // Set flag
        isRunning[env] = false;

        // Stop code executing
        node::Stop(env);

        // Stop libuv event loop
        auto it = std::find(uvLoopTask.begin(), uvLoopTask.end(), env);
        if (it != uvLoopTask.end()) {
            uvLoopTask.erase(it);
        }

        return true;
    } catch (...) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("Fail to stop engine {}", (void*)env);
        ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
        return false;
    }
}

bool stopEngine(script::ScriptEngine* engine) {
    auto env = NodeJsHelper::getEnvironmentOf(engine);
    return stopEngine(env);
}

script::ScriptEngine* getEngine(node::Environment* env) {
    for (auto& [engine, environment] : environments)
        if (env == environment) return engine;
    return nullptr;
}

std::string findEntryScript(const std::string& dirPath) {
    auto dirPath_obj = std::filesystem::path(dirPath);

    std::filesystem::path packageFilePath = dirPath_obj / "package.json";
    if (!std::filesystem::exists(packageFilePath)) return "";

    try {
        std::ifstream  file(ll::string_utils::u8str2str(packageFilePath.make_preferred().u8string()));
        nlohmann::json j;
        file >> j;
        std::string entryFile = "index.js";
        if (j.contains("main")) {
            entryFile = j["main"].get<std::string>();
        }
        auto entryPath = std::filesystem::canonical(dirPath_obj / std::filesystem::path(entryFile));
        if (!std::filesystem::exists(entryPath)) return "";
        else return ll::string_utils::u8str2str(entryPath.u8string());
    } catch (...) {
        return "";
    }
}

std::string getPluginPackageName(const std::string& dirPath) {
    auto dirPath_obj = std::filesystem::path(dirPath);

    std::filesystem::path packageFilePath = dirPath_obj / std::filesystem::path("package.json");
    if (!std::filesystem::exists(packageFilePath)) return "";

    try {
        std::ifstream  file(ll::string_utils::u8str2str(packageFilePath.make_preferred().u8string()));
        nlohmann::json j;
        file >> j;
        std::string packageName{};
        if (j.contains("name")) {
            packageName = j["name"].get<std::string>();
        }
        return packageName;
    } catch (...) {
        return "";
    }
}

bool doesPluginPackHasDependency(const std::string& dirPath) {
    auto dirPath_obj = std::filesystem::path(dirPath);

    std::filesystem::path packageFilePath = dirPath_obj / std::filesystem::path("package.json");
    if (!std::filesystem::exists(packageFilePath)) return false;

    try {
        std::ifstream  file(ll::string_utils::u8str2str(packageFilePath.make_preferred().u8string()));
        nlohmann::json j;
        file >> j;
        if (j.contains("dependencies")) {
            return true;
        }
        return false;
    } catch (...) {
        return false;
    }
}

bool isESModulesSystem(const std::string& dirPath) {
    auto dirPath_obj = std::filesystem::path(dirPath);

    std::filesystem::path packageFilePath = dirPath_obj / std::filesystem::path("package.json");
    if (!std::filesystem::exists(packageFilePath)) return false;

    try {
        std::ifstream  file(ll::string_utils::u8str2str(packageFilePath.make_preferred().u8string()));
        nlohmann::json j;
        file >> j;
        if (j.contains("type") && j["type"] == "module") {
            return true;
        }
        return false;
    } catch (...) {
        return false;
    }
}

bool processConsoleNpmCmd(const std::string& cmd) {
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS
    if (cmd.starts_with("npm ")) {
        executeNpmCommand(cmd);
        return false;
    } else {
        return true;
    }
#else
    return true;
#endif
}

int executeNpmCommand(const std::string& cmd, std::string workingDir) {
    if (!nodeJsInited && !initNodeJs()) {
        return -1;
    }
    std::string engineDir =
        ll::string_utils::u8str2str(lse::LegacyScriptEngine::getInstance().getSelf().getModDir().u8string());
    if (workingDir.empty()) workingDir = engineDir;
    std::vector<std::string>                      errors;
    std::unique_ptr<node::CommonEnvironmentSetup> setup = node::CommonEnvironmentSetup::Create(
        platform.get(),
        &errors,
        args,
        exec_args,
        node::EnvironmentFlags::kOwnsProcessState
    );
    // if kOwnsInspector set, inspector_agent.cc:681
    // CHECK_EQ(start_io_thread_async_initialized.exchange(true), false) fail!

    if (!setup) {
        for (const std::string& err : errors)
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                "CommonEnvironmentSetup Error: {}",
                err.c_str()
            );
        return -1;
    }
    v8::Isolate*       isolate   = setup->isolate();
    node::Environment* env       = setup->env();
    int                exit_code = 0;

    // Process workingDir
    workingDir = ll::string_utils::replaceAll(workingDir, "\\", "/");

    {
        using namespace v8;
        v8::Locker         locker(isolate);
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope    handle_scope(isolate);
        v8::Context::Scope context_scope(setup->context());

        std::string executeJs = fmt::format(
            R"(
                const engineDir = "{0}";
                const workingDir = "{1}";
                const command = "{2}";
                const oldCwd = process.cwd();
                const publicRequire = require("module").createRequire(
                    require("path").resolve(engineDir) + require("path").sep
                );
                process.chdir(workingDir);
                publicRequire("npm-js-interface")(command);
                process.chdir(oldCwd);
            )",
            engineDir,
            workingDir,
            cmd
        );

        try {
            node::SetProcessExitHandler(env, [&](node::Environment* env_, int exit_code) { node::Stop(env); });
            MaybeLocal<v8::Value> loadenv_ret = node::LoadEnvironment(env, executeJs);
            if (loadenv_ret.IsEmpty()) // There has been a JS exception.
                throw "error";
            exit_code = node::SpinEventLoop(env).FromMaybe(0);
        } catch (...) {
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
                "Fail to execute NPM command. Error occurs"
            );
            ll::error_utils::printCurrentException(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
        }
        node::Stop(env);
    }
    return exit_code;
}

} // namespace NodeJsHelper
