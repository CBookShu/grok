add_rules("mode.debug", "mode.release")

target("grok")
    set_kind("static")
    add_includedirs("$(projectdir)", {public = true})
    add_files("grok/*.cpp")
    add_files("grok/pb/*.cc")
    -- 第三方库
    add_links("hiredis","protobuf","mysqlclient")
    -- boost库
    add_links("boost_system", "boost_filesystem", "boost_thread")
    -- 系统库
    add_syslinks("pthread")

-- lua 5.3.5
target("lua")
    set_kind("static")
    -- 增加public=true，后面add_deps的项目，会继承该include
    add_includedirs("$(projectdir)/deps/lua/", {public = true})
    add_files("deps/lua/*.c|deps/lua/lua.c")

target("pbc")
    set_kind("static")
    add_includedirs("$(projectdir)/deps/pbc/", {public = true})
    add_files("deps/pbc/src/*.c")

target("example")
    set_kind("binary")
    -- grok
    add_deps("grok")
    -- lua
    add_deps("lua")
    -- pbc
    add_deps("pbc")
    add_files("$(projectdir)/deps/pbc/binding/lua53/pbc-lua53.c")
    -- example
    add_files("example/*.cpp")

target("bench_im")
    set_kind("binary")
    add_deps("grok")
    add_files("test/bench_im/*.cpp")

target("hiredis_test")
    set_kind("binary")
    add_files("test/hiredis_test/*.cpp")
    add_links("hiredis")

target("locklist_test")
    set_kind("binary")
    add_deps("grok")
    add_files("test/locklist_test/*.cpp")

target("unionlock_test")
    set_kind("binary")
    add_deps("grok")
    add_files("test/unionlock_test/*.cpp")

-- target("hiredis")
--     set_kind("static")
--     add_files("deps/hiredis/*.c")

target("redispool_test")
    set_kind("binary")
    add_deps("grok")
    add_files("test/redispool_test/*.cpp")


target("libmysqlclient_test")
    set_kind("binary")
    add_files("test/libmysqlclient_test/*.cpp")
    add_links("mysqlclient")

target("mysqlpool_test")
    set_kind("binary")
    add_deps("grok")
    add_files("test/mysqlpool_test/*.cpp")

target("netserver_test")
    set_kind("binary")
    add_deps("grok")
    add_files("test/netserver_test/*.cpp")

target("protobuf_test")
    set_kind("binary")
    add_links("protobuf")
    add_files("test/protobuf_test/*.cpp")
    add_files("test/protobuf_test/*.cc")
    add_files("grok/pb/*.cc")

target("nodeservice_test")
    set_kind("binary")
    add_deps("grok")
    add_files("test/nodeservice_test/*.cpp")
    add_files("test/nodeservice_test/*.cc")


target("luabind_test")
    set_kind("binary")
    add_deps("lua")
    add_files("test/luabind_test/*.cpp")

target("test_group")
    set_kind("phony")
    set_group("test")
    on_run(function(target) 
        os.exec("xmake run example $(projectdir)")
    end)

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
--    -- add macro defination
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

