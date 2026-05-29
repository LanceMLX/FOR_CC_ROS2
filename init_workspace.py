import os

structure = {
    "sdk": {
        "README.md": "存放底层硬件驱动及相关软件开发包（SDK）。\n例如电机驱动库、舵机控制库等。"
    },
    "third_party": {
        "README.md": "存放项目运行所依赖的第三方开源库或工具包。"
    },
    "src": {
        "cc_chassis": {
            "package.xml": """<?xml version="1.0"?>
<?xml-model href="http://download.ros.org/schema/package_format3.xsd" schematypens="http://www.w3.org/2001/XMLSchema"?>
<package format="3">
  <name>cc_chassis</name>
  <version>0.0.0</version>
  <description>底盘控制包：TTL电平电机与转向舵机驱动</description>
  <maintainer email="user@todo.todo">user</maintainer>
  <license>Apache-2.0</license>
  <buildtool_depend>ament_cmake</buildtool_depend>
  <depend>rclcpp</depend>
  <depend>geometry_msgs</depend>
  <export>
    <build_type>ament_cmake</build_type>
  </export>
</package>""",
            "CMakeLists.txt": """cmake_minimum_required(VERSION 3.8)
project(cc_chassis)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(chassis_node src/chassis_node.cpp)
ament_target_dependencies(chassis_node rclcpp geometry_msgs)

install(TARGETS
  chassis_node
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME})

ament_package()""",
            "src": {
                "chassis_node.cpp": """#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

class ChassisNode : public rclcpp::Node {
public:
    ChassisNode() : Node("chassis_node") {
        subscription_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "cmd_vel", 10, std::bind(&ChassisNode::topic_callback, this, std::placeholders::_1));
        RCLCPP_INFO(this->get_logger(), "底盘驱动节点已启动，等待 cmd_vel 指令...");
    }

private:
    void topic_callback(const geometry_msgs::msg::Twist::SharedPtr msg) const {
        RCLCPP_INFO(this->get_logger(), "收到速度指令: 线速度='%f', 角速度='%f'", msg->linear.x, msg->angular.z);
        // TODO: 通过串口或SDK将速度下发至TTL电机驱动板与转向舵机
    }
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscription_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ChassisNode>());
    rclcpp::shutdown();
    return 0;
}"""
            },
            "launch": {
                "bringup.launch.py": """from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='cc_chassis',
            executable='chassis_node',
            name='chassis_node',
            output='screen'
        ),
        # 这里可继续添加激光雷达等基础设备的启动节点
    ])"""
            }
        },
        "cc_teleop": {
            "package.xml": """<?xml version="1.0"?>
<?xml-model href="http://download.ros.org/schema/package_format3.xsd" schematypens="http://www.w3.org/2001/XMLSchema"?>
<package format="3">
  <name>cc_teleop</name>
  <version>0.0.0</version>
  <description>手动控制模式：手柄遥控功能包</description>
  <maintainer email="user@todo.todo">user</maintainer>
  <license>Apache-2.0</license>
  <buildtool_depend>ament_python</buildtool_depend>
  <depend>rclpy</depend>
  <depend>sensor_msgs</depend>
  <depend>geometry_msgs</depend>
  <export>
    <build_type>ament_python</build_type>
  </export>
</package>""",
            "setup.py": """from setuptools import setup
import os
from glob import glob

package_name = 'cc_teleop'

setup(
    name=package_name,
    version='0.0.0',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        (os.path.join('share', package_name, 'launch'), glob('launch/*.launch.py')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='user',
    maintainer_email='user@todo.todo',
    description='手动控制模式：手柄遥控功能包',
    license='Apache-2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'joy_to_cmd = cc_teleop.joy_to_cmd:main'
        ],
    },
)""",
            "resource": {
                "cc_teleop": ""
            },
            "cc_teleop": {
                "__init__.py": "",
                "joy_to_cmd.py": """import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy
from geometry_msgs.msg import Twist

class JoyToCmd(Node):
    def __init__(self):
        super().__init__('joy_to_cmd')
        self.subscription = self.create_subscription(Joy, 'joy', self.joy_callback, 10)
        self.publisher_ = self.create_publisher(Twist, 'cmd_vel', 10)
        self.get_logger().info('手柄遥控节点已启动')

    def joy_callback(self, msg):
        twist = Twist()
        # 假设摇杆轴1控制线速度，轴3控制角速度 (需根据实际手柄调整)
        if len(msg.axes) >= 4:
            twist.linear.x = msg.axes[1] * 0.5  # 最大 0.5 m/s
            twist.angular.z = msg.axes[3] * 1.0 # 最大 1.0 rad/s
            self.publisher_.publish(twist)

def main(args=None):
    rclpy.init(args=args)
    joy_to_cmd = JoyToCmd()
    rclpy.spin(joy_to_cmd)
    joy_to_cmd.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()"""
            },
            "launch": {
                "teleop_joy.launch.py": """from launch import LaunchDescription
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
            executable='joy_to_cmd',
            name='joy_to_cmd',
            output='screen'
        )
    ])"""
            }
        },
        "cc_nav": {
            "package.xml": """<?xml version="1.0"?>
<?xml-model href="http://download.ros.org/schema/package_format3.xsd" schematypens="http://www.w3.org/2001/XMLSchema"?>
<package format="3">
  <name>cc_nav</name>
  <version>0.0.0</version>
  <description>自动驾驶导航模式配置包</description>
  <maintainer email="user@todo.todo">user</maintainer>
  <license>Apache-2.0</license>
  <buildtool_depend>ament_cmake</buildtool_depend>
  <exec_depend>nav2_bringup</exec_depend>
  <export>
    <build_type>ament_cmake</build_type>
  </export>
</package>""",
            "CMakeLists.txt": """cmake_minimum_required(VERSION 3.8)
project(cc_nav)

find_package(ament_cmake REQUIRED)

install(DIRECTORY launch config maps
  DESTINATION share/${PROJECT_NAME}
)

ament_package()""",
            "launch": {
                "nav2_bringup.launch.py": """from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.substitutions import FindPackageShare
import os

def generate_launch_description():
    nav2_bringup_dir = FindPackageShare('nav2_bringup').find('nav2_bringup')
    
    return LaunchDescription([
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(os.path.join(nav2_bringup_dir, 'launch', 'bringup_launch.py')),
            launch_arguments={'map': 'maps/default_map.yaml'}.items(),
        )
    ])"""
            },
            "config": {
                "nav2_params.yaml": "# Nav2 配置参数留空，待根据实际车体参数配置"
            },
            "maps": {
                "default_map.yaml": "image: default_map.pgm\nresolution: 0.05\norigin: [0.0, 0.0, 0.0]\nnegate: 0\noccupied_thresh: 0.65\nfree_thresh: 0.196",
                "default_map.pgm": "P5\n2 2\n255\n\x00\x00\x00\x00"
            }
        },
        "cc_vision": {
            "package.xml": """<?xml version="1.0"?>
<?xml-model href="http://download.ros.org/schema/package_format3.xsd" schematypens="http://www.w3.org/2001/XMLSchema"?>
<package format="3">
  <name>cc_vision</name>
  <version>0.0.0</version>
  <description>自动跟随模式功能包</description>
  <maintainer email="user@todo.todo">user</maintainer>
  <license>Apache-2.0</license>
  <buildtool_depend>ament_python</buildtool_depend>
  <depend>rclpy</depend>
  <depend>sensor_msgs</depend>
  <depend>geometry_msgs</depend>
  <depend>cv_bridge</depend>
  <export>
    <build_type>ament_python</build_type>
  </export>
</package>""",
            "setup.py": """from setuptools import setup
import os
from glob import glob

package_name = 'cc_vision'

setup(
    name=package_name,
    version='0.0.0',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        (os.path.join('share', package_name, 'launch'), glob('launch/*.launch.py')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='user',
    maintainer_email='user@todo.todo',
    description='自动跟随模式功能包',
    license='Apache-2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'auto_follow = cc_vision.auto_follow:main'
        ],
    },
)""",
            "resource": {
                "cc_vision": ""
            },
            "cc_vision": {
                "__init__.py": "",
                "auto_follow.py": """import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from geometry_msgs.msg import Twist
from cv_bridge import CvBridge
import cv2

class AutoFollowNode(Node):
    def __init__(self):
        super().__init__('auto_follow')
        self.subscription = self.create_subscription(Image, 'camera/image_raw', self.image_callback, 10)
        self.publisher_ = self.create_publisher(Twist, 'cmd_vel', 10)
        self.bridge = CvBridge()
        self.get_logger().info('视觉跟随节点已启动，等待图像数据...')

    def image_callback(self, msg):
        try:
            cv_image = self.bridge.imgmsg_to_cv2(msg, "bgr8")
            # TODO: 实现目标检测与 PID 控制逻辑
            # 这里仅做示例，发布空速度
            twist = Twist()
            self.publisher_.publish(twist)
        except Exception as e:
            self.get_logger().error(f'图像处理异常: {e}')

def main(args=None):
    rclpy.init(args=args)
    node = AutoFollowNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()"""
            },
            "launch": {
                "auto_follow.launch.py": """from launch import LaunchDescription
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
            executable='auto_follow',
            name='auto_follow',
            output='screen'
        )
    ])"""
            }
        }
    }
}

def create_structure(base_path, d):
    for k, v in d.items():
        path = os.path.join(base_path, k)
        if isinstance(v, dict):
            os.makedirs(path, exist_ok=True)
            create_structure(path, v)
        else:
            with open(path, 'w', encoding='utf-8') as f:
                f.write(v)

create_structure('.', structure)
print("Project structure generated successfully.")
