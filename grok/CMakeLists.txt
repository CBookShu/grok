# CMakeList.txt: grok 的 CMake 项目，在此处包括源代码并定义 库
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("grok")


include_directories("F:\\opensource\\boost_1_67_0")
link_directories("F:\\opensource\\boost_1_67_0\\stage\\lib")
file(GLOB_RECURSE c_files ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(grok_lib_shared  SHARED ${c_files})
add_library(grok_lib_static ${c_files})


add_executable(demo ${PROJECT_SOURCE_DIR}/test/main.cpp)
target_link_libraries(demo grok_lib_static)