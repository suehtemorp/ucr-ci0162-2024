# === Main game project === 
cmake_minimum_required(VERSION 3.25)
project(game)

# Add dependencies
add_subdirectory(./deps)

# Add main targets
add_subdirectory(./src)
