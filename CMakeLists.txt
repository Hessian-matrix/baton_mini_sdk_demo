cmake_minimum_required(VERSION 3.0.2)
project(baton_mini)

add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rosmsg
  nav_msgs
  std_msgs
  cv_bridge
)

find_package(OpenCV REQUIRED)

catkin_package(
  CATKIN_DEPENDS
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  src
)

add_executable(baton_mini 
  src/sdk/Base64.cpp
  src/sdk/CHttpRequest.cpp
  src/sdk/cJSON.cpp
  src/sdk/CommonSocket.cpp
  src/sdk/syslib.cpp
  src/sdk/vio_msg.cpp
  src/sdk/vio_sdk.cpp
  src/baton_ros.cpp
  src/baton_cmd.cpp
  src/baton_mini.cpp
)

target_link_libraries(baton_mini ${catkin_LIBRARIES} ${OpenCV_LIBS})

