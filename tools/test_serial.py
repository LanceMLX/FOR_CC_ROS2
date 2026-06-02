import serial
import time
import struct

# 测试向单片机发送 16 字节协议帧
# 帧结构: 帧头(0xAA) | ID(0x01) | 长度(0x0B) | 状态 | 模式 | M1方向 | M1高8 | M1低8 | M2方向 | M2高8 | M2低8 | 电压 | 舵机 | 传感器 | 校验和 | 帧尾(0xBB)

def calc_checksum(data_bytes):
    return sum(data_bytes) % 256

try:
    # 打开串口
    ser = serial.Serial('/dev/ttyCH343USB0', 115200, timeout=1)
    print(f"成功打开串口: {ser.name}")
    
    # 构造数据区 (第3~13字节，共11字节)
    # 根据用户提供的正确帧: AA 01 08 00 00 00 32 00 00 00 00 00 00 00 32 BB
    status = 0x00      # 正常 (原为0x01，实测0x00有效)
    mode = 0x01        # 控制模式
    m1_dir = 0x01      # M1正转
    m1_spd_l = 0x32*3    # M1速度低8位 (0x32 = 50) -> 注意：这里发现单片机是小端序(低位在前)！
    m1_spd_h = 0x00    # M1速度高8位
    m2_dir = 0x00      # M2正转
    m2_spd_l = 0x00    # M2速度低8位
    m2_spd_h = 0x00    # M2速度高8位
    voltage = 0x00
    servo_angle = 0x00 # 舵机居中 (实测中位可能就是0x00，而不是128)
    sensor1 = 0x00

    data_bytes = [status, mode, m1_dir, m1_spd_l, m1_spd_h, m2_dir, m2_spd_l, m2_spd_h, voltage, servo_angle, sensor1]
    
    # 计算校验和: 数据区累加 % 256
    # 数据区是指从 index 3 (status) 到 index 13 (sensor1) 这 11 个字节
    checksum = sum(data_bytes) % 256
    
    # 组装完整帧 (恢复 Length 为 0x0B)
    frame = [0xAA, 0x01, 0x0B] + data_bytes + [checksum, 0xBB]
    frame_bytes = bytes(frame)
    
    print(f"正在发送数据帧 (16字节): {[hex(b) for b in frame_bytes]}")
    
    # 循环发送并尝试接收
    for i in range(5):
        ser.write(frame_bytes)
        print(f"第 {i+1} 次发送完成")
        time.sleep(0.1)
        
        # 尝试读取返回值
        if ser.in_waiting > 0:
            recv = ser.read(ser.in_waiting)
            print(f"收到单片机返回: {[hex(b) for b in recv]}")
        else:
            print("未收到返回数据")
            
    ser.close()
except Exception as e:
    print(f"串口测试失败: {e}")
