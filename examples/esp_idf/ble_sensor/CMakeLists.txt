# ESP-IDF Project CMakeLists.txt for JenLib BLE Sensor Example
cmake_minimum_required(VERSION 3.20)

# Set project name
set(PROJECT_NAME "jenlib_ble_sensor")

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Define the project
project(${PROJECT_NAME})
