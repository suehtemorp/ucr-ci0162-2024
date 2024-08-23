# Add compiling units
file(
    GLOB_RECURSE SOURCES
    "./*.cpp"
    "./.*c"
)

target_sources(sdl_game PUBLIC ${SOURCES})

# Add in-source headers
target_include_directories(sdl_game PUBLIC "./")