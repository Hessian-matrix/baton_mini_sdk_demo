#include "baton_cmd.h"
#include <cstring>
#include <iostream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

HANDLE loginHandle; //SDK connect handle,need initialize to login

/*
open imu receive
param:
    imu_switch : 
    OFF = 0, close imu receive
    ON = 1, open imu receive
*/
int baton_open_imu_recv(imu_switch sw){
    if(sw == ON)
        std::cout << "imu receive open" << std::endl;
    else
        std::cout << "imu receive close" << std::endl;

    std::string body = "";
	cJSON* obj = cJSON_CreateObject();
	if (obj) {
		cJSON_AddNumberToObject(obj, "imu_enable", (int)sw);
		char* out = cJSON_PrintUnformatted(obj);
		body.append(out);
		free(out);
		cJSON_Delete(obj);
	}
	if (body.empty())
		return -1;
	net_vio_set_cfg(loginHandle,ctrl_smart_status,body.c_str(),body.length());
    return 0;
}

/*
send baton client ip address to server for UDP receive
param:
    ip_addr : client local ip address
*/
int baton_client_ipaddress(const char* ip_addr){
    std::string body = "";
	cJSON* obj = cJSON_CreateObject();
	if (obj) {
		cJSON_AddStringToObject(obj, "client_ipaddr", ip_addr);
		char* out = cJSON_PrintUnformatted(obj);
		body.append(out);
		free(out);
		cJSON_Delete(obj);
	}
	if (body.empty())
		return -1;
	net_vio_set_cfg(loginHandle,client_ip_addr,body.c_str(),body.length());
    return 0;
}  

/*
open image receive
param:
    image_type_get : 
    none = 0, close image receive
    left = 1, only receive left image 
    right = 2, only receive right image 
    stereo = 3,receive left & right image 
*/
int baton_open_image_recv(image_get image_type_get){
    if(image_type_get == none)
        std::cout << "image receive close" << std::endl;
    else if(image_type_get == left)
        std::cout << "left image receive open" << std::endl;
    else if(image_type_get == right)
        std::cout << "right image receive open" << std::endl;
    else if(image_type_get == stereo)
        std::cout << "left and right image receive open" << std::endl;
    std::string body = "";
	cJSON* obj = cJSON_CreateObject();
	if (obj) {
		cJSON_AddNumberToObject(obj, "gray_image_enable", (int)image_type_get);
		char* out = cJSON_PrintUnformatted(obj);
		body.append(out);
		free(out);
		cJSON_Delete(obj);
	}
	if (body.empty())
		return -1;
	net_vio_set_cfg(loginHandle,ctrl_smart_status,body.c_str(),body.length());
    return 0;
}

//start stereo3 algorithm
bool alog_start(){
    std::cout << "alog_start" << std::endl;
    char buffer[] = "{}";
    return net_vio_set_cfg(loginHandle, algorithm_enable, buffer, strlen(buffer));
}

//stop stereo3 algorithm
bool algo_stop(){
    std::cout << "algo_stop" << std::endl;
    char buffer[] = "{}";
    return net_vio_set_cfg(loginHandle, algorithm_disable, buffer, strlen(buffer));
}

//restart stereo3 algorithm
bool algo_restart(){
    std::cout << "algo_restart" << std::endl;
    char buffer[]="{}";
    return net_vio_set_cfg(loginHandle, algorithm_reboot, buffer, strlen(buffer));
}

/*
get device version
include :
baton software version
stereo3 algorithm version
device hardware version
ros version
device sn code
md_sn_code
*/
void get_device_version(){
    char buffer[cfg_max];
    memset(buffer,'\0',sizeof(buffer));
    net_vio_get_cfg(loginHandle,cfg_version,buffer,cfg_max);
    //解析json
    json j = json::parse(buffer);
    // 提取字段值
    std::string baton_software_version = j["viobot_version"];
    std::string stereo3_version = j["stereo3_version"];
    std::string hardware_version = j["hardware_version"];
    std::string ros_version = j["ros_version"];
    std::string sn_code = j["SN_code"];
    std::string md_sn_code = j["MD_SN_code"];
    // 打印解析结果
    std::cout << "==========================version=========================\n";
    std::cout << "baton_software_version: " << baton_software_version << std::endl;
    std::cout << "stereo3_version: " << stereo3_version << std::endl;
    std::cout << "hardware_version: " << hardware_version << std::endl;
    std::cout << "ros_version: " << ros_version << std::endl;
    std::cout << "SN_code: " << sn_code << std::endl;
    std::cout << "MD_SN_code: " << md_sn_code << std::endl;
    std::cout << "==========================================================\n";
}

/*
get device param
include :
left & right camera instrinsic param fx fy cx cy xi alpha
the extrinsic param from right camera to left camera
the extrinsic param from left camera to imu
*/
void get_device_param(){
    char buffer[cfg_max];
    memset(buffer,'\0',sizeof(buffer));
    net_vio_get_cfg(loginHandle,device_param,buffer,cfg_max);
    // std::cout << buffer << std::endl;
    //解析json
    cam_fisheye_param baton_cam_param;
    extrinsic_Quaternion baton_camL2I_extrinsic;
    json j = json::parse(buffer);
    // 提取字段值
    baton_cam_param.left_cam.fx = j["left_cam.fx"];
    baton_cam_param.left_cam.fy = j["left_cam.fy"];
    baton_cam_param.left_cam.cx = j["left_cam.cx"];
    baton_cam_param.left_cam.cy = j["left_cam.cy"];
    baton_cam_param.left_cam.xi = j["left_cam.xi"];
    baton_cam_param.left_cam.alpha = j["left_cam.alpha"];
    baton_cam_param.right_cam.fx = j["right_cam.fx"];
    baton_cam_param.right_cam.fy = j["right_cam.fy"];
    baton_cam_param.right_cam.cx = j["right_cam.cx"];
    baton_cam_param.right_cam.cy = j["right_cam.cy"];
    baton_cam_param.right_cam.xi = j["right_cam.xi"];
    baton_cam_param.right_cam.alpha = j["right_cam.alpha"];
    baton_cam_param.extrinsic.px = j["cam_extrinsic.px"];
    baton_cam_param.extrinsic.py = j["cam_extrinsic.py"];
    baton_cam_param.extrinsic.pz = j["cam_extrinsic.pz"];
    baton_cam_param.extrinsic.qx = j["cam_extrinsic.qx"];
    baton_cam_param.extrinsic.qy = j["cam_extrinsic.qy"];
    baton_cam_param.extrinsic.qz = j["cam_extrinsic.qz"];
    baton_cam_param.extrinsic.qw = j["cam_extrinsic.qw"];
    baton_camL2I_extrinsic.px = j["CL2I_extrinsic.px"];
    baton_camL2I_extrinsic.py = j["CL2I_extrinsic.py"];
    baton_camL2I_extrinsic.pz = j["CL2I_extrinsic.pz"];
    baton_camL2I_extrinsic.qx = j["CL2I_extrinsic.qx"];
    baton_camL2I_extrinsic.qy = j["CL2I_extrinsic.qy"];
    baton_camL2I_extrinsic.qz = j["CL2I_extrinsic.qz"];
    baton_camL2I_extrinsic.qw = j["CL2I_extrinsic.qw"];

    // 打印解析结果
    std::cout << "==========================device param=========================\n";
    std::cout << "baton_left_instrinsic: \n";
    printf("fx: %lf\n", baton_cam_param.left_cam.fx);
    printf("fy: %lf\n", baton_cam_param.left_cam.fy);
    printf("cx: %lf\n", baton_cam_param.left_cam.cx);
    printf("cy: %lf\n", baton_cam_param.left_cam.cy);
    printf("xi: %lf\n", baton_cam_param.left_cam.xi);
    printf("alpha: %lf\n", baton_cam_param.left_cam.alpha);

    std::cout << "baton_right_instrinsic: \n";
    printf("fx: %lf\n", baton_cam_param.right_cam.fx);
    printf("fy: %lf\n", baton_cam_param.right_cam.fy);
    printf("cx: %lf\n", baton_cam_param.right_cam.cx);
    printf("cy: %lf\n", baton_cam_param.right_cam.cy);
    printf("xi: %lf\n", baton_cam_param.right_cam.xi);
    printf("alpha: %lf\n", baton_cam_param.right_cam.alpha);

    std::cout << "baton_right2left_extrinsic: \n";
    printf("px:%lf\n",baton_cam_param.extrinsic.px);
    printf("py:%lf\n",baton_cam_param.extrinsic.py);
    printf("pz:%lf\n",baton_cam_param.extrinsic.pz);
    printf("qx:%lf\n",baton_cam_param.extrinsic.qx);
    printf("qy:%lf\n",baton_cam_param.extrinsic.qy);
    printf("qz:%lf\n",baton_cam_param.extrinsic.qz);
    printf("qw:%lf\n",baton_cam_param.extrinsic.qw);

    std::cout << "baton_camL2I_extrinsic: \n";
    printf("px:%lf\n",baton_camL2I_extrinsic.px);
    printf("py:%lf\n",baton_camL2I_extrinsic.py);
    printf("pz:%lf\n",baton_camL2I_extrinsic.pz);
    printf("qx:%lf\n",baton_camL2I_extrinsic.qx);
    printf("qy:%lf\n",baton_camL2I_extrinsic.qy);
    printf("qz:%lf\n",baton_camL2I_extrinsic.qz);
    printf("qw:%lf\n",baton_camL2I_extrinsic.qw);
    std::cout << "==========================================================\n";
}

/*
set_network
param:
    ipaddr : The IP address you want to set for the device
*/
int set_network(const char* ipaddr){
    std::string body = "";
	cJSON* obj = cJSON_CreateObject();
    const char* submask = "255.255.255.0";
    char gateway[16]; 
    char ip_copy[16];
    strcpy(ip_copy, ipaddr); 

    char* token = strtok(ip_copy, ".");
    int part_count = 0;
    while (token != nullptr && part_count < 3) {
        strcat(gateway, token);
        strcat(gateway, ".");
        token = strtok(nullptr, ".");
        part_count++;
    }
    strcat(gateway, "0"); 
	if (obj) {
		cJSON_AddStringToObject(obj, "ipaddr", ipaddr);
        cJSON_AddStringToObject(obj, "submask", submask);
        cJSON_AddStringToObject(obj, "gateway", ipaddr);
		char* out = cJSON_PrintUnformatted(obj);
		body.append(out);
		free(out);
		cJSON_Delete(obj);
	}
	if (body.empty())
		return -1;
	net_vio_set_cfg(loginHandle,cfg_network,body.c_str(),body.length());
    return 0;
}

