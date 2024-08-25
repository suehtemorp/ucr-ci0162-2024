# === GLM Math Library === 
# Adapted from GitHub documentation
# https://github.com/g-truc/glm/blob/33b4a621a697a305bc3a7610d290677b96beb181/readme.md#build-and-install
cmake_minimum_required(VERSION 3.25)

# Opt to switch between a system glm library and a vendored glm libraries
option(GLM_VENDORED "Use GLM vendored libraries" ON)

# Opt to switch between the header-only or compiled version of the libraries
option(GLM_HEADER_ONLY "Use GLM header-only version" OFF)

# Opt to build the libraries as shared or static
option(GLM_SHARED "If building the GLM vendored libraries, build them as dynamic" OFF)

# Find GLM library
# - Vendored (built)
if(GLM_VENDORED)
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

# Link library
# - Link to either the compiled or header-only version
if (GLM_HEADER_ONLY)
    target_link_libraries(game PRIVATE glm::glm-header-only)
else()
    target_link_libraries(game PRIVATE glm::glm)
endif()
