#pragma once
#include <string>
using namespace std;

#include "vio_sdk.h"

typedef enum
{
	vio_method_get,
	vio_method_put
}vio_method_e;

typedef enum
{
	vio_private_msg_null,
	vio_private_msg_heartbeat,
	vio_private_msg_event
}vio_private_msg_cmd_type_e;

// http msg
//return status code
int vio_msg_check_resp(std::string respmsg);

//int vio_msg_build_reqpkt_network(std::string& reqmsg, vio_method_e method, vio_network_cfg_s* cfg);
//int vio_msg_parse_resppkt_network(std::string respmsg, vio_network_cfg_s* cfg);

int vio_msg_build_reqpkt_stream(std::string& reqmsg, int channel);
int vio_msg_build_reqpkt_reboot_system(std::string& reqmsg);
int vio_msg_build_reqpkt_system_shutdown(std::string& reqmsg);

int vio_msg_build_reqpkt_algorithm_control(std::string& reqmsg, int type, int cmd);

//int vio_msg_build_reqpkt_SmartRelocation(std::string& reqmsg, vio_mat3x4_s* mat);
int vio_msg_build_reqpkt_SmartAddKeyFrame(std::string& reqmsg);
int vio_msg_build_reqpkt_SmartSaveKeyFrame(std::string& reqmsg);

//int vio_msg_build_reqpkt_imu_inter(std::string& reqmsg);
//int vio_msg_parse_resppkt_imu_inter(std::string respmsg, vio_imu_inter_cfg_s* cfg);

//int vio_msg_build_reqpkt_lens(std::string& reqmsg, vio_method_e method, int camera, vio_lens_cfg_s* cfg);
//int vio_msg_parse_resppkt_lens(std::string respmsg, vio_lens_cfg_s* cfg);

//int vio_msg_build_reqpkt_smart(std::string& reqmsg, vio_method_e method, vio_smart_cfg_s* cfg);
//int vio_msg_parse_resppkt_smart(std::string respmsg, vio_smart_cfg_s* cfg);

//int vio_msg_build_reqpkt_cam2imu(std::string& reqmsg);
//int vio_msg_parse_resppkt_cam2imu(std::string respmsg, vio_mat3x4_s* cfg);

//int vio_msg_build_reqpkt_imu2cam(std::string& reqmsg);
//int vio_msg_parse_resppkt_imu2cam(std::string respmsg, vio_mat3x4_s* cfg);

//int vio_msg_build_reqpkt_tof2imu(std::string& reqmsg);
//int vio_msg_parse_resppkt_tof2imu(std::string respmsg, vio_mat3x4_s* cfg);

//int vio_msg_build_reqpkt_imu2tof(std::string& reqmsg);
//int vio_msg_parse_resppkt_imu2tof(std::string respmsg, vio_mat3x4_s* cfg);

//int vio_msg_build_reqpkt_osd(std::string& reqmsg, vio_method_e method, vio_osd_cfg_s* cfg);
//int vio_msg_parse_resppkt_osd(std::string respmsg, vio_osd_cfg_s* cfg);

// private msg
int vio_private_msg_parse_from_array(const char* data, int size, int* content_len, std::string& content_data);
vio_private_msg_cmd_type_e vio_private_msg_parse_cmd_type(std::string content);
int vio_private_msg_build_heartbeat(std::string& msg);