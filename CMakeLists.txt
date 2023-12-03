cmake_minimum_required(VERSION 3.8)
project(CANDevice VERSION 1.0.0 LANGUAGES CXX)

add_compile_options(-std=c++11)

# Set the build type
# set(CMAKE_BUILD_TYPE Release)

# Set include directory
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# Add all source files in the src directory to the library
aux_source_directory(${PROJECT_SOURCE_DIR}/src CANDEV_DIR_SRCS)
add_library(${PROJECT_NAME} SHARED ${CANDEV_DIR_SRCS})

# Link the pthread library for parallel execution
target_link_libraries(${PROJECT_NAME} pthread)

# The following line allows other projects to use this library as a dependency via add_subdirectory() & target_link_libraries()
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)