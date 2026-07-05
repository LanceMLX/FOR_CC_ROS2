#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "navcase_interfaces/msg/control_command.hpp"

class JoyToCmd : public rclcpp::Node {
public:
    JoyToCmd() : Node("joy_to_cmd") {
        this->declare_parameter<int>("axis_linear", 1);
        this->declare_parameter<int>("axis_angular", 3);
        this->declare_parameter<bool>("invert_linear_axis", true);
        this->declare_parameter<bool>("invert_angular_axis", false);
        this->declare_parameter<double>("max_linear_vel", 0.5);
        this->declare_parameter<double>("max_angular_vel", 1.0);
        this->declare_parameter<int>("btn_brake", 1);
        this->declare_parameter<int>("btn_mode_switch", 0); // 模式切换按键，默认 A 键

        subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
            "joy", 10, std::bind(&JoyToCmd::joy_callback, this, std::placeholders::_1));
        publisher_ = this->create_publisher<navcase_interfaces::msg::ControlCommand>("/navcase/chassis/control_cmd", 10);
        RCLCPP_INFO(this->get_logger(), "手柄遥控节点(C++)已启动，当前模式: %s", modes_[current_mode_idx_].c_str());
    }

private:
    void joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg) {
        auto cmd = navcase_interfaces::msg::ControlCommand();
        
        int axis_linear = this->get_parameter("axis_linear").as_int();
        int axis_angular = this->get_parameter("axis_angular").as_int();
        bool invert_linear_axis = this->get_parameter("invert_linear_axis").as_bool();
        bool invert_angular_axis = this->get_parameter("invert_angular_axis").as_bool();
        double max_linear_vel = this->get_parameter("max_linear_vel").as_double();
        double max_angular_vel = this->get_parameter("max_angular_vel").as_double();
        int btn_brake = this->get_parameter("btn_brake").as_int();
        int btn_mode_switch = this->get_parameter("btn_mode_switch").as_int();
        
        // 模式切换检测 (上升沿检测)
        if (msg->buttons.size() > (size_t)btn_mode_switch) {
            bool current_btn_state = (msg->buttons[btn_mode_switch] == 1);
            if (current_btn_state && !last_btn_mode_state_) {
                current_mode_idx_ = (current_mode_idx_ + 1) % modes_.size();
                RCLCPP_INFO(this->get_logger(), "驱动模式已切换为: %s", modes_[current_mode_idx_].c_str());
            }
            last_btn_mode_state_ = current_btn_state;
        }

        cmd.command = modes_[current_mode_idx_];
        cmd.command_source = "joy";
        
        // 摇杆控制线速度与角速度
        if (msg->axes.size() > (size_t)std::max(axis_linear, axis_angular)) {
            double linear_axis_value = msg->axes[axis_linear];
            double angular_axis_value = msg->axes[axis_angular];

            if (invert_linear_axis) {
                linear_axis_value = -linear_axis_value;
            }
            if (invert_angular_axis) {
                angular_axis_value = -angular_axis_value;
            }

            cmd.linear_velocity = linear_axis_value * max_linear_vel;
            cmd.angular_velocity = angular_axis_value * max_angular_vel;
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
    
    int current_mode_idx_ = 0;
    bool last_btn_mode_state_ = false;
    std::vector<std::string> modes_ = {"manual", "auto", "follow"};
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<JoyToCmd>());
    rclcpp::shutdown();
    return 0;
}
