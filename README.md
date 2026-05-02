# RT2 Assignment 1 — Component-Based 2D Navigation with ROS2 Actions

<p align="center">
  <b>ROS2 Jazzy · C++ · Actions · Components · Gazebo · tmux demo</b>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/ROS2-Jazzy-blue" />
  <img src="https://img.shields.io/badge/Language-C++-orange" />
  <img src="https://img.shields.io/badge/Architecture-Components-green" />
  <img src="https://img.shields.io/badge/Simulation-Gazebo-lightgrey" />
</p>

---

This project implements a **ROS2 C++ navigation system** for a differential-drive robot in Gazebo.

The robot receives a target pose:

```text
(x, y, theta)
```

and reaches it through an odometry-based action architecture.

| Feature | Implementation |
|---|---|
| Target setting | `goal <x> <y> <theta>` |
| Goal cancellation | `cancel` |
| Long-running task handling | ROS2 Action |
| Runtime feedback | Action feedback |
| Robot state estimation | `/odom` |
| Robot control | `/cmd_vel` |
| Deployment model | ROS2 components in one container |
| Demo interface | tmux launcher + command panel |

The final system runs the **action server** and the **UI/action client** as dynamically loaded ROS2 components inside the same container.

---

## Table of Contents

| Section | Description |
|---|---|
| [1. Quick Start](#1-quick-start) | Build and launch the project quickly |
| [2. (The very cool) Script-Based Demo](#2-the-very-cool-script-based-demo) | Recommended execution mode |
| [2.5 Script Requirements](#25-script-requirements) | Required utilities for the script-based demo |
| [3. Command Panel](#3-command-panel) | User commands for goal and cancel |
| [4. System Architecture](#4-system-architecture) | Components, topics, action, and robot |
| [5. Packages](#5-packages) | ROS2 package structure |
| [6. Custom Action](#6-custom-action) | `NavigateToPose.action` definition |
| [7. Navigation Logic](#7-navigation-logic) | 2D navigation state machine |
| [8. Components and Container](#8-components-and-container) | Plugin-based execution |
| [9. Manual (and boring) Execution Demo](#9-manual-and-boring-execution-demo) | Manual launch alternative |
| [10. Debug Commands](#10-debug-commands) | Useful ROS2 inspection commands |
| [11. Notes](#11-notes) | Additional implementation notes |

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
chmod +x launcher.sh run_assignment.sh odom_clean.sh command_panel.sh
./launcher.sh
```

Press:

```text
S
```

to start the full environment.

---

# 2. (The very cool) Script-Based Demo

The recommended way to run the assignment is through the provided scripts.

```text
launcher.sh        → opens the assignment menu
run_assignment.sh  → creates the tmux layout and starts the system
command_panel.sh   → provides short user commands
odom_clean.sh      → shows a readable odometry dashboard
```

## 2.1 Start the demo

From the workspace root:

```bash
cd /home/ubuntu/ros2_workshop
chmod +x launcher.sh run_assignment.sh odom_clean.sh command_panel.sh
./launcher.sh
```

The launcher opens a simple menu:

```text
S = Start
R = Restart
A = Annull
Q = Quit
```

Press:

```text
S
```

to start the full assignment environment.

## 2.2 Demo layout

`run_assignment.sh` creates a `tmux` 2×3 dashboard.

```text
┌───────────────────────────────┬───────────────────────────────┐
│ 0 ROBOT                       │ 1 SERVER+UI                   │
│ Gazebo + robot spawn          │ Component container            │
├───────────────────────────────┼───────────────────────────────┤
│ 2 ODOM                        │ 3 MONITOR                     │
│ Clean odometry dashboard      │ Actions, nodes, topics         │
├───────────────────────────────┼───────────────────────────────┤
│ 4 COMMAND                     │ 5 DEBUG                       │
│ goal / cancel interface       │ Free terminal                  │
└───────────────────────────────┴───────────────────────────────┘
```

| Pane | Name | What it does |
|---|---|---|
| 0 | ROBOT | Starts Gazebo and spawns the robot |
| 1 | SERVER+UI | Launches the component container with `NavServer` and `NavUi` |
| 2 | ODOM | Displays a compact odometry dashboard |
| 3 | MONITOR | Shows actions, nodes, and relevant topics |
| 4 | COMMAND | Provides the interactive command interface |
| 5 | DEBUG | Remains available for manual checks |

## 2.3 What each script does

| Script | Role |
|---|---|
| `launcher.sh` | Opens the start/restart/quit menu and starts the demo terminal |
| `run_assignment.sh` | Creates the tmux layout and launches all runtime processes |
| `command_panel.sh` | Lets the user type short commands such as `goal <x> <y> <theta>` and `cancel` |
| `odom_clean.sh` | Converts raw `/odom` into a readable pose/velocity dashboard |

## 2.4 Why the scripts are useful

Without the scripts, the assignment requires multiple terminals and long ROS2 commands.

With the scripts, the demo starts from a single menu and exposes only the relevant controls:

```bash
goal 2.0 2.0 1.57
cancel
```

## 2.5 Script Requirements

The script-based demo relies on a few terminal and window-management utilities.

| Requirement | Used by | Purpose |
|---|---|---|
| `tmux` | `run_assignment.sh` | Creates the 2×3 terminal dashboard |
| `x-terminal-emulator` | `launcher.sh` | Opens the assignment runner in a new terminal |
| `xdotool` | `run_assignment.sh` | Optionally minimizes RViz after startup |
| `wmctrl` | `run_assignment.sh` | Optionally focuses the Gazebo window |
| `bash` | all scripts | Executes the helper scripts |

Install them with:

```bash
sudo apt update
sudo apt install -y tmux xdotool wmctrl
```

Check that `x-terminal-emulator` is available with:

```bash
which x-terminal-emulator
```

If it is missing or not configured, run:

```bash
sudo update-alternatives --config x-terminal-emulator
```

In the provided Ubuntu MATE / noVNC environment, `x-terminal-emulator` is usually already available.

---

# 3. Command Panel

The `COMMAND` pane is the main user interface during the demo.

Instead of typing the full ROS2 command:

```bash
ros2 topic pub --once /nav_ui_command std_msgs/msg/String "{data: 'goal 2.0 2.0 1.57'}"
```

the user can type:

```bash
goal 2.0 2.0 1.57
```

## Available commands

| Command | Meaning |
|---|---|
| `goal <x> <y> <theta>` | Sends a new navigation target |
| `cancel` | Cancels the active target |
| `exit` / `quit` | Closes the command panel |

Example:

```bash
goal 5.0 5.0 0.0
```

This sends the robot to:

```text
x = 5.0
y = 5.0
theta = 0.0 rad
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
