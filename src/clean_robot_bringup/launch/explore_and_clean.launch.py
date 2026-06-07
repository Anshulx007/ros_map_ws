import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # Path to Nav2 tb3_simulation_launch.py
    nav2_bringup_dir = get_package_share_directory('nav2_bringup')
    sim_launch_path = os.path.join(nav2_bringup_dir, 'launch', 'tb3_simulation_launch.py')

    # Launch configuration variables
    use_rviz = LaunchConfiguration('use_rviz', default='True')
    headless = LaunchConfiguration('headless', default='True')

    # Path to custom clean_robot.rviz
    bringup_share_dir = get_package_share_directory('clean_robot_bringup')
    custom_rviz_path = os.path.join(bringup_share_dir, 'rviz', 'clean_robot.rviz')

    # Simulation inclusion
    sim_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(sim_launch_path),
        launch_arguments={
            'slam': 'True',
            'use_rviz': use_rviz,
            'headless': headless,
            'world': '/home/anshul/ros2_ws/random_room.world',
            'rviz_config_file': custom_rviz_path
        }.items()
    )

    # Explore Node (Frontier Exploration)
    explore_node = Node(
        package='explore',
        executable='explore_node',
        name='explore',
        parameters=[{
            'use_sim_time': True,
            'visualize': True,
            'costmap_topic': '/map',
            'costmap_updates_topic': '/map_updates',
            'robot_base_frame': 'base_link',
            'min_frontier_size': 0.5,
            'planner_frequency': 1.0,
            'progress_timeout': 30.0
        }],
        output='screen'
    )

    # Next Goal Node (Coverage Path Follower)
    next_goal_node = Node(
        package='clean_robot',
        executable='next_goal_node',
        name='next_goal',
        parameters=[{
            'use_sim_time': True,
            'tolerance_goal': 0.3
        }],
        output='screen'
    )

    # Path Planning Node (Coverage Planner)
    path_planning_node = Node(
        package='clean_robot',
        executable='path_planning_node',
        name='path_planning',
        parameters=[{
            'use_sim_time': True,
            'costmap_name': 'global_costmap/costmap',
            'size_of_cell': 3,
            'grid_covered_value': 0
        }],
        output='screen'
    )

    return LaunchDescription([
        DeclareLaunchArgument('use_rviz', default_value='True', description='Whether to start RViz'),
        DeclareLaunchArgument('headless', default_value='True', description='Whether to run Gazebo headless'),
        sim_launch,
        explore_node,
        next_goal_node,
        path_planning_node
    ])
