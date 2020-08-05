# CMakeList.txt: grok 的 CMake 项目，在此处包括源代码并定义 库
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("grok")

find_package(Boost REQUIRED system thread filesystem)
include_directories(BOOST_INCLUDEDIR)
link_directories(BOOST_LIBRARYDIR)


IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE("current platform: Linux ")

    if("${BUILD_TYPE}" STREQUAL "Debug")
        add_definitions("-g")
    elseif("${BUILD_TYPE}" STREQUAL "Relase")
        
    elseif(NOT BUILD_TYPE)
        add_definitions("-g")
    endif()

    file(GLOB_RECURSE c_files ${PROJECT_SOURCE_DIR}/grok/src/*.cpp)
    add_library(grok_lib_shared  SHARED ${c_files})
    add_library(grok_lib_static ${c_files})

    file(GLOB_RECURSE demo_cpp ${PROJECT_SOURCE_DIR}/test/*.cpp)
    add_executable(demo ${demo_cpp})
    target_link_libraries(demo grok_lib_static pthread boost_thread boost_system boost_filesystem  mysqlclient mysqlcppconn)
ELSEIF  (CMAKE_SYSTEM_NAME MATCHES "Windows")


ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

