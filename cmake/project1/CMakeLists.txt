# ======================================================================
# Global setting
# ======================================================================

project(cmakedemo)

# 把你现在系统里的 cmake 版本填上
cmake_minimum_required(VERSION 2.8.11)

# 下面这几个变量和设置 g++ 参数有关
# CMAKE_BUILD_TYPE 用于设置 build 类型，可以设置为 Release 或者 Debug
# 设置了 Release 就会读取 CMAKE_CXX_FLAGS_RELEASE 变量
# 设置了 Debug 就会读取 CMAKE_CXX_FLAGS_DEBUG 变量
set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -std=c++11")

# option 很容易理解了，取值 ON 和 OFF
option(ENABLE_Test "enable unit test" ON)

# find_package 用于设置你程序需要的 lib
# find_package 会被 cmake 转换为相应的脚本调用，脚本在 /usr/share/cmake/Modules
# 下面的 find_package 会调用 /usr/share/cmake/Modules/FindBoost.cmake
# 调用这些脚本通常都会设置一些变量，比如 FindBoost 脚本会设置 Boost_INCLUDE_DIRS,
# Boost_LIBRARY_DIRS 等，具体设置了那些变量可以参考脚本开头的注释
find_package(Boost REQUIRED filesystem program_options system)

# 设置项目的 include 目录，这里的 Boost_INCLUDE_DIRS 就是上面的 find_package
# 调用设置的
include_directories(src ${Boost_INCLUDE_DIRS})

# message 可以方便用于 cmake 调试
message("Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")

# ======================================================================
# building executables
# ======================================================================

# add_executable 就是用于生成可执行文件的，一个可执行文件对应一个 add_executable
# 从第二个参数，开始列出所有这个文件依赖的 cpp 文件（据说比较建议的做法是显式得
# 把所有依赖的文件列出来，而不是用类似 glob 的方法）
add_executable(cmakedemo
    src/main.cpp
    src/lib1/boost_filesystem.cpp)
# 这个调用指定可执行文件依赖的第三方 lib，这里只依赖了 boost
target_link_libraries(cmakedemo ${Boost_LIBRARIES})

# ======================================================================
# unit tests
# ======================================================================

if (ENABLE_Test) 
    # 下面几行正常 build 一个 executable 出来
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
    add_executable(test_boost_filesystem
        test/lib1/test_boost_filesystem.cpp
        src/lib1/boost_filesystem.cpp)
    target_link_libraries(test_boost_filesystem ${Boost_LIBRARIES} ${GTEST_LIBRARIES})

    # enable_testing 用于启动 CTest，配合 add_test() 使用，所有通过 add_test
    # 注册的 test 都可以通过 make test 直接跑
    enable_testing()
    add_test(some-description test_boost_filesystem)
endif()

# ======================================================================
# cmake 还有一个很常用的函数是 add_subdirectory
# add_subdirectory(dir) 用于调用 dir 下的 CMakeLists.txt
# ======================================================================

