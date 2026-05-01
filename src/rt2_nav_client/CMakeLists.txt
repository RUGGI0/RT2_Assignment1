cmake_minimum_required(VERSION 3.8)
project(rt2_nav_client)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rt2_nav_interfaces REQUIRED)

include_directories(include)

set(dependencies
  rclcpp
  rclcpp_action
  rclcpp_components
  std_msgs
  rt2_nav_interfaces
)

###############################################
# Component library
###############################################
add_library(nav_ui_component SHARED
  src/nav_ui.cpp
)

ament_target_dependencies(nav_ui_component
  ${dependencies}
)

rclcpp_components_register_node(nav_ui_component
  PLUGIN "rt2_nav_client::NavUi"
  EXECUTABLE nav_ui_component_exe
)

###############################################
# Standalone executable
###############################################
add_executable(nav_ui_node
  src/nav_ui.cpp
  src/nav_ui_main.cpp
)

ament_target_dependencies(nav_ui_node
  ${dependencies}
)

###############################################
# Install
###############################################
install(TARGETS
  nav_ui_component
  nav_ui_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

ament_export_targets(export_nav_ui_component)

install(TARGETS nav_ui_component
  EXPORT export_nav_ui_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
