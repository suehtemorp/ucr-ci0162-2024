# === GLM Math Library === 
# Adapted from GitHub documentation
cmake_minimum_required(VERSION 3.25)

# Opt to switch between a system glm library and a vendored glm libraries
option(GLM_VENDORED "Use GLM vendored libraries" ON)

# Opt to switch between the header-only or compiled version of the libraries
option(GLM_HEADER_ONLY "Use GLM header-only version" OFF)

# Opt to build the libraries as shared or static
option(GLM_SHARED "If building the GLM vendored libraries, build them as dynamic" OFF)

if(GLM_VENDORED)
    set(BUILD_SHARED_LIBS GLM_SHARED)

    # Do not build tests for the libraries
    set(GLM_BUILD_TESTS OFF)

    add_subdirectory(./glm EXCLUDE_FROM_ALL)
else()
    find_package(glm CONFIG REQUIRED)
endif()

# Link to the selected version of the GLM library
if (GLM_HEADER_ONLY)
    target_link_libraries(main PRIVATE glm::glm-header-only)
else()
    target_link_libraries(game PRIVATE glm::glm)
endif()
