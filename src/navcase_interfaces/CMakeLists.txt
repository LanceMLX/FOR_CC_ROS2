cmake_minimum_required(VERSION 3.8)
project(navcase_interfaces)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(builtin_interfaces REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/BatteryData.msg"
  "msg/ControlCommand.msg"
  "msg/FollowTarget.msg"
  "msg/GpsData.msg"
  "msg/ImuData.msg"
  "msg/ObstacleData.msg"
  "msg/RobotStatus.msg"
  "msg/SensorData.msg"
  "srv/SetConfig.srv"
  "action/ControlAction.action"
  DEPENDENCIES builtin_interfaces
)

ament_package()
