from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    config_dir = get_package_share_directory('cc_config')
    hardware_params = os.path.join(config_dir, 'config', 'hardware.yaml')

    return LaunchDescription([
        Node(
            package='joy',
            executable='joy_node',
            name='joy_node',
            output='screen',
            parameters=[hardware_params]
        ),
        Node(
            package='cc_teleop',
            executable='joy_to_cmd_node',
            name='joy_to_cmd',
            output='screen',
            parameters=[hardware_params]
        )
    ])