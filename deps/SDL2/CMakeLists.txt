# === SDL2 graphics library === 
# Adapted from SDL2 wiki
# https://wiki.libsdl.org/SDL2/README/cmake
cmake_minimum_required(VERSION 3.25)

# Opt to switch between a system sdl library and a vendored sdl library
option(SDL2_LOCAL "Use the locally vendored SDL2 library" ON)

# Opt to switch between the static or dynamic version of the library
option(SDL2_STATIC "Link the locally vendored SDL2 library statically" ON)

# Find SDL2 library
# - Vendored (built)
if(SDL2_LOCAL)
    # Override internal static/shared build options on the subproject
    # Hack based on: https://cmake.org/cmake/help/latest/policy/CMP0077.html
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
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
