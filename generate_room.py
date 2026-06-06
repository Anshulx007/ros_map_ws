import random

ROOM_SIZE = 10
NUM_WALLS = 15

world = """<?xml version="1.0" ?>
<sdf version="1.8">
<world name="random_room">
    <physics name="1ms" type="ignored">
      <max_step_size>0.001</max_step_size>
      <real_time_factor>1.0</real_time_factor>
    </physics>
    <plugin filename="gz-sim-physics-system" name="gz::sim::systems::Physics" />
    <plugin filename="gz-sim-user-commands-system" name="gz::sim::systems::UserCommands" />
    <plugin filename="gz-sim-scene-broadcaster-system" name="gz::sim::systems::SceneBroadcaster" />
    <plugin filename="gz-sim-sensors-system" name="gz::sim::systems::Sensors">
      <render_engine>ogre2</render_engine>
    </plugin>
    <plugin filename="gz-sim-imu-system" name="gz::sim::systems::Imu" />

    <light name="sun" type="directional">
      <cast_shadows>true</cast_shadows>
      <pose>0 0 10 0 0 0</pose>
      <diffuse>0.8 0.8 0.8 1</diffuse>
      <specular>0.2 0.2 0.2 1</specular>
      <attenuation>
        <range>1000</range>
        <constant>0.9</constant>
        <linear>0.01</linear>
        <quadratic>0.001</quadratic>
      </attenuation>
      <direction>-0.5 0.1 -0.9</direction>
    </light>

    <model name="ground_plane">
      <static>true</static>
      <link name="link">
        <collision name="collision">
          <geometry>
            <plane>
              <normal>0 0 1</normal>
              <size>100 100</size>
            </plane>
          </geometry>
        </collision>
        <visual name="visual">
          <geometry>
            <plane>
              <normal>0 0 1</normal>
              <size>100 100</size>
            </plane>
          </geometry>
          <material>
            <ambient>0.8 0.8 0.8 1</ambient>
            <diffuse>0.8 0.8 0.8 1</diffuse>
            <specular>0.8 0.8 0.8 1</specular>
          </material>
        </visual>
      </link>
    </model>
"""

# Outer walls
walls = [
    (0, ROOM_SIZE/2, ROOM_SIZE, 0.1, 1),
    (0, -ROOM_SIZE/2, ROOM_SIZE, 0.1, 1),
    (ROOM_SIZE/2, 0, 0.1, ROOM_SIZE, 1),
    (-ROOM_SIZE/2, 0, 0.1, ROOM_SIZE, 1)
]

# Random interior walls
for _ in range(NUM_WALLS):
    x = random.uniform(-4, 4)
    y = random.uniform(-4, 4)

    if random.random() < 0.5:
        sx = random.uniform(1, 3)
        sy = 0.1
    else:
        sx = 0.1
        sy = random.uniform(1, 3)

    walls.append((x, y, sx, sy, 1))

for i, (x, y, sx, sy, sz) in enumerate(walls):
    world += f"""
    <model name="wall_{i}">
      <static>true</static>
      <pose>{x} {y} 0.5 0 0 0</pose>
      <link name="link">
        <collision name="collision">
          <geometry>
            <box><size>{sx} {sy} {sz}</size></box>
          </geometry>
        </collision>
        <visual name="visual">
          <geometry>
            <box><size>{sx} {sy} {sz}</size></box>
          </geometry>
        </visual>
      </link>
    </model>
    """

world += """
</world>
</sdf>
"""

with open("random_room.world", "w") as f:
    f.write(world)

print("Generated random_room.world")
