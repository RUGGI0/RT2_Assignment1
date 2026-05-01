#ifndef RT2_NAV_SERVER__NAV_SERVER_HPP_
#define RT2_NAV_SERVER__NAV_SERVER_HPP_

#include <memory>
#include <mutex>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "nav_msgs/msg/odometry.hpp"
#include "geometry_msgs/msg/twist.hpp"

#include "rt2_nav_interfaces/action/navigate_to_pose.hpp"

namespace rt2_nav_server
{

class NavServer : public rclcpp::Node
{
public:
  using NavigateToPose = rt2_nav_interfaces::action::NavigateToPose;
  using GoalHandleNavigateToPose = rclcpp_action::ServerGoalHandle<NavigateToPose>;

  explicit NavServer(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

private:
  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID & uuid,
    std::shared_ptr<const NavigateToPose::Goal> goal);

  rclcpp_action::CancelResponse handle_cancel(
    const std::shared_ptr<GoalHandleNavigateToPose> goal_handle);

  void handle_accepted(const std::shared_ptr<GoalHandleNavigateToPose> goal_handle);

  void execute(const std::shared_ptr<GoalHandleNavigateToPose> goal_handle);

  void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg);

  void stop_robot();
  double saturate(double value, double min_value, double max_value) const;
  double normalize_angle(double angle) const;
  bool is_position_reached(double distance) const;
  bool is_angle_reached(double angle_error) const;

  rclcpp_action::Server<NavigateToPose>::SharedPtr action_server_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;

  std::mutex odom_mutex_;
  bool odom_received_;
  double current_x_;
  double current_y_;
  double current_theta_;

  double position_tolerance_;
  double angle_tolerance_;
    
  double linear_kp_;
  double angular_kp_;
    
  double max_linear_vel_;
  double max_angular_vel_;
    
  double control_frequency_;
};

}  // namespace rt2_nav_server

#endif  // RT2_NAV_SERVER__NAV_SERVER_HPP_