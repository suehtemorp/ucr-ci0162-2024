#pragma once

// Core definition for component
#include "ECS/ECS_Core.hpp"

// GLM vector math
#include <glm/vec2.hpp>

// SDL texture type
#include <SDL.h>

// Fixed-size trivial array
#include <array>

/// @brief Size, image and display text
class Drawing : public Component {
    public:
        /// @brief Dimensions of given drawing
        glm::uvec2 imageSize;
        /// @brief Dimensions of given display text
        glm::uvec2 textSize;
        /// @brief Non-owning reference to image texture
        SDL_Texture* imageRef;
        /// @brief Non-owning reference to display text texture
        SDL_Texture* textRef;
};

static_assert(
    ComponentType<Drawing>, 
    "Drawing component constraint violated"
);
