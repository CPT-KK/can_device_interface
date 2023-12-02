cmake_minimum_required(VERSION 3.8)

project(CANDevice VERSION 1.0.0 LANGUAGES CXX)

SET(CMAKE_BUILD_TYPE Debug)

include_directories(
  include
)

aux_source_directory(src DIR_SRCS)
add_executable(${PROJECT_NAME} ${DIR_SRCS})

target_link_libraries(${PROJECT_NAME} pthread)
