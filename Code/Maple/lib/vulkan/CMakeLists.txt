cmake_minimum_required(VERSION 3.10)

project(vulkan)

target_include_directories(vulkan PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>")

