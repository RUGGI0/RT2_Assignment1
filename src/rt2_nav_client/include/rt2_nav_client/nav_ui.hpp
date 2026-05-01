#ifndef RT2_NAV_CLIENT__NAV_UI_HPP_
#define RT2_NAV_CLIENT__NAV_UI_HPP_

#include <memory>
#include <mutex>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "std_msgs/msg/string.hpp"

#include "rt2_nav_interfaces/action/navigate_to_pose.hpp"

namespace rt2_nav_client
{

class NavUi : public rclcpp::Node
{
public:
  using NavigateToPose = rt2_nav_interfaces::action::NavigateToPose;
  using GoalHandleNavigateToPose = rclcpp_action::ClientGoalHandle<NavigateToPose>;

  explicit NavUi(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
  ~NavUi() override;

private:

  void command_callback(const std_msgs::msg::String::SharedPtr msg);
  void handle_command(const std::string & line); 

  void send_goal(double x, double y, double theta);
  void cancel_goal();

  void goal_response_callback(const GoalHandleNavigateToPose::SharedPtr & goal_handle);
  void feedback_callback(
    GoalHandleNavigateToPose::SharedPtr,
    const std::shared_ptr<const NavigateToPose::Feedback> feedback);
  void result_callback(const GoalHandleNavigateToPose::WrappedResult & result);

  rclcpp_action::Client<NavigateToPose>::SharedPtr action_client_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr command_sub_;

  std::mutex goal_mutex_;
  GoalHandleNavigateToPose::SharedPtr current_goal_handle_;
};

}  // namespace rt2_nav_client

#endif  // RT2_NAV_CLIENT__NAV_UI_HPP_