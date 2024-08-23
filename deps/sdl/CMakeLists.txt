# SDL2 main and core shared libraries
# - Common directories
set(WORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(INC_DIR "${WORK_DIR}/include")
set(LIB_DIR "${WORK_DIR}/lib/x64")

# - Main library
add_library(SDL2_Main SHARED IMPORTED)
set_property(
    TARGET SDL2_Main
    PROPERTY IMPORTED_LOCATION "${LIB_DIR}/SDL2main.lib"
)

# - Core library
add_library(SDL2_Core SHARED IMPORTED)
set_property(
    TARGET SDL2_Core 
    PROPERTY IMPORTED_LOCATION "${LIB_DIR}/SDL2.lib"
)

# * Link both in order
target_link_libraries(sdl_game PUBLIC SDL2_Main SDL2_Core)

# - Core library headers
target_include_directories(sdl_game PRIVATE "${INC_DIR}") 