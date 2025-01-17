#pragma once

#include <thread> 
#include <atomic>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <functional>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <netinet/in.h>

struct imu_data{
	double acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z;
	bool keyframe;
};

class IMU {
public:
    IMU(const std::function<void(imu_data&)>& imu_callback) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            std::cerr << "Error creating socket" << std::endl;
            return;
        }

        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(9996); // 端口号
        serverAddr.sin_addr.s_addr = INADDR_ANY; // 特定网卡的IP地址

        // 绑定socket
        if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Error binding socket" << std::endl;
            return;
        }

        memset(&fromAddr, 0, sizeof(fromAddr));
        fromAddrLen = sizeof(fromAddr);

        imu_data_use = imu_callback;
        imu_data_recv_thread = std::thread(&IMU::imu_data_recv_loop,this);
    }

    ~IMU() {
        running_ = false;
        if(imu_data_recv_thread.joinable())
            imu_data_recv_thread.join();
        close(sock);
    }

private:
    bool imu_data_pack(imu_data& imu){
        int rec_len = imu_udp_recv(imu_data_recv);
		if(rec_len == 54){
			if(imu_data_recv[0] == 0x66 && imu_data_recv[1] == 0x10 && imu_data_recv[52] == 0x55 && imu_data_recv[53] == 0x99){
				unsigned char check_sum = 0;
				for(int i = 2; i < 51;i++){
					check_sum +=  imu_data_recv[i];
				}
				if(check_sum == imu_data_recv[51]){
					// std::cout << "check_sum ok" << std::endl;
					if(imu_data_recv[2] == 0x01) imu.keyframe = true;
					else imu.keyframe = false;
					memcpy(&imu.acc_x,imu_data_recv + 3,sizeof(imu.acc_x));
					memcpy(&imu.acc_y,imu_data_recv + 11,sizeof(imu.acc_y));
					memcpy(&imu.acc_z,imu_data_recv + 19,sizeof(imu.acc_z));
					memcpy(&imu.gyro_x,imu_data_recv + 27,sizeof(imu.gyro_x));
					memcpy(&imu.gyro_y,imu_data_recv + 35,sizeof(imu.gyro_y));
					memcpy(&imu.gyro_z,imu_data_recv + 43,sizeof(imu.gyro_z));
                    return true;
                }
				else{
					std::cout << "check_sum error: " << std::endl;
					printf("recv : 0x%2x,check : 0x%2x\n",imu_data_recv[51],check_sum);
                    return false;
                } 
			}
			else{
				std::cout << "data recv error" << std::endl;
                return false;
            }
        }
        return false;
    }

    ssize_t imu_udp_recv(unsigned char* recv_buf) {
        ssize_t bytesReceived = recvfrom(sock, recv_buf, 1024, 0, (struct sockaddr*)&fromAddr, &fromAddrLen);
        return bytesReceived;
    }

    void imu_data_recv_loop(){
        imu_data imu_;
        while(running_){
            if(imu_data_pack(imu_))
                imu_data_use(imu_);
            else
                std::cout << "imu data recv error" << std::endl;
        }
    }

public:

private:
    int sock;
    struct sockaddr_in serverAddr;
    struct sockaddr_in fromAddr;
    socklen_t fromAddrLen;
    unsigned char imu_data_recv[54];
    std::thread imu_data_recv_thread;
    std::function<void(imu_data&)> imu_data_use;
    std::atomic<bool> running_{true};
};

