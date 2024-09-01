#pragma once

// Type constraints
#include <concepts>
#include <type_traits>
#include "Utils/Traits.hpp"

/// @brief Base type for POD components in an ECS
/// @remark Merely added for disambiguation in templates
/// with respect to other types
class Component {};

/// @brief Base type for service dependencies in an ECS
/// @remark Merely added for disambiguation in templates
/// with respect to other types
class Service {};

/// @brief Restraint required for PODs component in an ECS
template <class T>
concept ComponentType =
    // Enforce POD restraints
    std::is_standard_layout_v<T> &&
    std::is_trivial_v<T> &&
    // Require inheritance from base type (for disambiguation inside templates)
    std::derived_from<T, Component> &&
    // Forbid intersection with services
    !std::derived_from<T, Service>;

/// @brief Restraint required for service dependencies in an ECS
template <class T>
concept ServiceType =
    // Enforce construction assignment restraints
    ((
        std::is_copy_constructible_v<T> &&
        std::is_copy_assignable_v<T>
        ) || (
        std::is_move_constructible_v<T> &&
        std::is_move_assignable_v<T>
    )) &&
    // Enforce destruction contraints
    std::is_destructible_v<T> &&
    // Require inheritance from base type (for disambiguation inside templates)
    std::derived_from<T, Service> &&
    // Forbid intersection with components
    !std::derived_from<T, Component>;
