from setuptools import find_packages, setup

package_name = 'clean_robot_bringup'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name + '/launch', ['launch/explore_and_clean.launch.py']),
        ('share/' + package_name + '/rviz', ['rviz/clean_robot.rviz']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='anshul',
    maintainer_email='anshul@todo.todo',
    description='TODO: Package description',
    license='TODO: License declaration',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
        ],
    },
)
