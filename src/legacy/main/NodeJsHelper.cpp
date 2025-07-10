#include "main/NodeJsHelper.h"

#include "engine/EngineOwnData.h"
#include "fmt/format.h"
#include "ll/api/Expected.h"
#include "ll/api/base/Containers.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/io/Logger.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "ll/api/utils/ErrorUtils.h"
#include "ll/api/utils/StringUtils.h"
#include "lse/Entry.h"
#include "nlohmann/json.hpp"
#include "utils/Utils.h"
#include "uv/uv.h"
#include "v8/v8.h" // IWYU pragma: keep

#define NODE_LIBRARY_NAME_W   L"libnode.dll"
#define NODE_HOST_BINARY_NAME "node.exe"

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
std::set<node::Environment*>                 uvLoopTask;

ll::Expected<> PatchDelayImport(HMODULE hAddon, HMODULE hLibNode) {
    BYTE* base = (BYTE*)hAddon;
    auto  dos  = (PIMAGE_DOS_HEADER)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        return ll::makeStringError("Invalid DOS signature.");
    }
    auto nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        return ll::makeStringError("Invalid NT signature.");
    }
    DWORD rva = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress;
    if (!rva) {};
    auto pDesc = (PIMAGE_DELAYLOAD_DESCRIPTOR)(base + rva);
    for (; pDesc->DllNameRVA; ++pDesc) {
        char* szDll = (char*)(base + pDesc->DllNameRVA);
        if (_stricmp(szDll, NODE_HOST_BINARY_NAME) != 0) continue;

        auto pIAT = (PIMAGE_THUNK_DATA)(base + pDesc->ImportAddressTableRVA);
        auto pINT = (PIMAGE_THUNK_DATA)(base + pDesc->ImportNameTableRVA);

        for (; pIAT->u1.Function; ++pIAT, ++pINT) {
            FARPROC f = nullptr;
            if (pINT->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
                // Import by Ordinal
                WORD ordinal = IMAGE_ORDINAL(pINT->u1.Ordinal);
                f            = GetProcAddress(hLibNode, MAKEINTRESOURCEA(ordinal));
            } else {
                // Import by name
                auto name = (PIMAGE_IMPORT_BY_NAME)(base + pINT->u1.AddressOfData);
                f         = GetProcAddress(hLibNode, name->Name);
            }
            if (f) {
                DWORD oldProt;
                VirtualProtect(&pIAT->u1.Function, sizeof(void*), PAGE_READWRITE, &oldProt);
                pIAT->u1.Function = reinterpret_cast<decltype(pIAT->u1.Function)>(f);
                VirtualProtect(&pIAT->u1.Function, sizeof(void*), oldProt, &oldProt);
            }
        }
        break;
    }
    return {};
}

ll::DenseSet<HMODULE> cachedModules{};

// patch in node?
LL_STATIC_HOOK(
    PatchDelayImportHook,
    HookPriority::Normal,
    LoadLibraryExW,
    HMODULE,
    LPCWSTR lpLibFileName,
    HANDLE  hFile,
    DWORD   dwFlags
) {
    auto hAddon = origin(lpLibFileName, hFile, dwFlags);
    if (!cachedModules.contains(hAddon)) {
        cachedModules.emplace(hAddon);
        if (std::wstring_view(lpLibFileName).ends_with(L".node")) {
            static HMODULE hLibNode = GetModuleHandle(NODE_LIBRARY_NAME_W);
            if (!(hAddon && hLibNode)) return hAddon;
            auto res = PatchDelayImport(hAddon, hLibNode);
            if (res) return hAddon;
            res.error().log(lse::LegacyScriptEngine::getInstance().getSelf().getLogger());
        }
    }
    return hAddon;
}

std::unique_ptr<ll::memory::HookRegistrar<PatchDelayImportHook>> hook{};

bool initNodeJs() {
    if (lse::LegacyScriptEngine::getInstance().getConfig().fixOldAddon.value_or(true)) {
        hook = std::make_unique<ll::memory::HookRegistrar<PatchDelayImportHook>>();
    }
    // Init NodeJs
    auto  path  = ll::string_utils::u8str2str(ll::sys_utils::getModulePath(nullptr).value().u8string());
    char* cPath = (char*)path.c_str();
    uv_setup_args(1, &cPath);
    auto full_args = std::vector<std::string>{path};
#if defined(LSE_DEBUG) || defined(LSE_TEST)
    full_args.insert(
        full_args.end(),
        {"--experimental-strip-types", "--enable-source-maps", "--disable-warning=ExperimentalWarning"}
    );
#endif
    auto result = node::InitializeOncePerProcess(
        full_args,
        {node::ProcessInitializationFlags::kNoInitializeV8,
         node::ProcessInitializationFlags::kNoInitializeNodeV8Platform}
    );
    if (result->exit_code() != 0) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Failed to initialize node! NodeJs plugins won't be loaded"
        );
        for (const std::string& error : result->errors())
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(error);
        return false;
    }
    args      = result->args();
    exec_args = result->exec_args();

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

        std::string compiler = R"(
            ll.import=ll.imports;
            ll.export=ll.exports;)";

        if (esm) {
            compiler += fmt::format(
                R"(
                    const moduleUrl = require("url").pathToFileURL("{1}").href;
                    const {{ promise, resolve, reject }} = Promise.withResolvers();
                    let timeout = false;
                    import(moduleUrl)
                        .then(() => resolve())
                        .catch((error) => {{
                            const msg = `Failed to load ESM module: ${{require("util").inspect(error)}}`;
                            if (timeout) logger.error(msg), process.exit(1);
                            else resolve(msg);
                        }});
                    const timer = setTimeout(() => (timeout = true) && resolve(), 900);
                    return promise.finally(() => clearTimeout(timer));
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
                    try{{
                        require("{1}");
                    }}catch(error){{
                        return require("util").inspect(error);
                    }};
                    return;
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
        bool                  loadFailed  = loadenv_ret.IsEmpty();

        auto& logger = lse::LegacyScriptEngine::getInstance().getSelf().getLogger();

        if (!loadFailed) {
            v8::Local<v8::Value> errorMsg = loadenv_ret.ToLocalChecked();
            if (loadenv_ret.ToLocalChecked()->IsPromise()) {
                // wait for module loaded
                auto promise  = loadenv_ret.ToLocalChecked().As<v8::Promise>();
                auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds{1};
                while (promise->State() == v8::Promise::kPending && std::chrono::steady_clock::now() <= deadline) {
                    uv_run(it->second->event_loop(), UV_RUN_ONCE);
                    it->second->isolate()->PerformMicrotaskCheckpoint();
                }
                if (promise->State() == v8::Promise::kFulfilled) {
                    errorMsg = promise->Result();
                }
            }
            if (errorMsg->IsString()) {
                v8::String::Utf8Value value{it->second->isolate(), errorMsg};
                logger.error(std::string_view{*value, static_cast<size_t>(value.length())});
                loadFailed = true;
            }
        }
        if (loadFailed) {
            node::Stop(env);
            uv_stop(it->second->event_loop());
            return false;
        }
        // Start libuv event loop
        uvLoopTask.insert(env);
        ll::coro::keepThis(
            [engine,
             env,
             isolate{it->second->isolate()},
             isRunningMap{&isRunning},
             eventLoop{it->second->event_loop()}]() -> ll::coro::CoroTask<> {
                using namespace ll::chrono_literals;
                while (uvLoopTask.contains(env)) {
                    co_await 2_tick;
                    if (!(ll::getGamingStatus() != ll::GamingStatus::Running) && (*isRunningMap)[env]) {
                        EngineScope enter(engine);
                        // v8::MicrotasksScope microtaskScope(isolate, v8::MicrotasksScope::kRunMicrotasks);
                        uv_run(eventLoop, UV_RUN_NOWAIT);
                        // Manually perform microtasks because default MicrotasksPolicy is kExplicit
                        isolate->PerformMicrotaskCheckpoint();
                        // Or change MicrotasksPolicy to kScope and enter MicrotasksScope before uv_run
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
        auto it = uvLoopTask.find(env);
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
    if (cmd.starts_with("npm ") || cmd.starts_with("npx ")) {
        executeNpmCommand(SplitCmdLine(cmd));
        return false;
    } else {
        return true;
    }
#else
    return true;
#endif
}

int executeNpmCommand(std::vector<std::string> npmArgs, std::string workingDir) {
    if (!nodeJsInited && !initNodeJs()) {
        return -1;
    }
    std::string engineDir =
        ll::string_utils::u8str2str(lse::LegacyScriptEngine::getInstance().getSelf().getModDir().u8string());
    if (workingDir.empty()) workingDir = engineDir;

    auto npmPath = std::filesystem::absolute(engineDir) / "node_modules" / "npm" / "bin" / "npm-cli.js";
    std::vector<std::string>& env_args = npmArgs;
    if (!env_args.empty() && (env_args[0] == "npm" || env_args[0] == "npx")) {
        if (env_args[0] == "npx") {
            npmPath = std::filesystem::absolute(engineDir) / "node_modules" / "npm" / "bin" / "npx-cli.js";
        }
        env_args.erase(env_args.begin());
    }
    auto scriptPath = ll::string_utils::replaceAll(ll::string_utils::u8str2str(npmPath.u8string()), "\\", "/");
    env_args.insert(env_args.begin(), {args[0], scriptPath});

    std::vector<std::string> errors;

    std::unique_ptr<node::CommonEnvironmentSetup> setup = node::CommonEnvironmentSetup::Create(
        platform.get(),
        &errors,
        env_args,
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
                const engineDir = require("path").resolve("{0}") + require("path").sep;
                const workingDir = "{1}";
                const scriptPath = "{2}";
                const publicRequire = require("module").createRequire(engineDir);
                // Record states and restore at exit
                const oldCwd = process.cwd();
                const oldEnv = Object.entries(process.env).filter(([k]) => k.startsWith("npm_"));
                const oldTitle = process.title;
                process.once("exit", () => {{
                    Object.keys(process.env)
                        .filter((k) => k.startsWith("npm_"))
                        .forEach((k) => delete process.env[k]);
                    oldEnv.forEach(([k, v]) => (process.env[k] = v));
                    process.title = oldTitle;
                    process.chdir(oldCwd);
                }});
                // disable npm input
                function inputHandler(type, resolve, reject) {{
                    if (type === "read") {{
                        console.error("Input is not allow in server command.");
                        reject();
                    }}
                }}
                process.on("input", inputHandler);
                process.once("exit", () => process.off("input", inputHandler));

                process.chdir(workingDir);
                publicRequire(scriptPath);
            )",
            engineDir,
            workingDir,
            scriptPath
        );

        try {
            node::SetProcessExitHandler(env, [&](node::Environment*, int exit_code_) {
                exit_code = exit_code_;
                node::Stop(env);
            });
            MaybeLocal<v8::Value> loadenv_ret = node::LoadEnvironment(env, executeJs);
            if (loadenv_ret.IsEmpty()) // There has been a JS exception.
                throw std::runtime_error("Failed at LoadEnvironment");
            exit_code = node::SpinEventLoop(env).FromMaybe(exit_code);
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
