#pragma once

#include "Game/CharacterMovementComponent.h"
#include <memory>

namespace GameEngine {
    /**
     * @brief Factory for creating movement components
     */
    class MovementComponentFactory {
    public:
        enum class ComponentType {
            CharacterMovement,  ///< Basic movement with manual physics
            Physics,           ///< Full physics simulation
            Hybrid            ///< Physics collision with direct control
        };

        static std::unique_ptr<CharacterMovementComponent> CreateComponent(ComponentType type);
        static const char* GetComponentTypeName(ComponentType type);
    };
}