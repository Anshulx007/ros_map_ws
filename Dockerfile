FROM osrf/ros:jazzy-desktop

# Install ROS 2 dependencies and utilities
RUN apt-get update && apt-get install -y \
    ros-jazzy-navigation2 \
    ros-jazzy-nav2-bringup \
    ros-jazzy-slam-toolbox \
    ros-jazzy-ros-gz \
    ros-jazzy-turtlebot3-simulations \
    python3-colcon-common-extensions \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /ros2_ws

# Copy source code and world file
COPY ./src /ros2_ws/src
COPY ./random_room.world /ros2_ws/random_room.world

# Build the workspace
RUN /bin/bash -c "source /opt/ros/jazzy/setup.bash && colcon build --symlink-install"

# Create a clean runtime launch script for the container
RUN echo '#!/bin/bash' > /ros2_ws/docker_launch.sh && \
    echo 'source /ros2_ws/install/setup.bash' >> /ros2_ws/docker_launch.sh && \
    echo 'export LIBGL_ALWAYS_SOFTWARE=1' >> /ros2_ws/docker_launch.sh && \
    echo 'ros2 launch clean_robot_bringup explore_and_clean.launch.py "$@"' >> /ros2_ws/docker_launch.sh && \
    chmod +x /ros2_ws/docker_launch.sh

ENTRYPOINT ["/ros2_ws/docker_launch.sh"]
