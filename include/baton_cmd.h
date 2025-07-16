#pragma once
#include <stdio.h>
#include <unistd.h>
#include "sdk/cJSON.h"
#include "sdk/vio_sdk.h"

#define cfg_max 1024
#define cfg_version "/System/version"//硬件版本
#define cfg_network "/System/network"//网络信息
#define device_param "/System/param"//参数信息
#define ctrl_smart_status   "/Config/smart"
#define client_ip_addr   "/Config/client_IP"
#define system_reboot       "/System/reboot"
#define algorithm_enable    "/Algorithm/enable/4" 
#define algorithm_disable   "/Algorithm/disable/4"
#define algorithm_reboot    "/Algorithm/reboot/4"
#define algorithm_reset     "/Algorithm/reset/4"

typedef enum {
    ready = 0,
    stereo3_initializing = 7,
    stereo3_running = 8
}system_status;

typedef enum {
    none = 0,
    left = 1,
    right = 2,
    stereo = 3,
}image_get;

typedef enum{
    OFF = 0,
    ON = 1,
}recv_switch;

typedef struct{
    double fx,fy,cx,cy;
    double xi,alpha;
}cam_fisheye_instrinsic_param;

typedef struct{
    double px,py,pz;
    double qx,qy,qz,qw;
}extrinsic_Quaternion;

typedef struct{
    cam_fisheye_instrinsic_param left_cam,right_cam;
    extrinsic_Quaternion extrinsic;
}cam_fisheye_param;

extern HANDLE loginHandle; //SDK connect handle,need initialize to login

bool alog_start();
bool algo_stop();
bool algo_restart();
int baton_client_ipaddress(const char* ip_addr);
int baton_open_imu_recv(recv_switch sw);
int baton_open_fast_odom_recv(recv_switch sw);
int baton_open_image_recv(image_get image_type_get);
void get_device_version();
void get_device_param();
int set_network(const char* ipaddr);
