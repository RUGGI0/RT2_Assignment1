cmake_minimum_required(VERSION 3.8)
project(rt2_nav_server)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(rt2_nav_interfaces REQUIRED)

include_directories(include)

add_executable(nav_server_node
  src/nav_server.cpp
  src/nav_server_main.cpp
)

ament_target_dependencies(nav_server_node
  rclcpp
  rclcpp_action
  rclcpp_components
  nav_msgs
  geometry_msgs
  tf2
  tf2_geometry_msgs
  rt2_nav_interfaces
)

install(TARGETS
  nav_server_node
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()