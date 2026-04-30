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
  current_x_(0.0),
  current_y_(0.0),
  current_theta_(0.0)
{
  this->declare_parameter("position_tolerance", 0.05);
  this->declare_parameter("angle_tolerance", 0.05);

  this->declare_parameter("linear_kp", 0.8);
  this->declare_parameter("angular_kp", 1.5);

  this->declare_parameter("max_linear_vel", 0.4);
  this->declare_parameter("max_angular_vel", 1.0);

  this->declare_parameter("control_frequency", 10.0);

  position_tolerance_ = this->get_parameter("position_tolerance").as_double();
  angle_tolerance_ = this->get_parameter("angle_tolerance").as_double();
  
  linear_kp_ = this->get_parameter("linear_kp").as_double();
  angular_kp_ = this->get_parameter("angular_kp").as_double();
  
  max_linear_vel_ = this->get_parameter("max_linear_vel").as_double();
  max_angular_vel_ = this->get_parameter("max_angular_vel").as_double();
  
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

  RCLCPP_INFO(
    this->get_logger(),
    "Starting 2D navigation to x=%.3f y=%.3f theta=%.3f",
    goal->x, goal->y, goal->theta);

  enum class Phase
  {
    ROTATE_TO_TARGET,
    MOVE_TO_TARGET,
    ROTATE_TO_FINAL
  };

  Phase phase = Phase::ROTATE_TO_TARGET;

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
    double y_now;
    double theta_now;

    {
      std::lock_guard<std::mutex> lock(odom_mutex_);
      x_now = current_x_;
      y_now = current_y_;
      theta_now = current_theta_;
    }

    if (std::abs(x_now) < 1e-6) {
      x_now = 0.0;
    }
    if (std::abs(y_now) < 1e-6) {
      y_now = 0.0;
    }
    if (std::abs(theta_now) < 1e-6) {
      theta_now = 0.0;
    }

    const double dx = goal->x - x_now;
    const double dy = goal->y - y_now;
    const double distance = std::sqrt(dx * dx + dy * dy);

    const double target_heading = std::atan2(dy, dx);
    const double heading_error = normalize_angle(target_heading - theta_now);
    const double final_angle_error = normalize_angle(goal->theta - theta_now);

    geometry_msgs::msg::Twist cmd;
    std::string phase_name;

    if (phase == Phase::ROTATE_TO_TARGET) {
      phase_name = "ROTATE_TO_TARGET";

      if (is_angle_reached(heading_error)) {
        phase = Phase::MOVE_TO_TARGET;
      } else {
        cmd.linear.x = 0.0;
        cmd.angular.z = saturate(
          angular_kp_ * heading_error,
          -max_angular_vel_,
          max_angular_vel_);
      }
    }

    if (phase == Phase::MOVE_TO_TARGET) {
      phase_name = "MOVE_TO_TARGET";

      if (is_position_reached(distance)) {
        phase = Phase::ROTATE_TO_FINAL;
      } else {
        cmd.linear.x = saturate(
          linear_kp_ * distance,
          0.0,
          max_linear_vel_);

        cmd.angular.z = saturate(
          angular_kp_ * heading_error,
          -max_angular_vel_,
          max_angular_vel_);
      }
    }

    if (phase == Phase::ROTATE_TO_FINAL) {
      phase_name = "ROTATE_TO_FINAL";

      if (is_angle_reached(final_angle_error)) {
        stop_robot();

        result->success = true;
        result->message = "Target pose reached";
        goal_handle->succeed(result);

        RCLCPP_INFO(
          this->get_logger(),
          "Goal succeeded. Final pose: x=%.3f y=%.3f theta=%.3f | distance=%.3f final_angle_error=%.3f",
          x_now,
          y_now,
          theta_now,
          distance,
          final_angle_error);

        return;
      } else {
        cmd.linear.x = 0.0;
        cmd.angular.z = saturate(
          angular_kp_ * final_angle_error,
          -max_angular_vel_,
          max_angular_vel_);
      }
    }

    feedback->current_x = x_now;
    feedback->current_y = y_now;
    feedback->current_theta = theta_now;
    feedback->distance_error = distance;

    if (phase == Phase::ROTATE_TO_FINAL) {
      feedback->heading_error = final_angle_error;
    } else {
      feedback->heading_error = heading_error;
    }

    feedback->phase = phase_name;
    goal_handle->publish_feedback(feedback);

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
  current_y_ = msg->pose.pose.position.y;

  const auto & q = msg->pose.pose.orientation;

  const double siny_cosp = 2.0 * (q.w * q.z + q.x * q.y);
  const double cosy_cosp = 1.0 - 2.0 * (q.y * q.y + q.z * q.z);

  current_theta_ = std::atan2(siny_cosp, cosy_cosp);

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

double NavServer::normalize_angle(double angle) const
{
  while (angle > M_PI) {
    angle -= 2.0 * M_PI;
  }

  while (angle < -M_PI) {
    angle += 2.0 * M_PI;
  }

  return angle;
}

bool NavServer::is_position_reached(double distance) const
{
  return distance < position_tolerance_;
}

bool NavServer::is_angle_reached(double angle_error) const
{
  return std::abs(angle_error) < angle_tolerance_;
}

}  // namespace rt2_nav_server