import cv2
import sys
import os
import argparse

def test_camera(device_id=0, no_gui=False, max_frames=30, save_path=None):
    print(f"正在尝试打开摄像头: /dev/video{device_id}")
    cap = cv2.VideoCapture(device_id)
    
    if not cap.isOpened():
        print(f"错误: 无法打开摄像头 /dev/video{device_id}。请检查权限或连接。")
        return False

    if not no_gui and not os.environ.get("DISPLAY"):
        print("检测到 DISPLAY 为空，自动切换为无界面模式。")
        no_gui = True

    if no_gui:
        print(f"摄像头打开成功！无界面模式下将尝试抓取 {max_frames} 帧。")
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

        success_count = 0
        last_frame = None
        for _ in range(max_frames):
            ret, frame = cap.read()
            if ret and frame is not None:
                success_count += 1
                last_frame = frame

        cap.release()

        if success_count == 0:
            print("错误: 摄像头已打开，但未读取到有效图像帧。")
            return False

        print(f"无界面测试成功：共读取到 {success_count}/{max_frames} 帧有效图像。")
        if save_path and last_frame is not None:
            ok = cv2.imwrite(save_path, last_frame)
            if ok:
                print(f"已保存测试图片: {save_path}")
            else:
                print(f"警告: 图片保存失败: {save_path}")
        return True
        
    print("摄像头打开成功！按 'q' 键退出测试。")
    
    # 设置分辨率
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    
    while True:
        ret, frame = cap.read()
        if not ret:
            print("无法获取画面流！")
            cap.release()
            cv2.destroyAllWindows()
            return False
            
        cv2.imshow('Camera Test', frame)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
            
    cap.release()
    cv2.destroyAllWindows()
    return True

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="USB 摄像头测试脚本")
    parser.add_argument("device_id", nargs="?", type=int, default=0, help="摄像头设备编号，如 0 对应 /dev/video0")
    parser.add_argument("--no-gui", action="store_true", help="无界面模式，适用于无 DISPLAY 环境")
    parser.add_argument("--frames", type=int, default=30, help="无界面模式下抓取帧数")
    parser.add_argument("--save", type=str, default="", help="无界面模式下保存最后一帧图片路径")
    args = parser.parse_args()

    result = test_camera(
        device_id=args.device_id,
        no_gui=args.no_gui,
        max_frames=max(1, args.frames),
        save_path=args.save if args.save else None,
    )
    sys.exit(0 if result else 1)
