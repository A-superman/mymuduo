#!/bin/bash
set -e

# 如果没有build目录，创建该目录
if [ ! -d ../build ]; then
    mkdir -p ../build
fi

rm -rf ../build/*
cd ../build
cmake ../code
make

# 回到项目根目录
cd ../code

# 把头文件拷贝到/usr/include/mymuduo so库拷贝到 /usr/lib    PATH
if [ ! -d /usr/include/mymuduo ]; then
    mkdir -p /usr/include/mymuduo
fi

for header in *.h; do
    cp $header /usr/include/mymuduo/
done

cp  ../build/lib/libmymuduo.so /usr/lib

ldconfig