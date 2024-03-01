add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

add_requires(
    "demangler",
    "dyncall",
    "fmt",
    "legacymoney 0.4.0",
    "legacyparticleapi 0.4.0",
    "legacyremotecall 0.4.0",
    "levilamina 0.9.1",
    "lightwebsocketclient",
    "magic_enum",
    "more-events develop",
    "nlohmann_json",
    "openssl 1.1.1-w",
    "simpleini",
    "sqlite3 3.43.0+200",
    "toml++"
)
add_requires("cpp-httplib v0.14.0", {configs={ssl=true, zlib=true}})

if is_config("backend", "lua") then
    add_requires("scriptx main", {configs={backend="Lua"}})

elseif is_config("backend", "quickjs") then
    add_requires("scriptx main", {configs={backend="QuickJs"}})

elseif is_config("backend", "python") then
    add_requires("scriptx main", {configs={backend="Python"}})
    add_requires("microsoft-detours")

end

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("backend")
    set_default("lua")
    set_values("lua", "quickjs", "python")

package("more-events")
    add_urls("https://github.com/LiteLDev/MoreEvents.git")

    add_deps("levilamina 0.9.1")

    on_install(function (package)
        import("package.tools.xmake").install(package)
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
        "UNICODE", -- To enable Unicode support.
        "_AMD64_"
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
        "legacyremotecall",
        "levilamina",
        "lightwebsocketclient",
        "magic_enum",
        "more-events",
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

    elseif is_config("backend", "python") then
        add_defines(
            "LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON"
        )
        set_basename("legacy-script-engine-python")
        add_packages("microsoft-detours")

    end

    after_build(function (target)
        local plugin_packer = import("scripts.after_build")

        local plugin_define = {
            pluginName = target:basename(),
            pluginFile = path.filename(target:targetfile()),
        }
        
        plugin_packer.pack_plugin(target,plugin_define)
    end)

