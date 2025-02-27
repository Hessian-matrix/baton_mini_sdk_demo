#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>

const int BUFFER_SIZE = 640 * 480;
extern std::string server_ip;
class Image_tcp {
public:
    Image_tcp(const std::function<void(cv::Mat&)>& image_callback,bool is_left = true){
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        if(is_left)
            serverAddr.sin_port = htons(9997); // 固定端口左目9997
        else
            serverAddr.sin_port = htons(9998); // 固定端口右目9998
        if (inet_pton(AF_INET, server_ip.c_str(), &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid server IP address" << std::endl;
            close(sockfd);
            return;
        }

        if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Failed to connect to server" << std::endl;
            close(sockfd);
            return;
        }

        std::cout << "Connected to server at " << server_ip << std::endl;

        image_data_use = image_callback;
        image_recv_thread = std::thread(&Image_tcp::image_recv_loop, this);
    }

    ~Image_tcp() {
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
        uint32_t net_size;
        int total_received = 0;
        while (total_received < 4) {
            int received = recv(sockfd, (char*)&net_size + total_received, 4 - total_received, 0);
            if (received <= 0) {
                std::cerr << "Connection closed or error" << std::endl;
                close(sockfd);
                return ;
            }
            total_received += received;
        }

        uint32_t img_size = ntohl(net_size);
        // std::cout << "Expecting image of size: " << img_size << " bytes" << std::endl;

        // 接收图像数据
        total_received = 0;
        while (total_received < img_size) {
            int received = recv(sockfd, buffer + total_received, img_size - total_received, 0);
            if (received <= 0) {
                std::cerr << "Connection closed or error" << std::endl;
                close(sockfd);
                return ;
            }
            total_received += received;
        }
        memcpy(image_.data, buffer, img_size);
        // std::cout << "Received image of size " << img_size << " bytes" << std::endl;
    }

private:
    int sockfd;
    uint8_t buffer[BUFFER_SIZE];
    std::function<void(cv::Mat&)> image_data_use;
    std::atomic<bool> running_{true};
    std::thread image_recv_thread;
};