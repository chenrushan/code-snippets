#!/bin/sh

# ======================================================================
# 参数设置 build_type
# ======================================================================

build_type=Debug
if [[ $# -eq 1 ]]; then
    build_type=$1
fi

if [[ "$build_type" == "r" ]] || [[ "$build_type" == "release" ]]; then
    build_type=Release
fi

if [[ "$build_type" == "d" ]] || [[ "$build_type" == "debug" ]]; then
    build_type=Debug
fi

if [[ "$build_type" != "Release" ]] && [[ "$build_type" != "Debug" ]]; then
    echo "build_type should be [r|d|release|debug], but you specify $build_type"
    exit 1
fi

echo "BUILD_TYPE: ${build_type}"

# ======================================================================
# 创建必要的目录
# ======================================================================

[[ -d .build/${build_type} ]] || mkdir -p .build/${build_type}
if [[ $? -ne 0 ]]; then
    echo "fail to mkdir build"
    exit 1
fi
[[ -d bin ]] || mkdir bin
if [[ $? -ne 0 ]]; then
    echo "fail to mkdir bin"
    exit 1
fi

# ======================================================================
# 准备开始编译
# ======================================================================

dir=$(pwd)
echo "project dir: ${dir}"
cd .build/${build_type}
cmake -D CMAKE_INSTALL_PREFIX=${dir} -D CMAKE_BUILD_TYPE=${build_type} ${dir}
make -j install

