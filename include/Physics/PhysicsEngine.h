#pragma once

#include "Core/Math.h"
#include <vector>
#include <memory>
#include <unordered_map>

#ifdef GAMEENGINE_HAS_BULLET
#include <btBulletDynamicsCommon.h>
#endif

namespace GameEngine {
    struct RigidBody {
        Math::Vec3 position{0.0f};
        Math::Quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
        Math::Vec3 velocity{0.0f};
        Math::Vec3 angularVelocity{0.0f};
        float mass = 1.0f;
        float restitution = 0.5f;
        float friction = 0.5f;
        bool isStatic = false;
        bool isKinematic = false;
    };

    struct CollisionShape {
        enum Type {
            Box,
            Sphere,
            Capsule,
            Mesh
        } type = Box;
        
        Math::Vec3 dimensions{1.0f}; // For box: width, height, depth; For sphere: radius, 0, 0
    };

    class PhysicsWorld;

    class PhysicsEngine {
    public:
        PhysicsEngine();
        ~PhysicsEngine();

        bool Initialize();
        void Shutdown();
        void Update(float deltaTime);

        // World management
        std::shared_ptr<PhysicsWorld> CreateWorld(const Math::Vec3& gravity = Math::Vec3(0.0f, -9.81f, 0.0f));
        void SetActiveWorld(std::shared_ptr<PhysicsWorld> world);

        // Rigid body management
        uint32_t CreateRigidBody(const RigidBody& bodyDesc, const CollisionShape& shape);
        void DestroyRigidBody(uint32_t bodyId);
        void SetRigidBodyTransform(uint32_t bodyId, const Math::Vec3& position, const Math::Quat& rotation);
        void ApplyForce(uint32_t bodyId, const Math::Vec3& force);
        void ApplyImpulse(uint32_t bodyId, const Math::Vec3& impulse);

        // Queries
        bool Raycast(const Math::Vec3& origin, const Math::Vec3& direction, float maxDistance, uint32_t& hitBodyId);
        std::vector<uint32_t> OverlapSphere(const Math::Vec3& center, float radius);

    private:
        std::shared_ptr<PhysicsWorld> m_activeWorld;
        std::unordered_map<uint32_t, std::unique_ptr<RigidBody>> m_rigidBodies;
        uint32_t m_nextBodyId = 1;
        
#ifdef GAMEENGINE_HAS_BULLET
        // Mapping from body ID to Bullet rigid body for direct access
        std::unordered_map<uint32_t, btRigidBody*> m_bulletBodies;
#endif
    };

    class PhysicsWorld {
    public:
        PhysicsWorld(const Math::Vec3& gravity);
        virtual ~PhysicsWorld();

        virtual void SetGravity(const Math::Vec3& gravity) { m_gravity = gravity; }
        virtual const Math::Vec3& GetGravity() const { return m_gravity; }

        virtual void Step(float deltaTime);

    private:
        Math::Vec3 m_gravity;
    };
}