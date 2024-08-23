# === Main game application === 
cmake_minimum_required(VERSION 3.25)
project(game)

# Define executable target
add_executable(game main.cpp)

# Apply dependencies
add_subdirectory(./deps)

# Ship into game directory
set_target_properties(
    game 
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/game"
)