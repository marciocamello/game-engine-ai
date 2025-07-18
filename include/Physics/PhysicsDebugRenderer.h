#pragma once

#include "Physics/PhysicsDebugDrawer.h"
#include "Core/Math.h"
#include <memory>
#include <vector>

namespace GameEngine {
    class Shader;
    class Camera;
    
    namespace Graphics {
        class OpenGLRenderer;
    }
    
    namespace Physics {
        
        /**
         * @brief Configuration for physics debug rendering
         */
        struct PhysicsDebugConfig {
            // Line rendering
            float lineWidth = 2.0f;
            Math::Vec3 wireframeColor{0.0f, 1.0f, 0.0f};      // Green for wireframes
            Math::Vec3 aabbColor{1.0f, 1.0f, 0.0f};           // Yellow for AABBs
            Math::Vec3 contactColor{1.0f, 0.0f, 0.0f};        // Red for contact points
            Math::Vec3 constraintColor{0.0f, 0.0f, 1.0f};     // Blue for constraints
            
            // Contact point rendering
            float contactPointSize = 0.05f;
            float contactNormalLength = 0.2f;
            
            // Text rendering
            Math::Vec3 textColor{1.0f, 1.0f, 1.0f};           // White for text
            float textScale = 1.0f;
            
            // Performance settings
            bool enableFrustumCulling = true;
            float maxRenderDistance = 100.0f;
            int maxLinesPerFrame = 10000;
            
            // Transparency
            float alpha = 0.8f;
            bool enableDepthTest = true;
            
            static PhysicsDebugConfig Default() {
                return PhysicsDebugConfig{};
            }
        };
        
        /**
         * @brief OpenGL-based physics debug renderer
         * 
         * This class implements IPhysicsDebugDrawer to provide real-time
         * visualization of physics objects, collision shapes, contact points,
         * forces, and constraints using OpenGL rendering.
         */
        class PhysicsDebugRenderer : public IPhysicsDebugDrawer {
        public:
            PhysicsDebugRenderer();
            ~PhysicsDebugRenderer() override;
            
            // Initialization and cleanup
            bool Initialize();
            void Shutdown();
            
            // Configuration
            void SetConfig(const PhysicsDebugConfig& config) { m_config = config; }
            const PhysicsDebugConfig& GetConfig() const { return m_config; }
            
            // Camera and rendering setup
            void SetCamera(const Camera* camera) { m_camera = camera; }
            void BeginFrame();
            void EndFrame();
            void Render();
            
            // IPhysicsDebugDrawer interface
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
            
            // Performance monitoring
            struct RenderStats {
                int linesRendered = 0;
                int spheresRendered = 0;
                int boxesRendered = 0;
                int capsulesRendered = 0;
                int contactPointsRendered = 0;
                int textItemsRendered = 0;
                float renderTime = 0.0f;
                int totalVertices = 0;
                int drawCalls = 0;
            };
            
            const RenderStats& GetRenderStats() const { return m_stats; }
            void ResetStats() { m_stats = {}; }
            
        private:
            // Rendering data structures
            struct LineVertex {
                Math::Vec3 position;
                Math::Vec3 color;
            };
            
            struct SphereData {
                Math::Vec3 center;
                float radius;
                Math::Vec3 color;
            };
            
            struct BoxData {
                Math::Vec3 center;
                Math::Vec3 halfExtents;
                Math::Quat rotation;
                Math::Vec3 color;
            };
            
            struct CapsuleData {
                Math::Vec3 center;
                float radius;
                float height;
                Math::Quat rotation;
                Math::Vec3 color;
            };
            
            struct ContactData {
                Math::Vec3 point;
                Math::Vec3 normal;
                float distance;
                Math::Vec3 color;
            };
            
            struct TextData {
                Math::Vec3 position;
                std::string text;
                Math::Vec3 color;
            };
            
            // OpenGL resources
            bool InitializeShaders();
            bool InitializeBuffers();
            void CleanupResources();
            
            // Rendering methods
            void RenderLines();
            void RenderSpheres();
            void RenderBoxes();
            void RenderCapsules();
            void RenderContactPoints();
            void RenderText();
            
            // Geometry generation
            void GenerateSphereWireframe(const Math::Vec3& center, float radius, 
                                       const Math::Vec3& color, std::vector<LineVertex>& vertices);
            void GenerateBoxWireframe(const Math::Vec3& center, const Math::Vec3& halfExtents,
                                    const Math::Quat& rotation, const Math::Vec3& color, 
                                    std::vector<LineVertex>& vertices);
            void GenerateCapsuleWireframe(const Math::Vec3& center, float radius, float height,
                                        const Math::Quat& rotation, const Math::Vec3& color,
                                        std::vector<LineVertex>& vertices);
            
            // Frustum culling
            bool IsInFrustum(const Math::Vec3& center, float radius) const;
            
            // Configuration
            PhysicsDebugConfig m_config;
            
            // Camera reference
            const Camera* m_camera = nullptr;
            
            // Rendering data
            std::vector<LineVertex> m_lineVertices;
            std::vector<SphereData> m_spheres;
            std::vector<BoxData> m_boxes;
            std::vector<CapsuleData> m_capsules;
            std::vector<ContactData> m_contacts;
            std::vector<TextData> m_texts;
            
            // OpenGL resources
            std::unique_ptr<Shader> m_lineShader;
            uint32_t m_lineVAO = 0;
            uint32_t m_lineVBO = 0;
            
            // Statistics
            RenderStats m_stats;
            
            // State
            bool m_initialized = false;
        };
        
    } // namespace Physics
} // namespace GameEngine