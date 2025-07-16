# Baotn_mini_sdk

## 使用说明

此demo工程为mini相机接收 SDK，须确定设备软件版本新于20250317，如果软件版本比较旧，需要自行到官网下载最新更新包将设备固件更新。

包含了控制相机算法启停，接收相机状态和算法位姿结果。

支持接收IMU和左右目相机raw数据。

非ROS环境下编译使用build_non_ros.sh脚本

ROS环境下编译使用`ros_build.sh`脚本

baton_mini.launch文件配置设备IP和电脑本地IP，非ROS版本需要自行修改代码里面的IP设置部分后重新编译

非ros环境编译需要补充baton_mini.cpp中的四个回调函数：
~~~cpp
//154~160行
void imu_data_recv(const imu_data& imu){}

void fast_odom_data_recv(const odom_pack& imu){}

void image_left_data(const cv::Mat& image_){}

void image_right_data(const cv::Mat& image_){}
~~~

### 200hz odom使用（内测）
启动后提示输入[0~5]
1) 输入1 回车 启动算法
2) 输入5 回车 启动200hz sdk接收

注意：如无高频里程计必要，不推荐使用200hz，因为它通过imu插值之后抖动更大，在高频振动的场景更明显。

## 更新记录

[25-7-14] 更新200hz odom读取，弃用odom_data_print、stream_callback相关接口