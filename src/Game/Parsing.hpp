#pragma once

// Core game definitions
#include "Game/Core.hpp"

// Service parsing

/// @brief Construct a window service given some input config
/// @param input Input stream containing space-delimited parameters
/// @return Constructed window
WindowService ParseWindow(std::istream& input);

/// @brief Load a font asset given some parameters
/// @param assetStore Asset store to load font into
/// @param input Input stream containing space-delimited parameters
void ParseFont(AssetStore& assetStore, std::istream& input);

/// @brief Load an entity given some parameters
/// @param window Window to draw entities with
/// @param assets Assets to draw entites with
/// @param assets ECS to place entities into
/// @param input Input stream containing space-delimited parameters
/// @return True if loaded succesfully, false otherwise
bool ParseEntity(WindowService& window, AssetStore& assets,
    GameECS& ecs, std::istream& input);

// Configuration parsing

/// @brief Attempt to parse and utilize the configuration 
/// provided to build entities and assets in the game 
/// @param configPath Path to the configuration file
/// @param assets Asset store used to load texture
/// @param ecs ECS used to add entites
/// @return Window used to render textures
WindowService ParseConfig(
    const char* path,
    AssetStore& assets, 
    GameECS& ecs
);
