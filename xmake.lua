add_rules("mode.debug", "mode.release")

add_repositories("liteldev-xmake-repo https://github.com/LiteLDev/xmake-repo.git")

-- Packages from xmake-repo
add_requires("asio 1.28.0")
add_requires("nlohmann_json v3.11.2")
add_requires("openssl 1.1.1-t")
add_requires("gsl v3.1.0")
add_requires("simpleini v4.19")
add_requires("toml++ v3.4.0")
add_requires("magic_enum v0.8.2")
add_requires("entt v3.11.1")
add_requires("leveldb 1.23")
add_requires("rapidjson v1.1.0")

-- Packages from liteldev-xmake-repo
add_requires("dyncall 1.4")
add_requires("symbolprovider v1.1.0")
add_requires("lightwebsocketclient v1.0.0")
add_requires("threadpool v1.0.0")
add_requires("scriptx v0.1.0")
add_requires("levilamina v1.0.0")
add_requires("fifo_map v1.0.0")

target("LeviScript")
    set_kind("shared")
    set_languages("cxx20")
    add_files("src/**.cpp")
    add_includedirs("src")
    add_defines(
        "D",
        "D_AMD64_",
        "D_CONSOLE",
        "D_WINDLL",
        "D_UNICODE",
        "DCPPHTTPLIB_OPENSSL_SUPPORT",
        "DNDEBUG",
        "DNOMINMAX",
        "DSCRIPTX_BACKEND_TRAIT_PREFIX=../backend/${env SCRIPTX_BACKEND}/trait/Trait",
        "DUNICODE",
        "LLSE_BACKEND_${env LLSE_BACKEND}"
    )
    add_runenvs("LLSE_BACKEND", "LUA")
    add_runenvs("LLSE_BACKEND_LIBRARY", "Lua")
    add_runenvs("SCRIPTX_BACKEND", "Lua")
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
        "rapidjson"
    )

    -- Packages from liteldev-xmake-repo
    add_packages(
        "dyncall",
        "symbolprovider",
        "nbt_cpp",
        "lightwebsocketclient",
        "threadpool",
        "se-backend",
        "scriptx",
        "levilamina",
        "fifo_map"
    )
