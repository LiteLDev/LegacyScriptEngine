add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_requires("cpp-httplib v0.14.0", {configs={shared=false, ssl=true, zlib=true}})
add_requires("demangler v2.0.0", {configs={shared=false}})
add_requires("dyncall 1.4", {configs={shared=false}})
add_requires("fmt 10.1.1", {configs={shared=false}})
add_requires("legacymoney 0.1.5", {configs={shared=false}})
add_requires("legacyparticleapi 0.1.1", {configs={shared=false}})
add_requires("levilamina 0.6.3", {configs={shared=false}})
add_requires("lightwebsocketclient 1.0.0", {configs={shared=false}})
add_requires("magic_enum v0.9.0", {configs={shared=false}})
add_requires("nlohmann_json 3.11.2", {configs={shared=false}})
add_requires("scriptx", {configs={shared=false, backend=get_config("backend")}})
add_requires("simpleini v4.19", {configs={shared=false}})
add_requires("sqlite3 3.43.0+200", {configs={shared=false}})
add_requires("toml++ v3.4.0", {configs={shared=false}})

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
        "NOMINMAX", -- To avoid conflicts with std::min and std::max.
        "UNICODE" -- To enable Unicode support.
    )
    add_files(
        "src/**.cpp"
    )
    add_includedirs(
        "src",
        "src/legacy"
    )
    add_packages(
        "cpp-httplib",
        "demangler",
        "dyncall",
        "fmt",
        "legacymoney",
        "legacyparticleapi",
        "levilamina",
        "lightwebsocketclient",
        "magic_enum",
        "nlohmann_json",
        "scriptx",
        "simpleini",
        "sqlite3",
        "toml++"
    )
    add_shflags(
        "/DELAYLOAD:bedrock_server.dll" -- To use forged symbols of SymbolProvider.
    )
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("cxx20")

    if is_config("backend", "lua") then
        add_defines(
            "LEGACY_SCRIPT_ENGINE_BACKEND_LUA"
        )
        set_basename("legacy-script-engine-lua")

    elseif is_config("backend", "quickjs") then
        add_defines(
            "LEGACY_SCRIPT_ENGINE_BACKEND_QUICKJS"
        )
        set_basename("legacy-script-engine-quickjs")

    end

    after_build(function (target)
        local plugin_packer = import("scripts.after_build")

        local plugin_define = {
            pluginName = target:basename(),
            pluginFile = path.filename(target:targetfile()),
        }
        
        plugin_packer.pack_plugin(target,plugin_define)
    end)
