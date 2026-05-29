#   命令类型
string command              # "start", "stop", "follow", "return", "manual",
#   跟随命令参数
string follow_target_id     #跟随目标ID
float32 follow_distance    # 跟随距离 (m，0.5-5.0)
float32 follow_speed       # 跟随速度 (m/s，0.5-3.0)
bool follow_maintain_position # 是否保持相对位置
# 导航命令参数
float64 goal_latitude      # 目标纬度
float64 goal_longitude     # 目标经度
string goal_name           # 目标名称（如"登机口A1"）
bool use_optimal_path      # 是否使用最优路径
# 手动控制参数
float32 linear_velocity    # 线速度 (m/s，-3.0到3.0)
float32 angular_velocity   # 角速度 (rad/s，-1.57到1.57)
# 速度限制
float32 max_speed          # 最大速度限制 (m/s)
bool speed_limit_enabled   # 是否启用速度限制
# 安全参数
float32 obstacle_distance # 障碍物检测距离 (m)
bool collision_avoidance   # 是否启用避障
bool emergency_brake       # 是否启用紧急制动
# 充电命令
bool start_charging        # 开始充电
bool stop_charging         # 停止充电
# 系统命令
bool reboot                # 重启系统
bool shutdown              # 关闭系统
bool enter_standby         # 进入待机模式
bool wake_up               # 唤醒系统
#   配置命令
bool save_config            #   保存配置
bool load_default_config    #   加载默认配置
#   时间戳
builtin_interfaces/Time timestamp
string command_source       #   命令来源 "app", "voice", "auto", "web"
