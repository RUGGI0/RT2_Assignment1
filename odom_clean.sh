#!/bin/bash

source /opt/ros/jazzy/setup.bash
source /home/ubuntu/ros2_workshop/install/setup.bash

ros2 topic echo /odom --field pose.pose.position.x | python3 -u -c "
import sys

for line in sys.stdin:
    line = line.strip()
    if not line or line == '---':
        continue
    try:
        v = float(line)
        if abs(v) < 1e-6:
            v = 0.0
        print(f'{v:.3f}')
    except ValueError:
        pass
"