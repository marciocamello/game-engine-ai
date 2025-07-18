#pragma once

#include "Core/Math.h"
#include <vector>
#include <string>
#include <memory>

#ifdef GAMEENGINE_HAS_BULLET
#include <LinearMath/btIDebugDraw.h>
#endif

namespace GameEngine {
    namespace Physics {
        
        /**
         * @brief Debug drawing modes for physics visualization
         */
        enum class PhysicsDebugMode {
            None = 0,
            Wireframe = 1,
            AABB = 2,
            ContactPoints = 4,
            Constraints = 8,
            All = Wireframe | AABB | ContactPoints | Constraints
        };
        
        /**
         * @brief Abstract interface for physics debug drawing
         */
        class IPhysicsDebugDrawer {
        public:
            virtual ~IPhysicsDebugDrawer() = default;
            
            // Basic drawing primitives
            virtual void DrawLine(const Math::Vec3& from, const Math::Vec3& to, const Math::Vec3& color) = 0;
            virtual void DrawSphere(const Math::Vec3& center, float radius, const Math::Vec3& color) = 0;
            virtual void DrawBox(const Math::Vec3& center, const Math::Vec3& halfExtents, 
                               const Math::Quat& rotation, const Math::Vec3& color) = 0;
            virtual void DrawCapsule(const Math::Vec3& center, float radius, float height, 
                                   const Math::Quat& rotation, const Math::Vec3& color) = 0;
            virtual void DrawText(const Math::Vec3& position, const std::string& text, const Math::Vec3& color) = 0;
            virtual void DrawContactPoint(const Math::Vec3& point, const Math::Vec3& normal, 
                                        float distance, const Math::Vec3& color) = 0;
            
            // Clear all drawing commands
            virtual void Clear() = 0;
        };
        
        /**
         * @brief Simple debug drawer that stores drawing commands for later rendering
         */
        class SimplePhysicsDebugDrawer : public IPhysicsDebugDrawer {
        public:
            struct LineCommand {
                Math::Vec3 from, to, color;
            };
            
            struct SphereCommand {
                Math::Vec3 center, color;
                float radius;
            };
            
            struct BoxCommand {
                Math::Vec3 center, halfExtents, color;
                Math::Quat rotation;
            };
            
            struct CapsuleCommand {
                Math::Vec3 center, color;
                Math::Quat rotation;
                float radius, height;
            };
            
            struct TextCommand {
                Math::Vec3 position, color;
                std::string text;
            };
            
            struct ContactCommand {
                Math::Vec3 point, normal, color;
                float distance;
            };
            
            void DrawLine(const Math::Vec3& from, const Math::Vec3& to, const Math::Vec3& color) override;
            void DrawSphere(const Math::Vec3& center, float radius, const Math::Vec3& color) override;
            void DrawBox(const Math::Vec3& center, const Math::Vec3& halfExtents, 
                        const Math::Quat& rotation, const Math::Vec3& color) override;
            void DrawCapsule(const Math::Vec3& center, float radius, float height, 
                           const Math::Quat& rotation, const Math::Vec3& color) override;
            void DrawText(const Math::Vec3& position, const std::string& text, const Math::Vec3& color) override;
            void DrawContactPoint(const Math::Vec3& point, const Math::Vec3& normal, 
                                float distance, const Math::Vec3& color) override;
            void Clear() override;
            
            // Access stored commands
            const std::vector<LineCommand>& GetLines() const { return m_lines; }
            const std::vector<SphereCommand>& GetSpheres() const { return m_spheres; }
            const std::vector<BoxCommand>& GetBoxes() const { return m_boxes; }
            const std::vector<CapsuleCommand>& GetCapsules() const { return m_capsules; }
            const std::vector<TextCommand>& GetTexts() const { return m_texts; }
            const std::vector<ContactCommand>& GetContacts() const { return m_contacts; }
            
        private:
            std::vector<LineCommand> m_lines;
            std::vector<SphereCommand> m_spheres;
            std::vector<BoxCommand> m_boxes;
            std::vector<CapsuleCommand> m_capsules;
            std::vector<TextCommand> m_texts;
            std::vector<ContactCommand> m_contacts;
        };
        
#ifdef GAMEENGINE_HAS_BULLET
        /**
         * @brief Bullet Physics debug drawer implementation
         */
        class BulletDebugDrawer : public btIDebugDraw {
        public:
            explicit BulletDebugDrawer(std::shared_ptr<IPhysicsDebugDrawer> drawer);
            
            // btIDebugDraw interface
            void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
            void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, 
                                btScalar distance, int lifeTime, const btVector3& color) override;
            void reportErrorWarning(const char* warningString) override;
            void draw3dText(const btVector3& location, const char* textString) override;
            void setDebugMode(int debugMode) override;
            int getDebugMode() const override;
            
            // Additional drawing methods
            void drawSphere(const btVector3& p, btScalar radius, const btVector3& color) override;
            void drawBox(const btVector3& boxMin, const btVector3& boxMax, const btVector3& color) override;
            void drawCapsule(btScalar radius, btScalar halfHeight, int upAxis, 
                           const btTransform& transform, const btVector3& color) override;
            
            // Configuration
            void SetDrawer(std::shared_ptr<IPhysicsDebugDrawer> drawer) { m_drawer = drawer; }
            void SetEnabled(bool enabled) { m_enabled = enabled; }
            bool IsEnabled() const { return m_enabled; }
            
        private:
            std::shared_ptr<IPhysicsDebugDrawer> m_drawer;
            int m_debugMode = 0;
            bool m_enabled = false;
        };
#endif
        
    } // namespace Physics
} // namespace GameEngine