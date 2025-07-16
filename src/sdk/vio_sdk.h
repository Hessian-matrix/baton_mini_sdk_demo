//#pragma once
#ifndef _vio_sdk_h_
#define _vio_sdk_h_

#if (defined(_WIN32)) //windows
#ifdef VIOSDK_EXPORTS
#define NET_VIO_API  extern "C" __declspec(dllexport)
#else
#define NET_VIO_API  extern "C" __declspec(dllimport)
#endif
#else
#define NET_VIO_API extern "C"
#endif

#ifndef _WINDOWS_
#if (defined(_WIN32) || defined(_WIN64))
#include <winsock2.h>
#include <windows.h>    
#endif
#endif

#ifdef __linux__
typedef int BOOL;
typedef long LONG;
typedef void* HANDLE;
#define __stdcall
#endif

#define vio_call __stdcall

/**
 * @brief 锟斤拷锟接回碉拷
 * @param [out] state:锟斤拷锟斤拷状态,1-锟斤拷锟斤拷锟接ｏ拷0-锟窖断匡拷锟斤拷-1-锟斤拷锟斤拷
 * @param [out] userData:锟矫伙拷锟皆讹拷锟斤拷锟斤拷锟斤拷
 * @return 锟斤拷
 */
typedef void(vio_call* vio_connect_callback)(int state, void* userData);

/**
 * @brief 锟铰硷拷锟截碉拷
 * @param [out] data:锟斤拷锟斤拷锟斤拷锟斤拷
 * @param [out] length:锟斤拷锟捷筹拷锟斤拷
 * @param [out] userData:锟矫伙拷锟皆讹拷锟斤拷锟斤拷锟斤拷
 * @return 锟斤拷
 */
typedef void(vio_call* vio_event_callback)(const char* data, int length, void* userData);

//锟斤拷录锟斤拷息
typedef struct
{
	char	ipaddr[32];		
	int		port;
	char	username[32];
	char	password[32];
	void* userData;
	vio_connect_callback connect_cb;
	vio_event_callback event_cb;
}vio_login_info_s;

//network config
typedef struct{
	char ipaddr[32];
	char submask[32];
	char gateway[32];
	char macaddr[32];
	int commandPort;
	int heartbeatPort;
	int udpPort;
}vio_network_cfg_s;

//camera_param
typedef struct{
	float focal_length_x;
	float focal_length_y;
	float optical_center_point_x;
	float optical_center_point_y;
	float radia_distortion_coef_k1;
	float radia_distortion_coef_k2;
	float tangential_distortion_p1;
	float tangential_distortion_p2;
}vio_lens_cfg_s;

//hardware switch
typedef struct{
	int gray_image_enable;
	int imu_enable;
	int tof_enable;
	int tof_deep_image_enable;
	int tof_amp_image_enable;
	int light;
	int odom;
}vio_smart_cfg_s;

//锟斤拷锟斤拷时锟斤拷OSD
typedef struct
{
	int display;
	int x, y;
}vio_osd_datetime_s;

//OSD锟斤拷锟斤拷
typedef struct
{
	vio_osd_datetime_s osd_datetime;
	int dispaly_feature_pots;//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
}vio_osd_cfg_s;

//imu锟节诧拷
typedef struct{
	float acc_n, acc_w, gyr_n, gyr_w;
}vio_imu_inter_cfg_s;

//4x3锟斤拷锟斤拷
typedef struct{
	float value[3][4];
}vio_mat3x4_s;

//帧锟斤拷锟斤拷
typedef enum{
	vio_frame_algo_pose = 5,
	vio_frame_loop_pose = 6,
	vio_frame_sys_state = 8,
	vio_frame_pose_and_twist = 26,
}vio_frame_type_e;

//帧锟斤拷息
typedef struct
{
	unsigned int 	type;
	double			timestamp;
	unsigned int	seq;
	unsigned int 	width;
	unsigned int 	height;
	unsigned int 	length;
}vio_frame_info_s;

//imu帧锟斤拷锟斤拷
typedef struct
{
	double acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z;
}vio_frame_imu_s;

/**
 * @brief 锟斤拷取SDK锟芥本锟斤拷息
 * @param 锟斤拷
 * @return SDK锟芥本锟斤拷息锟斤拷2锟斤拷锟斤拷锟街节憋拷示锟斤拷锟芥本锟斤拷2锟斤拷锟斤拷锟街节憋拷示锟轿版本锟斤拷锟斤拷0x00030000:锟斤拷示锟芥本为3.0
 */
NET_VIO_API LONG vio_call net_vio_sdk_version();

/**
 * @brief SDK锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷始锟斤拷锟斤拷锟揭伙拷渭锟斤拷锟�
 * @param 锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_sdk_init();

/**
 * @brief SDK锟剿筹拷锟斤拷锟斤拷锟斤拷锟斤拷锟角帮拷锟斤拷锟揭伙拷渭锟斤拷锟�
 * @param 锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_sdk_exit();

/**
 * @brief 锟借备锟斤拷录
 * @param [in] login_info:锟斤拷录锟斤拷息
 * @return 锟缴癸拷锟斤拷锟截碉拷录锟斤拷锟斤拷锟绞э拷芊锟斤拷锟絅ULL
 */
NET_VIO_API HANDLE vio_call net_vio_login(vio_login_info_s loginInfo);

/**
 * @brief 锟借备锟角筹拷
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_logout(HANDLE loginHandle);

/**
 * @brief 锟斤拷锟斤拷锟捷回碉拷
 * @param [out] channel:通锟斤拷锟斤拷
 * @param [out] frameInfo:帧锟斤拷息
 * @param [out] frame:帧锟斤拷锟斤拷
 * @param [out] userData:锟矫伙拷锟皆讹拷锟斤拷锟斤拷锟斤拷
 * @return 锟斤拷
 */
typedef void(vio_call *vio_stream_callback)(int channel, const vio_frame_info_s* frameInfo, const char* frameData, void* userData);

/**
 * @brief 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [in] channel:通锟斤拷锟脚ｏ拷1-IMU+锟截伙拷位锟斤拷+锟姐法位锟剿ｏ拷2-锟缴硷拷锟斤拷叶锟酵硷拷锟�3-锟斤拷锟酵�+锟斤拷锟斤拷图锟斤拷4-锟斤拷图锟斤拷锟斤拷;5-tof锟斤拷锟斤拷;6-锟斤拷目图锟斤拷;
 * @param [in] cb:锟斤拷锟斤拷锟捷回碉拷
 * @return 锟缴癸拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟绞э拷芊锟斤拷锟絅ULL
 */
NET_VIO_API HANDLE vio_call net_vio_stream_connect(HANDLE loginHandle, int channel, vio_stream_callback cb);

/**
 * @brief 锟斤拷锟斤拷锟捷断匡拷
 * @param [in] streamHandle:锟斤拷锟斤拷锟�
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_stream_disconnect(HANDLE streamHandle);

/**
 * @brief 锟斤拷取锟斤拷锟斤拷
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [in] uri:锟接口凤拷锟绞碉拷址
 * @param [out] dataBuff:锟斤拷锟捷伙拷锟斤拷锟斤拷
 * @param [in] buffSize:锟斤拷锟斤拷锟斤拷锟斤拷小
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_get_cfg(HANDLE loginHandle, const char* uri, char* dataBuff, int buffSize);

/**
 * @brief 锟斤拷锟矫诧拷锟斤拷
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [in] uri:锟接口凤拷锟绞碉拷址
 * @param [in] data:锟斤拷锟斤拷锟斤拷锟�
 * @param [in] length:锟斤拷锟斤拷锟斤拷锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_set_cfg(HANDLE loginHandle, const char* uri, const char* data, int length);

/**
 * @brief 锟斤拷取锟斤拷锟斤拷锟斤拷锟�
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [out] cfg:锟斤拷锟斤拷锟斤拷锟街革拷锟�
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_get_cfg_network(HANDLE loginHandle, vio_network_cfg_s* cfg);

/**
 * @brief 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [in] cfg:锟斤拷锟斤拷锟斤拷锟街革拷锟�
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_set_cfg_network(HANDLE loginHandle, vio_network_cfg_s* cfg);

/**
 * @brief 锟斤拷取锟斤拷头锟斤拷锟斤拷
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [in] camera:1锟斤拷锟斤拷锟斤拷目锟缴硷拷锟解；2锟斤拷锟斤拷锟斤拷目锟缴硷拷锟解；3锟斤拷锟斤拷TOF锟斤拷锟�
 * @param [out] cfg:锟斤拷头锟斤拷锟斤拷指锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_get_cfg_lens(HANDLE loginHandle, int camera, vio_lens_cfg_s* cfg);

/**
 * @brief 锟斤拷取imu锟节诧拷
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [out] cfg:imu锟节诧拷指锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_get_cfg_imu_inter(HANDLE loginHandle, vio_imu_inter_cfg_s* cfg);

/**
 * @brief 锟斤拷取Smart锟斤拷锟斤拷
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [out] cfg:Smart锟斤拷锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_get_cfg_smart(HANDLE loginHandle, vio_smart_cfg_s* cfg);

/**
 * @brief 锟斤拷锟斤拷Smart锟斤拷锟斤拷
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [in] cfg:Smart锟斤拷锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_set_cfg_smart(HANDLE loginHandle, vio_smart_cfg_s* cfg);

/**
 * @brief 锟姐法锟斤拷锟斤拷
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [in] type:锟姐法锟斤拷锟酵★拷1-锟姐法1锟斤拷2-锟姐法2
 * @param [in] cmd:锟斤拷锟斤拷指锟筋。0-锟斤拷锟矫ｏ拷1-锟斤拷锟矫ｏ拷2-锟斤拷锟斤拷锟斤拷3-锟斤拷锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_algorithm_control(HANDLE loginHandle, int type, int cmd);

/**
 * @brief 锟截讹拷位
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [in] mat:锟斤拷锟斤拷 
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_relocation(HANDLE loginHandle, vio_mat3x4_s* mat);

/**
 * @brief 锟斤拷取锟斤拷锟絚am to imu
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [out] mat:锟斤拷锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_get_cfg_cam2imu(HANDLE loginHandle, vio_mat3x4_s* mat);

/**
 * @brief 锟斤拷取锟斤拷锟絠mu to cam
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [out] mat:锟斤拷锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_get_cfg_imu2cam(HANDLE loginHandle, vio_mat3x4_s* mat);

/**
 * @brief 锟斤拷取锟斤拷锟絫of to imu
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [out] mat:锟斤拷锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_get_cfg_tof2imu(HANDLE loginHandle, vio_mat3x4_s* mat);

/**
 * @brief 锟斤拷取锟斤拷锟絠mu to tof
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [out] mat:锟斤拷锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_get_cfg_imu2tof(HANDLE loginHandle, vio_mat3x4_s* mat);

/**
 * @brief 锟斤拷取OSD锟斤拷锟斤拷
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [out] cfg:osd锟斤拷锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_get_cfg_osd(HANDLE loginHandle, vio_osd_cfg_s* cfg);

/**
 * @brief 锟斤拷锟斤拷OSD锟斤拷锟斤拷
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @param [in] cfg:osd锟斤拷锟斤拷
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_set_cfg_osd(HANDLE loginHandle, vio_osd_cfg_s* cfg);

/**
 * @brief 系统锟斤拷锟斤拷
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_system_reboot(HANDLE loginHandle);

/**
 * @brief 锟截伙拷
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_system_shutdown(HANDLE loginHandle);

/**
 * @brief 锟斤拷锟接关硷拷帧
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_add_key_frame(HANDLE loginHandle);

/**
 * @brief 锟斤拷锟斤拷丶锟街�
 * @param [in] loginHandle:锟斤拷录锟斤拷锟�
 * @return 锟缴癸拷锟斤拷锟斤拷1锟斤拷失锟杰凤拷锟截达拷锟斤拷锟斤拷
 */
NET_VIO_API BOOL vio_call net_vio_save_key_frame(HANDLE loginHandle);

#endif
