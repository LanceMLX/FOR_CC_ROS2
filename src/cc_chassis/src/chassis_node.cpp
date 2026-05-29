#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

// 强制1字节对齐，防止结构体填充影响协议解析
#pragma pack(push, 1)
struct ChassisFrame {
    uint8_t header;       // [0] 帧头 0xAA
    uint8_t id;           // [1] 身份 0x01(主机) / 0x03(从机)
    uint8_t length;       // [2] 数据区长度 0x0B (11字节)
    uint8_t status;       // [3] 系统状态 0-空闲,1-正常,2-故障,3-过流,4-过热
    uint8_t mode;         // [4] 运行模式 0-控制, 1-刹车
    uint8_t m1_dir;       // [5] 电机1方向 0-正转, 1-反转
    uint8_t m1_spd_h;     // [6] 电机1速度 高8位 (0-1024)
    uint8_t m1_spd_l;     // [7] 电机1速度 低8位
    uint8_t m2_dir;       // [8] 电机2方向 0-正转, 1-反转
    uint8_t m2_spd_h;     // [9] 电机2速度 高8位 (0-1024)
    uint8_t m2_spd_l;     // [10] 电机2速度 低8位
    uint8_t voltage;      // [11] 电池电压 (0-255)
    uint8_t servo_angle;  // [12] 舵机角度 (0-255)
    uint8_t sensor1;      // [13] 传感器1 (0-255)
    uint8_t checksum;     // [14] 校验和 (第3~13字节累加 % 256)
    uint8_t footer;       // [15] 帧尾 0xBB
};
#pragma pack(pop)

class ChassisNode : public rclcpp::Node {
public:
    ChassisNode() : Node("chassis_node"), serial_fd_(-1) {
        this->declare_parameter<std::string>("port_name", "/dev/ttyUSB0");
        this->declare_parameter<int>("baud_rate", 115200);
        
        std::string port_name = this->get_parameter("port_name").as_string();
        int baud_rate = this->get_parameter("baud_rate").as_int();
        
        init_serial(port_name, baud_rate);
        
        subscription_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "cmd_vel", 10, std::bind(&ChassisNode::topic_callback, this, std::placeholders::_1));
        
        // 50Hz 定时器读取串口返回数据
        read_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(20),
            std::bind(&ChassisNode::read_serial, this));
            
        RCLCPP_INFO(this->get_logger(), "底盘串口驱动节点已启动，监听串口: %s", port_name.c_str());
    }
    
    ~ChassisNode() {
        if (serial_fd_ >= 0) close(serial_fd_);
    }

private:
    void init_serial(const std::string& port_name, int baud_rate) {
        serial_fd_ = open(port_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (serial_fd_ < 0) {
            RCLCPP_ERROR(this->get_logger(), "无法打开串口 %s，请检查设备连接或权限！", port_name.c_str());
            return;
        }
        
        struct termios options;
        tcgetattr(serial_fd_, &options);
        
        speed_t speed = B115200;
        if (baud_rate == 9600) speed = B9600;
        // 可按需添加其他波特率支持
        
        cfsetispeed(&options, speed);
        cfsetospeed(&options, speed);
        
        options.c_cflag |= (CLOCAL | CREAD);
        options.c_cflag &= ~PARENB;  // 无校验
        options.c_cflag &= ~CSTOPB;  // 1个停止位
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;      // 8个数据位
        
        // 原始数据模式
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        options.c_oflag &= ~OPOST;
        options.c_iflag &= ~(IXON | IXOFF | IXANY);
        
        tcsetattr(serial_fd_, TCSANOW, &options);
    }

    void topic_callback(const geometry_msgs::msg::Twist::SharedPtr msg) const {
        if (serial_fd_ < 0) return;
        
        ChassisFrame frame;
        frame.header = 0xAA;
        frame.id = 0x01; // 主机身份
        frame.length = 0x0B;
        frame.status = 0x01; // 状态: 正常
        frame.mode = 0x00;   // 模式: 控制
        
        // 线速度映射到电机速度 (假设 1.0 m/s 对应最大 PWM 1024)
        int speed = std::min(1024, static_cast<int>(std::abs(msg->linear.x) * 1024.0));
        uint8_t dir = (msg->linear.x >= 0) ? 0x00 : 0x01; // 0:正转 1:反转
        
        frame.m1_dir = dir;
        frame.m1_spd_h = (speed >> 8) & 0xFF;
        frame.m1_spd_l = speed & 0xFF;
        frame.m2_dir = dir;
        frame.m2_spd_h = (speed >> 8) & 0xFF;
        frame.m2_spd_l = speed & 0xFF;
        
        // 角速度映射到舵机角度 (假设 -1.0~1.0 rad/s 映射到 0~255，128为正中)
        int angle = 128 + static_cast<int>(msg->angular.z * 127.0);
        angle = std::max(0, std::min(255, angle));
        frame.servo_angle = angle;
        
        frame.voltage = 0x00; // 主机下发时不包含电压
        frame.sensor1 = 0x00; // 预留
        
        // 计算校验和
        uint8_t sum = 0;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(&frame);
        for (int i = 3; i <= 13; ++i) {
            sum += ptr[i];
        }
        frame.checksum = sum;
        frame.footer = 0xBB;
        
        // 串口发送
        write(serial_fd_, &frame, sizeof(frame));
    }
    
    void read_serial() {
        if (serial_fd_ < 0) return;
        
        uint8_t buffer[64];
        int n = read(serial_fd_, buffer, sizeof(buffer));
        if (n > 0) {
            for (int i = 0; i < n; ++i) {
                rx_buffer_.push_back(buffer[i]);
            }
            
            // 解析协议帧 (寻找连续16字节)
            while (rx_buffer_.size() >= 16) {
                if (rx_buffer_[0] == 0xAA && rx_buffer_[1] == 0x03 && rx_buffer_[2] == 0x0B) {
                    uint8_t sum = 0;
                    for (int i = 3; i <= 13; ++i) sum += rx_buffer_[i];
                    
                    if (sum == rx_buffer_[14] && rx_buffer_[15] == 0xBB) {
                        // 校验通过，提取从机(驱动板)数据
                        uint8_t voltage_raw = rx_buffer_[11];
                        uint8_t status = rx_buffer_[3];
                        
                        // 降频打印电池电压和驱动板状态
                        RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                            "收到驱动板数据 -> 状态: %d, 电池电压ADC: %d", status, voltage_raw);
                        
                        // 移除已处理的一帧
                        rx_buffer_.erase(rx_buffer_.begin(), rx_buffer_.begin() + 16);
                    } else {
                        // 校验失败，丢弃头部字节重新匹配
                        rx_buffer_.erase(rx_buffer_.begin());
                    }
                } else {
                    // 帧头不匹配，滑动窗口
                    rx_buffer_.erase(rx_buffer_.begin());
                }
            }
        }
    }
    
    int serial_fd_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscription_;
    rclcpp::TimerBase::SharedPtr read_timer_;
    std::vector<uint8_t> rx_buffer_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ChassisNode>());
    rclcpp::shutdown();
    return 0;
}