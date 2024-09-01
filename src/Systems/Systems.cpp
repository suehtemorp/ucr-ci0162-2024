// System's base definition
#include "Systems/Systems.hpp"

// Auxilary rotation math
#include <cmath>

// For math constants, like PI
#include <numbers>

// DEBUG
#include <iostream>

void PhysicsSystem(
    std::tuple<Physics&> components, 
    std::tuple<const WindowService&, const StopwatchService&> services
) {
    // Capture component and services
    Physics& physicsComponent = std::get<0>(components);
    const WindowService& windowService = std::get<0>(services);
    const StopwatchService& stopwatch = std::get<1>(services);

    
    // For sanity, lets constraint the angle on edge cases
    double& angle = physicsComponent.angle;
    if (angle < 0 || angle > 360.0) {
        angle = std::fmod(angle, 360.0);
        if (angle < 0) { angle = + 360.0; }
    }

    // Get corners based on size...
    const glm::dvec2 size = physicsComponent.size;

    glm::dvec2 corners[4];
    corners[0] = size / glm::dvec2{-2, -2};
    corners[1] = size / glm::dvec2{2, -2};
    corners[2] = size / glm::dvec2{-2, 2};
    corners[3] = size / glm::dvec2{2, 2};

    // Rotation...
    const double radians = (angle * std::numbers::pi) / 180.0;

    for (glm::dvec2& corner : corners) {
        corner = {
            corner.x * std::cos(radians) - corner.y * std::sin(radians),
            corner.x * std::sin(radians) + corner.y * std::cos(radians)
        };
    }

    // And position
    glm::dvec2& position = physicsComponent.position;

    for (glm::dvec2& corner : corners) {
        corner += position;
    }

    // Check if it collides on a window border, and figure
    // out the offset required
    const glm::uvec2& windowSize = windowService.Size();

    bool collidesX = false, collidesY = false;
    glm::dvec2 collisionOffset {0,0};
    for (const glm::dvec2& corner : corners) {
        if (corner.x < 0 || corner.x > windowSize.x) {
            // Calculate X offset
            const double cornerOffset = corner.x < 0 ? 
                -corner.x + 5 : (windowSize.x - corner.x) - 5;

            // Keep greatest offset
            if (std::abs(cornerOffset) > 
                std::abs(collisionOffset.x)) {
                collisionOffset.x = cornerOffset;
            }

            // Make note of collision
            collidesX = true;
        }

        if (corner.y < 0 || corner.y > windowSize.y) {
            // Calculate Y offset
            const double cornerOffset = corner.y < 0 ? 
                -corner.y + 5 : (windowSize.y - corner.y) - 5;

            // Keep greatest offset
            if (std::abs(cornerOffset) > 
                std::abs(collisionOffset.y)) {
                collisionOffset.y = cornerOffset;
            }

            // Make note of collision
            collidesY = true;
        }
    }

    // Flip the velocty and angle if it collides
    glm::dvec2& velocity = physicsComponent.velocity;
    
    if (collidesX) { velocity.x *= -1; }
    if (collidesY) { velocity.y *= -1; }

    // Update position according to speed and offset
    // We'll scale by delta time elapsed since the last sweep
    const double delta = stopwatch.Milliseconds() / 4.5E6;
    position += velocity * glm::dvec2{delta, delta} + collisionOffset;
}

void DrawingSystem(
    std::tuple<const Drawing&, const Physics&> components, 
    std::tuple<WindowService&> services
) {
    // Capture component and services
    const Drawing& drawingComponent = std::get<0>(components);
    const Physics& physicsComponent = std::get<1>(components);
    WindowService& windowService = std::get<0>(services);

    // Only consider drawing if the drawing service recommends we do
    // for the current frame (saves us some slack)
    if (!windowService.OnDrawFrame()) {
        return;
    }

    // Compute the rectangle on which to draw the entity's image...
    const glm::uvec2& imageSize = drawingComponent.imageSize;
    const glm::uvec2& textSize = drawingComponent.textSize;
    const glm::dvec2& position = physicsComponent.position;

    const SDL_Rect imageBounds{
        .x = (int) (position.x - (imageSize.x / 2.0)),
        .y = (int) (position.y - (imageSize.y / 2.0)),
        .w = (int) imageSize.x, 
        .h = (int) imageSize.y
    };

    // And display text
    const SDL_Rect textBounds{
        // Offset the horizontal position to match the image's center
        // This keep the text nice and cented
        .x = (int) (imageBounds.x + ((imageSize.x - textSize.x) / 2.0)),
        // Offset the vertical position by the radius of the image 
        // This aids in avoiding overlap regardless of rotation
        .y = (int) (position.y + (
                std::sqrt(std::pow(imageSize.x, 2) + std::pow(imageSize.y, 2))
            ) / 2.0),
        .w = (int) textSize.x,
        .h = (int) textSize.y
    };

    // Queue the drawing and text for rendering
    const double& angle = physicsComponent.angle;
    SDL_Texture* image = drawingComponent.imageRef;
    SDL_Texture* text = drawingComponent.textRef;

    windowService.PushTexture(image, imageBounds, angle);
    windowService.PushTexture(text, textBounds, 0);
}
