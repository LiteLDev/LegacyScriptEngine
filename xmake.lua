add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo " .. (get_config("levimc_repo") or "https://github.com/LiteLDev/xmake-repo.git"))

if is_config("target_type", "server") then
    add_requires("levilamina eb5a505faf11dfbc86cee806386ef135e979bace", {configs = {target_type = "server"}})
else
    add_requires("levilamina eb5a505faf11dfbc86cee806386ef135e979bace", {configs = {target_type = "client"}})
end

add_requires("levibuildscript")

add_requires(
    "legacymoney 0.10.0",
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
    add_requires("scriptx 22184aeecdcee2c683ad99db6362837e4ec6b7fe", {configs={backend="Lua"}})

elseif is_config("backend", "quickjs") then
    add_requires("openssl 1.1.1-w")
    add_requires("mariadb-connector-c 3.3.9")
    add_requires("scriptx 22184aeecdcee2c683ad99db6362837e4ec6b7fe", {configs={backend="QuickJs"}})

elseif is_config("backend", "python") then
    add_requires("openssl 1.1.1-w")
    add_requires("mariadb-connector-c 3.3.9")
    add_requires("scriptx 22184aeecdcee2c683ad99db6362837e4ec6b7fe", {configs={backend="Python"}})

elseif is_config("backend", "nodejs") then
    add_requires("scriptx 22184aeecdcee2c683ad99db6362837e4ec6b7fe", {configs={backend="V8"}})

end

add_requires("openssl3")
add_requires("cpp-httplib 0.26.0", {configs = {ssl = true, zlib = true}})

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("levimc_repo")
    set_default("https://github.com/LiteLDev/xmake-repo.git")
    set_showmenu(true)
    set_description("Set the levimc-repo path or url")
option_end()

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
    add_cxflags(
        "/EHa",
        "/utf-8",
        "/W4",
        "/w44265",
        "/w44289",
        "/w44296",
        "/w45263",
        "/w44738",
        "/w45204",
        "/Zm2000",
        "/wd4100",
        {force = true}
    )
    add_defines(
        "NOMINMAX",
        "UNICODE",
        "_AMD64_"
    )
    add_packages(
        "cpp-httplib",
        "legacymoney",
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
    set_configdir("$(builddir)/config")
    set_configvar("LSE_WORKSPACE_FOLDER", "$(projectdir)")
    add_configfiles("src/(lse/Version.h.in)")
    add_files(
        "src/**.cpp",
        "src/**.rc"
    )
    add_includedirs(
        "src",
        "src/legacy",
        "$(builddir)/config"
    )
    if is_config("target_type", "server") then
        add_defines("LL_PLAT_S")
    else
        add_defines("LL_PLAT_C")
    end
    if has_config("publish") then
        add_defines("LSE_VERSION_PUBLISH")
    end
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
            if prerelease then
                target:set("configvar", "LSE_VERSION_PRERELEASE", prerelease)
                versionStr = versionStr.."-"..prerelease
            end
        end
        target:set("configvar", "LSE_VERSION_MAJOR", major)
        target:set("configvar", "LSE_VERSION_MINOR", minor)
        target:set("configvar", "LSE_VERSION_PATCH", patch)
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
            "LSE_BACKEND_LUA"
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
            "LSE_BACKEND_QUICKJS"
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
            "LSE_BACKEND_PYTHON"
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
            "LSE_BACKEND_NODEJS"
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