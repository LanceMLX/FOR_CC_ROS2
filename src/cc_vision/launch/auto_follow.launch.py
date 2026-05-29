from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='usb_cam',
            executable='usb_cam_node_exe',
            name='usb_cam',
            output='screen'
        ),
        Node(
            package='cc_vision',
            executable='auto_follow_node',
            name='auto_follow',
            output='screen'
        )
    ])