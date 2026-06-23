#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/image_encodings.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <cv_bridge/cv_bridge.h>
#include "image_transport/image_transport.hpp"
#include <opencv2/opencv.hpp>

class AutoFollowNode : public rclcpp::Node {
public:
    AutoFollowNode() : Node("auto_follow") {
        subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
            "camera/image_raw", 10, std::bind(&AutoFollowNode::image_callback, this, std::placeholders::_1));
        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
        RCLCPP_INFO(this->get_logger(), "视觉跟随节点(C++)已启动，等待图像数据...");
    }

private:
    void image_callback(const sensor_msgs::msg::Image::SharedPtr msg) const {
        try {
            cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
            cv::Mat cv_image = cv_ptr->image;
            
            // TODO: 实现目标检测与 PID 控制逻辑
            // 这里仅做示例，发布空速度
            auto twist = geometry_msgs::msg::Twist();
            publisher_->publish(twist);
        } catch (cv_bridge::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge 异常: %s", e.what());
        }
    }
    
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AutoFollowNode>());
    rclcpp::shutdown();
    return 0;
}