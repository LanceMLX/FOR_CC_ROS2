import cv2
import sys

def test_camera(device_id=0):
    print(f"正在尝试打开摄像头: /dev/video{device_id}")
    cap = cv2.VideoCapture(device_id)
    
    if not cap.isOpened():
        print(f"错误: 无法打开摄像头 /dev/video{device_id}。请检查权限或连接。")
        sys.exit(1)
        
    print("摄像头打开成功！按 'q' 键退出测试。")
    
    # 设置分辨率
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    
    while True:
        ret, frame = cap.read()
        if not ret:
            print("无法获取画面流！")
            break
            
        cv2.imshow('Camera Test', frame)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
            
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    dev_id = 0
    if len(sys.argv) > 1:
        dev_id = int(sys.argv[1])
    test_camera(dev_id)
