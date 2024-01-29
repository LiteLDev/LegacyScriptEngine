add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_requires(
    "demangler v2.0.0",
    "dyncall 1.4",
    "legacymoney 0.1.5",
    "legacyparticleapi 0.1.1",
    "levilamina 0.6.0",
    "lightwebsocketclient 1.0.0",
    "magic_enum v0.9.0",
    -- "mariadb-connector-c 3.3.4",
    "simpleini v4.19",
    "sqlite3 3.43.0+200",
    "toml++ v3.4.0"
)
add_requires("cpp-httplib v0.14.0", {configs = {ssl=true, zlib=true}})
add_requires("scriptx", {configs={backend=get_config("backend")}})

set_runtimes("MD") -- For compatibility with the /MD build configuration of ScriptX.

option("backend")
    set_default("lua")
    set_values("lua", "quickjs")

package("quickjs")
    add_urls("https://github.com/LiteLDev/ScriptX/releases/download/prebuilt/quickjs.zip")
    add_versions("latest", "af0c38b0cf80aa1deb58e727e408477fffcc6f5f57da537dffc335861d652ed0")

    on_install(function (package)
        os.cp("*", package:installdir())
    end)

package("scriptx")
    add_configs("backend", {default = "lua", values = {"lua", "quickjs"}})
    add_includedirs(
        "src/include/"
    )
    add_urls("https://github.com/LiteLDev/ScriptX/releases/download/prebuilt/scriptx.zip")
    add_versions("latest", "dd5fb21370a59f38e4c33f48f4a6eecb25692283e4d49bbee983453e05b128ab")

    on_install(function (package)
        os.cp("*", package:installdir())
    end)

    on_load(function (package)
        local backend = package:config("backend")

        local deps = {
            lua = "lua v5.4.6",
            quickjs = "quickjs",
        }

        local scriptx_backends = {
            lua = "Lua",
            quickjs = "QuickJs",
        }

        print("Using ScriptX config: backend=" .. backend .. ", scriptx_backend=" .. scriptx_backends[backend])
        
        package:add("defines", "SCRIPTX_BACKEND=" .. scriptx_backends[backend])
        package:add("defines", "SCRIPTX_BACKEND_TRAIT_PREFIX=../backend/" .. scriptx_backends[backend] .. "/trait/Trait")
        package:add("deps", deps[backend])
        package:add("links", "scriptx_" .. scriptx_backends[backend])
    end)

target("legacy-script-engine")
    add_cxflags(
        "/EHa",
        "/utf-8"
    )
    add_defines(
        "_HAS_CXX23=1", -- To enable C++23 features.
        "CPPHTTPLIB_OPENSSL_SUPPORT", -- To enable SSL support for cpp-httplib.
        "LEGACY_SCRIPT_ENGINE_BACKEND=$(backend)", -- To use the backend specified by the user.
        "NOMINMAX", -- To avoid conflicts with std::min and std::max.
        "UNICODE" -- To enable Unicode support.
    )
    add_files(
        "src/**.cpp"
    )
    add_includedirs(
        "src"
    )
    add_packages(
        "cpp-httplib",
        "demangler",
        "dyncall",
        "legacymoney",
        "legacyparticleapi",
        "levilamina",
        "lightwebsocketclient",
        "magic_enum",
        -- "mariadb-connector-c",
        "scriptx",
        "simpleini",
        "sqlite3",
        "toml++"
    )
    add_shflags(
        "/DELAYLOAD:bedrock_server.dll" -- To use forged symbols of SymbolProvider.
    )
    set_basename("legacy-script-engine-$(backend)")
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("cxx20")

    after_build(function (target)
        local plugin_packer = import("scripts.after_build")

        local plugin_define = {
            pluginName = target:basename(),
            pluginFile = path.filename(target:targetfile()),
        }
        
        plugin_packer.pack_plugin(target,plugin_define)
    end)
