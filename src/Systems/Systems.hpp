#pragma once

// Window rendering service
#include "Services/WindowService.hpp"

// Timekeeping stopwatch service
#include "Services/StopwatchService.hpp"

// Drawing component
#include "Components/Drawing.hpp"

// Physics component
#include "Components/Physics.hpp"

// Parameter packing in tuple
#include <tuple>

void PhysicsSystem(
    std::tuple<Physics&> components, 
    std::tuple<const WindowService&, const StopwatchService&> services
);

void DrawingSystem(
    std::tuple<const Drawing&, const Physics&> components, 
    std::tuple<WindowService&> services
);
