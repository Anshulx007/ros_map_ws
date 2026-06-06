import random

ROOM_SIZE = 10
NUM_WALLS = 15

world = """<?xml version="1.0" ?>
<sdf version="1.8">
<world name="random_room">
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
