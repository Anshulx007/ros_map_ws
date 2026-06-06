#include <rclcpp/rclcpp.hpp>

#include <nav_msgs/msg/odometry.hpp>
#include <nav_msgs/msg/path.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>

#include <vector>
#include <cmath>
#include <iostream>

using std::placeholders::_1;

float x_current = 0.0;
float y_current = 0.0;
float normeNextGoal = 0.3;

class QuaternionROS
{
public:
  float w = 1.0;
  float x = 0.0;
  float y = 0.0;
  float z = 0.0;

  void toQuaternion(float pitch, float roll, float yaw)
  {
    float cy = cos(yaw * 0.5);
    float sy = sin(yaw * 0.5);
    float cr = cos(roll * 0.5);
    float sr = sin(roll * 0.5);
    float cp = cos(pitch * 0.5);
    float sp = sin(pitch * 0.5);

    w = cy * cr * cp + sy * sr * sp;
    x = cy * sr * cp - sy * cr * sp;
    y = cy * cr * sp + sy * sr * cp;
    z = sy * cr * cp - cy * sr * sp;
  }
};

struct Goal
{
  float x;
  float y;
  bool visited;
};

class NextGoalNode : public rclcpp::Node
{
public:
  NextGoalNode() : Node("next_goal")
  {
    this->declare_parameter("tolerance_goal", 0.3);
    this->get_parameter("tolerance_goal", normeNextGoal);

    odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
      "/odom", 10,
      std::bind(&NextGoalNode::poseCallback, this, _1));

    path_sub_ = create_subscription<nav_msgs::msg::Path>(
      "/path_planning_node/cleaning_plan_nodehandle/cleaning_path",
      10,
      std::bind(&NextGoalNode::pathCallback, this, _1));

    goal_pub_ =
      create_publisher<geometry_msgs::msg::PoseStamped>(
        "/move_base_simple/goal", 10);

    passed_path_pub_ =
      create_publisher<nav_msgs::msg::Path>(
        "/clean_robot/passed_path", 10);

    timer_ =
      create_wall_timer(
        std::chrono::milliseconds(100),
        std::bind(&NextGoalNode::update, this));

    RCLCPP_INFO(
      get_logger(),
      "tolerance_goal=%f",
      normeNextGoal);
  }

private:
  void poseCallback(
    const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    x_current = msg->pose.pose.position.x;
    y_current = msg->pose.pose.position.y;

    passed_path_.header = msg->header;

    geometry_msgs::msg::PoseStamped p;
    p.header = msg->header;
    p.pose = msg->pose.pose;

    passed_path_.poses.push_back(p);

    passed_path_pub_->publish(passed_path_);
  }

  void pathCallback(
    const nav_msgs::msg::Path::SharedPtr msg)
  {
    if (path_.empty() ||
        msg->poses.size() != last_path_size_)
    {
      path_.clear();
      new_path_ = true;

      for (auto & pose : msg->poses)
      {
        path_.push_back({
          static_cast<float>(pose.pose.position.x),
          static_cast<float>(pose.pose.position.y),
          false});
      }

      last_path_size_ = msg->poses.size();
    }
  }

  void update()
  {
    if (new_path_)
    {
      count_ = 0;
      new_path_ = false;
    }

    if (path_.empty())
      return;

    if (count_ >= static_cast<int>(path_.size()))
      return;

    double dist =
      std::sqrt(
        std::pow(x_current - path_[count_].x, 2) +
        std::pow(y_current - path_[count_].y, 2));

    if (dist <= normeNextGoal)
    {
      count_++;
      goal_reached_ = false;
    }

    if (count_ >= static_cast<int>(path_.size()))
      return;

    if (!goal_reached_)
    {
      geometry_msgs::msg::PoseStamped goal;

      goal.header.frame_id = "odom";
      goal.header.stamp = now();

      goal.pose.position.x = path_[count_].x;
      goal.pose.position.y = path_[count_].y;

      QuaternionROS q;
      q.toQuaternion(0, 0, 0);

      goal.pose.orientation.w = q.w;
      goal.pose.orientation.x = q.x;
      goal.pose.orientation.y = q.y;
      goal.pose.orientation.z = q.z;

      goal_pub_->publish(goal);

      goal_reached_ = true;
    }
  }

  std::vector<Goal> path_;

  nav_msgs::msg::Path passed_path_;

  bool new_path_ = false;
  bool goal_reached_ = false;

  int count_ = 0;
  int last_path_size_ = 0;

  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr path_sub_;

  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr goal_pub_;
  rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr passed_path_pub_;

  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<NextGoalNode>());
  rclcpp::shutdown();
  return 0;
}
