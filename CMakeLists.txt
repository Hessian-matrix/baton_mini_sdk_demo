cmake_minimum_required(VERSION 3.0.2)
project(baton_mini)

add_compile_options(-std=c++11)

option(USE_ROS "Enable ROS support" ON)
if(USE_ROS)
  add_definitions(-DROS)

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
    ${catkin_INCLUDE_DIRS}
  )
else()
  message(STATUS "Building without ROS support")
  find_package(OpenCV REQUIRED)
endif()

include_directories(
  include
  ${OpenCV_INCLUDE_DIRS}
  src
)

set(SOURCE_FILES
    src/sdk/Base64.cpp
    src/sdk/CHttpRequest.cpp
    src/sdk/cJSON.cpp
    src/sdk/CommonSocket.cpp
    src/sdk/syslib.cpp
    src/sdk/vio_msg.cpp
    src/sdk/vio_sdk.cpp
    src/baton_cmd.cpp
    src/baton_mini.cpp
)

if(USE_ROS)
    list(APPEND SOURCE_FILES src/baton_ros.cpp)
endif()

add_executable(baton_mini ${SOURCE_FILES})

if(USE_ROS)
    target_link_libraries(baton_mini ${catkin_LIBRARIES} ${OpenCV_LIBS})
else()
    target_link_libraries(baton_mini ${OpenCV_LIBS} pthread)
endif()
