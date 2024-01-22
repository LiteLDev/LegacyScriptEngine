add_rules("mode.debug", "mode.release", "mode.releasedbg")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_requires(
    "demangler v2.0.0",
    "dyncall 1.4",
    "legacymoney 0.1.5",
    "legacyparticleapi 0.1.1",
    "levilamina 0.5.1",
    "lightwebsocketclient 1.0.0",
    "magic_enum v0.9.0",
    "mariadb-connector-c 3.3.4",
    "simpleini v4.19",
    "sqlite3 3.43.0+200",
    "toml++ v3.4.0"
)
add_requires("cpp-httplib v0.14.0", {configs = {ssl=true, zlib=true}})
add_requires("scriptx 0.1.0", {configs={backend=get_config("backend")}})

set_runtimes("MD") -- For compatibility with the /MT build configuration of ScriptX.

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
            libnode = "V8",
            lua = "Lua",
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
        "_HAS_CXX23=1", -- To enable C++23 features.
        "_WIN32_WINNT=0x0601",
        "_AMD64_",
        "_CONSOLE",
        "_WINDLL",
        "_UNICODE",
        "CPPHTTPLIB_OPENSSL_SUPPORT",
        "NDEBUG",
        "NOMINMAX",
        "UNICODE",
        "LLSE_BACKEND_LUA",
        "ENTT_PACKED_PAGE=128"
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
        "mariadb-connector-c",
        "scriptx",
        "simpleini",
        "sqlite3",
        "toml++"
    )
    add_shflags(
        "/DELAYLOAD:bedrock_server.dll" -- To allow including Minecraft headers without linking.
    )
    set_basename("legacy-script-engine-$(backend)")
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("cxx20")
