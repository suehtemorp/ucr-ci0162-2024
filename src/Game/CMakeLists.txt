# Add game-specific code

# - Parsing utilities
target_sources(game PRIVATE Parsing.cpp)

# - Service actions
target_sources(game PRIVATE ServiceActions.cpp)

# - Bootstrapping
target_sources(game PRIVATE main.cpp)
