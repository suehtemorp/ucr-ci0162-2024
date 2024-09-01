#include "Services/WindowService.hpp"

// Error throwing
#include <stdexcept>

// Error output
#include <SDL_log.h>
#include <stdio.h>
#include "WindowService.hpp"

WindowService::WindowService(
    unsigned width, unsigned height, unsigned framerate, 
    SDL_Color bgColor, const char *name): 
_size(width, height) {
    // Create window with given size on the middle of the screen
    _window.reset(
        SDL_CreateWindow(
        name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        width, height, 
        SDL_WINDOW_SHOWN
    ));

    if (_window == nullptr) {
        SDL_Log("SDL_CreateWindow error: %s\n", SDL_GetError());
        throw std::runtime_error("Unable to create window");
    }

    // Create a renderer for said window
    _renderer.reset(
        SDL_CreateRenderer(_window.get(), -1, SDL_RENDERER_ACCELERATED)
    );

    if (_renderer == nullptr) {
        SDL_Log("SDL_CreateRenderer error: %s\n", SDL_GetError());
        throw std::runtime_error("Unable to create window renderer");
    }

    // Assign the selected background color to it
    if (
        SDL_SetRenderDrawColor(_renderer.get(), 
        bgColor.r, bgColor.g, bgColor.b, bgColor.a) != 0
    ) {
        SDL_Log("SDL_SetRenderDrawColor error: %s\n", SDL_GetError());
    }

    // Clear screen anticipating next drawing calls
    SDL_RenderClear(_renderer.get());

    // Compute the milliseconds between frames
    _msPerFrame = 1000.0 / framerate;
}

WindowService::WindowService(WindowService &&other):
    _size(other._size), _sinceCommit(other._sinceCommit),
    _msPerFrame(other._msPerFrame), _texturesPushed(other._texturesPushed),
    _onDrawFrame(other._onDrawFrame)
{
    // Steal the other window service's window and renderer
    _window = std::move(other._window);
    _renderer = std::move(other._renderer);
}

WindowService::~WindowService()
{}

WindowService& WindowService::operator=(WindowService &&other) {
    // Steal the other window service's window and renderer
    _window = std::move(other._window);
    _renderer = std::move(other._renderer);

    // Keep track of its other statistics
    _size = other._size;
    _sinceCommit = other._sinceCommit;
    _msPerFrame = other._msPerFrame;
    _texturesPushed = other._texturesPushed;
    _onDrawFrame = other._onDrawFrame;

    return *this;
}

const glm::uvec2 &WindowService::Size() const
{ return _size; }

bool WindowService::AcquireDrawFrame()
{
    if ((SDL_GetTicks64() - _sinceCommit) >= _msPerFrame) {
        _onDrawFrame = true;
    }

    return _onDrawFrame;
}

bool WindowService::OnDrawFrame() const 
{ return _onDrawFrame; }

size_t WindowService::Pushed() const
{ return _texturesPushed; }

bool WindowService::PushTexture(SDL_Texture *texture, 
    const SDL_Rect &rect, double angle) {
    // Enforce being on a draw frame to call
    if (!_onDrawFrame) {
        fprintf(
            stderr, 
            "WindowService: PushTexture called but not on draw frame\n"
        );

        return false;
    }

    // Attempt to queue drawing the texture rect and
    // report any errors
    if (SDL_RenderCopyEx(_renderer.get(), texture, NULL, &rect, 
        angle, NULL, SDL_FLIP_NONE) != 0) {
        SDL_Log("SDL_RenderCopy error: %s\n", SDL_GetError());
        return false;
    }

    // Otherwise, update pushed statistic
    _texturesPushed += 1;
    return true;
}

void WindowService::Commit() {
    // Enforce being on a draw frame to call
    if (!_onDrawFrame) {
        fprintf(
            stderr, 
            "WindowService: Commit called but not on draw frame\n"
        );

        return;
    }

    // Apply back-buffer to main buffer
    SDL_RenderPresent(_renderer.get());

    // Clear screen anticipating next drawing calls
    SDL_RenderClear(_renderer.get());

    // Update commit timestamp and pushed statistic
    _sinceCommit = SDL_GetTicks64();
    _texturesPushed = 0;

    // Release draw frame permit
    _onDrawFrame = false;
}
