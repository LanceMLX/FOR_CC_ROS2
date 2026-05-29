from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='joy',
            executable='joy_node',
            name='joy_node',
            output='screen'
        ),
        Node(
            package='cc_teleop',
            executable='joy_to_cmd_node',
            name='joy_to_cmd',
            output='screen'
        )
    ])