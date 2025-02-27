#pragma once
#include <thread>
#include <atomic>
#include <cstring>
#include <iostream>
#include <functional>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct imu_data {
    double acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z;
    bool keyframe;
};

extern std::string server_ip;
class IMU {
public:
    IMU(const std::function<void(imu_data&)>& imu_callback) {
        sock = socket(AF_INET, SOCK_STREAM, 0); // 使用 SOCK_STREAM 表示 TCP
        if (sock < 0) {
            std::cerr << "Error creating socket" << std::endl;
            return;
        }

        // 配置服务器地址
        targetAddr.sin_family = AF_INET;
        targetAddr.sin_port = htons(9996);
        if (inet_pton(AF_INET, server_ip.c_str(), &targetAddr.sin_addr) <= 0) {
            std::cerr << "Invalid server IP address" << std::endl;
            close(sock);
            return;
        }

        // 连接到服务器
        if (connect(sock, (struct sockaddr*)&targetAddr, sizeof(targetAddr)) < 0) {
            std::cerr << "Failed to connect to server at " << server_ip << " :9996" << std::endl;
            close(sock);
            return;
        }

        imu_data_use = imu_callback;
        imu_data_recv_thread = std::thread(&IMU::imu_data_recv_loop, this);
    }

    ~IMU() {
        running_ = false;
        if (imu_data_recv_thread.joinable())
            imu_data_recv_thread.join();
        close(sock);
    }

private:
    bool imu_data_unpack(imu_data& imu) {
        int rec_len = imu_tcp_recv(imu_data_recv);
        if (rec_len == 54) {
            if (imu_data_recv[0] == 0x66 && imu_data_recv[1] == 0x10 && imu_data_recv[52] == 0x55 && imu_data_recv[53] == 0x99) {
                unsigned char check_sum = 0;
                for (int i = 2; i < 51; i++) {
                    check_sum += imu_data_recv[i];
                }
                if (check_sum == imu_data_recv[51]) {
                    // std::cout << "check_sum ok" << std::endl;
                    if (imu_data_recv[2] == 0x01) imu.keyframe = true;
                    else imu.keyframe = false;
                    memcpy(&imu.acc_x, imu_data_recv + 3, sizeof(imu.acc_x));
                    memcpy(&imu.acc_y, imu_data_recv + 11, sizeof(imu.acc_y));
                    memcpy(&imu.acc_z, imu_data_recv + 19, sizeof(imu.acc_z));
                    memcpy(&imu.gyro_x, imu_data_recv + 27, sizeof(imu.gyro_x));
                    memcpy(&imu.gyro_y, imu_data_recv + 35, sizeof(imu.gyro_y));
                    memcpy(&imu.gyro_z, imu_data_recv + 43, sizeof(imu.gyro_z));
                    return true;
                } else {
                    std::cout << "Check sum error: 0x" << std::hex << static_cast<int>(imu_data_recv[51]) << ", expected 0x" << static_cast<int>(check_sum) << std::dec << std::endl;
                    return false;
                }
            } else {
                std::cout << "Data format error" << std::endl;
                return false;
            }
        }
        return false;
    }

    ssize_t imu_tcp_recv(unsigned char* recv_buf) {
        ssize_t bytesReceived = recv(sock, recv_buf, 54, 0); // 每次接收 54 字节
        if (bytesReceived != 54) {
            std::cerr << "Received incomplete data: " << bytesReceived << " bytes" << std::endl;
            if(bytesReceived == 0) exit(0);
            return -1;
        }
        return bytesReceived;
    }

    void imu_data_recv_loop() {
        imu_data imu_;
        while (running_) {
            if (imu_data_unpack(imu_))
                imu_data_use(imu_);
            else
                std::cout << "IMU data receive error" << std::endl;
        }
    }

public:

private:
    int sock;
    struct sockaddr_in targetAddr;
    unsigned char imu_data_recv[54];
    std::thread imu_data_recv_thread;
    std::function<void(imu_data&)> imu_data_use;
    std::atomic<bool> running_{true};
};