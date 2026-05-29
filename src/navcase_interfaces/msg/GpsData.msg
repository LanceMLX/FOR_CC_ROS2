# GPS  位置
float64 latitude            # 纬度
float64 longitude           # 经度
float32 altitude            # 海拔高度 (m)
#   精度
float32 hdop                # 水平精度因子
float32 vdop                # 垂直精度因子
#   速度
float32 speed              # 地面速度 (m/s)
float32 heading            # 航向角 (度)
#   卫星信息
int32 satellites_used      # 使用的卫星数量
int32 satellites_visible   # 可见卫星数量
#   时间戳
int64 gps_time             # GPS  时间戳 (UTC)
