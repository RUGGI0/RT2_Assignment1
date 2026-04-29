#include "rclcpp/rclcpp.hpp"
#include "rt2_nav_server/nav_server.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rt2_nav_server::NavServer>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}