add_rules("mode.release")

add_repositories("liteldev-xmake-repo https://github.com/LiteLDev/xmake-repo.git")

-- Packages from xmake-repo
add_requires("asio 1.28.0")
add_requires("openssl 1.1.1-t")
add_requires("simpleini v4.19")
add_requires("toml++ v3.4.0")

-- Packages from liteldev-xmake-repo
add_requires("dyncall 1.4")
add_requires("lightwebsocketclient v1.0.0")
add_requires("threadpool v1.0.0")
add_requires("scriptx v0.1.0")
add_requires("levilamina develop")
add_requires("fifo_map v1.0.0")
add_requires("demangler v2.0.0")

package("levilamina")
    add_urls("https://github.com/LiteLDev/LeviLamina.git")

    -- Dependencies from xmake-repo.
    add_deps("entt 3.12.2")
    add_deps("fmt 10.1.1")
    add_deps("gsl 4.0.0")
    add_deps("leveldb 1.23")
    add_deps("magic_enum 0.9.0")
    add_deps("nlohmann_json 3.11.2")
    add_deps("rapidjson 1.1.0")

    -- Dependencies from liteldev-repo.
    add_deps("ctre 3.8.1")
    add_deps("pcg_cpp 1.0.0")
    add_deps("pfr 2.1.1")
    add_deps("preloader 1.3.1")
    add_deps("symbolprovider 1.1.0")
    add_deps("bdslibrary 1.20.50.03")

    on_install(function (package)
        import("package.tools.xmake").install(package)
    end)

target("LeviScript")
    set_kind("shared")
    set_languages("cxx20")
    add_files("src/**.cpp")
    add_includedirs("src")
    add_defines(
        "_WIN32_WINNT=0x0601",
        "_AMD64_",
        "_CONSOLE",
        "_WINDLL",
        "_UNICODE",
        "CPPHTTPLIB_OPENSSL_SUPPORT",
        "NDEBUG",
        "NOMINMAX",
        "SCRIPTX_BACKEND_TRAIT_PREFIX=../backend/${env SCRIPTX_BACKEND}/trait/Trait",
        "UNICODE",
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
        "rapidjson",
        "fmt"
    )

    -- Packages from liteldev-xmake-repo
    add_packages(
        "dyncall",
        "symbolprovider",
        "nbt_cpp",
        "lightwebsocketclient",
        "threadpool",
        "scriptx",
        "levilamina",
        "fifo_map",
        "demangler"
    )
