add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_requires(
    "demangler v2.0.0",
    "dyncall 1.4",
    "fmt 10.1.1",
    "legacymoney 0.2.0",
    "legacyparticleapi 0.2.0",
    "levilamina 0.7.2",
    "lightwebsocketclient 1.0.0",
    "magic_enum v0.9.0",
    "nlohmann_json 3.11.2",
    "openssl 1.1.1-w",
    "simpleini v4.19",
    "sqlite3 3.43.0+200",
    "toml++ v3.4.0",
    "legacyremotecall 0.2.0"
)
add_requires("cpp-httplib v0.14.0", {configs={shared=false, ssl=true, zlib=true}})

if is_config("backend", "lua") then
    add_requires("scriptx 3.2.0", {configs={backend="Lua"}})

elseif is_config("backend", "quickjs") then
    add_requires("scriptx 3.2.0", {configs={backend="QuickJs"}})

end

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("backend")
    set_default("lua")
    set_values("lua", "quickjs")

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
        "toml++",
        "legacyremotecall"
    )
    add_shflags(
        "/DELAYLOAD:bedrock_server.dll" -- To use forged symbols of SymbolProvider.
    )
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("cxx20")
    set_symbols("debug")

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
