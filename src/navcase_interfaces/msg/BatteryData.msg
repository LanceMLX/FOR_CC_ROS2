#   电池状态
float32 voltage             # 电压 (V)
float32 current             # 电流 (A)
float32 percentage          # 电量百分比 (0-100)
float32 temperature         # 电池温度 (°C)
#   电池健康
float32 health_percentage   #   健康度百分比 (0-100)
int32 cycle_count           #   充电循环次数
#   预计续航
float32 estimated_range      # 预计续航里程 (km)
float32 time_to_empty        # 预计剩余时间 (分钟)
#   充电状态
bool charging                #   是否正在充电
bool charging_complete       #   是否充电完成
