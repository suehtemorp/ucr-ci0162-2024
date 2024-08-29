# === Main game application === 
cmake_minimum_required(VERSION 3.25)

# Define executable target
add_executable(game main.cpp)

# ==== Add headers ====
target_include_directories(game PUBLIC ./)

# ==== Link dependencies ====
# [[[ SDL2 ]]]
# - Link the SDL2main component if it is required and found
# * SDL2::SDL2main may or may not be available. 
# * It is e.g. required by Windows GUI applications
if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions
    # It MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(game PRIVATE SDL2::SDL2main)
endif()
# - Link to the actual SDL2 library
if(SDL2_STATIC)
    # Static library
    target_link_libraries(game PRIVATE SDL2::SDL2-static)
else()
    # Shared library
    target_link_libraries(game PRIVATE SDL2::SDL2)
endif()

# [[[ GLM ]]]
# - Link to either the compiled or header-only version
if (GLM_HEADER_ONLY)
    target_link_libraries(game PRIVATE glm::glm-header-only)
else()
    target_link_libraries(game PRIVATE glm::glm)
endif()

# Ship into game directory
set_target_properties(
    game 
    PROPERTIES
        CXX_STANDARD 20
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/game"
        OUTPUT_NAME "Game"
        SUFFIX ".exe"
)