#include "Game/MovementComponentFactory.h"
#include "Game/PhysicsMovementComponent.h"
#include "Game/DeterministicMovementComponent.h"  // Used as basic CharacterMovement
#include "Game/HybridMovementComponent.h"
#include "Core/Logger.h"

namespace GameEngine {
    std::unique_ptr<CharacterMovementComponent> MovementComponentFactory::CreateComponent(ComponentType type) {
        switch (type) {
            case ComponentType::CharacterMovement:
                LOG_DEBUG("Creating DeterministicMovementComponent (basic character movement)");
                return std::make_unique<DeterministicMovementComponent>();
                
            case ComponentType::Physics:
                LOG_DEBUG("Creating PhysicsMovementComponent");
                return std::make_unique<PhysicsMovementComponent>();
                
            case ComponentType::Hybrid:
                LOG_DEBUG("Creating HybridMovementComponent");
                return std::make_unique<HybridMovementComponent>();
                
            default:
                LOG_ERROR("Unknown movement component type requested");
                return nullptr;
        }
    }

    const char* MovementComponentFactory::GetComponentTypeName(ComponentType type) {
        switch (type) {
            case ComponentType::CharacterMovement:
                return "CharacterMovementComponent";
            case ComponentType::Physics:
                return "PhysicsMovementComponent";
            case ComponentType::Hybrid:
                return "HybridMovementComponent";
            default:
                return "UnknownMovementComponent";
        }
    }
}