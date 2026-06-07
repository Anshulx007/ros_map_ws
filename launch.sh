#!/bin/bash

# Source local workspace setup
if [ -f "/home/anshul/ros2_explore_final_backup/install/setup.bash" ]; then
    echo "Sourcing workspace setup..."
    source /home/anshul/ros2_explore_final_backup/install/setup.bash
else
    echo "Error: /home/anshul/ros2_explore_final_backup/install/setup.bash not found. Please build the workspace first."
    exit 1
fi

# Force Software Rendering / CPU rendering (ensures compatibility in VMs and WSL2)
export LIBGL_ALWAYS_SOFTWARE=1


# Set default parameters (GUI enabled by default)
USE_RVIZ="True"
HEADLESS="False"

# Parse optional command line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --headless) HEADLESS="True"; shift ;;
        --no-rviz) USE_RVIZ="False"; shift ;;
        -h|--help) 
            echo "Usage: ./launch.sh [options]"
            echo "Options:"
            echo "  --headless   Run Gazebo simulation without GUI (headless)"
            echo "  --no-rviz    Disable RViz visualization window"
            exit 0
            ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

echo "Starting Simulation (RViz=$USE_RVIZ, Headless=$HEADLESS)..."
ros2 launch clean_robot_bringup explore_and_clean.launch.py use_rviz:=$USE_RVIZ headless:=$HEADLESS
