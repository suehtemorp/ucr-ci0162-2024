# === Auxiliary SDL2 TTF support API library === 
cmake_minimum_required(VERSION 3.25)

# Opt to switch between a system SDL_TTF library and a vendored version of it
option(SDL2_TTF_LOCAL "Build the SDL_ttf (SDL2) vendored source as a librarie" ON)

# Opt to build the libraries as shared or static
option(SDL2_TTF_SHARED "If building the vendored SDL_ttf (SDL2) library, build them as dynamic" OFF)

# Find FreeType library
# - Vendored (built)
if(SDL2_TTF_LOCAL)
    # Override internal BUILD_SHARED_LIBS option on the subproject
    # Hack based on: https://cmake.org/cmake/help/latest/policy/CMP0077.html
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set(BUILD_SHARED_LIBS ${SDL2_TTF_SHARED})

    # Also forcefully use vendored libraries
    set(SDL2TTF_VENDORED ON)
    add_subdirectory(./SDL_ttf EXCLUDE_FROM_ALL)
# - System-wide
else()
    find_package(SDL2_ttf CONFIG REQUIRED)
endif()
