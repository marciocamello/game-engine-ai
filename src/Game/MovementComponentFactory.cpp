#include "Game/MovementComponentFactory.h"
#include "Game/PhysicsMovementComponent.h"
#include "Game/DeterministicMovementComponent.h"
#include "Game/HybridMovementComponent.h"
#include "Core/Logger.h"

namespace GameEngine {
    std::unique_ptr<CharacterMovementComponent> MovementComponentFactory::CreateComponent(ComponentType type) {
        switch (type) {
            case ComponentType::Physics:
                LOG_DEBUG("Creating PhysicsMovementComponent");
                return std::make_unique<PhysicsMovementComponent>();
                
            case ComponentType::Deterministic:
                LOG_DEBUG("Creating DeterministicMovementComponent");
                return std::make_unique<DeterministicMovementComponent>();
                
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
            case ComponentType::Physics:
                return "PhysicsMovementComponent";
            case ComponentType::Deterministic:
                return "DeterministicMovementComponent";
            case ComponentType::Hybrid:
                return "HybridMovementComponent";
            default:
                return "UnknownMovementComponent";
        }
    }
}