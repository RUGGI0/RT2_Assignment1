#!/bin/bash

source /opt/ros/jazzy/setup.bash
source /home/ubuntu/ros2_workshop/install/setup.bash

while true; do
  clear
  echo "ODOM X POSITION"
  echo "---------------"

  value=$(ros2 topic echo /odom --once --field pose.pose.position.x 2>/dev/null)

  python3 - <<PY
try:
    v = float("$value")
    if abs(v) < 1e-6:
        v = 0.0
    print(f"x = {v:.3f}")
except ValueError:
    print("Waiting for /odom...")
PY

  sleep 1
done