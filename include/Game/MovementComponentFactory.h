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
            Physics,        ///< Full physics simulation
            Deterministic,  ///< Precise character control
            Hybrid         ///< Physics collision with direct control
        };

        static std::unique_ptr<CharacterMovementComponent> CreateComponent(ComponentType type);
        static const char* GetComponentTypeName(ComponentType type);
    };
}