#include "baton_ros.h"
#include <mutex>

ros::Publisher pub_imu;
ros::Publisher pub_odometry;
ros::Publisher pub_image_left;
ros::Publisher pub_image_right;
ros::Time sim_stamp;
bool sim_stamp_left_flag = false,sim_stamp_right_flag = false;
std::mutex sim_stamp_mutex;

void baton_ros_init(ros::NodeHandle nh, std::string &server_ip,std::string &local_ip){
    nh.param<std::string>("server_ip", server_ip, "192.168.1.10");
    nh.param<std::string>("local_ip", local_ip, "192.168.1.15");
	pub_imu = nh.advertise<sensor_msgs::Imu>("/baton_mini/imu", 50);
	pub_odometry = nh.advertise<nav_msgs::Odometry>("/baton_mini/odometry", 10);
	pub_image_left = nh.advertise<sensor_msgs::Image>("/baton_mini/image_left", 5);
	pub_image_right = nh.advertise<sensor_msgs::Image>("/baton_mini/image_right", 5);
}

//publish odometry message
void publish_odom(const odom_t& odom){
    ros::Time stamp = ros::Time::now(); 
    nav_msgs::Odometry odom_msg;
    odom_msg.header.stamp = stamp;
    odom_msg.header.frame_id = "odom";
    odom_msg.pose.pose.position.x = odom.pose.px;
    odom_msg.pose.pose.position.y = odom.pose.py;
    odom_msg.pose.pose.position.z = odom.pose.pz;
    odom_msg.pose.pose.orientation.x = odom.pose.qx;
    odom_msg.pose.pose.orientation.y = odom.pose.qy;
    odom_msg.pose.pose.orientation.z = odom.pose.qz;
    odom_msg.pose.pose.orientation.w = odom.pose.qw;
    odom_msg.twist.twist.linear.x = odom.speed.lx;
    odom_msg.twist.twist.linear.y = odom.speed.ly;
    odom_msg.twist.twist.linear.z = odom.speed.lz;
    odom_msg.twist.twist.angular.x = odom.speed.ax;
    odom_msg.twist.twist.angular.y = odom.speed.ay;
    odom_msg.twist.twist.angular.z = odom.speed.az;
    pub_odometry.publish(odom_msg);
}

//publish imu data message
void publish_imu(const imu_data& imu){
    ros::Time stamp = ros::Time::now();
    if(imu.keyframe){
        sim_stamp_mutex.lock();
        sim_stamp_left_flag = true;
        sim_stamp_right_flag = true;
        sim_stamp = stamp;
        sim_stamp_mutex.unlock();
    }
    sensor_msgs::Imu imu_msg;
    imu_msg.header.stamp = stamp;
    imu_msg.header.frame_id = "imu";
    imu_msg.linear_acceleration.x = imu.acc_x;
    imu_msg.linear_acceleration.y = imu.acc_y;
    imu_msg.linear_acceleration.z = imu.acc_z;
    imu_msg.angular_velocity.x = imu.gyro_x;
    imu_msg.angular_velocity.y = imu.gyro_y;
    imu_msg.angular_velocity.z = imu.gyro_z;
    pub_imu.publish(imu_msg);
}

//publish left image data message 
void publish_image_left(const cv::Mat& image_){
    cv_bridge::CvImage out_msg;
    ros::Time stamp;
    sim_stamp_mutex.lock();
    if(!sim_stamp_left_flag){
        stamp = ros::Time::now();
        out_msg.header.stamp = stamp;
    }
    else{
        out_msg.header.stamp = sim_stamp;
        sim_stamp_left_flag = false;
    }
    sim_stamp_mutex.unlock();
    out_msg.header.frame_id = "cam_left";
    out_msg.encoding = sensor_msgs::image_encodings::MONO8;
    out_msg.image = image_;
    pub_image_left.publish(out_msg.toImageMsg());
}

//publish right image data message 
void publish_image_right(const cv::Mat& image_){
    cv_bridge::CvImage out_msg;
    ros::Time stamp;
    sim_stamp_mutex.lock();
    if(!sim_stamp_right_flag){
        stamp = ros::Time::now();
        out_msg.header.stamp = stamp;
    }
    else{
        out_msg.header.stamp = sim_stamp;
        sim_stamp_right_flag = false;
    }
    sim_stamp_mutex.unlock();
    out_msg.header.frame_id = "cam_right";
    out_msg.encoding = sensor_msgs::image_encodings::MONO8;
    out_msg.image = image_;
    pub_image_right.publish(out_msg.toImageMsg());
}

