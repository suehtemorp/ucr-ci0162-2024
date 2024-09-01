#pragma once

// Components
#include "Components/Drawing.hpp"
#include "Components/Physics.hpp"

// Systems
#include "Systems/Systems.hpp"

// Services
#include "Services/WindowService.hpp"
#include "Services/AssetStore.hpp"
#include "Services/StopwatchService.hpp"

// Entity-component systems' manager
#include "ECS/ECS.hpp"

// Alias for ECS to use
using GameECS = ECS<
    Physics,
    Drawing
>::WithServices<
    StopwatchService,
    AssetStore,
    WindowService
>;
