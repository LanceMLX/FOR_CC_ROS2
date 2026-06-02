import pygame
import sys
import time

def test_joystick():
    pygame.init()
    pygame.joystick.init()
    
    joystick_count = pygame.joystick.get_count()
    if joystick_count == 0:
        print("错误: 没有检测到任何游戏手柄。请检查连接。")
        pygame.quit()
        sys.exit(1)
        
    print(f"检测到 {joystick_count} 个手柄。")
    
    # 默认连接第一个手柄
    joystick = pygame.joystick.Joystick(0)
    joystick.init()
    
    name = joystick.get_name()
    axes = joystick.get_numaxes()
    buttons = joystick.get_numbuttons()
    
    print(f"已连接手柄: {name}")
    print(f"包含 {axes} 个摇杆轴，{buttons} 个按键。")
    print("请随意拨动摇杆或按下按键进行测试。按 'Ctrl+C' 退出。")
    print("-" * 50)
    
    try:
        while True:
            pygame.event.pump()
            
            # 读取所有轴的值
            axis_vals = [f"轴{i}:{joystick.get_axis(i):>5.2f}" for i in range(axes)]
            # 读取所有按键的值
            btn_vals = [f"键{i}:{joystick.get_button(i)}" for i in range(buttons)]
            
            # 清除终端当前行并打印新数据
            sys.stdout.write('\r' + ' | '.join(axis_vals) + ' || ' + ' '.join(btn_vals))
            sys.stdout.flush()
            
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\n退出测试。")
        pygame.quit()

if __name__ == "__main__":
    test_joystick()
