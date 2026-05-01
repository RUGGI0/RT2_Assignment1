from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from ament_index_python.packages import get_package_share_directory

import os


def generate_launch_description():
    bringup_share = get_package_share_directory('rt2_nav_bringup')
    params_file = os.path.join(bringup_share, 'config', 'nav_params.yaml')

    container = ComposableNodeContainer(
        name='rt2_nav_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            ComposableNode(
                package='rt2_nav_server',
                plugin='rt2_nav_server::NavServer',
                name='rt2_nav_server',
                parameters=[params_file],
            ),
            ComposableNode(
                package='rt2_nav_client',
                plugin='rt2_nav_client::NavUi',
                name='rt2_nav_ui',
            ),
        ],
        output='screen',
    )

    return LaunchDescription([
        container
    ])