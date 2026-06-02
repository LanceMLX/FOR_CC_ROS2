#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "navcase_interfaces/msg/control_command.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "tf2/LinearMath/Quaternion.h"
#include "geometry_msgs/msg/transform_stamped.hpp"

// 强制1字节对齐，防止结构体填充影响协议解析
#pragma pack(push, 1)
struct ChassisFrame {
    uint8_t header;       // [0] 帧头 0xAA
    uint8_t id;           // [1] 身份 0x01(主机) / 0x03(从机)
    uint8_t length;       // [2] 数据区长度 0x0B (11字节)
    uint8_t status;       // [3] 系统状态 0-空闲,1-正常,2-故障,3-过流,4-过热
    uint8_t mode;         // [4] 运行模式 0-控制, 1-刹车
    uint8_t m1_dir;       // [5] 电机1方向 0-正转, 1-反转
    uint8_t m1_spd_l;     // [6] 电机1速度 低8位 (小端序)
    uint8_t m1_spd_h;     // [7] 电机1速度 高8位 (0-1024)
    uint8_t m2_dir;       // [8] 电机2方向 0-正转, 1-反转
    uint8_t m2_spd_l;     // [9] 电机2速度 低8位 (小端序)
    uint8_t m2_spd_h;     // [10] 电机2速度 高8位 (0-1024)
    uint8_t voltage;      // [11] 电池电压 (0-255)
    uint8_t servo_angle;  // [12] 舵机角度 (0-255)
    uint8_t sensor1;      // [13] 传感器1 (0-255)
    uint8_t checksum;     // [14] 校验和 (第3~13字节累加 % 256)
    uint8_t footer;       // [15] 帧尾 0xBB
};
#pragma pack(pop)

class ChassisNode : public rclcpp::Node {
public:
    enum class RobotMode {
        MANUAL,
        AUTO,
        FOLLOW
    };

    ChassisNode() : Node("chassis_node"), serial_fd_(-1), x_(0.0), y_(0.0), theta_(0.0), current_mode_(RobotMode::MANUAL) {
        this->declare_parameter<std::string>("port_name", "/dev/ttyUSB0");
        this->declare_parameter<int>("baud_rate", 115200);
        
        std::string port_name = this->get_parameter("port_name").as_string();
        int baud_rate = this->get_parameter("baud_rate").as_int();
        
        init_serial(port_name, baud_rate);
        
        subscription_ = this->create_subscription<navcase_interfaces::msg::ControlCommand>(
            "/navcase/chassis/control_cmd", 10, std::bind(&ChassisNode::topic_callback, this, std::placeholders::_1));
        
        odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("odom", 10);
        tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);
        last_time_ = this->get_clock()->now();
        
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

    void send_chassis_cmd(bool brake, double linear_vel, double angular_vel) const {
        if (serial_fd_ < 0) return;
        
        ChassisFrame frame;
        frame.header = 0xAA;
        frame.id = 0x01; // 主机身份
        frame.length = 0x0B;
        frame.status = 0x00; // 状态: 实测 0x00
        
        if (brake) {
            frame.mode = 0x01; // 模式: 刹车
            frame.m1_dir = 0x00;
            frame.m1_spd_l = 0;
            frame.m1_spd_h = 0;
            frame.m2_dir = 0x00;
            frame.m2_spd_l = 0;
            frame.m2_spd_h = 0;
            frame.servo_angle = 0; // 实测 0 为中位
        } else {
            frame.mode = 0x00;   // 模式: 控制
            
            // 线速度映射到电机速度 (1.0 m/s 对应最高速度 150)
            int speed = std::min(150, static_cast<int>(std::abs(linear_vel) * 150.0));
            uint8_t dir = (linear_vel >= 0) ? 0x00 : 0x01; // 0:正转 1:反转
            
            frame.m1_dir = dir;
            frame.m1_spd_l = speed & 0xFF;         // 低位在前 (小端序)
            frame.m1_spd_h = (speed >> 8) & 0xFF;  // 高位在后
            frame.m2_dir = dir;
            frame.m2_spd_l = speed & 0xFF;
            frame.m2_spd_h = (speed >> 8) & 0xFF;
            
            // 角速度映射到舵机角度 (需根据实际实测范围调整，假设中位为0，正负偏转)
            int angle = static_cast<int>(angular_vel * 127.0); // 范围可能不是0-255，而是以0为中位
            angle = std::max(-128, std::min(127, angle));
            frame.servo_angle = static_cast<uint8_t>(angle);
        }
        
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

    void topic_callback(const navcase_interfaces::msg::ControlCommand::SharedPtr msg) {
        if (serial_fd_ < 0) return;
        
        // 1. 状态机切换逻辑
        RobotMode req_mode = current_mode_;
        if (msg->command == "manual") req_mode = RobotMode::MANUAL;
        else if (msg->command == "auto") req_mode = RobotMode::AUTO;
        else if (msg->command == "follow") req_mode = RobotMode::FOLLOW;

        if (req_mode != current_mode_) {
            std::string mode_str = (req_mode == RobotMode::MANUAL) ? "MANUAL (手动)" : 
                                   (req_mode == RobotMode::AUTO) ? "AUTO (自动导航)" : "FOLLOW (视觉跟随)";
            RCLCPP_INFO(this->get_logger(), "底盘 FSM: 驱动模式已切换至 %s", mode_str.c_str());
            current_mode_ = req_mode;
            
            // 模式切换时，主动发送一次停止指令，防止暴冲
            send_chassis_cmd(false, 0.0, 0.0);
            return; 
        }

        // 2. 紧急刹车 (最高优先级)
        if (msg->emergency_brake) {
            send_chassis_cmd(true, 0.0, 0.0);
            return;
        }

        // 3. 权限校验：根据当前 FSM 状态，决定是否响应指令来源
        bool accept_cmd = false;
        if (current_mode_ == RobotMode::MANUAL && msg->command_source == "joy") {
            accept_cmd = true;
        } else if (current_mode_ == RobotMode::AUTO && msg->command_source == "auto") {
            accept_cmd = true;
        } else if (current_mode_ == RobotMode::FOLLOW && msg->command_source == "vision") {
            accept_cmd = true;
        }

        if (accept_cmd) {
            send_chassis_cmd(false, msg->linear_velocity, msg->angular_velocity);
        }
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
                        
                        // 里程计推算 (Odometry)
                        int m1_speed = (rx_buffer_[7] << 8) | rx_buffer_[6]; // 小端序解析
                        int m2_speed = (rx_buffer_[10] << 8) | rx_buffer_[9]; // 小端序解析
                        double v = ((m1_speed + m2_speed) / 2.0) / 150.0;
                        if (rx_buffer_[5] == 0x01) v = -v; // 反转则速度为负
                        
                        int servo_angle = rx_buffer_[12]; // 如果中位是0，可能有符号位，这里暂且转成 signed char
                        int8_t signed_angle = static_cast<int8_t>(servo_angle);
                        double w = 0.0;
                        if (std::abs(v) > 0.001) { // 只有在移动时，舵机偏转才会产生角速度
                            w = signed_angle / 127.0; 
                        }
                        
                        rclcpp::Time current_time = this->get_clock()->now();
                        double dt = (current_time - last_time_).seconds();
                        last_time_ = current_time;
                        
                        x_ += v * cos(theta_) * dt;
                        y_ += v * sin(theta_) * dt;
                        theta_ += w * dt;
                        
                        // 发布 TF
                        geometry_msgs::msg::TransformStamped odom_tf;
                        odom_tf.header.stamp = current_time;
                        odom_tf.header.frame_id = "odom";
                        odom_tf.child_frame_id = "base_link";
                        odom_tf.transform.translation.x = x_;
                        odom_tf.transform.translation.y = y_;
                        odom_tf.transform.translation.z = 0.0;
                        tf2::Quaternion q;
                        q.setRPY(0, 0, theta_);
                        odom_tf.transform.rotation.x = q.x();
                        odom_tf.transform.rotation.y = q.y();
                        odom_tf.transform.rotation.z = q.z();
                        odom_tf.transform.rotation.w = q.w();
                        tf_broadcaster_->sendTransform(odom_tf);
                        
                        // 发布 Odometry 消息
                        nav_msgs::msg::Odometry odom;
                        odom.header.stamp = current_time;
                        odom.header.frame_id = "odom";
                        odom.child_frame_id = "base_link";
                        odom.pose.pose.position.x = x_;
                        odom.pose.pose.position.y = y_;
                        odom.pose.pose.orientation = odom_tf.transform.rotation;
                        odom.twist.twist.linear.x = v;
                        odom.twist.twist.angular.z = w;
                        odom_pub_->publish(odom);
                        
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
    double x_, y_, theta_;
    RobotMode current_mode_;
    rclcpp::Time last_time_;
    rclcpp::Subscription<navcase_interfaces::msg::ControlCommand>::SharedPtr subscription_;
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    rclcpp::TimerBase::SharedPtr read_timer_;
    std::vector<uint8_t> rx_buffer_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ChassisNode>());
    rclcpp::shutdown();
    return 0;
}