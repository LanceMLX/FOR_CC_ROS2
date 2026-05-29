cmake_minimum_required(VERSION 3.8)
project(cc_chassis)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(navcase_interfaces REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

add_executable(chassis_node src/chassis_node.cpp)
ament_target_dependencies(chassis_node rclcpp geometry_msgs navcase_interfaces nav_msgs tf2_ros tf2_geometry_msgs)

install(TARGETS
  chassis_node
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME})

ament_package()