#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
CODE_DIR="${SCRIPT_DIR}"

# 如果没有 build 目录，创建该目录
mkdir -p "${BUILD_DIR}"

rm -rf "${BUILD_DIR}"/*
cd "${BUILD_DIR}"
cmake "${CODE_DIR}"
make

# 回到代码目录
cd "${CODE_DIR}"

# 把头文件拷贝到 /usr/include/mymuduo，so 库拷贝到 /usr/lib
if [ ! -d /usr/include/mymuduo ]; then
    mkdir -p /usr/include/mymuduo
fi

for header in "${CODE_DIR}"/*.h; do
    cp "$header" /usr/include/mymuduo/
done

cp "${BUILD_DIR}/lib/libmymuduo.so" /usr/lib

ldconfig