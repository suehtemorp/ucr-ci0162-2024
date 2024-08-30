# === GLM Math Library === 
# Adapted from GitHub documentation
# https://github.com/g-truc/glm/blob/33b4a621a697a305bc3a7610d290677b96beb181/readme.md#build-and-install
cmake_minimum_required(VERSION 3.25)

# Opt to switch between a system glm library and a vendored glm libraries
option(GLM_LOCAL "Use the locally vendored GLM library" ON)

# Opt to switch between the header-only or compiled version of the libraries
option(GLM_HEADER_ONLY "Use the header-only version of the GLM library" OFF)

# Opt to build the libraries as shared or static
option(GLM_SHARED "Link the locally vendored GLM library dynamically" OFF)

# Find GLM library
# - Vendored (built)
if(GLM_LOCAL)
    # Override internal BUILD_SHARED_LIBS option on the subproject
    # Hack based on: https://cmake.org/cmake/help/latest/policy/CMP0077.html
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set(BUILD_SHARED_LIBS ${GLM_SHARED})

    # Do not build tests for the GLM libraries
    set(GLM_BUILD_TESTS OFF)
    
    add_subdirectory(./glm EXCLUDE_FROM_ALL)
# - System-wide
else()
    find_package(glm CONFIG REQUIRED)
endif()
