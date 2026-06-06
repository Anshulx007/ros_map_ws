/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2008, Robert Bosch LLC.
 *  Copyright (c) 2015-2016, Jiri Horner.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Jiri Horner nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************/

#include <explore/explore.h>

#include <thread>

inline static bool pointsClose(const geometry_msgs::msg::Point& one,
                               const geometry_msgs::msg::Point& two)
{
  double dx = one.x - two.x;
  double dy = one.y - two.y;
  double dist = sqrt(dx * dx + dy * dy);
  return dist < 0.01;
}

namespace explore
{
Explore::Explore(rclcpp::Node::SharedPtr node)
  : node_(node)
  , tf_buffer_(node_->get_clock())
  , tf_listener_(tf_buffer_)
  , costmap_client_(node_, &tf_buffer_)
  , prev_distance_(0)
  , last_markers_count_(0)
  , progress_timeout_(rclcpp::Duration::from_seconds(0.0))
{
  double timeout;
  double min_frontier_size;
  node_->declare_parameter("planner_frequency", 1.0);
  node_->get_parameter("planner_frequency", planner_frequency_);
  node_->declare_parameter("progress_timeout", 30.0);
  node_->get_parameter("progress_timeout", timeout);
  progress_timeout_ = rclcpp::Duration::from_seconds(timeout);
  node_->declare_parameter("visualize", false);
  node_->get_parameter("visualize", visualize_);
  node_->declare_parameter("potential_scale", 1e-3);
  node_->get_parameter("potential_scale", potential_scale_);
  node_->declare_parameter("orientation_scale", 0.0);
  node_->get_parameter("orientation_scale", orientation_scale_);
  node_->declare_parameter("gain_scale", 1.0);
  node_->get_parameter("gain_scale", gain_scale_);
  node_->declare_parameter("min_frontier_size", 0.5);
  node_->get_parameter("min_frontier_size", min_frontier_size);

  search_ = frontier_exploration::FrontierSearch(costmap_client_.getCostmap(),
                                                 potential_scale_, gain_scale_,
                                                 min_frontier_size);

  if (visualize_) {
    marker_array_publisher_ =
        node_->create_publisher<visualization_msgs::msg::MarkerArray>("frontiers", 10);
  }

  nav_to_pose_client_ =
      rclcpp_action::create_client<NavigateToPose>(node_, "navigate_to_pose");

  cleanup_trigger_pub_ =
      node_->create_publisher<std_msgs::msg::Bool>("/explore/cleanup_trigger", 10);

  RCLCPP_INFO(node_->get_logger(), "Waiting to connect to navigate_to_pose server");
  nav_to_pose_client_->wait_for_action_server();
  RCLCPP_INFO(node_->get_logger(), "Connected to navigate_to_pose server");

  exploring_timer_ =
      node_->create_wall_timer(
          std::chrono::duration<double>(1. / planner_frequency_),
          [this]() { makePlan(); });
}

Explore::~Explore()
{
  stop();
}

void Explore::visualizeFrontiers(
    const std::vector<frontier_exploration::Frontier>& frontiers)
{
  std_msgs::msg::ColorRGBA blue;
  blue.r = 0;
  blue.g = 0;
  blue.b = 1.0;
  blue.a = 1.0;
  std_msgs::msg::ColorRGBA red;
  red.r = 1.0;
  red.g = 0;
  red.b = 0;
  red.a = 1.0;
  std_msgs::msg::ColorRGBA green;
  green.r = 0;
  green.g = 1.0;
  green.b = 0;
  green.a = 1.0;

  RCLCPP_DEBUG(node_->get_logger(), "visualising %lu frontiers", frontiers.size());
  visualization_msgs::msg::MarkerArray markers_msg;
  std::vector<visualization_msgs::msg::Marker>& markers = markers_msg.markers;
  visualization_msgs::msg::Marker m;

  m.header.frame_id = costmap_client_.getGlobalFrameID();
  m.header.stamp = node_->now();
  m.ns = "frontiers";
  m.scale.x = 1.0;
  m.scale.y = 1.0;
  m.scale.z = 1.0;
  m.color.r = 0;
  m.color.g = 0;
  m.color.b = 255;
  m.color.a = 255;
  // lives forever
  m.lifetime = rclcpp::Duration::from_seconds(0);
  m.frame_locked = true;

  // weighted frontiers are always sorted
  double min_cost = frontiers.empty() ? 0. : frontiers.front().cost;

  m.action = visualization_msgs::msg::Marker::ADD;
  size_t id = 0;
  for (auto& frontier : frontiers) {
    m.type = visualization_msgs::msg::Marker::POINTS;
    m.id = int(id);
    m.pose.position = geometry_msgs::msg::Point();
    m.scale.x = 0.1;
    m.scale.y = 0.1;
    m.scale.z = 0.1;
    m.points = frontier.points;
    if (goalOnBlacklist(frontier.centroid)) {
      m.color = red;
    } else {
      m.color = blue;
    }
    markers.push_back(m);
    ++id;
    m.type = visualization_msgs::msg::Marker::SPHERE;
    m.id = int(id);
    m.pose.position = frontier.initial;
    // scale frontier according to its cost (costier frontiers will be smaller)
    double scale = std::min(std::abs(min_cost * 0.4 / frontier.cost), 0.5);
    m.scale.x = scale;
    m.scale.y = scale;
    m.scale.z = scale;
    m.points = {};
    m.color = green;
    markers.push_back(m);
    ++id;
  }
  size_t current_markers_count = markers.size();

  // delete previous markers, which are now unused
  m.action = visualization_msgs::msg::Marker::DELETE;
  for (; id < last_markers_count_; ++id) {
    m.id = int(id);
    markers.push_back(m);
  }

  last_markers_count_ = current_markers_count;
  marker_array_publisher_->publish(markers_msg);
}

void Explore::makePlan()
{
  // find frontiers
  auto pose = costmap_client_.getRobotPose();

  // get frontiers sorted according to cost
  auto frontiers = search_.searchFrom(pose.position);
  RCLCPP_DEBUG(node_->get_logger(), "found %lu frontiers, robot at (%.2f, %.2f)",
      frontiers.size(), pose.position.x, pose.position.y);
  for (size_t i = 0; i < frontiers.size(); ++i) {
    RCLCPP_DEBUG(node_->get_logger(), "frontier %zd cost: %f", i, frontiers[i].cost);
  }

  if (frontiers.empty()) {
    stop();
    auto trigger_msg = std_msgs::msg::Bool();
    trigger_msg.data = true;
    cleanup_trigger_pub_->publish(trigger_msg);
    return;
  }

  // publish frontiers as visualization markers
  if (visualize_) {
    visualizeFrontiers(frontiers);
  }

  // find non blacklisted frontier
  auto frontier =
      std::find_if_not(frontiers.begin(), frontiers.end(),
                       [this](const frontier_exploration::Frontier& f) {
                         return goalOnBlacklist(f.centroid);
                       });
  if (frontier == frontiers.end()) {
    stop();
    return;
  }
  geometry_msgs::msg::Point target_position = frontier->centroid;

  // time out if we are not making any progress
  bool same_goal = pointsClose(prev_goal_, target_position);
  prev_goal_ = target_position;
  if (!same_goal || prev_distance_ > frontier->min_distance) {
    // we have different goal or we made some progress
    last_progress_ = node_->now();
    prev_distance_ = frontier->min_distance;
  }
  // black list if we've made no progress for a long time
  if (node_->now() - last_progress_ > progress_timeout_) {
    frontier_blacklist_.push_back(target_position);
    RCLCPP_DEBUG(node_->get_logger(), "Adding current goal to black list");
    makePlan();
    return;
  }

  // we don't need to do anything if we still pursuing the same goal
  if (same_goal) {
    return;
  }

  // send goal to NavigateToPose
  NavigateToPose::Goal goal;
  goal.pose.pose.position = target_position;
  goal.pose.pose.orientation.w = 1.0;
  goal.pose.header.frame_id = costmap_client_.getGlobalFrameID();
  goal.pose.header.stamp = node_->now();

  auto send_goal_options =
      rclcpp_action::Client<NavigateToPose>::SendGoalOptions();
  send_goal_options.result_callback =
      [this, target_position](const auto &)
  {
      reachedGoal(target_position);
  };

  nav_to_pose_client_->async_send_goal(goal, send_goal_options);
}

bool Explore::goalOnBlacklist(const geometry_msgs::msg::Point& goal)
{
  constexpr static size_t tolerace = 5;
  nav2_costmap_2d::Costmap2D* costmap2d = costmap_client_.getCostmap();

  // check if a goal is on the blacklist for goals that we're pursuing
  for (auto& frontier_goal : frontier_blacklist_) {
    double x_diff = fabs(goal.x - frontier_goal.x);
    double y_diff = fabs(goal.y - frontier_goal.y);

    if (x_diff < tolerace * costmap2d->getResolution() &&
        y_diff < tolerace * costmap2d->getResolution())
      return true;
  }
  return false;
}

void Explore::reachedGoal(const geometry_msgs::msg::Point& frontier_goal)
{
  frontier_blacklist_.push_back(frontier_goal);
  makePlan();
}

void Explore::start()
{
}

void Explore::stop()
{
  if (nav_to_pose_client_) {
    nav_to_pose_client_->async_cancel_all_goals();
  }
  RCLCPP_INFO(node_->get_logger(), "Exploration stopped.");
}

}  // namespace explore

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("explore");
  
  // Spin the node in a background thread to allow constructor's spin_some calls to receive callbacks
  std::thread spin_thread([node]() {
    rclcpp::spin(node);
  });

  auto explore = std::make_shared<explore::Explore>(node);
  
  spin_thread.join();
  rclcpp::shutdown();
  return 0;
}
