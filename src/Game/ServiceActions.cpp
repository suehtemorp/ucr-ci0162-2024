// Definitions
#include "Game/ServiceActions.hpp"

// SDL utilities
#include <SDL_events.h>

void HandleInput(GameECS::ManagerService& ecsManager, 
    StopwatchService& stopwatch) {
    bool exitGame = false;

    // Check for input events
    for (SDL_Event currentEvent; 
        SDL_PollEvent(&currentEvent) == 1;) {
        switch (currentEvent.type)
        {
            // If exiting, stop the ECS
            case SDL_QUIT:
                exitGame = true;
                break;

            // Check if a key was pressed (indirectly by its release)
            case SDL_KEYUP:
                switch (currentEvent.key.keysym.sym)
                {
                    // If P is released, toggle the simulation delta
                    case SDLK_p:
                        stopwatch.Toggle();
                        break;

                    // If ESC is released, stop the ECS
                    case SDLK_ESCAPE:
                        exitGame = true;
                        break;
                
                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }

    // Request the ECS manager to stop running 
    // the systems and services
    if (exitGame) {
        ecsManager.RequestStop();
    }
}

void DrawEntities(WindowService& window) {
    // If the last frame was a draw frame, commit the drawings
    if (window.OnDrawFrame()) {
        window.Commit();
    }

    // Attempt to acquire the next drawing frame
    window.AcquireDrawFrame();
}

void ResetDeltaTimer(StopwatchService& stopwatch) {
    // Reset the physics timer after 1000 ticks
    if (stopwatch.Milliseconds() >= 1000) {
        stopwatch.Reset();
    }
}
