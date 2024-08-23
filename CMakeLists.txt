# Metadata
cmake_minimum_required(VERSION 3.25.0)
project(sdl_game VERSION 0.1.0 LANGUAGES C CXX)

# Policies
# - C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executable
add_executable(sdl_game "src/main.cpp")

# Solve dependencies
# - External dependencies
add_subdirectory("./deps/")
# - Own source code
add_subdirectory("./src/")

# Ship into game directory
set_target_properties(
    sdl_game 
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/game"
)