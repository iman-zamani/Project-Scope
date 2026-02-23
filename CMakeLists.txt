cmake_minimum_required(VERSION 3.5)

project(LAC)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# executable target
add_executable(${PROJECT_NAME} main.cpp)

# Add Clip library submodule
# Disable Clip examples and tests
set(CLIP_EXAMPLES OFF CACHE BOOL "Compile clip examples")
set(CLIP_TESTS OFF CACHE BOOL "Compile clip tests")

# Add the clip subdirectory
add_subdirectory(clip)

# Link with the Clip library
target_link_libraries(${PROJECT_NAME} clip)
