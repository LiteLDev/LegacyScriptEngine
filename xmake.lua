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
add_requires("scriptx", {configs={backend=get_config("backend")}})

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

local LLSE_BACKEND = "LUA"

option("backend")
    set_default("lua")
    set_values("libnode", "lua", "python310", "quickjs")

package("scriptx")
    add_configs("backend", {default = "lua", values = {"libnode", "lua", "python310", "quickjs"}})
    add_includedirs(
        "include/scriptx/src/include/"
    )
    add_urls("https://github.com/LiteLDev/ScriptX/releases/download/v$(version)/scriptx-windows-x64.zip")
    add_versions("0.1.0", "d47729b73f37eaeb6c5dead4301e16feffd692ca10156a42449826997a1256c2")

    on_install(function (package)
        os.cp("*", package:installdir())
    end)

    on_load(function (package)
        local backend = package:config("backend")

        local scriptx_backend = {
            lua = "Lua",
            nodejs = "V8",
            python310 = "Python",
            quickjs = "QuickJs",
        }

        print("Using ScriptX config: backend=" .. backend .. ", scriptx_backend=" .. scriptx_backend[backend])
        
        package:add("defines", "SCRIPTX_BACKEND=" .. scriptx_backend[backend])
        package:add("defines", "SCRIPTX_BACKEND_TRAIT_PREFIX=../backend/" .. scriptx_backend[backend] .. "/trait/Trait")
        package:add("includedirs", "include/" .. backend .. "/")
        package:add("links", backend)
        package:add("links", "scriptx_" .. scriptx_backend[backend])
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
