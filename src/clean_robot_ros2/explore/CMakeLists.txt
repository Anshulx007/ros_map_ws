cmake_minimum_required(VERSION 3.8)
project(explore)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)

find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)

find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(map_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)

find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)

find_package(nav2_costmap_2d REQUIRED)
find_package(nav2_msgs REQUIRED)

add_executable(explore_node
  src/explore.cpp
  src/frontier_search.cpp
  src/costmap_client.cpp
)

target_include_directories(explore_node PUBLIC
  include
)

ament_target_dependencies(
  explore_node

  rclcpp
  rclcpp_action

  geometry_msgs
  nav_msgs
  map_msgs
  visualization_msgs

  tf2
  tf2_ros

  nav2_costmap_2d
  nav2_msgs
)

install(TARGETS
  explore_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
