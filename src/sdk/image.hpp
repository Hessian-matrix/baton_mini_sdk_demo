#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>

class Image {
public:
    Image(const std::function<void(cv::Mat&)>& image_callback,bool is_left = true){
        // 创建 UDP 套接字
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in recv_addr;
        memset(&recv_addr, 0, sizeof(recv_addr));
        recv_addr.sin_family = AF_INET;
        if(is_left)
            recv_addr.sin_port = htons(9997); // 固定端口左目9997
        else
            recv_addr.sin_port = htons(9998); // 固定端口右目9998
        recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sockfd, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) < 0) {
            perror("bind failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        memset(recv_buffer, 0, sizeof(recv_buffer));

        image_data_use = image_callback;
        image_recv_thread = std::thread(&Image::image_recv_loop,this);
    }

    ~Image() {
        running_ = false;
        if(image_recv_thread.joinable())
            image_recv_thread.join();
        close(sockfd);
    }

private:
    void image_recv_loop(){
        cv::Mat image_ = cv::Mat::zeros(480, 640, CV_8UC1);
        while(running_){
            image_.setTo(cv::Scalar(0));
            receive_image(image_);
            image_data_use(image_);
        }
    }    

    void receive_image(cv::Mat& image_) {
        int image_len = image_.total() * image_.elemSize();
        int max_payload_size = 1024; // 每个 UDP 包的最大负载大小
        int header_size = 4; // 帧头大小（包括包序号）
        int footer_size = 1; // 校验和大小
        int max_data_size = max_payload_size - header_size - footer_size;

        int num_packets = (image_len + max_data_size - 1) / max_data_size;
        // std::cout << "num_packets = " << num_packets << std::endl;
        std::vector<bool> packet_received(num_packets, false);
        int received_bytes = 0;
        int expected_packet = 0;

        while (received_bytes < image_len) {
            int n = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, NULL, NULL);
            if (n < 0) {
                perror("recvfrom failed");
                return;
            }

            // 校验帧头
            if (recv_buffer[0] != 0x77 || recv_buffer[1] != 0x11) {
                std::cerr << "Invalid frame header" << std::endl;
                continue;
            }

            // 获取包序号
            unsigned short packet_number = (recv_buffer[2] << 8) | recv_buffer[3];
            // printf("recv_buffer[2] = %d,recv_buffer[3] = %d,packet_number = %d\n",recv_buffer[2],recv_buffer[3],packet_number);
            // 校验和
            unsigned char check_sum = 0;
            for (int j = 0; j < n - 1; ++j) {
                check_sum += recv_buffer[j];
            }
            if (check_sum != recv_buffer[n - 1]) {
                std::cerr << "Checksum error,check_sum = " << check_sum << ",recv_buffer[n - 1] = " << recv_buffer[n - 1] << std::endl;
                continue;
            }

            // 检查包序号是否在范围内
            if (packet_number >= num_packets || packet_received[packet_number]) {
                std::cerr << "Invalid or duplicate packet number: " << packet_number << std::endl;
                continue;
            }

            int offset = packet_number * max_data_size;
            int data_size = n - header_size - footer_size;
            memcpy(image_.data + offset, recv_buffer + header_size, data_size);
            packet_received[packet_number] = true;
            received_bytes += data_size;

            // 检查是否所有包都已接收
            if (std::all_of(packet_received.begin(), packet_received.end(), [](bool v) { return v; })) {
                break;
            }
        }
    }

private:
    int sockfd;
    unsigned char recv_buffer[1024]; // 每个 UDP 包的最大大小
    std::function<void(cv::Mat&)> image_data_use;
    std::atomic<bool> running_{true};
    std::thread image_recv_thread;
};