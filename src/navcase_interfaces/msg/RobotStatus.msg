#   工作模式
string mode                 # "idle", "follow", "return", "manual",
#   连接状态
bool connected              #   是否连接
string connection_type      # "wifi", "ble", "none"
string connection_quality   # "excellent", "good", "fair", "poor"
int32 signal_strength       #   信号强度 (0-100)
#   电池状态
float32 battery_level      # 电量百分比 (0-100)
bool low_battery_warning   # 低电量警告
bool critical_battery      # 严重低电量
#   跟随状态
FollowTarget follow_target
bool following             # 是否正在跟随
float32 follow_distance    # 当前跟随距离 (m)
float32 follow_offset      # 跟随偏移 (m)
#   导航状态
bool navigating            # 是否正在导航
string current_goal        # 当前目标点
float32 distance_to_goal   # 到目标距离 (m)
float32 estimated_time     # 预计到达时间 (秒)
#   速度状态
float32 current_speed      # 当前速度 (m/s)
float32 max_speed          # 最大速度 (m/s)
float32 average_speed      # 平均速度 (m/s)
#   安全状态
bool emergency_stop        # 紧急停止
bool obstacle_detected     # 检测到障碍物
bool tilted                # 倾斜警告
bool overloaded            # 超载警告
#   错误信息
int32 error_code           # 错误代码
string error_message       # 错误信息
bool error_active          # 是否有活动错误
#   系统状态
float32 cpu_usage # 使用率 (%)
float32 memory_usage       # 内存使用率 (%)
float32 system_temp        # 系统温度 (°C)
#   运行时间
int32 uptime_seconds       # 运行时间 (秒)
int32 total_distance       # 总行驶距离 (m)
