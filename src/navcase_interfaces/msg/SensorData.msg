#   时间戳
builtin_interfaces/Time timestamp
# GPS 数据
GpsData gps
#   电池数据
BatteryData battery
# IMU    数据
ImuData imu
#   障碍物数据
ObstacleData obstacles
#   轮速数据
float32 wheel_speed_left   #左轮速度 (m/s)
float32 wheel_speed_right # 右轮速度 (m/s)
# 电机温度
float32 motor_temp_left   # 左电机温度 (°C)
float32 motor_temp_right  # 右电机温度 (°C)
# 距离传感器
float32 distance_front    # 前方距离 (m)
float32 distance_rear     # 后方距离 (m)
float32 distance_left     # 左侧距离 (m)
float32 distance_right    # 右侧距离 (m)
# 重量传感器
float32 weight            # 箱子重量 (kg)
# 环境数据
float32 temperature       # 环境温度 (°C)
float32 humidity          # 环境湿度 (%)
