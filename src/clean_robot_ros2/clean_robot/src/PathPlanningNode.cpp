#include "../include/CleaningPathPlanner.h"
#include <std_msgs/msg/bool.hpp>
#include <nav2_costmap_2d/costmap_2d_ros.hpp>
#include <rclcpp/rclcpp.hpp>

class PathPlanningNode : public rclcpp::Node
{
public:
  PathPlanningNode() : Node("path_planning_node")
  {
    // Declare parameter for costmap name
    this->declare_parameter("costmap_name", std::string("global_costmap/costmap"));
    std::string costmap_name;
    this->get_parameter("costmap_name", costmap_name);

    // Initialize Costmap2DROS using Jazzy-compliant signature: (name, use_sim_time)
    costmap_ros_ = std::make_shared<nav2_costmap_2d::Costmap2DROS>(
        "global_costmap", true);

    // Subscribe to cleanup trigger
    trigger_sub_ = this->create_subscription<std_msgs::msg::Bool>(
        "/explore/cleanup_trigger", 10,
        std::bind(&PathPlanningNode::triggerCallback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "Path Planning Node initialized. Waiting for cleanup trigger...");
  }

  void startPlanning()
  {
    costmap_ros_->on_configure(rclcpp_lifecycle::State());
    costmap_ros_->on_activate(rclcpp_lifecycle::State());

    RCLCPP_INFO(this->get_logger(), "Generating lawnmower coverage path...");
    CleaningPathPlanning planner(costmap_ros_.get());
    planner.GetPathInROS();
    planner.PublishCoveragePath();
    RCLCPP_INFO(this->get_logger(), "Lawnmower coverage path published!");
  }

private:
  void triggerCallback(const std_msgs::msg::Bool::SharedPtr msg)
  {
    if (msg->data) {
      RCLCPP_INFO(this->get_logger(), "Received cleanup trigger! Starting path generation...");
      startPlanning();
    }
  }

  std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros_;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr trigger_sub_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<PathPlanningNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
