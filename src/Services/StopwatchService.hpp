#pragma once

// SDL_GetTicks64

// SDL-provided datatype for timer ticks
#include <SDL_timer.h>

// Core definition for service
#include "ECS/ECS_Core.hpp"

/// @brief Timekeeping service 
class StopwatchService : public Service {
    private:
        /// @brief Milliseconds elapsed from SDL initialization to
        /// the latest start / reset of the stopwatch
        Uint64 _msOnReset = 0;

        /// @brief Whether the current timekeeping service is toggled
        /// on or not
        bool _active = true;

    public:
        /// @brief Create and start a stopwatch
        StopwatchService();

        /// @brief Construct a stopwatch by copying the resources of another
        /// @param other Stopwatch to copy information from
        StopwatchService(StopwatchService&& other);

        /// @brief Destroy the current stopwatch
        ~StopwatchService();

        /// @brief Copy the stopwatch information from another one
        /// @param other Stopwatch to copy information from
        /// @return Reference to this stopwatch
        StopwatchService& operator=(StopwatchService&& other);

        /// @brief Get the amount of seconds elapsed on the timer 
        /// @return Seconds that have elapsed from the latest set / reset
        Uint64 Seconds() const;

        /// @brief Get the amount of milliseconds elapsed on the timer
        /// @return Milliseconds that have elapsed from the latest set / reset
        Uint64 Milliseconds() const;

        /// @brief Reset the timer
        /// @return Millseconds elapsed from previous set / reset
        Uint64 Reset();

        /// @brief Toggle and then reset the timer. Future calls to retrieve
        /// the elapsed time yield zero until it is toggled back on
        /// @return Whether the timer is on or off after the toggle call
        bool Toggle();
};

static_assert(
    ServiceType<StopwatchService>, 
    "StopwatchService service constraint violated"
);
