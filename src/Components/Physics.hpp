#pragma once

// Core definition for component
#include "ECS/ECS_Core.hpp"

// GLM vector math
#include <glm/vec2.hpp>

/// @brief Position, velocity and angle of a given thing
class Physics : public Component {
    public:
        /// @brief Current velocity vector 
        glm::dvec2 velocity;
        /// @brief Current position coordinates
        glm::dvec2 position;
        /// @brief Size of colliding box
        glm::uvec2 size;
        /// @brief Angle of orientation (degrees) (regardless of speed)
        double angle;
};

static_assert(
    ComponentType<Physics>, 
    "Physics component constraint violated"
);
