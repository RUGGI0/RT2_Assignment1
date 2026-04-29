#include "rt2_nav_server/nav_server.hpp"

#include <chrono>
#include <cmath>
#include <functional>
#include <utility>

using namespace std::chrono_literals;

namespace rt2_nav_server
{

NavServer::NavServer()
: Node("rt2_nav_server"),
  odom_received_(false),
  current_x_(0.0)
{
  this->declare_parameter("position_tolerance", 0.05);
  this->declare_parameter("linear_kp", 0.8);
  this->declare_parameter("max_linear_vel", 0.4);
  this->declare_parameter("control_frequency", 10.0);

  position_tolerance_ = this->get_parameter("position_tolerance").as_double();
  linear_kp_ = this->get_parameter("linear_kp").as_double();
  max_linear_vel_ = this->get_parameter("max_linear_vel").as_double();
  control_frequency_ = this->get_parameter("control_frequency").as_double();

  odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
    "/odom",
    10,
    std::bind(&NavServer::odom_callback, this, std::placeholders::_1));

  cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

  action_server_ = rclcpp_action::create_server<NavigateToPose>(
    this,
    "navigate_to_pose",
    std::bind(&NavServer::handle_goal, this, std::placeholders::_1, std::placeholders::_2),
    std::bind(&NavServer::handle_cancel, this, std::placeholders::_1),
    std::bind(&NavServer::handle_accepted, this, std::placeholders::_1));

  RCLCPP_INFO(this->get_logger(), "NavServer ready.");
}

rclcpp_action::GoalResponse NavServer::handle_goal(
  const rclcpp_action::GoalUUID & uuid,
  std::shared_ptr<const NavigateToPose::Goal> goal)
{
  (void)uuid;

  RCLCPP_INFO(
    this->get_logger(),
    "Received goal request: x=%.3f y=%.3f theta=%.3f",
    goal->x, goal->y, goal->theta);

  if (!odom_received_) {
    RCLCPP_WARN(this->get_logger(), "Rejecting goal: odometry not received yet.");
    return rclcpp_action::GoalResponse::REJECT;
  }

  return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse NavServer::handle_cancel(
  const std::shared_ptr<GoalHandleNavigateToPose> goal_handle)
{
  (void)goal_handle;
  RCLCPP_WARN(this->get_logger(), "Received cancel request.");
  return rclcpp_action::CancelResponse::ACCEPT;
}

void NavServer::handle_accepted(const std::shared_ptr<GoalHandleNavigateToPose> goal_handle)
{
  std::thread{std::bind(&NavServer::execute, this, std::placeholders::_1), goal_handle}.detach();
}

void NavServer::execute(const std::shared_ptr<GoalHandleNavigateToPose> goal_handle)
{
  const auto goal = goal_handle->get_goal();
  auto feedback = std::make_shared<NavigateToPose::Feedback>();
  auto result = std::make_shared<NavigateToPose::Result>();

  rclcpp::Rate rate(control_frequency_);

  RCLCPP_INFO(this->get_logger(), "Starting 1D navigation along x.");

  while (rclcpp::ok()) {
    if (goal_handle->is_canceling()) {
      stop_robot();
      result->success = false;
      result->message = "Goal canceled";
      goal_handle->canceled(result);
      RCLCPP_WARN(this->get_logger(), "Goal canceled.");
      return;
    }

    double x_now;
    {
      std::lock_guard<std::mutex> lock(odom_mutex_);
      x_now = current_x_;
    }

    if (std::abs(x_now) < 1e-6) {
      x_now = 0.0;
    }

    const double error_x = goal->x - x_now;

    feedback->current_x = x_now;
    feedback->current_y = 0.0;
    feedback->current_theta = 0.0;
    feedback->distance_error = std::abs(error_x);
    feedback->heading_error = 0.0;
    feedback->phase = "MOVE_ALONG_X";
    goal_handle->publish_feedback(feedback);

    if (std::abs(error_x) < position_tolerance_) {
      stop_robot();
      result->success = true;
      result->message = "Target x reached";
      goal_handle->succeed(result);
      RCLCPP_INFO(
        this->get_logger(),
        "Goal succeeded. Final x=%.3f (error=%.3f, tolerance=%.3f)",
        x_now,
        error_x,
        position_tolerance_);
      return;
    }

    geometry_msgs::msg::Twist cmd;
    cmd.linear.x = saturate(linear_kp_ * error_x, -max_linear_vel_, max_linear_vel_);
    cmd.angular.z = 0.0;
    cmd_vel_pub_->publish(cmd);

    rate.sleep();
  }

  stop_robot();
  result->success = false;
  result->message = "Node stopped before goal completion";
  goal_handle->abort(result);
  RCLCPP_ERROR(this->get_logger(), "Goal aborted.");
}

void NavServer::odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
{
  std::lock_guard<std::mutex> lock(odom_mutex_);
  current_x_ = msg->pose.pose.position.x;
  odom_received_ = true;
}

void NavServer::stop_robot()
{
  geometry_msgs::msg::Twist cmd;
  cmd.linear.x = 0.0;
  cmd.angular.z = 0.0;
  cmd_vel_pub_->publish(cmd);
}

double NavServer::saturate(double value, double min_value, double max_value) const
{
  if (value > max_value) {
    return max_value;
  }
  if (value < min_value) {
    return min_value;
  }
  return value;
}

}  // namespace rt2_nav_server