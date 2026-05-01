#!/bin/bash

source /opt/ros/jazzy/setup.bash
source /home/ubuntu/ros2_workshop/install/setup.bash

PY_SCRIPT=$(mktemp)

cat > "$PY_SCRIPT" <<'PY'
import sys
import math
import time
import os

top = None
level2 = None
level3 = None

data = {
    "x": None,
    "y": None,
    "qx": None,
    "qy": None,
    "qz": None,
    "qw": None,
    "vx": None,
    "vy": None,
    "wz": None,
}

last_print = 0.0
period = 1.0

def clean(v):
    if v is None:
        return None
    return 0.0 if abs(v) < 1e-6 else v

def parse_value(line):
    try:
        return float(line.split(":", 1)[1].strip())
    except Exception:
        return None

def print_dashboard():
    os.system("clear")

    print("ODOM STATE")
    print("----------")

    if any(v is None for v in data.values()):
        print("Waiting for /odom...")
        return

    qx = data["qx"]
    qy = data["qy"]
    qz = data["qz"]
    qw = data["qw"]

    siny_cosp = 2.0 * (qw * qz + qx * qy)
    cosy_cosp = 1.0 - 2.0 * (qy * qy + qz * qz)
    theta = math.atan2(siny_cosp, cosy_cosp)

    print(f"position.x      = {clean(data['x']): .3f}")
    print(f"position.y      = {clean(data['y']): .3f}")
    print(f"theta/yaw       = {clean(theta): .3f} rad")
    print("")
    print(f"linear.x        = {clean(data['vx']): .3f}")
    print(f"linear.y        = {clean(data['vy']): .3f}")
    print(f"angular.z       = {clean(data['wz']): .3f}")

for raw_line in sys.stdin:
    line = raw_line.rstrip("\n")
    stripped = line.strip()

    if not stripped or stripped == "---":
        continue

    indent = len(line) - len(line.lstrip(" "))

    if indent == 0 and stripped.endswith(":"):
        top = stripped[:-1]
        level2 = None
        level3 = None
        continue

    if indent == 2 and stripped.endswith(":"):
        level2 = stripped[:-1]
        level3 = None
        continue

    if indent == 4 and stripped.endswith(":"):
        level3 = stripped[:-1]
        continue

    if ":" not in stripped:
        continue

    value = parse_value(stripped)
    if value is None:
        continue

    # pose.pose.position
    if top == "pose" and level2 == "pose" and level3 == "position":
        if stripped.startswith("x:"):
            data["x"] = value
        elif stripped.startswith("y:"):
            data["y"] = value

    # pose.pose.orientation
    elif top == "pose" and level2 == "pose" and level3 == "orientation":
        if stripped.startswith("x:"):
            data["qx"] = value
        elif stripped.startswith("y:"):
            data["qy"] = value
        elif stripped.startswith("z:"):
            data["qz"] = value
        elif stripped.startswith("w:"):
            data["qw"] = value

    # twist.twist.linear
    elif top == "twist" and level2 == "twist" and level3 == "linear":
        if stripped.startswith("x:"):
            data["vx"] = value
        elif stripped.startswith("y:"):
            data["vy"] = value

    # twist.twist.angular
    elif top == "twist" and level2 == "twist" and level3 == "angular":
        if stripped.startswith("z:"):
            data["wz"] = value

    now = time.time()
    if now - last_print >= period:
        print_dashboard()
        last_print = now
PY

ros2 topic echo /odom | python3 -u "$PY_SCRIPT"

rm -f "$PY_SCRIPT"