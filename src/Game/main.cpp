// Main definitions
#include "Game/Core.hpp"

// Service actions
#include "Game/ServiceActions.hpp"

// Input config parsing
#include "Game/Parsing.hpp"

// Easy I/O
#include <iostream>

// Filesystem navigation
#include <filesystem>

// SDL bootstrapping & cleanup
#include <SDL.h>
#include <SDL_ttf.h>

/// @brief Create game's resources and ECS given some config file
/// @param configFilepath C-string path to config file on disk 
void RunGame(const char* configFilepath) {
    // Create ECS
    std::cout << "Initializing ECS..." << std::endl;
    GameECS ecs;

    // Create outside services
    std::cout << "Initializing services..." << std::endl;

    // Asset store
    AssetStore assetStore;

    // Window (also adds entities)
    std::cout << "Loading config, window & entities..." << std::endl;
    WindowService windowService = std::move(
        ParseConfig(configFilepath, assetStore, ecs)
    );

    // Add systems
    std::cout << "Adding systems..." << std::endl;
    ecs.AddSystem(PhysicsSystem);
    ecs.AddSystem(DrawingSystem);

    // Install services
    std::cout << "Installing services..." << std::endl;
    ecs.InstallService(std::move(assetStore));
    ecs.InstallService(std::move(windowService));
    ecs.InstallService(StopwatchService());

    // Add actions
    std::cout << "Adding service actions..." << std::endl;
    ecs.AddServiceAction(HandleInput);
    ecs.AddServiceAction(DrawEntities);
    ecs.AddServiceAction(ResetDeltaTimer);

    // Starting running the ECS thread
    std::cout << "Starting ECS..." << std::endl;
    // ecs.Dispatch(); -> Multithreading and SDL might not work together
    ecs.Run();

    // Wait for it to stop
    // ecs.AwaitStop(); -> See comment above
    std::cout << "Quitting ECS..." << std::endl;
}

/// @brief Initialize game resources and cleanup afterwards
/// @param argc Amount of CLI arguments
/// @param args Vector of c-string CLI arguments
/// @return 0 on success, negative status code on error
int main(int argc, char* args[]) {
    // Make note of the usage when not provided
    // the proper amount of args
    if (argc != 2) {
        std::cout 
            << "Usage: " << std::endl 
            << args[0] << " <config filename path>" << std::endl;

        return -2;
    }

    if (
        !std::filesystem::exists(args[1]) ||
        !std::filesystem::path(args[1]).has_filename()
    ) {
        std::cerr 
            << "Invalid path for configuration file: "
            << "\"" << args[1] << "\""
            << std::endl;

        return -1;
    }

    // Initialize SDL
    std::cout << "Initializing SDL..." << std::endl;
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("SDL_Init error: %s\n", SDL_GetError());
        return -1;
    }

    // Initialize SDL's TTF
    std::cout << "Initializing SDL TTF..." << std::endl;
    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init error: %s\n", SDL_GetError());
        return -1;
    }

    // Run game until exited or an error occurs
    std::cout << "Bootstrapping game..." << std::endl;
    try {
        RunGame(args[1]);
    } catch(const std::exception& e) {
        std::cerr 
            << "An error ocurred while running the game: "
            << "\"" << e.what() << "\""
            << std::endl;
    }

    // Cleanup SDL's TTF
    std::cout << "Cleaning up SDL TTF..." << std::endl;
    TTF_Quit();

    // Cleanup SDL
    std::cout << "Cleaning up SDL..." << std::endl;
    SDL_Quit();

    // All is done
    return 0;
}
