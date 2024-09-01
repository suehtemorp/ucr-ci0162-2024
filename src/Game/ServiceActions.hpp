#pragma once

// Core game definitions
#include "Game/Core.hpp"

/// @brief Handle the input on a given frame
/// @param ecsManager ECS currently taking place
/// @param stopwatch Physics timekeeping service
void HandleInput(GameECS::ManagerService& ecsManager, 
    StopwatchService& stopwatch);

/// @brief Render the entities drawn on the current frame 
/// @param window Window service to draw entities from
void DrawEntities(WindowService& window);

/// @brief Reset the physics timer after a fixed
/// timestep
/// @param stopwatch 
void ResetDeltaTimer(StopwatchService& stopwatch);
