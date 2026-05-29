#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "navcase_interfaces/msg/control_command.hpp"

class JoyToCmd : public rclcpp::Node {
public:
    JoyToCmd() : Node("joy_to_cmd") {
        this->declare_parameter<int>("axis_linear", 1);
        this->declare_parameter<int>("axis_angular", 3);
        this->declare_parameter<double>("max_linear_vel", 0.5);
        this->declare_parameter<double>("max_angular_vel", 1.0);
        this->declare_parameter<int>("btn_brake", 1);

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
        
        int axis_linear = this->get_parameter("axis_linear").as_int();
        int axis_angular = this->get_parameter("axis_angular").as_int();
        double max_linear_vel = this->get_parameter("max_linear_vel").as_double();
        double max_angular_vel = this->get_parameter("max_angular_vel").as_double();
        int btn_brake = this->get_parameter("btn_brake").as_int();
        
        // 摇杆控制线速度与角速度
        if (msg->axes.size() > (size_t)std::max(axis_linear, axis_angular)) {
            cmd.linear_velocity = msg->axes[axis_linear] * max_linear_vel;
            cmd.angular_velocity = msg->axes[axis_angular] * max_angular_vel;
        }
        
        // 紧急刹车控制
        if (msg->buttons.size() > (size_t)btn_brake) {
            cmd.emergency_brake = (msg->buttons[btn_brake] == 1);
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