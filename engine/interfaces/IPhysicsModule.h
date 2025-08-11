#pragma once

#include "IEngineModule.h"

namespace GameEngine {
    class PhysicsEngine;
    
    enum class PhysicsFeature {
        RigidBodies,
        SoftBodies,
        Fluids,
        Cloth,
        Particles,
        Constraints,
        Triggers
    };

    struct PhysicsSettings {
        float gravity = -9.81f;
        int maxRigidBodies = 1000;
        int solverIterations = 10;
        float timeStep = 1.0f / 60.0f;
        bool enableCCD = true;
        bool enableMultithreading = false;
    };

    namespace Physics {
        class IPhysicsModule : public IEngineModule {
        public:
            virtual PhysicsEngine* GetPhysicsEngine() = 0;
            virtual bool SupportsFeature(PhysicsFeature feature) = 0;
            virtual void SetPhysicsSettings(const PhysicsSettings& settings) = 0;
        };
    }
}