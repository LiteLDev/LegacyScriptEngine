add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

-- liteldev-repo
add_requires("dyncall 1.4")
add_requires("symbolprovider v1.1.0")
add_requires("nbt_cpp v1.0.1")
add_requires("lightwebsocketclient v1.0.0")
add_requires("threadpool v1.0.0")
add_requires("se-backend v1.0.0")
add_requires("scriptx v1.0.0")

-- xmake-repo
add_requires("nlohmann_json v3.11.2")
add_requires("openssl 1.1.1-t")
add_requires("gsl v3.1.0")
add_requires("simpleini v4.19")
add_requires("toml++ v3.4.0")
add_requires("magic_enum v0.8.2")

target("ScriptEngine")
    set_kind("shared")
    add_files("src/**.cpp")
    set_languages("c++20")
    -- liteldev-repo
    add_packages("dyncall", "symbolprovider", "nbt_cpp", "lightwebsocketclient", "threadpool", "se-backend", "scriptx")
    -- xmake-repo
    add_packages("nlohmann_json", "openssl", "gsl", "simpleini", "toml++", "magic_enum")
    add_includedirs("src")
--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

