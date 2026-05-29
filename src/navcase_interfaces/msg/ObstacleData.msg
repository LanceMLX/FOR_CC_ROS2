#   障碍物数量
int32 obstacle_count
#   障碍物列表（最多10个）
float32[10] obstacle_distances      # 距离 (m)
float32[10] obstacle_angles         # 角度 (度，相对于行李箱前方)
int32[10] obstacle_types            # 0=未知, 1=人, 2=物体, 3=墙壁
float32[10] obstacle_sizes          # 大小 (m)
#   最近障碍物
float32 closest_distance     #   最近障碍物距离 (m)
float32 closest_angle        #   最近障碍物角度 (度)
#   安全距离
float32 safety_distance      # 安全距离阈值 (m)
bool collision_warning       # 碰撞警告
