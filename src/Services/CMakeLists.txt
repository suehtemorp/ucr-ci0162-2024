# Add services

# - Window service
target_sources(game PRIVATE WindowService.cpp)

# - Asset store
target_sources(game PRIVATE AssetStore.cpp)

# - Stopwatch service
target_sources(game PRIVATE StopwatchService.cpp)
