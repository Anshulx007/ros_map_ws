# ROS 2 Explore & Clean Simulation (Dockerized)

This repository contains a ROS 2 Jazzy simulation for an autonomous exploring and cleaning robot. The project has been fully containerized using Docker to run out of the box on other systems.

## Prerequisites

Make sure you have the following installed on your host system:
1. **Docker**: [Get Docker](https://docs.docker.com/get-docker/)
2. **Docker Compose**: [Get Docker Compose](https://docs.docker.com/compose/install/)
3. **X11 Server** (if running GUI / RViz):
   * **Linux**: Running natively. Make sure to run `xhost +local:docker` before launching to permit the container's GUI to display on your host.
   * **Windows (WSL2)**: Ensure WSLg is running or an X server like VcXsrv is active.

---

## Getting Started

### 1. Permit GUI Access
On Linux hosts, allow X11 connections from the local docker containers:
```bash
xhost +local:docker
```

### 2. Build the Docker Image
Navigate to this directory and build the container:
```bash
docker compose build
```

### 3. Run the Simulation

* **Option A: Run with RViz (Gazebo Headless)**
  Runs the physics simulation headlessly but pops up the RViz GUI on your screen:
  ```bash
  docker compose up
  ```

* **Option B: Run fully headless (No RViz, No Gazebo GUI)**
  Useful for CLI-only environments:
  ```bash
  docker compose run --rm simulation use_rviz:=False headless:=True
  ```

* **Option C: Run with all GUIs (Gazebo GUI and RViz)**
  ```bash
  docker compose run --rm simulation use_rviz:=True headless:=False
  ```

---

## Project Structure

* `src/` - ROS 2 packages (`explore`, `clean_robot`, `clean_robot_bringup`).
* `Dockerfile` - Builds the ROS 2 environment with Nav2, Slam Toolbox, and Gazebo.
* `docker-compose.yml` - Sets up environment variables, network bridges, and X11 sockets for GUI rendering.
