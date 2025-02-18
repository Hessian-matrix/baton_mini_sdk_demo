#pragma once 
#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/Image.h>
#include <nav_msgs/Odometry.h>
#include <cv_bridge/cv_bridge.h>

#include "baton_mini.h"

void baton_ros_init(ros::NodeHandle nh, std::string &server_ip,std::string &local_ip);
void publish_odom(const odom_t& odom);
void publish_imu(const imu_data& imu);
void publish_image_left(const cv::Mat& image_);
void publish_image_right(const cv::Mat& image_);

