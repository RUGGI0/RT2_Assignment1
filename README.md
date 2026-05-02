# RT2 Assignment 1: Component-Based 2D Navigation with ROS2 Actions

This project implements a ROS2 navigation architecture for a differential-drive robot in Gazebo.

---

# Table of Contents

1. [System Architecture](#1-system-architecture)
2. [Package Structure](#2-package-structure)
   - [rt2_nav_interfaces](#21-rt2_nav_interfaces)
   - [rt2_nav_server](#22-rt2_nav_server)
   - [rt2_nav_client](#23-rt2_nav_client)
   - [rt2_nav_bringup](#24-rt2_nav_bringup)
   - [bme_gazebo_sensors](#25-bme_gazebo_sensors)
3. [Custom Action Interface](#3-custom-action-interface)
4. [ROS Interfaces](#4-ros-interfaces)
   - [Action](#41-action)
   - [Topics](#42-topics)
5. [Navigation Behaviour](#5-navigation-behaviour)
6. [Parameters](#6-parameters)
7. [Components and Container](#7-components-and-container)
8. [Build Instructions](#8-build-instructions)
9. [Running the Project Manually](#9-running-the-project-manually)
10. [Running the Project with Scripts](#10-running-the-project-with-scripts)
11. [Command Panel](#11-command-panel)
12. [Useful Debug Commands](#12-useful-debug-commands)
13. [Standalone Executables](#13-standalone-executables)
14. [Assignment Requirements Checklist](#14-assignment-requirements-checklist)
15. [Notes](#15-notes)
16. [Repository Content](#16-repository-content)

---

The system allows the user to:

- set a target pose `(x, y, theta)`;
- cancel the active target;
- monitor the robot state through action feedback;
- run the action server and the UI/action client as ROS2 components loaded in the same container.

The project was developed in **C++** and follows the main concepts covered in the first part of the Research Track 2 course:

- ROS2 Actions;
- custom action interfaces;
- action feedback, result, and cancellation;
- ROS2 Components;
- component containers;
- launch files;
- odometry-based robot control.

---

# 1. System Architecture

The final architecture is based on two main components:

1. **NavServer** — action server implementing the robot navigation logic.
2. **NavUi** — action client/UI component that receives user commands and sends/cancels navigation goals.

Both components are loaded into the same ROS2 component container.

```text
                           +--------------------------------------+
                           |          rt2_nav_container           |
                           |      (ROS2 Component Container)      |
                           |                                      |
                           |   +------------------------------+   |
                           |   |        rt2_nav_ui            |   |
                           |   |   (Action Client / UI)       |   |
                           |   +------------------------------+   |
                           |          ^              |            |
                           |          |              |            |
                           |  /nav_ui_command        | NavigateToPose goal/cancel
                           |          |              v            |
                           |   +------------------------------+   |
                           |   |      rt2_nav_server          |   |
                           |   |      (Action Server)         |   |
                           |   +------------------------------+   |
                           +----------|--------------^------------+
                                      |              |
                                      | /cmd_vel     | /odom
                                      v              |
                           +--------------------------------------+
                           |              Gazebo Robot            |
                           |        bme_gazebo_sensors world      |
                           +--------------------------------------+
```

---

# 2. Package Structure

The project contains the following ROS2 packages:

```text
src/
├── bme_gazebo_sensors
├── rt2_nav_bringup
├── rt2_nav_client
├── rt2_nav_interfaces
└── rt2_nav_server
```

## 2.1 `rt2_nav_interfaces`

Contains the custom action interface:

```text
rt2_nav_interfaces/action/NavigateToPose.action
```

The action defines:

- goal pose;
- result status;
- feedback about current state and navigation phase.

## 2.2 `rt2_nav_server`

Contains the C++ action server responsible for robot navigation.

It subscribes to `/odom`, publishes `/cmd_vel`, and executes a simple 2D navigation state machine.

The server is available both as:

- standalone executable;
- ROS2 component/plugin.

## 2.3 `rt2_nav_client`

Contains the C++ UI/action client.

The UI receives user commands through the `/nav_ui_command` topic and sends or cancels action goals accordingly.

The client is available both as:

- standalone executable;
- ROS2 component/plugin.

## 2.4 `rt2_nav_bringup`

Contains launch and configuration files.

Main files:

```text
rt2_nav_bringup/launch/nav_components.launch.py
rt2_nav_bringup/config/nav_params.yaml
```

## 2.5 `bme_gazebo_sensors`

Gazebo simulation package used to spawn the robot and provide the simulation environment.

The world is configured as a free environment, without obstacles, consistently with the assignment requirement of navigation without obstacle avoidance.

---

# 3. Custom Action Interface

The action used by the system is:

```text
NavigateToPose.action
```

```text
float64 x
float64 y
float64 theta
---
bool success
string message
---
float64 current_x
float64 current_y
float64 current_theta
float64 distance_error
float64 heading_error
string phase
```

## Goal

```text
x
y
theta
```

The desired target pose of the robot.

## Result

```text
success
message
```

Indicates whether the goal was reached, canceled, or aborted.

## Feedback

```text
current_x
current_y
current_theta
distance_error
heading_error
phase
```

Provides runtime information about the navigation process.

---

# 4. ROS Interfaces

## 4.1 Action

```text
/navigate_to_pose
```

Type:

```text
rt2_nav_interfaces/action/NavigateToPose
```

Used by the UI/action client to send target poses and cancel active goals.

## 4.2 Topics

### Subscribed by `rt2_nav_server`

```text
/odom
```

Type:

```text
nav_msgs/msg/Odometry
```

Used to estimate the current robot position and orientation.

### Published by `rt2_nav_server`

```text
/cmd_vel
```

Type:

```text
geometry_msgs/msg/Twist
```

Used to command the robot linear and angular velocity.

### Subscribed by `rt2_nav_client`

```text
/nav_ui_command
```

Type:

```text
std_msgs/msg/String
```

Used as a lightweight user-command interface.

Accepted commands:

```text
goal <x> <y> <theta>
cancel
```

---

# 5. Navigation Behaviour

The navigation server implements a simple state machine.

```text
ROTATE_TO_TARGET → MOVE_TO_TARGET → ROTATE_TO_FINAL → SUCCESS
```

## 5.1 `ROTATE_TO_TARGET`

The robot first rotates toward the target position.

The target heading is computed as:

```text
atan2(goal_y - current_y, goal_x - current_x)
```

The robot rotates until the heading error is below the angular tolerance.

## 5.2 `MOVE_TO_TARGET`

Once aligned, the robot moves toward the target position.

During this phase:

- linear velocity is proportional to the distance from the goal;
- angular velocity corrects the heading error;
- both velocities are saturated using configurable maximum values.

## 5.3 `ROTATE_TO_FINAL`

When the robot reaches the target position, it stops translating and rotates toward the desired final orientation `theta`.

## 5.4 Success

The goal succeeds when both conditions are satisfied:

```text
distance_error < position_tolerance
abs(final_angle_error) < angle_tolerance
```

---

# 6. Parameters

Navigation parameters are defined in:

```text
src/rt2_nav_bringup/config/nav_params.yaml
```

Current parameters:

```yaml
rt2_nav_server:
  ros__parameters:
    position_tolerance: 0.05
    angle_tolerance: 0.05

    linear_kp: 0.8
    angular_kp: 1.5

    max_linear_vel: 0.4
    max_angular_vel: 1.0

    control_frequency: 10.0
```

These parameters allow the navigation behaviour to be tuned without changing the source code.

---

# 7. Components and Container

Both the navigation server and the UI/action client are implemented as ROS2 components.

The server component is registered as:

```text
rt2_nav_server::NavServer
```

The UI/action client component is registered as:

```text
rt2_nav_client::NavUi
```

They are loaded into the same container by:

```text
rt2_nav_bringup/launch/nav_components.launch.py
```

The launch file starts:

```text
rt2_nav_container
```

and loads:

```text
rt2_nav_server::NavServer
rt2_nav_client::NavUi
```

This satisfies the assignment requirement that the action server and the UI/action client run as libraries/plugins within the same container.

---

# 8. Build Instructions

From the ROS2 workspace root:

```bash
cd /home/ubuntu/ros2_workshop
source /opt/ros/jazzy/setup.bash
colcon build
source install/setup.bash
```

To build only the assignment packages:

```bash
cd /home/ubuntu/ros2_workshop
source /opt/ros/jazzy/setup.bash
colcon build --packages-select \
  rt2_nav_interfaces \
  rt2_nav_server \
  rt2_nav_client \
  rt2_nav_bringup \
  bme_gazebo_sensors
source install/setup.bash
```

---

# 9. Running the Project Manually

## 9.1 Start Gazebo

Open Terminal 1:

```bash
cd /home/ubuntu/ros2_workshop
source /opt/ros/jazzy/setup.bash
source install/setup.bash
ros2 launch bme_gazebo_sensors spawn_robot.launch.py
```

## 9.2 Start the Component Container

Open Terminal 2:

```bash
cd /home/ubuntu/ros2_workshop
source /opt/ros/jazzy/setup.bash
source install/setup.bash
ros2 launch rt2_nav_bringup nav_components.launch.py
```

This launch file starts the component container and loads both the navigation server and the UI/action client.

## 9.3 Send a Goal

Open Terminal 3:

```bash
cd /home/ubuntu/ros2_workshop
source /opt/ros/jazzy/setup.bash
source install/setup.bash
ros2 topic pub --once /nav_ui_command std_msgs/msg/String "{data: 'goal 2.0 2.0 1.57'}"
```

## 9.4 Cancel the Active Goal

```bash
ros2 topic pub --once /nav_ui_command std_msgs/msg/String "{data: 'cancel'}"
```

---

# 10. Running the Project with Scripts

Two helper scripts are provided:

```text
launcher.sh
run_assignment.sh
```

The launcher starts a tmux-based 2×3 layout.

```text
┌───────────────┬───────────────┐
│ 0 ROBOT       │ 1 SERVER+UI   │
├───────────────┼───────────────┤
│ 2 ODOM        │ 3 MONITOR     │
├───────────────┼───────────────┤
│ 4 COMMAND     │ 5 DEBUG       │
└───────────────┴───────────────┘
```

## 10.1 Start the Launcher

From the workspace root:

```bash
cd /home/ubuntu/ros2_workshop
chmod +x launcher.sh
chmod +x run_assignment.sh
chmod +x odom_clean.sh
chmod +x command_panel.sh
./launcher.sh
```

Then press:

```text
S
```

to start the assignment environment.

## 10.2 Pane Description

| Pane | Name | Function |
|---|---|---|
| 0 | ROBOT | Starts Gazebo and spawns the robot |
| 1 | SERVER+UI | Starts the component container with server and UI |
| 2 | ODOM | Displays a cleaned odometry dashboard |
| 3 | MONITOR | Shows actions, nodes, and relevant topics |
| 4 | COMMAND | User command panel |
| 5 | DEBUG | Free terminal for debugging |

---

# 11. Command Panel

The `COMMAND` pane is managed by:

```text
command_panel.sh
```

It allows short commands instead of long `ros2 topic pub` commands.

## 11.1 Send a Goal

```bash
goal <x> <y> <theta>
```

Example:

```bash
goal 2.0 2.0 1.57
```

## 11.2 Cancel a Goal

```bash
cancel
```

## 11.3 Exit the Command Panel

```bash
exit
```

or:

```bash
quit
```

---

# 12. Useful Debug Commands

Check available actions:

```bash
ros2 action list
```

Check action type:

```bash
ros2 action type /navigate_to_pose
```

Check action interface:

```bash
ros2 interface show rt2_nav_interfaces/action/NavigateToPose
```

Check active components:

```bash
ros2 component list
```

Check active nodes:

```bash
ros2 node list
```

Echo odometry:

```bash
ros2 topic echo /odom
```

Echo velocity commands:

```bash
ros2 topic echo /cmd_vel
```

---

# 13. Standalone Executables

Although the final assignment setup uses components, standalone executables are also provided for testing.

## 13.1 Navigation Server

```bash
ros2 run rt2_nav_server nav_server_node
```

## 13.2 Navigation UI

```bash
ros2 run rt2_nav_client nav_ui_node
```

The final recommended execution mode remains the component-based launch:

```bash
ros2 launch rt2_nav_bringup nav_components.launch.py
```

---

# 14. Assignment Requirements Checklist

| Requirement | Implementation |
|---|---|
| Set target `(x, y, theta)` | `goal <x> <y> <theta>` command |
| Cancel target | `cancel` command |
| Action server for navigation | `rt2_nav_server::NavServer` |
| UI/action client | `rt2_nav_client::NavUi` |
| Feedback during execution | `NavigateToPose` feedback |
| Result at completion | `NavigateToPose` result |
| Cancel handling | action cancel callback + `stop_robot()` |
| C++ implementation | all main packages are C++ |
| Components/plugins | server and UI registered with `RCLCPP_COMPONENTS_REGISTER_NODE` |
| Same container | `rt2_nav_container` |
| Launch file | `nav_components.launch.py` |
| GitHub submission | repository link |

---

# 15. Notes

- The navigation logic does not implement obstacle avoidance, consistently with the assignment specification.
- The robot uses `/odom` to estimate its current pose.
- The server commands the robot through `/cmd_vel`.
- The final target orientation is expressed in radians.
- Example values:
  - `0.0` means facing forward;
  - `1.57` approximately corresponds to 90 degrees;
  - `3.14` approximately corresponds to 180 degrees.

---

# 16. Repository Content

Expected relevant files:

```text
.
├── command_panel.sh
├── launcher.sh
├── odom_clean.sh
├── run_assignment.sh
├── .gitignore
└── src
    ├── bme_gazebo_sensors
    ├── rt2_nav_bringup
    ├── rt2_nav_client
    ├── rt2_nav_interfaces
    └── rt2_nav_server
```

Generated build folders such as `build/`, `install/`, and `log/` are intentionally excluded from the repository.
