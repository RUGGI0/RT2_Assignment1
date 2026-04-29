#!/bin/bash

SESSION="rt2_assignment1"
WORKSPACE="/home/ubuntu/ros2_workshop"

# Kill previous session if it exists
tmux kill-session -t "$SESSION" 2>/dev/null
# kill old servers (avoid duplicates)
pkill -f nav_server_node 2>/dev/null

###############################################
# Layout 2x3:
#
#  ┌───────────────┬───────────────┐
#  │ 0 ROBOT       │ 1 SERVER      │
#  ├───────────────┼───────────────┤
#  │ 2 ODOM        │ 3 MONITOR     │
#  ├───────────────┼───────────────┤
#  │ 4 GOAL        │ 5 DEBUG       │
#  └───────────────┴───────────────┘
###############################################

###############################################
# Create session
###############################################
tmux new-session -d -s "$SESSION" -c "$WORKSPACE"

###############################################
# tmux settings
###############################################
tmux set-option -t "$SESSION" pane-border-status top
tmux set-option -t "$SESSION" pane-border-format " #{pane_index} #{pane_title} "
tmux set-option -t "$SESSION" status-style bg=black,fg=white
tmux set-option -t "$SESSION" pane-active-border-style fg=green
tmux set-option -t "$SESSION" pane-border-style fg=white
tmux set-option -t "$SESSION" mouse on

###############################################
# CREATE 6 PANES (2 columns x 3 rows)
###############################################

# Create second column
tmux split-window -h -t "$SESSION":0

# LEFT column (3 rows)
tmux select-pane -t "$SESSION":0.0
tmux split-window -v
tmux select-pane -t "$SESSION":0.0
tmux split-window -v

# RIGHT column (3 rows)
tmux select-pane -t "$SESSION":0.1
tmux split-window -v
tmux select-pane -t "$SESSION":0.3
tmux split-window -v

###############################################
# Balance layout
###############################################
tmux select-layout -t "$SESSION" tiled

###############################################
# Set pane titles
###############################################
tmux select-pane -t "$SESSION":0.0 -T "ROBOT"
tmux select-pane -t "$SESSION":0.1 -T "SERVER"
tmux select-pane -t "$SESSION":0.2 -T "ODOM"
tmux select-pane -t "$SESSION":0.3 -T "MONITOR"
tmux select-pane -t "$SESSION":0.4 -T "GOAL"
tmux select-pane -t "$SESSION":0.5 -T "DEBUG"

###############################################
# PANE 0 — ROBOT
###############################################
tmux select-pane -t "$SESSION":0.0
tmux send-keys "cd $WORKSPACE" C-m
tmux send-keys "source /opt/ros/jazzy/setup.bash" C-m
tmux send-keys "source install/setup.bash" C-m
tmux send-keys "(sleep 8 && wmctrl -r 'RViz' -e 0,10000,10000,800,600 && wmctrl -a 'Gazebo Sim') &" C-m
tmux send-keys "ros2 launch bme_gazebo_sensors spawn_robot.launch.py" C-m

###############################################
# PANE 1 — SERVER
###############################################
tmux select-pane -t "$SESSION":0.1
tmux send-keys "cd $WORKSPACE" C-m
tmux send-keys "source /opt/ros/jazzy/setup.bash" C-m
tmux send-keys "source install/setup.bash" C-m
tmux send-keys "ros2 run rt2_nav_server nav_server_node" C-m

###############################################
# PANE 2 — ODOM
###############################################
tmux select-pane -t "$SESSION":0.2
tmux send-keys "cd $WORKSPACE" C-m
tmux send-keys "source /opt/ros/jazzy/setup.bash" C-m
tmux send-keys "source install/setup.bash" C-m
tmux send-keys "/home/ubuntu/ros2_workshop/odom_clean.sh" C-m

###############################################
# PANE 3 — MONITOR
###############################################
tmux select-pane -t "$SESSION":0.3
tmux send-keys "cd $WORKSPACE" C-m
tmux send-keys "source /opt/ros/jazzy/setup.bash" C-m
tmux send-keys "source install/setup.bash" C-m
tmux send-keys "watch -n 1 'ros2 action list; echo; ros2 node list; echo; ros2 topic list | grep odom'" C-m

###############################################
# PANE 4 — GOAL
###############################################
tmux select-pane -t "$SESSION":0.4
tmux send-keys "cd $WORKSPACE" C-m
tmux send-keys "source /opt/ros/jazzy/setup.bash" C-m
tmux send-keys "source install/setup.bash" C-m

###############################################
# PANE 5 — DEBUG
###############################################
tmux select-pane -t "$SESSION":0.5
tmux send-keys "cd $WORKSPACE" C-m
tmux send-keys "source /opt/ros/jazzy/setup.bash" C-m
tmux send-keys "source install/setup.bash" C-m
tmux send-keys "clear" C-m

###############################################
# Start on GOAL pane
###############################################
tmux select-pane -t "$SESSION":0.4

###############################################
# Attach
###############################################
tmux attach -t "$SESSION"
