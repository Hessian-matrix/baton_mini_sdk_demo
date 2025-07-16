#pragma once
#include <thread>
#include <atomic>
#include <cstring>
#include <iostream>
#include <functional>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct pose_pack{
    float px,py,pz,qx,qy,qz,qw;
    // 重载赋值运算符
    pose_pack& operator=(const pose_pack& other) noexcept{
        if (this != &other) {
            px = other.px;
            py = other.py;
            pz = other.pz;
            qx = other.qx;
            qy = other.qy;
            qz = other.qz;
            qw = other.qw;
        }
        return *this;
    }
};

struct speed_pack{
    float lx,ly,lz,ax,ay,az;
    // 重载赋值运算符的声明
    speed_pack& operator=(const speed_pack& other) noexcept{
        if (this != &other) {
            lx = other.lx;
            ly = other.ly;
            lz = other.lz;
            ax = other.ax;
            ay = other.ay;
            az = other.az;
        }
        return *this;
    }
};

struct odom_pack{
    pose_pack pose;
    speed_pack twist;
    double t_s;
    odom_pack& operator=(const odom_pack& other) noexcept{
        if (this != &other) {
            pose = other.pose;
            twist = other.twist;
            t_s = other.t_s;
        }
        return *this;
    }
};

const int odom_buf_len = 65;

extern std::string server_ip;
class Fast_odom {
public:
    Fast_odom(const std::function<void(odom_pack&)>& odom_callback) {
        sock = socket(AF_INET, SOCK_STREAM, 0); // 使用 SOCK_STREAM 表示 TCP
        if (sock < 0) {
            std::cerr << "Error creating socket" << std::endl;
            return;
        }

        // 配置服务器地址
        targetAddr.sin_family = AF_INET;
        targetAddr.sin_port = htons(9994);
        if (inet_pton(AF_INET, server_ip.c_str(), &targetAddr.sin_addr) <= 0) {
            std::cerr << "Invalid server IP address" << std::endl;
            close(sock);
            return;
        }

        // 连接到服务器
        if (connect(sock, (struct sockaddr*)&targetAddr, sizeof(targetAddr)) < 0) {
            std::cerr << "Failed to connect to server at " << server_ip << " :9994" << std::endl;
            close(sock);
            return;
        }

        Fast_odom_data_use = odom_callback;
        odom_data_recv_thread = std::thread(&Fast_odom::fast_odom_data_recv_loop, this);
    }

    ~Fast_odom() {
        running_ = false;
        if (odom_data_recv_thread.joinable())
            odom_data_recv_thread.join();
        close(sock);
    }

private:
    bool odom_data_unpack(odom_pack& odom) {
        int rec_len = odom_tcp_recv(odom_data_recv);
        if (rec_len == odom_buf_len) {
            if (odom_data_recv[0] == 0x66 && odom_data_recv[1] == 0x10 && odom_data_recv[odom_buf_len - 2] == 0x55 && odom_data_recv[odom_buf_len - 1] == 0x99) {
                unsigned char check_sum = 0;
                for (int i = 2; i < odom_buf_len - 3; i++) {
                    check_sum += odom_data_recv[i];
                }
                if (check_sum == odom_data_recv[odom_buf_len - 3]) {
                    // std::cout << "check_sum ok" << std::endl;
                    memcpy(&odom.pose, odom_data_recv + 2, sizeof(odom.pose));
                    memcpy(&odom.twist, odom_data_recv + 30, sizeof(odom.twist));
                    memcpy(&odom.t_s, odom_data_recv + 54, sizeof(odom.t_s));
                    return true;
                } else {
                    std::cout << "Check sum error: 0x" << std::hex << static_cast<int>(odom_data_recv[odom_buf_len - 3]) << ", expected 0x" << static_cast<int>(check_sum) << std::dec << std::endl;
                    return false;
                }
            } else {
                std::cout << "Data format error" << std::endl;
                return false;
            }
        }
        return false;
    }

    ssize_t odom_tcp_recv(unsigned char* recv_buf) {
        ssize_t bytesReceived = recv(sock, recv_buf, odom_buf_len, 0); // 每次接收 odom_buf_len 字节
        if (bytesReceived != odom_buf_len) {
            std::cerr << "Received incomplete data: " << bytesReceived << " bytes" << std::endl;
            if(bytesReceived == 0) exit(0);
            return -1;
        }
        return bytesReceived;
    }

    void fast_odom_data_recv_loop() {
        odom_pack odom_;
        while (running_) {
            if (odom_data_unpack(odom_))
                Fast_odom_data_use(odom_);
            else
                std::cout << "Fast_odom data receive error" << std::endl;
        }
    }

public:

private:
    int sock;
    struct sockaddr_in targetAddr;
    unsigned char odom_data_recv[odom_buf_len];
    std::thread odom_data_recv_thread;
    std::function<void(odom_pack&)> Fast_odom_data_use;
    std::atomic<bool> running_{true};
};