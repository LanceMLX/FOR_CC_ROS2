import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    slam_toolbox_dir = get_package_share_directory('slam_toolbox')
    
    # SLAM Toolbox 在线异步建图节点
    slam_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(slam_toolbox_dir, 'launch', 'online_async_launch.py')),
        launch_arguments={'use_sim_time': 'false'}.items()
    )

    # 静态 TF 树：base_link -> laser
    # (假设激光雷达安装在小车中心正上方 0.15 米处)
    tf_base_to_laser = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_base_to_laser',
        arguments=['0', '0', '0.15', '0', '0', '0', 'base_link', 'laser']
    )

    # (不再需要静态 odom_to_base，底盘节点已动态发布真实的轮式里程计)

    return LaunchDescription([
        tf_base_to_laser,
        slam_launch
    ])
