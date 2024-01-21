add_rules("mode.debug", "mode.release", "mode.releasedbg")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_requires(
    "simpleini v4.19",
    "toml++ 3.4.0",
    "sqlite3 3.43.0+200",
    "mariadb-connector-c 3.3.4",
    "dyncall 1.4",
    "lightwebsocketclient 1.0.0",
    "demangler 2.0.0",
    "levilamina 0.5.1",
    "legacymoney",
    "legacyparticleapi"
)
add_requires("cpp-httplib v0.14.0", {configs = {ssl=true, zlib=true}})
add_requires("scriptx 0.1.0", {configs={backend=get_config("backend")}})

local LLSE_BACKEND = "LUA"
local LLSE_BACKEND_LIBRARY = "Lua"
local SCRIPTX_BACKEND = "Lua"

option("backend")
    set_default("lua")
    set_values("lua", "nodejs", "python310", "quickjs")

package("scriptx")
    add_configs("backend", {default = "lua", values = {"lua", "nodejs", "python310", "quickjs"}})
    add_includedirs(
        "include/scriptx/src/include/",
        "include/$(backend)/"
    )
    add_linkdirs(
        "lib/scriptx/"
    )
    add_urls("https://github.com/LiteLDev/ScriptX/releases/download/v$(version)/scriptx-windows-x64.zip")
    add_versions("0.1.0", "c0077eed8daf0e50a455cfde6396c2c04ba4d7a03a40424aa7da3571f9e8b7b4")

    on_install(function (package)
        local backend = package:config("backend")

        local backend_info = {
            ["lua"] = {
                backend = "lua",
                scriptx = "Lua",
            },
            ["nodejs"] = {
                backend = "libnode",
                scriptx = "V8",
            },
            ["python310"] = {
                backend = "python310",
                scriptx = "Python",
            },
            ["quickjs"] = {
                backend = "quickjs",
                scriptx = "QuickJs",
            },
        }

        -- ScriptX
        os.cp("include/scriptx/*", package:installdir("include", "scriptx"))
        os.cp("lib/scriptx/" .. backend_info[backend].scriptx .. ".lib",
            package:installdir("lib", "scriptx"))
        package:add("defines", "SCRIPTX_BACKEND=" .. backend_info[backend].scriptx)
        package:add("defines", "SCRIPTX_BACKEND_TRAIT_PREFIX=../backend/" .. backend_info[backend].scriptx .. "/trait/Trait")

        -- Backend
        os.cp("include/" .. backend_info[backend].backend .. "/*",
            package:installdir("include", backend_info[backend].backend))
        os.cp("lib/" .. backend_info[backend].backend .. ".lib",
            package:installdir("lib"))
    end)

target("legacy-script-engine")
    add_cxflags(
        "/EHa",
        "/utf-8"
    )
    add_defines(
        "_HAS_CXX23=1", -- To enable C++23 features

        "_WIN32_WINNT=0x0601",
        "_AMD64_",
        "_CONSOLE",
        "_WINDLL",
        "_UNICODE",
        "CPPHTTPLIB_OPENSSL_SUPPORT",
        "NDEBUG",
        "NOMINMAX",
        "UNICODE",
        "LLSE_BACKEND_" .. LLSE_BACKEND,
        "ENTT_PACKED_PAGE=128"
    )
    add_files(
        "src/**.cpp"
    )
    add_includedirs(
        "src"
    )
    add_packages(
        "levilamina",
        "scriptx",

        "nlohmann_json",
        "simpleini",
        "toml++",
        "magic_enum",
        "leveldb",
        "rapidjson",
        "cpp-httplib",
        "sqlite3",
        "mariadb-connector-c",

        "dyncall",
        "lightwebsocketclient",
        "demangler",
        "legacymoney",
        "legacyparticleapi"
    )
    add_shflags(
        "/DELAYLOAD:bedrock_server.dll"
    )
    set_basename("legacy-script-engine-$(backend)")
    set_exceptions("none") -- To avoid conflicts with /EHa
    set_kind("shared")
    set_languages("cxx20")
