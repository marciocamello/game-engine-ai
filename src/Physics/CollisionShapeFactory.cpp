#ifdef GAMEENGINE_HAS_BULLET

#include "Physics/CollisionShapeFactory.h"
#include "Physics/BulletUtils.h"
#include <iostream>
#include <algorithm>

namespace GameEngine {
    namespace Physics {
        
        std::unique_ptr<btCollisionShape> CollisionShapeFactory::CreateShape(const CollisionShape& desc) {
            if (!ValidateShapeParameters(desc)) {
                std::cerr << "CollisionShapeFactory: Invalid shape parameters provided" << std::endl;
                return nullptr;
            }

            switch (desc.type) {
                case CollisionShape::Box:
                    return CreateBoxShape(desc.dimensions);
                
                case CollisionShape::Sphere:
                    return CreateSphereShape(desc.dimensions.x); // radius stored in x component
                
                case CollisionShape::Capsule:
                    return CreateCapsuleShape(desc.dimensions.x, desc.dimensions.y); // radius in x, height in y
                
                case CollisionShape::Mesh:
                    std::cerr << "CollisionShapeFactory: Mesh collision shapes not yet implemented" << std::endl;
                    return nullptr;
                
                default:
                    std::cerr << "CollisionShapeFactory: Unknown collision shape type: " << desc.type << std::endl;
                    return nullptr;
            }
        }

        std::unique_ptr<btBoxShape> CollisionShapeFactory::CreateBoxShape(const Math::Vec3& dimensions) {
            // Bullet expects half-extents, so divide by 2
            btVector3 halfExtents = BulletUtils::ToBullet(dimensions * 0.5f);
            return std::make_unique<btBoxShape>(halfExtents);
        }

        std::unique_ptr<btSphereShape> CollisionShapeFactory::CreateSphereShape(float radius) {
            return std::make_unique<btSphereShape>(radius);
        }

        std::unique_ptr<btCapsuleShape> CollisionShapeFactory::CreateCapsuleShape(float radius, float height) {
            // btCapsuleShape constructor takes radius and height (total height including hemispheres)
            return std::make_unique<btCapsuleShape>(radius, height);
        }

        bool CollisionShapeFactory::ValidateShapeParameters(const CollisionShape& desc) {
            switch (desc.type) {
                case CollisionShape::Box:
                    // Box dimensions must be positive
                    return desc.dimensions.x > 0.0f && desc.dimensions.y > 0.0f && desc.dimensions.z > 0.0f;
                
                case CollisionShape::Sphere:
                    // Sphere radius must be positive
                    return desc.dimensions.x > 0.0f;
                
                case CollisionShape::Capsule:
                    // Capsule radius and height must be positive
                    return desc.dimensions.x > 0.0f && desc.dimensions.y > 0.0f;
                
                case CollisionShape::Mesh:
                    // Mesh validation would require checking mesh data
                    // For now, return false since mesh shapes aren't implemented
                    return false;
                
                default:
                    return false;
            }
        }
    }
}

#endif // GAMEENGINE_HAS_BULLET