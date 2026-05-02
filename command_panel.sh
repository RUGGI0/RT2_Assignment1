#!/bin/bash

cd /home/ubuntu/ros2_workshop || exit 1
source /opt/ros/jazzy/setup.bash
source install/setup.bash

send_goal() {
  ros2 topic pub --once /nav_ui_command std_msgs/msg/String "{data: 'goal $1 $2 $3'}"
}

send_cancel() {
  ros2 topic pub --once /nav_ui_command std_msgs/msg/String "{data: 'cancel'}"
}

clear
cat <<'EOF'
COMMAND PANEL

Format:
  goal <x> <y> <theta>
  [Example:goal 2.0 2.0 1.57]
  [Theta = 1.57rad = 90°]

Other commands:
  cancel
  exit
EOF

while true; do
  read -r -p "rt2_cmd> " cmd x y theta

  case "$cmd" in
    goal)
      if [ -z "$x" ] || [ -z "$y" ] || [ -z "$theta" ]; then
        echo "Usage: goal x y theta"
      else
        send_goal "$x" "$y" "$theta"
      fi
      ;;
    cancel)
      send_cancel
      ;;
    exit|quit)
      break
      ;;
    "")
      ;;
    *)
      echo "Unknown command. Use: goal x y theta | cancel | exit"
      ;;
  esac
done
