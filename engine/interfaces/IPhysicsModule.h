#pragma once

#include "IEngineModule.h"
#include "../../include/Physics/PhysicsEngine.h"

namespace GameEngine {
    namespace Physics {
        enum class PhysicsAPI {
            Bullet,
            PhysX
        };

        struct PhysicsSettings {
            PhysicsAPI api = PhysicsAPI::Bullet;
            PhysicsConfiguration configuration = PhysicsConfiguration::Default();
            bool enableDebugDrawing = false;
            bool enableCCD = true;
            int maxRigidBodies = 10000;
            int maxGhostObjects = 1000;
        };

        enum class PhysicsFeature {
            RigidBodies,
            SoftBodies,
            Fluids,
            Cloth,
            Vehicles,
            CharacterController,
            Constraints,
            Triggers
        };

        class IPhysicsModule : public IEngineModule {
        public:
            virtual ~IPhysicsModule() = default;
            
            // Physics-specific methods
            virtual PhysicsEngine* GetPhysicsEngine() = 0;
            virtual bool SupportsAPI(PhysicsAPI api) = 0;
            virtual bool SupportsFeature(PhysicsFeature feature) = 0;
            virtual void SetPhysicsSettings(const PhysicsSettings& settings) = 0;
            virtual PhysicsSettings GetPhysicsSettings() const = 0;
            
            // World management
            virtual std::shared_ptr<PhysicsWorld> CreateWorld(const PhysicsConfiguration& config) = 0;
            virtual void SetActiveWorld(std::shared_ptr<PhysicsWorld> world) = 0;
            virtual std::shared_ptr<PhysicsWorld> GetActiveWorld() = 0;
            
            // Debug and diagnostics
            virtual void EnableDebugDrawing(bool enabled) = 0;
            virtual bool IsDebugDrawingEnabled() const = 0;
            virtual PhysicsEngine::PhysicsDebugInfo GetDebugInfo() const = 0;
        };
    }
}