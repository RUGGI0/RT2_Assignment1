#include "rt2_nav_client/nav_ui.hpp"

#include <iostream>
#include <sstream>

#include "rclcpp_components/register_node_macro.hpp"

namespace rt2_nav_client
{

NavUi::NavUi(const rclcpp::NodeOptions & options)
: Node("rt2_nav_ui", options)
{
  action_client_ = rclcpp_action::create_client<NavigateToPose>(
    this,
    "navigate_to_pose");

  command_sub_ = this->create_subscription<std_msgs::msg::String>(
    "/nav_ui_command",
    10,
    std::bind(&NavUi::command_callback, this, std::placeholders::_1));

  RCLCPP_INFO(this->get_logger(), "Nav UI component ready.");
  RCLCPP_INFO(this->get_logger(), "Send commands on topic /nav_ui_command");
  RCLCPP_INFO(this->get_logger(), "Examples:");
  RCLCPP_INFO(
    this->get_logger(),
    "  ros2 topic pub --once /nav_ui_command std_msgs/msg/String \"{data: 'goal 2.0 2.0 1.57'}\"");
  RCLCPP_INFO(
    this->get_logger(),
    "  ros2 topic pub --once /nav_ui_command std_msgs/msg/String \"{data: 'cancel'}\"");
}

NavUi::~NavUi()
{
}

void NavUi::command_callback(const std_msgs::msg::String::SharedPtr msg)
{
  handle_command(msg->data);
}

void NavUi::handle_command(const std::string & line)
{
  std::stringstream ss(line);
  std::string command;
  ss >> command;

  if (command == "goal") {
    double x;
    double y;
    double theta;

    if (!(ss >> x >> y >> theta)) {
      RCLCPP_WARN(this->get_logger(), "Usage: goal x y theta");
      return;
    }

    send_goal(x, y, theta);
  } else if (command == "cancel") {
    cancel_goal();
  } else if (command == "quit") {
    RCLCPP_WARN(
      this->get_logger(),
      "Quit command ignored in component mode. Stop the launch/container instead.");
  } else if (command.empty()) {
    return;
  } else {
    RCLCPP_WARN(
      this->get_logger(),
      "Unknown command. Use: goal x y theta | cancel");
  }
}

void NavUi::send_goal(double x, double y, double theta)
{
  if (!action_client_->wait_for_action_server(std::chrono::seconds(3))) {
    RCLCPP_ERROR(this->get_logger(), "Action server /navigate_to_pose not available.");
    return;
  }

  NavigateToPose::Goal goal_msg;
  goal_msg.x = x;
  goal_msg.y = y;
  goal_msg.theta = theta;

  RCLCPP_INFO(
    this->get_logger(),
    "Sending goal: x=%.3f y=%.3f theta=%.3f",
    x, y, theta);

  rclcpp_action::Client<NavigateToPose>::SendGoalOptions send_goal_options;

  send_goal_options.goal_response_callback =
    std::bind(&NavUi::goal_response_callback, this, std::placeholders::_1);

  send_goal_options.feedback_callback =
    std::bind(&NavUi::feedback_callback, this, std::placeholders::_1, std::placeholders::_2);

  send_goal_options.result_callback =
    std::bind(&NavUi::result_callback, this, std::placeholders::_1);

  action_client_->async_send_goal(goal_msg, send_goal_options);
}

void NavUi::cancel_goal()
{
  std::lock_guard<std::mutex> lock(goal_mutex_);

  if (!current_goal_handle_) {
    RCLCPP_WARN(this->get_logger(), "No active goal to cancel.");
    return;
  }

  RCLCPP_WARN(this->get_logger(), "Sending cancel request...");
  action_client_->async_cancel_goal(current_goal_handle_);
}

void NavUi::goal_response_callback(const GoalHandleNavigateToPose::SharedPtr & goal_handle)
{
  if (!goal_handle) {
    RCLCPP_WARN(this->get_logger(), "Goal rejected by server.");
    return;
  }

  {
    std::lock_guard<std::mutex> lock(goal_mutex_);
    current_goal_handle_ = goal_handle;
  }

  RCLCPP_INFO(this->get_logger(), "Goal accepted by server.");
}

void NavUi::feedback_callback(
  GoalHandleNavigateToPose::SharedPtr,
  const std::shared_ptr<const NavigateToPose::Feedback> feedback)
{
  RCLCPP_INFO_THROTTLE(
    this->get_logger(),
    *this->get_clock(),
    1000,
    "[%s] x=%.2f y=%.2f theta=%.2f distance=%.2f heading_error=%.2f",
    feedback->phase.c_str(),
    feedback->current_x,
    feedback->current_y,
    feedback->current_theta,
    feedback->distance_error,
    feedback->heading_error);
}

void NavUi::result_callback(const GoalHandleNavigateToPose::WrappedResult & result)
{
  {
    std::lock_guard<std::mutex> lock(goal_mutex_);
    current_goal_handle_.reset();
  }

  switch (result.code) {
    case rclcpp_action::ResultCode::SUCCEEDED:
      RCLCPP_INFO(
        this->get_logger(),
        "Goal succeeded: %s",
        result.result->message.c_str());
      break;

    case rclcpp_action::ResultCode::ABORTED:
      RCLCPP_ERROR(
        this->get_logger(),
        "Goal aborted: %s",
        result.result->message.c_str());
      break;

    case rclcpp_action::ResultCode::CANCELED:
      RCLCPP_WARN(
        this->get_logger(),
        "Goal canceled: %s",
        result.result->message.c_str());
      break;

    default:
      RCLCPP_ERROR(this->get_logger(), "Unknown result code.");
      break;
  }
}

}  // namespace rt2_nav_client

RCLCPP_COMPONENTS_REGISTER_NODE(rt2_nav_client::NavUi)