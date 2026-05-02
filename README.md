# RT2 Assignment 1 — Component-Based 2D Navigation with ROS2 Actions

This project implements a **ROS2 C++ navigation system** for a differential-drive robot in Gazebo.

The robot can receive a target pose:

```text
(x, y, theta)
```

and navigate toward it using:

- a custom ROS2 Action;
- odometry feedback from `/odom`;
- velocity commands on `/cmd_vel`;
- a component-based architecture;
- a terminal command panel for user interaction.

The final system runs the **action server** and the **UI/action client** as ROS2 components inside the same container.

---

## Table of Contents

1. [Quick Start](#1-quick-start)
2. [(The very cool) Script-Based Demo](#2-(the-very-cool)-script-based-demo)
3. [Command Panel](#3-command-panel)
4. [System Architecture](#4-system-architecture)
5. [Packages](#5-packages)
6. [Custom Action](#6-custom-action)
7. [Navigation Logic](#7-navigation-logic)
8. [Components and Container](#8-components-and-container)
9. [Manual Execution](#9-manual-execution)
10. [Debug Commands](#10-debug-commands)
11. [Notes](#11-notes)

---

# 1. Quick Start

From the ROS2 workspace root:

```bash
cd /home/ubuntu/ros2_workshop
source /opt/ros/jazzy/setup.bash
colcon build
source install/setup.bash
```

Then run the assignment launcher:

```bash
chmod +x launcher.sh
chmod +x run_assignment.sh
chmod +x odom_clean.sh
chmod +x command_panel.sh

./launcher.sh
```

Press:

```text
S
```

to start the full environment.

---

# 2. (The very cool) Script-Based Demo 

The recommended way to run the project is through the provided scripts:

```text
launcher.sh
run_assignment.sh
command_panel.sh
odom_clean.sh
```

## 2.1 `launcher.sh`

This is the entry point of the demo.

It opens a small menu:

```text
S = Start
R = Restart
A = Annull
Q = Quit
```

When `S` is pressed, it starts `run_assignment.sh` in a new terminal.

## 2.2 `run_assignment.sh`

This script creates a `tmux` 2×3 layout and starts all required processes.

```text
┌───────────────┬───────────────┐
│ 0 ROBOT       │ 1 SERVER+UI   │
├───────────────┼───────────────┤
│ 2 ODOM        │ 3 MONITOR     │
├───────────────┼───────────────┤
│ 4 COMMAND     │ 5 DEBUG       │
└───────────────┴───────────────┘
```

| Pane | Name | Purpose |
|---|---|---|
| 0 | ROBOT | Starts Gazebo and spawns the robot |
| 1 | SERVER+UI | Starts the component container with server and UI |
| 2 | ODOM | Shows a cleaned odometry dashboard |
| 3 | MONITOR | Shows actions, nodes, and relevant topics |
| 4 | COMMAND | Allows user commands such as `goal` and `cancel` |
| 5 | DEBUG | Free terminal for additional checks |

## 2.3 `odom_clean.sh`

This script displays a simplified odometry dashboard instead of raw `/odom` output.

It shows:

```text
position.x
position.y
theta/yaw
linear.x
linear.y
angular.z
```

This makes the robot state easier to read during the demo.

## 2.4 `command_panel.sh`

This script provides a simple command interface.

Instead of typing a full ROS2 topic command, the user can simply write:

```bash
goal 2.0 2.0 1.57
```

or:

```bash
cancel
```

The script internally publishes the corresponding command on `/nav_ui_command`.

---

# 3. Command Panel

The `COMMAND` pane is the main user interface during the demo.

## Send a goal

Format:

```bash
goal <x> <y> <theta>
```

Example:

```bash
goal 2.0 2.0 1.57
```

This sends the robot to:

```text
x = 2.0
y = 2.0
theta = 1.57 rad
```

## Cancel the active goal

```bash
cancel
```

## Exit the command panel

```bash
exit
```

or:

```bash
quit
```

---

# 4. System Architecture

The final system is based on two ROS2 components:

1. **NavServer** — the action server that controls the robot.
2. **NavUi** — the action client/UI that receives commands and sends/cancels goals.

Both are loaded into the same component container.

```text
                           +--------------------------------------+
                           |          rt2_nav_container           |
                           |      ROS2 Component Container        |
                           |                                      |
                           |   +------------------------------+   |
                           |   |        rt2_nav_ui            |   |
                           |   |   Action Client / UI         |   |
                           |   +------------------------------+   |
                           |          ^              |            |
                           |          |              |            |
                           |  /nav_ui_command        | action goal/cancel
                           |          |              v            |
                           |   +------------------------------+   |
                           |   |      rt2_nav_server          |   |
                           |   |      Action Server           |   |
                           |   +------------------------------+   |
                           +----------|--------------^------------+
                                      |              |
                                      | /cmd_vel     | /odom
                                      v              |
                           +--------------------------------------+
                           |              Gazebo Robot            |
                           +--------------------------------------+
```

---

# 5. Packages

```text
src/
├── bme_gazebo_sensors
├── rt2_nav_bringup
├── rt2_nav_client
├── rt2_nav_interfaces
└── rt2_nav_server
```

## `rt2_nav_interfaces`

Contains the custom action:

```text
NavigateToPose.action
```

## `rt2_nav_server`

Contains the C++ action server.

It:

- subscribes to `/odom`;
- publishes `/cmd_vel`;
- computes the navigation control;
- publishes feedback;
- handles goal cancellation;
- runs both as standalone node and as component.

## `rt2_nav_client`

Contains the C++ UI/action client.

It:

- subscribes to `/nav_ui_command`;
- parses commands such as `goal <x> <y> <theta>` and `cancel`;
- sends goals to the action server;
- receives feedback and final result;
- runs both as standalone node and as component.

## `rt2_nav_bringup`

Contains:

```text
launch/nav_components.launch.py
config/nav_params.yaml
```

## `bme_gazebo_sensors`

Simulation package used to spawn the robot in Gazebo.

The world is configured as a free environment, consistently with the assignment requirement of navigation without obstacle avoidance.

---

# 6. Custom Action

The system uses a custom action:

```text
rt2_nav_interfaces/action/NavigateToPose.action
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

Target pose of the robot.

## Result

```text
success
message
```

Final status of the action.

## Feedback

```text
current_x
current_y
current_theta
distance_error
heading_error
phase
```

Runtime information about the navigation process.

---

# 7. Navigation Logic

The navigation server implements a simple 2D state machine:

```text
ROTATE_TO_TARGET → MOVE_TO_TARGET → ROTATE_TO_FINAL → SUCCESS
```

## `ROTATE_TO_TARGET`

The robot rotates toward the target position.

The desired heading is computed from:

```text
atan2(goal_y - current_y, goal_x - current_x)
```

## `MOVE_TO_TARGET`

The robot moves toward the target while correcting its heading.

The linear and angular velocities are computed with proportional control and saturated using configurable maximum values.

## `ROTATE_TO_FINAL`

Once the target position is reached, the robot rotates to the desired final orientation `theta`.

## Success condition

The goal succeeds when:

```text
distance_error < position_tolerance
abs(final_angle_error) < angle_tolerance
```

---

# 8. Components and Container

Both main nodes are implemented as ROS2 components.

## Server component

```text
rt2_nav_server::NavServer
```

## UI/action client component

```text
rt2_nav_client::NavUi
```

They are loaded by:

```bash
ros2 launch rt2_nav_bringup nav_components.launch.py
```

The launch file starts:

```text
rt2_nav_container
```

and loads both plugins inside it.

This satisfies the assignment requirement that the UI/action client and the action server are executed as libraries/plugins within the same container.

---

# 9. Manual (and boring) Execution Demo

The script-based execution is recommended, but the system can also be launched manually.

## Terminal 1 — Gazebo

```bash
cd /home/ubuntu/ros2_workshop
source /opt/ros/jazzy/setup.bash
source install/setup.bash

ros2 launch bme_gazebo_sensors spawn_robot.launch.py
```

## Terminal 2 — Component container

```bash
cd /home/ubuntu/ros2_workshop
source /opt/ros/jazzy/setup.bash
source install/setup.bash

ros2 launch rt2_nav_bringup nav_components.launch.py
```

## Terminal 3 — Send a goal

```bash
cd /home/ubuntu/ros2_workshop
source /opt/ros/jazzy/setup.bash
source install/setup.bash

ros2 topic pub --once /nav_ui_command std_msgs/msg/String "{data: 'goal 2.0 2.0 1.57'}"
```

## Cancel

```bash
ros2 topic pub --once /nav_ui_command std_msgs/msg/String "{data: 'cancel'}"
```

---

# 10. Debug Commands

Check available actions:

```bash
ros2 action list
```

Check the action interface:

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

Inspect odometry:

```bash
ros2 topic echo /odom
```

Inspect velocity commands:

```bash
ros2 topic echo /cmd_vel
```

Inspect UI commands:

```bash
ros2 topic echo /nav_ui_command
```

---

# 11. Notes

- The navigation does not implement obstacle avoidance, as requested by the assignment.
- The robot pose is estimated from `/odom`.
- The robot is controlled through `/cmd_vel`.
- The final orientation `theta` is expressed in radians.
- Useful angle values:
  - `0.0` = facing forward;
  - `1.57` ≈ 90 degrees;
  - `3.14` ≈ 180 degrees.
- Generated folders such as `build/`, `install/`, and `log/` are excluded from the repository.
