add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

if is_config("target_type", "server") then
    add_requires("levilamina main", {configs = {target_type = "server"}})
else
    add_requires("levilamina main", {configs = {target_type = "client"}})
end

add_requires("levibuildscript")

add_requires(
    "demangler",
    "dyncall",
    "fmt",
    "legacymoney 0.9.0-rc.1",
    "legacyparticleapi 0.9.0-rc.1",
    "legacyremotecall 0.9.0-rc.1",
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

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

option("backend")
    set_default("lua")
    set_values("lua", "quickjs", "python", "nodejs")

target("legacy-script-engine")
    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")
    add_cxflags("/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204","/Zm2000", {force = true})
    add_defines(
        "NOMINMAX",
        "UNICODE",
        "_AMD64_"
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
    set_exceptions("none")
    set_kind("shared")
    set_languages("cxx20")
    set_symbols("debug")

    if is_config("backend", "lua") then
        add_defines(
            "LEGACY_SCRIPT_ENGINE_BACKEND_LUA"
        )
        set_basename("legacy-script-engine-lua")
        after_build(function(target)
            local baselibPath = path.join(os.projectdir(), "src/baselib/BaseLib.lua")
            local langPath = path.join(os.projectdir(), "src/lang")
            local outputPath = path.join(os.projectdir(), "bin/" .. target:name())
            local baselibOutputPath = path.join(outputPath, "baselib")
            os.mkdir(baselibOutputPath)
            os.cp(baselibPath, baselibOutputPath)
            os.cp(langPath, outputPath)
        end)

    elseif is_config("backend", "quickjs") then
        add_defines(
            "LEGACY_SCRIPT_ENGINE_BACKEND_QUICKJS"
        )
        set_basename("legacy-script-engine-quickjs")
        after_build(function(target)
            local baselibPath = path.join(os.projectdir(), "src/baselib/BaseLib.js")
            local langPath = path.join(os.projectdir(), "src/lang")
            local outputPath = path.join(os.projectdir(), "bin/" .. target:name())
            local baselibOutputPath = path.join(outputPath, "baselib")
            os.mkdir(baselibOutputPath)
            os.cp(baselibPath, baselibOutputPath)
            os.cp(langPath, outputPath)
        end)

    elseif is_config("backend", "python") then
        add_defines(
            "LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON"
        )
        set_basename("legacy-script-engine-python")
        after_build(function(target)
            local baselibPath = path.join(os.projectdir(), "src/baselib/BaseLib.py")
            local langPath = path.join(os.projectdir(), "src/lang")
            local outputPath = path.join(os.projectdir(), "bin/" .. target:name())
            local baselibOutputPath = path.join(outputPath, "baselib")
            os.mkdir(baselibOutputPath)
            os.cp(baselibPath, baselibOutputPath)
            os.cp(langPath, outputPath)
        end)

    elseif is_config("backend", "nodejs") then
        add_defines(
            "LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS"
        )
        set_basename("legacy-script-engine-nodejs")
        after_build(function(target)
            local langPath = path.join(os.projectdir(), "src/lang")
            local outputPath = path.join(os.projectdir(), "bin/" .. target:name() .. "/lang")
            os.cp(langPath, outputPath)
        end)
    end

    add_files(
        "src/**.cpp"
    )
    add_includedirs(
        "src",
        "src/legacy"
    )