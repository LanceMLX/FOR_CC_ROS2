from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    config_dir = get_package_share_directory('cc_config')
    hardware_params = os.path.join(config_dir, 'config', 'hardware.yaml')

    return LaunchDescription([
        Node(
            package='usb_cam',
            executable='usb_cam_node_exe',
            name='usb_cam',
            output='screen',
            parameters=[hardware_params]
        ),
        Node(
            package='cc_vision',
            executable='auto_follow_node',
            name='auto_follow',
            output='screen',
            parameters=[hardware_params]
        )
    ])