import argparse
import pygame
import sys
import time


def create_parser():
    parser = argparse.ArgumentParser(description="游戏手柄测试工具")
    parser.add_argument("--index", type=int, default=0, help="手柄索引，默认 0")
    parser.add_argument(
        "--rumble",
        action="store_true",
        help="启动后执行一次振动测试并退出",
    )
    parser.add_argument(
        "--rumble-loop",
        action="store_true",
        help="循环执行振动测试，按 Ctrl+C 退出",
    )
    parser.add_argument(
        "--strong",
        type=float,
        default=1.0,
        help="强振动强度，范围 0.0 到 1.0，默认 1.0",
    )
    parser.add_argument(
        "--weak",
        type=float,
        default=0.5,
        help="弱振动强度，范围 0.0 到 1.0，默认 0.5",
    )
    parser.add_argument(
        "--duration",
        type=float,
        default=1.0,
        help="单次振动时长，单位秒，默认 1.0",
    )
    parser.add_argument(
        "--interval",
        type=float,
        default=1.0,
        help="循环振动间隔，单位秒，默认 1.0",
    )
    return parser


def clamp(value):
    return max(0.0, min(1.0, value))


def init_joystick(index):
    pygame.init()
    pygame.joystick.init()

    joystick_count = pygame.joystick.get_count()
    if joystick_count == 0:
        print("错误: 没有检测到任何游戏手柄。请检查连接。")
        pygame.quit()
        sys.exit(1)

    if index < 0 or index >= joystick_count:
        print(f"错误: 手柄索引 {index} 超出范围，当前只检测到 {joystick_count} 个手柄。")
        pygame.quit()
        sys.exit(1)

    print(f"检测到 {joystick_count} 个手柄。")

    joystick = pygame.joystick.Joystick(index)
    joystick.init()
    return joystick


def print_joystick_info(joystick):
    print(f"已连接手柄: {joystick.get_name()}")
    print(f"包含 {joystick.get_numaxes()} 个摇杆轴，{joystick.get_numbuttons()} 个按键。")


def run_rumble_test(joystick, strong, weak, duration):
    strong = clamp(strong)
    weak = clamp(weak)
    duration_ms = max(1, int(duration * 1000))

    if not hasattr(joystick, "rumble"):
        print("错误: 当前 pygame 版本不支持手柄振动接口。")
        return False

    pygame.event.pump()
    try:
        supported = joystick.rumble(strong, weak, duration_ms)
    except pygame.error as exc:
        print(f"错误: 振动测试失败，驱动或手柄可能不支持振动。{exc}")
        return False

    if not supported:
        print("错误: 当前手柄或驱动未启用振动反馈。")
        return False

    print(
        f"振动测试已触发: strong={strong:.2f}, weak={weak:.2f}, duration={duration:.2f}s"
    )
    time.sleep(duration)
    return True


def monitor_joystick(joystick):
    axes = joystick.get_numaxes()
    buttons = joystick.get_numbuttons()

    print("请随意拨动摇杆或按下按键进行测试。按 'Ctrl+C' 退出。")
    print("-" * 50)

    try:
        while True:
            pygame.event.pump()
            axis_vals = [f"轴{i}:{joystick.get_axis(i):>5.2f}" for i in range(axes)]
            btn_vals = [f"键{i}:{joystick.get_button(i)}" for i in range(buttons)]
            sys.stdout.write("\r" + " | ".join(axis_vals) + " || " + " ".join(btn_vals))
            sys.stdout.flush()
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\n退出测试。")


def main():
    args = create_parser().parse_args()
    joystick = init_joystick(args.index)
    print_joystick_info(joystick)

    try:
        if args.rumble_loop:
            print("进入循环振动测试模式。按 Ctrl+C 退出。")
            while True:
                if not run_rumble_test(joystick, args.strong, args.weak, args.duration):
                    sys.exit(1)
                time.sleep(max(0.0, args.interval))
        elif args.rumble:
            if not run_rumble_test(joystick, args.strong, args.weak, args.duration):
                sys.exit(1)
        else:
            monitor_joystick(joystick)
    except KeyboardInterrupt:
        print("\n退出测试。")
    finally:
        pygame.quit()


if __name__ == "__main__":
    main()
