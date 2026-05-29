#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "navcase_interfaces/msg/control_command.hpp"

class JoyToCmd : public rclcpp::Node {
public:
    JoyToCmd() : Node("joy_to_cmd") {
        subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
            "joy", 10, std::bind(&JoyToCmd::joy_callback, this, std::placeholders::_1));
        publisher_ = this->create_publisher<navcase_interfaces::msg::ControlCommand>("/navcase/chassis/control_cmd", 10);
        RCLCPP_INFO(this->get_logger(), "手柄遥控节点(C++)已启动");
    }

private:
    void joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg) const {
        auto cmd = navcase_interfaces::msg::ControlCommand();
        cmd.command = "manual";
        cmd.command_source = "joy";
        
        // 假设摇杆轴1控制线速度，轴3控制角速度 (需根据实际手柄调整)
        if (msg->axes.size() >= 4) {
            cmd.linear_velocity = msg->axes[1] * 0.5;  // 最大 0.5 m/s
            cmd.angular_velocity = msg->axes[3] * 1.0; // 最大 1.0 rad/s
        }
        
        // 假设按键1 (B/圆圈) 作为紧急刹车
        if (msg->buttons.size() >= 2) {
            cmd.emergency_brake = (msg->buttons[1] == 1);
        } else {
            cmd.emergency_brake = false;
        }
        
        publisher_->publish(cmd);
    }
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;
    rclcpp::Publisher<navcase_interfaces::msg::ControlCommand>::SharedPtr publisher_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<JoyToCmd>());
    rclcpp::shutdown();
    return 0;
}