#!/bin/bash

SCRIPT_PATH=$(readlink -f "$BASH_SOURCE")
echo $SCRIPT_PATH
cd "$(dirname "$SCRIPTP_ATH")"

if ! command -v catkin_make &> /dev/null; then
    echo "ROS environment not found! Please source your ROS setup.bash."
    exit 1
fi

cd ../..

catkin_make --cmake-args -DUSE_ROS=ON

echo "ROS build completed!"