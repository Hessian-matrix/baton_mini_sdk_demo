#pragma once
#include "baton_cmd.h"
// #include "sdk/imu.hpp"
#include "sdk/imu_tcp.hpp"
// #include "sdk/image.hpp"
#include "sdk/image_tcp.hpp"
#include "sdk/odom_tcp.hpp"
struct pose_t{
    float px,py,pz,qx,qy,qz,qw;
};

struct speed_t{
    float lx,ly,lz,ax,ay,az;
};

struct odom_t{
    pose_t pose;
    speed_t speed;
};
