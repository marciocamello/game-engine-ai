#pragma once

#include "../interfaces/IPhysicsModule.h"
#include "../../include/Physics/PhysicsEngine.h"
#include <memory>

namespace GameEngine {
    namespace Physics {
        class BulletPhysicsModule : public IPhysicsModule {
        public:
            BulletPhysicsModule();
            virtual ~BulletPhysicsModule();

            // IEngineModule interface
            bool Initialize(const ModuleConfig& config) override;
            void Update(float deltaTime) override;
            void Shutdown() override;
            
            const char* GetName() const override { return "BulletPhysics"; }
            const char* GetVersion() const override { return "1.0.0"; }
            ModuleType GetType() const override { return ModuleType::Physics; }
            std::vector<std::string> GetDependencies() const override;
            
            bool IsInitialized() const override { return m_initialized; }
            bool IsEnabled() const override { return m_enabled; }
            void SetEnabled(bool enabled) override { m_enabled = enabled; }

            // IPhysicsModule interface
            PhysicsEngine* GetPhysicsEngine() override;
            bool SupportsAPI(PhysicsAPI api) override;
            bool SupportsFeature(PhysicsFeature feature) override;
            void SetPhysicsSettings(const PhysicsSettings& settings) override;
            PhysicsSettings GetPhysicsSettings() const override;
            
            // World management
            std::shared_ptr<PhysicsWorld> CreateWorld(const PhysicsConfiguration& config) override;
            void SetActiveWorld(std::shared_ptr<PhysicsWorld> world) override;
            std::shared_ptr<PhysicsWorld> GetActiveWorld() override;
            
            // Debug and diagnostics
            void EnableDebugDrawing(bool enabled) override;
            bool IsDebugDrawingEnabled() const override;
            PhysicsEngine::PhysicsDebugInfo GetDebugInfo() const override;

        private:
            bool InitializePhysicsEngine();
            void ShutdownPhysicsEngine();
            void ApplyConfiguration();
            
            std::unique_ptr<PhysicsEngine> m_physicsEngine;
            PhysicsSettings m_physicsSettings;
            bool m_initialized = false;
            bool m_enabled = true;
        };
    }
}