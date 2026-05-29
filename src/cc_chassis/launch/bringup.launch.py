from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
import os

def generate_launch_description():
    # 获取 sllidar_ros2 包的路径
    sllidar_dir = FindPackageShare('sllidar_ros2').find('sllidar_ros2')
    
    return LaunchDescription([
        # 1. 启动底盘节点
        Node(
            package='cc_chassis',
            executable='chassis_node',
            name='chassis_node',
            output='screen'
        ),
        
        # 2. 启动激光雷达节点 (默认以 A1 型号为例)
        # 如果您的雷达是 A2、A3 或 S1 等型号，请修改下方 launch 文件名
        # 例如: sllidar_a2m7_launch.py, sllidar_a3_launch.py 等
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(sllidar_dir, 'launch', 'sllidar_a1_launch.py')
            )
        )
    ])