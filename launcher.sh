#!/bin/bash

SCRIPT_DIR="$(dirname "$0")"
RUN="$SCRIPT_DIR/run_assignment.sh"
SESSION="rt2_assignment1"
WORKSPACE_DIR="/home/ubuntu/ros2_workshop"

# Gradient colors
C1='\e[31m'   # red
C2='\e[33m'   # yellow
C3='\e[32m'   # green
C4='\e[36m'   # cyan
C5='\e[34m'   # blue
RESET='\e[0m'

kill_assignment_processes() {
    tmux kill-session -t "$SESSION" 2>/dev/null

    pkill -f "nav_server_node" 2>/dev/null
    pkill -f "nav_ui_node" 2>/dev/null
    pkill -f "spawn_robot.launch.py" 2>/dev/null
    pkill -f "rviz2" 2>/dev/null

    pkill -f "ros_gz_sim" 2>/dev/null
    pkill -f "gz sim" 2>/dev/null
    pkill -f "gzserver" 2>/dev/null
    pkill -f "gzclient" 2>/dev/null
}

open_assignment_terminal() {
    if command -v x-terminal-emulator >/dev/null 2>&1; then
        x-terminal-emulator -e "$RUN" &

        # Try to bring the assignment terminal to front and maximize it.
        # The sleep gives the window manager time to create the window.
        (
            sleep 1
            wmctrl -r :ACTIVE: -b add,maximized_vert,maximized_horz 2>/dev/null
        ) &

    elif command -v mate-terminal >/dev/null 2>&1; then
        mate-terminal --maximize -- bash -c "$RUN"

    elif command -v gnome-terminal >/dev/null 2>&1; then
        gnome-terminal --maximize -- bash -c "$RUN"

    else
        echo "No supported terminal emulator found."
        read -n1 -s
    fi
}

while true; do
    clear

    echo -e "${C1}                          ___           ___           ___                    ${RESET}"
    echo -e "${C1}                         /\  \         /\  \         /\  \                   ${RESET}"
    echo -e "${C1}                        /::\  \       /::\  \       /::\  \                  ${RESET}"
    echo -e "${C2}                       /:/\:\  \     /:/\:\  \     /:/\:\  \                 ${RESET}"
    echo -e "${C2}                      /::\ \:\  \   /:/  \:\  \   /:/  \:\  \                ${RESET}"
    echo -e "${C3}                     /:/\:\ \:\__\ /:/__/_\:\__\ /:/__/ \:\__\               ${RESET}"
    echo -e "${C3}                     \/_|::\/:/  / \:\  /\ \/__/ \:\  \ /:/  /               ${RESET}"
    echo -e "${C4}                        |:|::/  /   \:\ \:\__\    \:\  /:/  /                ${RESET}"
    echo -e "${C4}                        |:|\/__/     \:\/:/  /     \:\/:/  /                 ${RESET}"
    echo -e "${C5}                        |:|  |        \::/  /       \::/  /                  ${RESET}"
    echo -e "${C5}                         \|__|         \/__/         \/__/                   ${RESET}"

    echo ""
    echo "                           =============================="
    echo "                                   ASSIGNMENT TOOL"
    echo "                           =============================="
    echo ""
    echo "                                      S = Start"
    echo "                                      R = Restart"
    echo "                                      B = Build"
    echo "                                      A = Annull"
    echo "                                      Q = Quit"
    echo ""
    echo "                           =============================="
    echo -e "${C2}                                            [RT2 Version]${RESET}"

    read -n1 -s KEY

    case "$KEY" in
        S|s)
            kill_assignment_processes
            sleep 1
            open_assignment_terminal
            ;;
        R|r)
            kill_assignment_processes
            sleep 1
            open_assignment_terminal
            ;;
        B|b)
            echo -e "\n${C3}--- Avvio compilazione ---${RESET}"
            (
                cd "$WORKSPACE_DIR" || exit
                source /opt/ros/jazzy/setup.bash
                colcon build
            )
            echo -e "\n${C4}Build terminato. Premi un tasto per tornare al menu...${RESET}"
            read -n1 -s
            ;;
        A|a)
            kill_assignment_processes
            ;;
        Q|q)
            kill_assignment_processes
            clear
            exit 0
            ;;
    esac
done
