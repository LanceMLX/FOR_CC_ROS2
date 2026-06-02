from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    # 获取全局硬件配置文件路径
    config_dir = get_package_share_directory('cc_config')
    hardware_params = os.path.join(config_dir, 'config', 'hardware.yaml')

    return LaunchDescription([
        # 1. 启动底盘节点
        Node(
            package='cc_chassis',
            executable='chassis_node',
            name='chassis_node',
            output='screen',
            parameters=[hardware_params]
        ),
        
        # 2. 启动激光雷达节点
        Node(
            package='sllidar_ros2',
            executable='sllidar_node',
            name='sllidar_node',
            output='screen',
            parameters=[hardware_params]
        ),
        
        # 3. 启动手柄驱动节点
        Node(
            package='joy',
            executable='joy_node',
            name='joy_node',
            output='screen',
            parameters=[hardware_params]
        ),
        
        # 4. 启动手柄指令转换节点
        Node(
            package='cc_teleop',
            executable='joy_to_cmd_node',
            name='joy_to_cmd',
            output='screen',
            parameters=[hardware_params]
        )
    ])