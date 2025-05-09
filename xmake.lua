add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

if is_config("target_type", "server") then
    add_requires("levilamina e5676586bc0afdfd96e4529a416ea8f303b453e7", {configs = {target_type = "server"}})
else
    add_requires("levilamina e5676586bc0afdfd96e4529a416ea8f303b453e7", {configs = {target_type = "client"}})
end

add_requires("levibuildscript")

add_requires(
    "legacymoney 0.10.0",
    "legacyparticleapi 0.10.0",
    "legacyremotecall 0.10.0",
    "lightwebsocketclient 1.0.1",
    "magic_enum v0.9.7",
    "nlohmann_json v3.11.3",
    "simpleini v4.22",
    "sqlite3 3.43.0+200",
    "toml++ v3.4.0"
)

if is_config("backend", "lua") then
    add_requires("openssl 1.1.1-w")
    add_requires("mariadb-connector-c 3.3.9")
    add_requires("scriptx 775b9f5de5ded72956762aaf8708f2e46ddeb93c", {configs={backend="Lua"}})

elseif is_config("backend", "quickjs") then
    add_requires("openssl 1.1.1-w")
    add_requires("mariadb-connector-c 3.3.9")
    add_requires("scriptx 775b9f5de5ded72956762aaf8708f2e46ddeb93c", {configs={backend="QuickJs"}})

elseif is_config("backend", "python") then
    add_requires("openssl 1.1.1-w")
    add_requires("mariadb-connector-c 3.3.9")
    add_requires("scriptx 775b9f5de5ded72956762aaf8708f2e46ddeb93c", {configs={backend="Python"}})

elseif is_config("backend", "nodejs") then
    add_requires("scriptx 775b9f5de5ded72956762aaf8708f2e46ddeb93c", {configs={backend="V8"}})

end

add_requires("openssl3 3.3.2")
add_requires("cpp-httplib 0.18.7", {configs = {ssl = true, zlib = true}})

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("publish")
    set_default(false)
    set_showmenu(true)
option_end()

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
    add_cxflags("/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204","/Zm2000", {force = true})
    add_defines(
        "NOMINMAX",
        "UNICODE",
        "_AMD64_"
    )
    add_packages(
        "cpp-httplib",
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
        "toml++",
        "mariadb-connector-c"
    )
    set_exceptions("none")
    set_kind("shared")
    set_languages("cxx20")
    set_symbols("debug")
    add_files(
        "src/**.cpp"
    )
    add_includedirs(
        "src",
        "src/legacy"
    )
    on_load(function (target)
        local tag = os.iorun("git describe --tags --abbrev=0 --always")
        local major, minor, patch, suffix = tag:match("v(%d+)%.(%d+)%.(%d+)(.*)")
        if not major then
            print("Failed to parse version tag, using 0.0.0")
            major, minor, patch = 0, 0, 0
        end
        local versionStr =  major.."."..minor.."."..patch
        if suffix then
            prerelease = suffix:match("-(.*)")
            if prerelease then
                prerelease = prerelease:gsub("\n", "")
            end
        end

        if not has_config("publish") then
            local hash = os.iorun("git rev-parse --short HEAD")
            versionStr = versionStr.."+"..hash:gsub("\n", "")
        end

        target:add("rules", "@levibuildscript/modpacker",{
            modName = target:basename(),
            modVersion = versionStr
        })
    end)

    if is_config("backend", "lua") then
        add_defines(
            "LEGACY_SCRIPT_ENGINE_BACKEND_LUA"
        )
        remove_files("src/legacy/main/NodeJsHelper.cpp")
        remove_files("src/legacy/main/PythonHelper.cpp")
        set_basename("legacy-script-engine-lua")
        after_build(function(target)
            local baselibPath = path.join(os.projectdir(), "src/baselib/BaseLib.lua")
            local langPath = path.join(os.projectdir(), "src/lang/")
            local outputPath = path.join(os.projectdir(), "bin/" .. target:basename())
            local baselibOutputPath = path.join(outputPath, "baselib")
            os.mkdir(baselibOutputPath)
            os.cp(baselibPath, baselibOutputPath)
            os.cp(langPath, outputPath)
        end)

    elseif is_config("backend", "quickjs") then
        add_defines(
            "LEGACY_SCRIPT_ENGINE_BACKEND_QUICKJS"
        )
        remove_files("src/legacy/main/NodeJsHelper.cpp")
        remove_files("src/legacy/main/PythonHelper.cpp")
        set_basename("legacy-script-engine-quickjs")
        after_build(function(target)
            local baselibPath = path.join(os.projectdir(), "src/baselib/BaseLib.js")
            local langPath = path.join(os.projectdir(), "src/lang/")
            local outputPath = path.join(os.projectdir(), "bin/" .. target:basename())
            local baselibOutputPath = path.join(outputPath, "baselib")
            os.mkdir(baselibOutputPath)
            os.cp(baselibPath, baselibOutputPath)
            os.cp(langPath, outputPath)
        end)

    elseif is_config("backend", "python") then
        add_defines(
            "LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON"
        )
        remove_files("src/legacy/main/NodeJsHelper.cpp")
        set_basename("legacy-script-engine-python")
        after_build(function(target)
            local baselibPath = path.join(os.projectdir(), "src/baselib/BaseLib.py")
            local langPath = path.join(os.projectdir(), "src/lang/")
            local outputPath = path.join(os.projectdir(), "bin/" .. target:basename())
            local baselibOutputPath = path.join(outputPath, "baselib")
            os.mkdir(baselibOutputPath)
            os.cp(baselibPath, baselibOutputPath)
            os.cp(langPath, outputPath)
        end)

    elseif is_config("backend", "nodejs") then
        add_defines(
            "LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS"
        )
        remove_files("src/legacy/main/PythonHelper.cpp")
        remove_files("src/legacy/legacyapi/db/impl/mysql/*.cpp")
        set_basename("legacy-script-engine-nodejs")
        after_build(function(target)
            local langPath = path.join(os.projectdir(), "src/lang/")
            local outputPath = path.join(os.projectdir(), "bin/" .. target:basename())
            os.mkdir(outputPath)
            os.cp(langPath, outputPath)
        end)
    end