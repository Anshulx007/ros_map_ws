#!/bin/bash

# Source local workspace setup
if [ -f "/home/anshul/ros2_ws/install/setup.bash" ]; then
    echo "Sourcing workspace setup..."
    source /home/anshul/ros2_ws/install/setup.bash
else
    echo "Error: /home/anshul/ros2_ws/install/setup.bash not found. Please build the workspace first."
    exit 1
fi

# Force Hardware Acceleration / GPU Rendering (Mesa & NVIDIA support, works in WSL2/Linux)
export __NV_PRIME_RENDER_OFFLOAD=1
export __GLX_VENDOR_LIBRARY_NAME=nvidia
export MESA_D3D12_DEFAULT_ADAPTER_NAME=NVIDIA
export LIBGL_ALWAYS_SOFTWARE=0
export GALLIUM_DRIVER=zink


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
