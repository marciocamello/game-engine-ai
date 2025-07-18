#include "Physics/PhysicsDebugDrawer.h"
#include "Core/Logger.h"

#ifdef GAMEENGINE_HAS_BULLET
#include "Physics/BulletUtils.h"
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>
#endif

namespace GameEngine {
    namespace Physics {
        
        // SimplePhysicsDebugDrawer implementation
        void SimplePhysicsDebugDrawer::DrawLine(const Math::Vec3& from, const Math::Vec3& to, const Math::Vec3& color) {
            m_lines.push_back({from, to, color});
        }
        
        void SimplePhysicsDebugDrawer::DrawSphere(const Math::Vec3& center, float radius, const Math::Vec3& color) {
            m_spheres.push_back({center, color, radius});
        }
        
        void SimplePhysicsDebugDrawer::DrawBox(const Math::Vec3& center, const Math::Vec3& halfExtents, 
                                             const Math::Quat& rotation, const Math::Vec3& color) {
            m_boxes.push_back({center, halfExtents, color, rotation});
        }
        
        void SimplePhysicsDebugDrawer::DrawCapsule(const Math::Vec3& center, float radius, float height, 
                                                 const Math::Quat& rotation, const Math::Vec3& color) {
            m_capsules.push_back({center, color, rotation, radius, height});
        }
        
        void SimplePhysicsDebugDrawer::DrawText(const Math::Vec3& position, const std::string& text, const Math::Vec3& color) {
            m_texts.push_back({position, color, text});
        }
        
        void SimplePhysicsDebugDrawer::DrawContactPoint(const Math::Vec3& point, const Math::Vec3& normal, 
                                                      float distance, const Math::Vec3& color) {
            m_contacts.push_back({point, normal, color, distance});
        }
        
        void SimplePhysicsDebugDrawer::Clear() {
            m_lines.clear();
            m_spheres.clear();
            m_boxes.clear();
            m_capsules.clear();
            m_texts.clear();
            m_contacts.clear();
        }
        
#ifdef GAMEENGINE_HAS_BULLET
        // BulletDebugDrawer implementation
        BulletDebugDrawer::BulletDebugDrawer(std::shared_ptr<IPhysicsDebugDrawer> drawer)
            : m_drawer(drawer) {
        }
        
        void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
            if (m_drawer) {
                m_drawer->DrawLine(BulletUtils::FromBullet(from), BulletUtils::FromBullet(to), BulletUtils::FromBullet(color));
            }
        }
        
        void BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, 
                                               btScalar distance, int lifeTime, const btVector3& color) {
            if (m_drawer) {
                m_drawer->DrawContactPoint(BulletUtils::FromBullet(PointOnB), BulletUtils::FromBullet(normalOnB), 
                                         static_cast<float>(distance), BulletUtils::FromBullet(color));
            }
        }
        
        void BulletDebugDrawer::reportErrorWarning(const char* warningString) {
            LOG_WARNING("Bullet Physics Warning: " + std::string(warningString));
        }
        
        void BulletDebugDrawer::draw3dText(const btVector3& location, const char* textString) {
            if (m_drawer) {
                m_drawer->DrawText(BulletUtils::FromBullet(location), std::string(textString), Math::Vec3(1.0f, 1.0f, 1.0f));
            }
        }
        
        void BulletDebugDrawer::setDebugMode(int debugMode) {
            m_debugMode = debugMode;
        }
        
        int BulletDebugDrawer::getDebugMode() const {
            return m_debugMode;
        }
        
        void BulletDebugDrawer::drawSphere(const btVector3& p, btScalar radius, const btVector3& color) {
            if (m_drawer) {
                m_drawer->DrawSphere(BulletUtils::FromBullet(p), static_cast<float>(radius), BulletUtils::FromBullet(color));
            }
        }
        
        void BulletDebugDrawer::drawBox(const btVector3& boxMin, const btVector3& boxMax, const btVector3& color) {
            if (m_drawer) {
                Math::Vec3 center = (BulletUtils::FromBullet(boxMin) + BulletUtils::FromBullet(boxMax)) * 0.5f;
                Math::Vec3 halfExtents = (BulletUtils::FromBullet(boxMax) - BulletUtils::FromBullet(boxMin)) * 0.5f;
                m_drawer->DrawBox(center, halfExtents, Math::Quat(1, 0, 0, 0), BulletUtils::FromBullet(color));
            }
        }
        
        void BulletDebugDrawer::drawCapsule(btScalar radius, btScalar halfHeight, int upAxis, 
                                          const btTransform& transform, const btVector3& color) {
            if (m_drawer) {
                Math::Vec3 center = BulletUtils::FromBullet(transform.getOrigin());
                Math::Quat rotation = BulletUtils::FromBullet(transform.getRotation());
                float height = static_cast<float>(halfHeight) * 2.0f;
                m_drawer->DrawCapsule(center, static_cast<float>(radius), height, rotation, BulletUtils::FromBullet(color));
            }
        }
#endif
        
    } // namespace Physics
} // namespace GameEngine