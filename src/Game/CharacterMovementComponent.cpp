#include "Game/CharacterMovementComponent.h"
#include "Game/PhysicsMovementComponent.h"
#include "Game/DeterministicMovementComponent.h"
#include "Game/HybridMovementComponent.h"
#include "Core/Logger.h"
#include <algorithm>

namespace GameEngine {
    CharacterMovementComponent::CharacterMovementComponent() {
    }

    CharacterMovementComponent::~CharacterMovementComponent() {
    }

    Math::Vec3 CharacterMovementComponent::ConstrainInputVector(const Math::Vec3& inputVector) const {
        Math::Vec3 result = inputVector;
        
        // Clamp input magnitude to 1.0
        float magnitude = glm::length(result);
        if (magnitude > 1.0f) {
            result = result / magnitude;
        }
        
        return result;
    }

    Math::Vec3 CharacterMovementComponent::ScaleInputAcceleration(const Math::Vec3& inputAcceleration) const {
        // Scale input acceleration based on current movement mode and configuration
        float scale = 1.0f;
        
        switch (m_movementMode) {
            case MovementMode::Walking:
                scale = 1.0f;
                break;
            case MovementMode::Falling:
                scale = m_config.airControl;
                break;
            case MovementMode::Flying:
                scale = 1.0f;
                break;
            default:
                scale = 1.0f;
                break;
        }
        
        return inputAcceleration * scale;
    }


}