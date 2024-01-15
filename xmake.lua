add_rules("mode.release")

add_repositories("liteldev-xmake-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("local-repo repo")

-- Packages from xmake-repo
add_requires("asio 1.28.0")
add_requires("openssl")
add_requires("simpleini v4.19")
add_requires("toml++ v3.4.0")
add_requires("cpp-httplib v0.14.0")
add_requires("sqlite3 3.43.0+200")
add_requires("mariadb-connector-c 3.3.4")

-- Packages from liteldev-xmake-repo
add_requires("dyncall 1.4")
add_requires("lightwebsocketclient v1.0.0")
add_requires("levilamina")
add_requires("demangler v2.0.0")
add_requires("legacymoney")

-- Packages from local
add_requires("scriptx-legacy")
add_requires("threadpool")

local LLSE_BACKEND = "LUA"
local LLSE_BACKEND_LIBRARY = "Lua"
local SCRIPTX_BACKEND = "Lua"

target("LeviScript")
    set_kind("shared")
    set_languages("cxx20")
    add_files("src/**.cpp")
    add_includedirs("src")
    set_exceptions("none")
    add_cxflags(
        "/utf-8",
        "/permissive-",
        "/EHa"
    )
    add_defines(
        "_WIN32_WINNT=0x0601",
        "_AMD64_",
        "_CONSOLE",
        "_WINDLL",
        "_UNICODE",
        "CPPHTTPLIB_OPENSSL_SUPPORT",
        "NDEBUG",
        "NOMINMAX",
        "SCRIPTX_BACKEND_TRAIT_PREFIX=../backend/" .. SCRIPTX_BACKEND .. "/trait/Trait",
        "UNICODE",
        "LLSE_BACKEND_" .. LLSE_BACKEND,
        "ENTT_PACKED_PAGE=128",
        "_HAS_CXX23=1"
    )
    add_shflags("/DELAYLOAD:bedrock_server.dll")

    -- Packages from xmake-repo
    add_packages(
        "nlohmann_json",
        "openssl",
        "gsl",
        "simpleini",
        "toml++",
        "magic_enum",
        "asio",
        "entt",
        "leveldb",
        "rapidjson",
        "fmt",
        "cpp-httplib",
        "sqlite3",
        "mariadb-connector-c"
    )

    -- Packages from liteldev-xmake-repo
    add_packages(
        "dyncall",
        "symbolprovider",
        "lightwebsocketclient",
        "levilamina",
        "demangler",
        "legacymoney"
    )

    -- Packages from local
    add_packages(
        "scriptx-legacy",
        "threadpool"
    )
