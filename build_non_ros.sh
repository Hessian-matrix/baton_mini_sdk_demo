#!/bin/bash

# 创建构建目录
BUILD_DIR="build_non_ros"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# 使用 CMake 配置项目
cmake .. -DUSE_ROS=OFF

# 编译
make

# 返回上级目录
cd ..

echo "Non-ROS build completed!"