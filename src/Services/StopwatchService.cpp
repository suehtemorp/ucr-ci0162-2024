#include "Services/StopwatchService.hpp"
#include "StopwatchService.hpp"

StopwatchService::StopwatchService():
_msOnReset(SDL_GetTicks64())
{}

StopwatchService::StopwatchService(StopwatchService &&other) :
_msOnReset(other._msOnReset), _active(other._active) 
{}

StopwatchService::~StopwatchService()
{}

StopwatchService &StopwatchService::operator=(StopwatchService &&other) {
    _msOnReset = other._msOnReset;
    _active = other._active;
    return *this;
}

Uint64 StopwatchService::Seconds() const {
    if (!_active) {return 0;}

    // Whole division rounds down, which seems proper
    return (SDL_GetTicks64() - _msOnReset) / 1000;
}

Uint64 StopwatchService::Milliseconds() const {
    if (!_active) {return 0;}
    return SDL_GetTicks64() - _msOnReset;
}

Uint64 StopwatchService::Reset() {
    if (!_active) { return 0; }
    return SDL_GetTicks64() - (_msOnReset = SDL_GetTicks64());
}

bool StopwatchService::Toggle() {
    _active = !_active;
    if (_active) {_msOnReset = SDL_GetTicks64(); }

    return _active;
}
