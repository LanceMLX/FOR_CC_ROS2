#   目标ID
string target_id            #   目标设备ID
#   目标位置
float64 target_latitude     # 目标纬度
float64 target_longitude    # 目标经度
#   目标状态
float32 target_distance     #到目标距离 (m)
float32 target_bearing     # 目标方位角 (度)
bool target_visible        # 目标是否可见
bool target_lost           # 目标是否丢失
#   跟随参数
float32 desired_distance   # 期望跟随距离 (m)
float32 max_speed          # 跟随最大速度 (m/s)
bool maintain_position     # 保持相对位置
