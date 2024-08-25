# === SDL2 graphics library === 
# Adapted from SDL2 wiki
# https://wiki.libsdl.org/SDL2/README/cmake
cmake_minimum_required(VERSION 3.25)

# Opt to switch between a system sdl library and a vendored sdl library
option(SDL2_VENDORED "Use SDL2 vendored libraries" ON)

# Opt to switch between the static or dynamic version of the library
option(SDL2_STATIC "Use SDL2 static libraries" ON)

# Find SDL2 library
# - Vendored (built)
if(SDL2_VENDORED)
    # Override internal static/shared build options on the subproject
    if(SDL2_STATIC)
        set(SDL_SHARED OFF)
        set(SDL_STATIC ON)
    else()
        set(SDL_SHARED ON)
        set(SDL_STATIC OFF)
    endif()
    
    add_subdirectory(./SDL EXCLUDE_FROM_ALL)
# - System-wide
else()
    # 1. SDL2 package, required SDL2 component
    if (SDL2_STATIC)
        find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2-static)
    else()
        find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
    endif()

    # 2. SDL2 package, optional SDL2main component
    find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
endif()

# Link libraries
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
