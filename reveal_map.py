import rclpy
from rclpy.node import Node
from sensor_msgs.msg import LaserScan
from nav_msgs.msg import OccupancyGrid
import numpy as np
import math

class RevealMap(Node):
    def __init__(self):
        super().__init__('reveal_map')

        self.resolution = 0.05
        self.width = 400
        self.height = 400

        self.grid = -np.ones((self.height, self.width), dtype=np.int8)

        self.pub = self.create_publisher(
            OccupancyGrid,
            '/reveal_map',
            10)

        self.create_subscription(
            LaserScan,
            '/scan',
            self.scan_callback,
            10)

    def scan_callback(self, msg):

        angle = msg.angle_min

        for r in msg.ranges:

            if np.isfinite(r):

                x = r * math.cos(angle)
                y = r * math.sin(angle)

                gx = int(x / self.resolution + self.width / 2)
                gy = int(y / self.resolution + self.height / 2)

                if 0 <= gx < self.width and 0 <= gy < self.height:
                    self.grid[gy, gx] = 100

            angle += msg.angle_increment

        out = OccupancyGrid()
        out.info.resolution = self.resolution
        out.info.width = self.width
        out.info.height = self.height
        out.data = self.grid.flatten().tolist()

        self.pub.publish(out)

def main():
    rclpy.init()
    node = RevealMap()
    rclpy.spin(node)

if __name__ == '__main__':
    main()
