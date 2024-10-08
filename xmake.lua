add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

add_requires(
    "demangler",
    "dyncall",
    "fmt",
    "legacymoney 0.8.3",
    "legacyparticleapi 0.8.3",
    "legacyremotecall 0.8.3",
    "levilamina 0.13.5",
    "lightwebsocketclient",
    "magic_enum",
    "nlohmann_json",
    "simpleini",
    "sqlite3 3.43.0+200",
    "toml++"
)
add_requires("openssl 1.1.1-w", {configs = {shared = false}})
add_requires("cpp-httplib 0.14.3", {configs = {ssl = true}})

if is_config("backend", "lua") then
    add_requires("scriptx main", {configs={backend="Lua"}})

elseif is_config("backend", "quickjs") then
    add_requires("scriptx main", {configs={backend="QuickJs"}})

elseif is_config("backend", "python") then
    add_requires("scriptx main", {configs={backend="Python"}})

elseif is_config("backend", "nodejs") then
    add_requires("scriptx main", {configs={backend="V8"}})

end

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("backend")
    set_default("lua")
    set_values("lua", "quickjs", "python", "nodejs")

target("legacy-script-engine")
    add_cxflags(
        "/EHa",
        "/utf-8",
        "/sdl",
        "/W4"
    )
    add_defines(
        "NOMINMAX",
        "UNICODE",
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
        "nlohmann_json",
        "scriptx",
        "simpleini",
        "sqlite3",
        "toml++"
    )
    add_shflags(
        "/DELAYLOAD:bedrock_server.dll"
    )
    set_exceptions("none")
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

    elseif is_config("backend", "nodejs") then
        add_defines(
            "LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS"
        )
        set_basename("legacy-script-engine-nodejs")

    end

    after_build(function (target)
        local plugin_packer = import("scripts.after_build")

        local plugin_define = {
            pluginName = target:basename(),
            pluginFile = path.filename(target:targetfile()),
        }
        
        plugin_packer.pack_plugin(target,plugin_define)
    end)

