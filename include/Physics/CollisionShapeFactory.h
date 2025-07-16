#pragma once

#ifdef GAMEENGINE_HAS_BULLET

#include "PhysicsEngine.h"
#include <btBulletDynamicsCommon.h>
#include <memory>

namespace GameEngine {
    namespace Physics {
        class CollisionShapeFactory {
        public:
            // Main factory method to create Bullet collision shapes from engine CollisionShape
            static std::unique_ptr<btCollisionShape> CreateShape(const CollisionShape& desc);

            // Specific shape creation methods (public as per task requirements)
            static std::unique_ptr<btBoxShape> CreateBoxShape(const Math::Vec3& dimensions);
            static std::unique_ptr<btSphereShape> CreateSphereShape(float radius);
            static std::unique_ptr<btCapsuleShape> CreateCapsuleShape(float radius, float height);

        private:
            // Helper method to validate shape parameters
            static bool ValidateShapeParameters(const CollisionShape& desc);
        };
    }
}

#endif // GAMEENGINE_HAS_BULLET