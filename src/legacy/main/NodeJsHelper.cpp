#include <ll/api/chrono/GameChrono.h>
#include <ll/api/service/ServerInfo.h>

#pragma warning(disable : 4251)
#include "main/Configs.h"
#if defined(LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS)
#include "api/CommandAPI.h"
#include "api/CommandCompatibleAPI.h"
#include "api/EventAPI.h"
#include "engine/EngineManager.h"
#include "engine/EngineOwnData.h"
#include "engine/RemoteCall.h"
#include "main/Global.h"
#include "main/NodeJsHelper.h"

#include <functional>
#include <ll/api/io/FileUtils.h>
#include <ll/api/schedule/Scheduler.h>
#include <ll/api/schedule/Task.h>
#include <ll/api/utils/StringUtils.h>
#include <uv/uv.h>
#include <v8/v8.h>

ll::schedule::ServerTimeScheduler nodeScheduler;
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
std::unordered_map<node::Environment*, bool>   isRunning;
std::unordered_map<node::Environment*, uint64> uvLoopTask;

bool initNodeJs() {
    // Init NodeJs
    WCHAR buf[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, buf);
    auto  path  = ll::string_utils::wstr2str(buf) + "\\bedrock_server_mod.exe";
    char* cPath = (char*)path.c_str();
    uv_setup_args(1, &cPath);
    args = {path};
    std::vector<std::string> errors;
    auto                     exitCode = node::InitializeNodeWithArgs(&args, &exec_args, &errors);
    if (exitCode != 0) {
        lse::getSelfPluginInstance().getLogger().error("Failed to initialize node! NodeJs plugins won't be loaded");
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
    v8::V8::ShutdownPlatform();
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
            lse::getSelfPluginInstance().getLogger().error("CommonEnvironmentSetup Error: {}", err.c_str());
        return nullptr;
    }
    v8::Isolate*       isolate = setup->isolate();
    node::Environment* env     = setup->env();

    v8::Locker         locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope    handle_scope(isolate);
    v8::Context::Scope context_scope(setup->context());

    script::ScriptEngine* engine = new script::ScriptEngineImpl({}, isolate, setup->context(), false);

    lse::getSelfPluginInstance().getLogger().debug("Initialize ScriptEngine for node.js [{}]", (void*)engine);
    environments[engine] = env;
    (*setups)[engine]    = std::move(setup);
    isRunning[env]       = true;

    node::AddEnvironmentCleanupHook(
        isolate,
        [](void* arg) {
            static_cast<script::ScriptEngine*>(arg)->destroy();
            lse::getSelfPluginInstance().getLogger().debug("Destory ScriptEngine for node.js [{}]", arg);
            lse::getSelfPluginInstance().getLogger().debug("Destroy EnvironmentCleanupHook");
        },
        engine
    );
    return engine;
}

bool loadPluginCode(script::ScriptEngine* engine, std::string entryScriptPath, std::string pluginDirPath) {
    auto mainScripts = ll::file_utils::readFile(ll::string_utils::str2u8str(entryScriptPath));
    if (!mainScripts) {
        return false;
    }

    // Process requireDir
    if (!pluginDirPath.ends_with('/')) pluginDirPath += "/";
    pluginDirPath   = ll::string_utils::replaceAll(pluginDirPath, "\\", "/");
    entryScriptPath = ll::string_utils::replaceAll(entryScriptPath, "\\", "/");

    // Find setup
    auto it = setups->find(engine);
    if (it == setups->end()) return false;

    auto isolate = it->second->isolate();
    auto env     = it->second->env();

    try {
        using namespace v8;
        EngineScope enter(engine);

        string executeJs = "const __LLSE_PublicRequire = "
                           "require('module').createRequire(process.cwd() + '/"
                         + pluginDirPath + "');"
                         + "const __LLSE_PublicModule = require('module'); "
                           "__LLSE_PublicModule.exports = {};"
                         + "ll.export = ll.exports; ll.import = ll.imports; "

                         + "(function (exports, require, module, __filename, __dirname) { " + mainScripts.value()
                         + "\n})({}, __LLSE_PublicRequire, __LLSE_PublicModule, '" + entryScriptPath + "', '"
                         + pluginDirPath + "'); "; // TODO __filename & __dirname need to be reviewed
        // TODO: ESM Support

        // Set exit handler
        node::SetProcessExitHandler(env, [](node::Environment* env_, int exit_code) { stopEngine(getEngine(env_)); });

        // Load code
        MaybeLocal<v8::Value> loadenv_ret = node::LoadEnvironment(env, executeJs.c_str());
        if (loadenv_ret.IsEmpty()) // There has been a JS exception.
        {
            node::Stop(env);
            uv_stop(it->second->event_loop());
            return false;
        }

        // Start libuv event loop
        uvLoopTask[env] =
            nodeScheduler
                .add<ll::schedule::RepeatTask>(
                    2_tick,
                    [engine, env, isRunningMap{&isRunning}, eventLoop{it->second->event_loop()}]() {
                        if (!(ll::getServerStatus() != ll::ServerStatus::Running) && (*isRunningMap)[env]) {
                            EngineScope enter(engine);
                            uv_run(eventLoop, UV_RUN_NOWAIT);
                        }
                        if ((ll::getServerStatus() != ll::ServerStatus::Running)) {
                            uv_stop(eventLoop);
                            lse::getSelfPluginInstance().getLogger().debug("Destroy ServerStopping");
                        }
                    }
                )
                ->getId();

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
        auto it = uvLoopTask.find(env);
        if (it != uvLoopTask.end()) {
            nodeScheduler.remove(it->second);
        }

        return true;
    } catch (...) {
        lse::getSelfPluginInstance().getLogger().error("Fail to stop engine {}", (void*)env);
        return false;
    }
}

bool stopEngine(script::ScriptEngine* engine) {
    lse::getSelfPluginInstance().getLogger().info("NodeJs plugin {} exited.", ENGINE_GET_DATA(engine)->pluginName);
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
        std::string packageName = "";
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

bool processConsoleNpmCmd(const std::string& cmd) {
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS
    if (cmd.starts_with("npm ")) {
        executeNpmCommand(cmd);
        return false;
    } else return true;
#else
    return true;
#endif
}

int executeNpmCommand(std::string cmd, std::string workingDir) {
    if (!nodeJsInited && !initNodeJs()) {
        return -1;
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
            lse::getSelfPluginInstance().getLogger().error("CommonEnvironmentSetup Error: {}", err.c_str());
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

        string executeJs = "const oldCwd = process.cwd();"
                           "const publicRequire = require('module').createRequire(oldCwd + "
                           "'/plugins/legacy-script-engine-nodejs/');"
                           "require('process').chdir('"
                         + workingDir + "');" + "publicRequire('npm-js-interface')('" + cmd + "');"
                         + "require('process').chdir(oldCwd);";

        try {
            node::SetProcessExitHandler(env, [&](node::Environment* env_, int exit_code) { node::Stop(env); });
            MaybeLocal<v8::Value> loadenv_ret = node::LoadEnvironment(env, executeJs.c_str());
            if (loadenv_ret.IsEmpty()) // There has been a JS exception.
                throw "error";
            exit_code = node::SpinEventLoop(env).FromMaybe(1);
        } catch (...) {
            lse::getSelfPluginInstance().getLogger().error("Fail to execute NPM command. Error occurs");
        }
    }

    node::Stop(env);
    return exit_code;
}

} // namespace NodeJsHelper

#endif
