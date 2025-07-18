#include "Physics/PhysicsDebugRenderer.h"
#include "Graphics/Shader.h"
#include "Graphics/Camera.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <chrono>
#include <cmath>

namespace GameEngine {
    namespace Physics {
        
        PhysicsDebugRenderer::PhysicsDebugRenderer() {
        }
        
        PhysicsDebugRenderer::~PhysicsDebugRenderer() {
            Shutdown();
        }
        
        bool PhysicsDebugRenderer::Initialize() {
            if (m_initialized) {
                LOG_WARNING("PhysicsDebugRenderer already initialized");
                return true;
            }
            
            if (!InitializeShaders()) {
                LOG_ERROR("Failed to initialize debug renderer shaders");
                return false;
            }
            
            if (!InitializeBuffers()) {
                LOG_ERROR("Failed to initialize debug renderer buffers");
                return false;
            }
            
            m_initialized = true;
            LOG_INFO("PhysicsDebugRenderer initialized successfully");
            return true;
        }
        
        void PhysicsDebugRenderer::Shutdown() {
            if (!m_initialized) {
                return;
            }
            
            CleanupResources();
            m_initialized = false;
            LOG_INFO("PhysicsDebugRenderer shut down");
        }
        
        bool PhysicsDebugRenderer::InitializeShaders() {
            // Create line shader
            m_lineShader = std::make_unique<Shader>();
            
            if (!m_lineShader->LoadFromFiles("assets/shaders/debug_line.vert", "assets/shaders/debug_line.frag")) {
                LOG_ERROR("Failed to load debug line shader from files");
                return false;
            }
            
            return true;
        }
        
        bool PhysicsDebugRenderer::InitializeBuffers() {
            // Generate VAO and VBO for lines
            glGenVertexArrays(1, &m_lineVAO);
            glGenBuffers(1, &m_lineVBO);
            
            glBindVertexArray(m_lineVAO);
            glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
            
            // Position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)0);
            glEnableVertexAttribArray(0);
            
            // Color attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            
            glBindVertexArray(0);
            
            return true;
        }
        
        void PhysicsDebugRenderer::CleanupResources() {
            if (m_lineVAO != 0) {
                glDeleteVertexArrays(1, &m_lineVAO);
                m_lineVAO = 0;
            }
            
            if (m_lineVBO != 0) {
                glDeleteBuffers(1, &m_lineVBO);
                m_lineVBO = 0;
            }
            
            m_lineShader.reset();
        }
        
        void PhysicsDebugRenderer::BeginFrame() {
            if (!m_initialized) {
                return;
            }
            
            // Clear previous frame data
            Clear();
            ResetStats();
        }
        
        void PhysicsDebugRenderer::EndFrame() {
            if (!m_initialized) {
                return;
            }
            
            // Render all debug geometry
            Render();
        }
        
        void PhysicsDebugRenderer::Render() {
            if (!m_initialized || !m_camera) {
                return;
            }
            
            auto startTime = std::chrono::high_resolution_clock::now();
            
            // Set up OpenGL state for debug rendering
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            if (m_config.enableDepthTest) {
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
            } else {
                glDisable(GL_DEPTH_TEST);
            }
            
            glLineWidth(m_config.lineWidth);
            
            // Render all geometry types
            RenderLines();
            RenderSpheres();
            RenderBoxes();
            RenderCapsules();
            RenderContactPoints();
            RenderText();
            
            // Restore OpenGL state
            glLineWidth(1.0f);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            
            auto endTime = std::chrono::high_resolution_clock::now();
            m_stats.renderTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        }
        
        void PhysicsDebugRenderer::DrawLine(const Math::Vec3& from, const Math::Vec3& to, const Math::Vec3& color) {
            if (!m_initialized || m_lineVertices.size() >= static_cast<size_t>(m_config.maxLinesPerFrame * 2)) {
                return;
            }
            
            // Frustum culling check
            if (m_config.enableFrustumCulling && m_camera) {
                Math::Vec3 center = (from + to) * 0.5f;
                float radius = glm::length(glm::vec3(to.x - from.x, to.y - from.y, to.z - from.z)) * 0.5f;
                if (!IsInFrustum(center, radius)) {
                    return;
                }
            }
            
            m_lineVertices.push_back({from, color});
            m_lineVertices.push_back({to, color});
        }
        
        void PhysicsDebugRenderer::DrawSphere(const Math::Vec3& center, float radius, const Math::Vec3& color) {
            if (!m_initialized) {
                return;
            }
            
            // Frustum culling check
            if (m_config.enableFrustumCulling && m_camera && !IsInFrustum(center, radius)) {
                return;
            }
            
            m_spheres.push_back({center, radius, color});
        }
        
        void PhysicsDebugRenderer::DrawBox(const Math::Vec3& center, const Math::Vec3& halfExtents, 
                                         const Math::Quat& rotation, const Math::Vec3& color) {
            if (!m_initialized) {
                return;
            }
            
            // Frustum culling check
            if (m_config.enableFrustumCulling && m_camera) {
                float radius = glm::length(glm::vec3(halfExtents.x, halfExtents.y, halfExtents.z));
                if (!IsInFrustum(center, radius)) {
                    return;
                }
            }
            
            m_boxes.push_back({center, halfExtents, rotation, color});
        }
        
        void PhysicsDebugRenderer::DrawCapsule(const Math::Vec3& center, float radius, float height, 
                                             const Math::Quat& rotation, const Math::Vec3& color) {
            if (!m_initialized) {
                return;
            }
            
            // Frustum culling check
            if (m_config.enableFrustumCulling && m_camera) {
                float cullRadius = std::max(radius, height * 0.5f);
                if (!IsInFrustum(center, cullRadius)) {
                    return;
                }
            }
            
            m_capsules.push_back({center, radius, height, rotation, color});
        }
        
        void PhysicsDebugRenderer::DrawText(const Math::Vec3& position, const std::string& text, const Math::Vec3& color) {
            if (!m_initialized) {
                return;
            }
            
            // Frustum culling check
            if (m_config.enableFrustumCulling && m_camera && !IsInFrustum(position, 1.0f)) {
                return;
            }
            
            m_texts.push_back({position, text, color});
        }
        
        void PhysicsDebugRenderer::DrawContactPoint(const Math::Vec3& point, const Math::Vec3& normal, 
                                                  float distance, const Math::Vec3& color) {
            if (!m_initialized) {
                return;
            }
            
            // Frustum culling check
            if (m_config.enableFrustumCulling && m_camera && !IsInFrustum(point, m_config.contactNormalLength)) {
                return;
            }
            
            m_contacts.push_back({point, normal, distance, color});
        }
        
        void PhysicsDebugRenderer::Clear() {
            m_lineVertices.clear();
            m_spheres.clear();
            m_boxes.clear();
            m_capsules.clear();
            m_contacts.clear();
            m_texts.clear();
        }
        
        void PhysicsDebugRenderer::RenderLines() {
            if (m_lineVertices.empty() || !m_lineShader || !m_camera) {
                return;
            }
            
            m_lineShader->Use();
            
            // Set uniforms
            Math::Mat4 viewProjection = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();
            m_lineShader->SetMat4("u_viewProjection", viewProjection);
            m_lineShader->SetFloat("u_alpha", m_config.alpha);
            
            // Upload vertex data
            glBindVertexArray(m_lineVAO);
            glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
            glBufferData(GL_ARRAY_BUFFER, m_lineVertices.size() * sizeof(LineVertex), 
                        m_lineVertices.data(), GL_DYNAMIC_DRAW);
            
            // Draw lines
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_lineVertices.size()));
            
            glBindVertexArray(0);
            m_lineShader->Unuse();
            
            // Update stats
            m_stats.linesRendered = static_cast<int>(m_lineVertices.size() / 2);
            m_stats.totalVertices += static_cast<int>(m_lineVertices.size());
            m_stats.drawCalls++;
        }
        
        void PhysicsDebugRenderer::RenderSpheres() {
            if (m_spheres.empty()) {
                return;
            }
            
            std::vector<LineVertex> sphereLines;
            
            for (const auto& sphere : m_spheres) {
                GenerateSphereWireframe(sphere.center, sphere.radius, sphere.color, sphereLines);
            }
            
            if (!sphereLines.empty()) {
                // Add sphere lines to main line buffer
                m_lineVertices.insert(m_lineVertices.end(), sphereLines.begin(), sphereLines.end());
                m_stats.spheresRendered = static_cast<int>(m_spheres.size());
            }
        }
        
        void PhysicsDebugRenderer::RenderBoxes() {
            if (m_boxes.empty()) {
                return;
            }
            
            std::vector<LineVertex> boxLines;
            
            for (const auto& box : m_boxes) {
                GenerateBoxWireframe(box.center, box.halfExtents, box.rotation, box.color, boxLines);
            }
            
            if (!boxLines.empty()) {
                // Add box lines to main line buffer
                m_lineVertices.insert(m_lineVertices.end(), boxLines.begin(), boxLines.end());
                m_stats.boxesRendered = static_cast<int>(m_boxes.size());
            }
        }
        
        void PhysicsDebugRenderer::RenderCapsules() {
            if (m_capsules.empty()) {
                return;
            }
            
            std::vector<LineVertex> capsuleLines;
            
            for (const auto& capsule : m_capsules) {
                GenerateCapsuleWireframe(capsule.center, capsule.radius, capsule.height, 
                                       capsule.rotation, capsule.color, capsuleLines);
            }
            
            if (!capsuleLines.empty()) {
                // Add capsule lines to main line buffer
                m_lineVertices.insert(m_lineVertices.end(), capsuleLines.begin(), capsuleLines.end());
                m_stats.capsulesRendered = static_cast<int>(m_capsules.size());
            }
        }
        
        void PhysicsDebugRenderer::RenderContactPoints() {
            if (m_contacts.empty()) {
                return;
            }
            
            for (const auto& contact : m_contacts) {
                // Draw contact point as small sphere
                std::vector<LineVertex> pointLines;
                GenerateSphereWireframe(contact.point, m_config.contactPointSize, contact.color, pointLines);
                m_lineVertices.insert(m_lineVertices.end(), pointLines.begin(), pointLines.end());
                
                // Draw contact normal
                Math::Vec3 normalEnd = contact.point + contact.normal * m_config.contactNormalLength;
                m_lineVertices.push_back({contact.point, contact.color});
                m_lineVertices.push_back({normalEnd, contact.color});
            }
            
            m_stats.contactPointsRendered = static_cast<int>(m_contacts.size());
        }
        
        void PhysicsDebugRenderer::RenderText() {
            // Text rendering would require a more complex implementation
            // For now, we'll just count the text items
            m_stats.textItemsRendered = static_cast<int>(m_texts.size());
            
            // TODO: Implement text rendering using a font system
            // This would require bitmap fonts or signed distance field fonts
        }
        
        void PhysicsDebugRenderer::GenerateSphereWireframe(const Math::Vec3& center, float radius, 
                                                         const Math::Vec3& color, std::vector<LineVertex>& vertices) {
            const int segments = 16;
            const float angleStep = 2.0f * 3.14159265f / segments;
            
            // Generate three orthogonal circles
            for (int axis = 0; axis < 3; ++axis) {
                for (int i = 0; i < segments; ++i) {
                    float angle1 = i * angleStep;
                    float angle2 = (i + 1) * angleStep;
                    
                    Math::Vec3 p1, p2;
                    
                    if (axis == 0) { // XY plane
                        p1 = center + Math::Vec3(std::cos(angle1) * radius, std::sin(angle1) * radius, 0.0f);
                        p2 = center + Math::Vec3(std::cos(angle2) * radius, std::sin(angle2) * radius, 0.0f);
                    } else if (axis == 1) { // XZ plane
                        p1 = center + Math::Vec3(std::cos(angle1) * radius, 0.0f, std::sin(angle1) * radius);
                        p2 = center + Math::Vec3(std::cos(angle2) * radius, 0.0f, std::sin(angle2) * radius);
                    } else { // YZ plane
                        p1 = center + Math::Vec3(0.0f, std::cos(angle1) * radius, std::sin(angle1) * radius);
                        p2 = center + Math::Vec3(0.0f, std::cos(angle2) * radius, std::sin(angle2) * radius);
                    }
                    
                    vertices.push_back({p1, color});
                    vertices.push_back({p2, color});
                }
            }
        }
        
        void PhysicsDebugRenderer::GenerateBoxWireframe(const Math::Vec3& center, const Math::Vec3& halfExtents,
                                                      const Math::Quat& rotation, const Math::Vec3& color, 
                                                      std::vector<LineVertex>& vertices) {
            // Define box vertices in local space
            Math::Vec3 localVertices[8] = {
                {-halfExtents.x, -halfExtents.y, -halfExtents.z}, // 0: left-bottom-back
                { halfExtents.x, -halfExtents.y, -halfExtents.z}, // 1: right-bottom-back
                { halfExtents.x,  halfExtents.y, -halfExtents.z}, // 2: right-top-back
                {-halfExtents.x,  halfExtents.y, -halfExtents.z}, // 3: left-top-back
                {-halfExtents.x, -halfExtents.y,  halfExtents.z}, // 4: left-bottom-front
                { halfExtents.x, -halfExtents.y,  halfExtents.z}, // 5: right-bottom-front
                { halfExtents.x,  halfExtents.y,  halfExtents.z}, // 6: right-top-front
                {-halfExtents.x,  halfExtents.y,  halfExtents.z}  // 7: left-top-front
            };
            
            // Transform vertices to world space (simplified - just add center for now)
            Math::Vec3 worldVertices[8];
            for (int i = 0; i < 8; ++i) {
                worldVertices[i] = center + localVertices[i];
            }
            
            // Define box edges (12 edges total)
            int edges[12][2] = {
                // Back face
                {0, 1}, {1, 2}, {2, 3}, {3, 0},
                // Front face
                {4, 5}, {5, 6}, {6, 7}, {7, 4},
                // Connecting edges
                {0, 4}, {1, 5}, {2, 6}, {3, 7}
            };
            
            // Generate line segments for each edge
            for (int i = 0; i < 12; ++i) {
                vertices.push_back({worldVertices[edges[i][0]], color});
                vertices.push_back({worldVertices[edges[i][1]], color});
            }
        }
        
        void PhysicsDebugRenderer::GenerateCapsuleWireframe(const Math::Vec3& center, float radius, float height,
                                                          const Math::Quat& rotation, const Math::Vec3& color,
                                                          std::vector<LineVertex>& vertices) {
            const int segments = 16;
            const float angleStep = 2.0f * 3.14159265f / segments;
            const float halfHeight = height * 0.5f;
            
            // Generate cylinder body (vertical lines and top/bottom circles)
            Math::Vec3 up = Math::Vec3(0, 1, 0);
            Math::Vec3 right = Math::Vec3(1, 0, 0);
            Math::Vec3 forward = Math::Vec3(0, 0, 1);
            
            Math::Vec3 topCenter = center + up * halfHeight;
            Math::Vec3 bottomCenter = center - up * halfHeight;
            
            // Vertical lines
            for (int i = 0; i < 4; ++i) {
                float angle = i * 3.14159265f * 0.5f;
                Math::Vec3 offset = (right * std::cos(angle) + forward * std::sin(angle)) * radius;
                
                vertices.push_back({topCenter + offset, color});
                vertices.push_back({bottomCenter + offset, color});
            }
            
            // Top and bottom circles
            for (int i = 0; i < segments; ++i) {
                float angle1 = i * angleStep;
                float angle2 = (i + 1) * angleStep;
                
                Math::Vec3 offset1 = (right * std::cos(angle1) + forward * std::sin(angle1)) * radius;
                Math::Vec3 offset2 = (right * std::cos(angle2) + forward * std::sin(angle2)) * radius;
                
                // Top circle
                vertices.push_back({topCenter + offset1, color});
                vertices.push_back({topCenter + offset2, color});
                
                // Bottom circle
                vertices.push_back({bottomCenter + offset1, color});
                vertices.push_back({bottomCenter + offset2, color});
            }
            
            // Hemisphere caps (simplified as arcs)
            for (int i = 0; i < segments / 2; ++i) {
                float angle1 = i * 3.14159265f / (segments / 2);
                float angle2 = (i + 1) * 3.14159265f / (segments / 2);
                
                // Top hemisphere arcs
                Math::Vec3 p1 = topCenter + up * (std::cos(angle1) * radius) + right * (std::sin(angle1) * radius);
                Math::Vec3 p2 = topCenter + up * (std::cos(angle2) * radius) + right * (std::sin(angle2) * radius);
                vertices.push_back({p1, color});
                vertices.push_back({p2, color});
                
                // Bottom hemisphere arcs
                p1 = bottomCenter - up * (std::cos(angle1) * radius) + right * (std::sin(angle1) * radius);
                p2 = bottomCenter - up * (std::cos(angle2) * radius) + right * (std::sin(angle2) * radius);
                vertices.push_back({p1, color});
                vertices.push_back({p2, color});
            }
        }
        
        bool PhysicsDebugRenderer::IsInFrustum(const Math::Vec3& center, float radius) const {
            if (!m_camera) {
                return true;
            }
            
            // Simple distance-based culling for now
            Math::Vec3 cameraPos = m_camera->GetPosition();
            float distance = glm::length(glm::vec3(center.x - cameraPos.x, center.y - cameraPos.y, center.z - cameraPos.z));
            
            return distance <= (m_config.maxRenderDistance + radius);
        }
        
    } // namespace Physics
} // namespace GameEngine