#include <mutex>
#include <thread>
#include "baton_mini.h"
#include "baton_ros.h"

using namespace std;

system_status sys_status = ready;//define the status of system

vio_login_info_s login_info = {
    "192.168.1.10",
    8000,
    "",
    "",
    NULL,
    NULL,
    NULL
};

//connect_callback get status for connect
void vio_call connect_callback(int state, void* userData){
    if (state == 1)
        printf("connect ok !\n");
    else if (state == 0)
        printf("disconnected !\n");
    else if(state == -1)
        printf("connect error !\n");
    else if (state == -2)
        printf("keepalive thread exit !\n");
    else
        printf("unknown !\n");
}

//receive device heartbeat signal
void vio_call event_callback(const char* data, int length, void* userData){
    printf("event_callback:%s\n",data);
}

void odom_data_print(int length, const char* frameData) {
    // printf("odom_data:\n");
	odom_t odom;
	memcpy(&odom.pose,frameData,sizeof(odom.pose));
	// std::cout << "\t Position:" << odom.pose.px << "," << odom.pose.py << "," << odom.pose.pz << "\n";
	// std::cout << "\t Quaternion:" << odom.pose.qx << "," << odom.pose.qy << "," << odom.pose.qz << "," << odom.pose.qw << "\n";
	memcpy(&odom.speed,frameData + sizeof(odom.pose),sizeof(odom.speed));
	// std::cout << "\t twist:" << odom.speed.lx << "," << odom.speed.ly << "," << odom.speed.lz 
	//           << "," << odom.speed.ax << "," << odom.speed.ay << "," << odom.speed.az << "\n";
	publish_odom(odom);//ros publish odometry
}

void vio_call stream_callback(int channel, const vio_frame_info_s* frameInfo, const char* frameData, void* userData){
    if (frameInfo->type == vio_frame_pose_and_twist) {//get the algo odometry
        odom_data_print(frameInfo->length,frameData);
    }
    else if (frameInfo->type == vio_frame_sys_state) {//get the status of system
        sys_status = static_cast<system_status>(frameData[frameInfo->length - 1]);
        // printf("sys_status:%d\n", frameData[frameInfo->length - 1]);
    }
}

void vio_sdk_init(){
    printf("sdk version : 0x%08lx\n", net_vio_sdk_version());
    net_vio_sdk_init();
    // login_info.ipaddr = "192.168.1.10"; //Set the IP address of the device you want to connect to
    login_info.event_cb = event_callback;//print heartbreat
    login_info.connect_cb = connect_callback;//get connect status

    loginHandle = net_vio_login(login_info);
    if (!loginHandle){
        cout << "login fail\n";
        net_vio_sdk_exit();
        cout << "ByeBye\n";
        exit(0);
    }
    std::cout << "login success\n";
}

void command_thread(){
    vio_sdk_init();
    HANDLE streamHandle1 = net_vio_stream_connect(loginHandle, 1, stream_callback);
    
    get_device_version();
    get_device_param();
    // set_network("192.168.1.10");
    cout << "Please input the operation[0-4] : ";   

    imu_switch imu_status = OFF;
    int v;
    while (cin >> v){
        if (v == 0) {//logout connect
            net_vio_logout(loginHandle);
            break;
        }
        else if (v == 1){//start or stop
            if (sys_status == ready) {//The system is currently in a ready state and can be started directly
                alog_start();
            }
            else if (sys_status == stereo3_running) {//The system is currently in a running state and can be stoped
                algo_stop();
            }
        }
        else if (v == 2) {//algo restart
            if (sys_status == stereo3_running) {//The stereo3 algorithm is currently in a running state and can be restart
                algo_restart();
            }
        }
		else if(v == 3){//open or close imu receive
            if(imu_status == ON){
                imu_status = OFF;
			    baton_open_imu_recv(imu_status);
            }
            else{
                imu_status = ON;
                baton_open_imu_recv(imu_status);
            }
		}
        else if(v == 4){//Select to receive image data 
            baton_open_image_recv(stereo);
        }
    }
    net_vio_sdk_exit();
    cout << "ByeBye\n";
}

int main(int argc, char** argv){
	ros::init(argc, argv, "baton_mini");
    ros::NodeHandle nh;
    baton_ros_init(nh);

	IMU imu_recv(&publish_imu);
    Image image_left_recv(&publish_image_left);//left default
    Image image_right_recv(&publish_image_right,false);//right must set false
    
    std::thread http_command{command_thread};

    ros::MultiThreadedSpinner spinner(3);
	spinner.spin();
    ros::shutdown();
    http_command.join();
    return 0;
}

