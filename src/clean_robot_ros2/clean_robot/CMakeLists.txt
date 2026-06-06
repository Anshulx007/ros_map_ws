cmake_minimum_required(VERSION 3.8)
project(clean_robot)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(nav2_costmap_2d REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(next_goal_node src/next_goal.cpp)
add_executable(path_planning_node src/PathPlanningNode.cpp)

ament_target_dependencies(next_goal_node
  rclcpp
  nav_msgs
  geometry_msgs
)

ament_target_dependencies(path_planning_node
  rclcpp
  tf2
  tf2_ros
  nav2_costmap_2d
)

target_link_libraries(path_planning_node
  ${OpenCV_LIBS}
)

install(TARGETS
  next_goal_node
  path_planning_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
