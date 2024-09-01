#pragma once

// Core definition for service
#include "ECS/ECS_Core.hpp"

// GLM vector math
#include <glm/vec2.hpp>

// SDL window
#include <SDL_video.h>

// SDL renderer
#include <SDL_render.h>

// SDL color
#include <SDL_pixels.h>

// Memory safe container for resources
#include "Utils/MemoryAliases.hpp"

// Provide friendly access for asset store
#include "Services/AssetStore.hpp" 

// Forward declaration required (circular dependency)
class AssetStore;

/// @brief Window lifetime & drawing service 
class WindowService : public Service {
    friend class AssetStore;
    private:
        /// @brief Window used across drawings
        Memory::unique_ptr_with_deleter<SDL_Window, SDL_DestroyWindow> 
        _window = nullptr;
        /// @brief Window renderer used across drawings
        Memory::unique_ptr_with_deleter<SDL_Renderer, SDL_DestroyRenderer>
        _renderer = nullptr;
        /// @brief Window width and height
        glm::uvec2 _size{0,0};
        /// @brief If acquired a draw frame before a commit call 
        bool _onDrawFrame = false;
        /// @brief Millisecond timestamp for last draw commit call
        Uint64 _sinceCommit = 0;
        /// @brief Milliseconds between frame (for framerate)
        Uint64 _msPerFrame;
        /// @brief Textures pushed before latest commit call
        size_t _texturesPushed = 0;

    public:
        /// @brief Construct a window
        /// @param width Width (pixels)
        /// @param height Height (pixels)
        /// @param framerate Frames per second between rendering
        /// @param bgColor Color (RGBA)
        /// @param name Title name of the window (cstring)
        WindowService(
            unsigned width, unsigned height,
            unsigned framerate, 
            SDL_Color bgColor = SDL_Color{0, 0, 0}, 
            const char* name = "New window"
        );

        /// @brief Construct a window by stealing the resources of another
        /// @param other Window to take underlying resources from
        WindowService(WindowService&& other);

        /// @brief Destroy the current window and its associated resources
        ~WindowService();

        /// @brief Steal then own the resources of another window
        /// @param other Window to take underlying resources from
        /// @return Reference to this window
        WindowService& operator=(WindowService&& other);

        /// @brief Get the current dimensions of the window
        /// @return 2D vector of width and height
        const glm::uvec2& Size() const;

        /// @brief Attempt to acquire the current frame as a draw frame
        /// @return True if conditions hold and a draw frame was acquired,
        /// false otherwise
        bool AcquireDrawFrame();

        /// @brief Whether this window service is accepting draw calls
        /// on the current frame (like PushTexture)
        /// @return True is yes, false otherwise
        bool OnDrawFrame() const;

        /// @brief Get the amount of textures pushed since the latest
        /// commit call
        size_t Pushed() const;

        /// @brief Queue a texture rect drawing to the underlying window
        /// @param texture Texture to draw from
        /// @param rect Destination rect coordinates
        /// @param angle Angle (in degrees) to rotate texture at
        /// @return True if drawn succesfully, false otherwise
        bool PushTexture(SDL_Texture* texture, const SDL_Rect& rect, double angle);

        /// @brief Apply the queue drawings to the current view
        /// @param stopwatch Stopwatch service to check timer from
        /// @return True if applied succesfully, false otherwise
        void Commit();
};

static_assert(
    ServiceType<WindowService>, 
    "WindowService service constraint violated"
);
