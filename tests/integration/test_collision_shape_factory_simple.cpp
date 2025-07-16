#ifdef GAMEENGINE_HAS_BULLET

#include "Physics/CollisionShapeFactory.h"
#include "Physics/PhysicsEngine.h"
#include <iostream>
#include <btBulletDynamicsCommon.h>

using namespace GameEngine;
using namespace GameEngine::Physics;

int main() {
    std::cout << "Testing CollisionShapeFactory..." << std::endl;

    // Test Box Shape
    {
        CollisionShape boxDesc;
        boxDesc.type = CollisionShape::Box;
        boxDesc.dimensions = Math::Vec3(2.0f, 4.0f, 6.0f);

        auto boxShape = CollisionShapeFactory::CreateShape(boxDesc);
        if (boxShape && boxShape->getShapeType() == BOX_SHAPE_PROXYTYPE) {
            std::cout << "✓ Box shape creation successful" << std::endl;
        } else {
            std::cout << "✗ Box shape creation failed" << std::endl;
            return 1;
        }
    }

    // Test Sphere Shape
    {
        CollisionShape sphereDesc;
        sphereDesc.type = CollisionShape::Sphere;
        sphereDesc.dimensions = Math::Vec3(2.5f, 0.0f, 0.0f);

        auto sphereShape = CollisionShapeFactory::CreateShape(sphereDesc);
        if (sphereShape && sphereShape->getShapeType() == SPHERE_SHAPE_PROXYTYPE) {
            std::cout << "✓ Sphere shape creation successful" << std::endl;
        } else {
            std::cout << "✗ Sphere shape creation failed" << std::endl;
            return 1;
        }
    }

    // Test Capsule Shape
    {
        CollisionShape capsuleDesc;
        capsuleDesc.type = CollisionShape::Capsule;
        capsuleDesc.dimensions = Math::Vec3(1.0f, 3.0f, 0.0f);

        auto capsuleShape = CollisionShapeFactory::CreateShape(capsuleDesc);
        if (capsuleShape && capsuleShape->getShapeType() == CAPSULE_SHAPE_PROXYTYPE) {
            std::cout << "✓ Capsule shape creation successful" << std::endl;
        } else {
            std::cout << "✗ Capsule shape creation failed" << std::endl;
            return 1;
        }
    }

    // Test Invalid Shape (zero dimensions)
    {
        CollisionShape invalidDesc;
        invalidDesc.type = CollisionShape::Box;
        invalidDesc.dimensions = Math::Vec3(0.0f, 1.0f, 1.0f);

        auto invalidShape = CollisionShapeFactory::CreateShape(invalidDesc);
        if (invalidShape == nullptr) {
            std::cout << "✓ Invalid shape correctly rejected" << std::endl;
        } else {
            std::cout << "✗ Invalid shape should have been rejected" << std::endl;
            return 1;
        }
    }

    // Test Mesh Shape (not implemented)
    {
        CollisionShape meshDesc;
        meshDesc.type = CollisionShape::Mesh;
        meshDesc.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);

        auto meshShape = CollisionShapeFactory::CreateShape(meshDesc);
        if (meshShape == nullptr) {
            std::cout << "✓ Mesh shape correctly returns null (not implemented)" << std::endl;
        } else {
            std::cout << "✗ Mesh shape should return null (not implemented)" << std::endl;
            return 1;
        }
    }

    std::cout << "All CollisionShapeFactory tests passed!" << std::endl;
    return 0;
}

#else

int main() {
    std::cout << "Bullet Physics not available, skipping CollisionShapeFactory test" << std::endl;
    return 0;
}

#endif // GAMEENGINE_HAS_BULLET