此demo工程为mini相机接收 SDK，须确定设备软件版本新于20250317，如果软件版本比较旧，需要自行到官网下载最新更新包将设备固件更新。

包含了控制相机算法启停，接收相机状态和算法位姿结果。

支持接收IMU和左右目相机raw数据。

非ROS环境下编译使用build_non_ros.sh脚本

ROS环境下编译使用`ros_build.sh`脚本

baton_mini.launch文件配置设备IP和电脑本地IP，非ROS版本需要自行修改代码里面的IP设置部分后重新编译

