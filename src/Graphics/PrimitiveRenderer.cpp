#include "Graphics/PrimitiveRenderer.h"
#include "Graphics/Mesh.h"
#include "Graphics/Shader.h"
#include "Core/Logger.h"
#include <cmath>

namespace GameEngine {
    PrimitiveRenderer::PrimitiveRenderer() {
    }

    PrimitiveRenderer::~PrimitiveRenderer() {
        Shutdown();
    }

    bool PrimitiveRenderer::Initialize() {
        CreateShaders();
        CreatePrimitiveMeshes();
        
        LOG_INFO("Primitive Renderer initialized");
        return true;
    }

    void PrimitiveRenderer::Shutdown() {
        m_shader.reset();
        m_cubeMesh.reset();
        m_sphereMesh.reset();
        m_capsuleMesh.reset();
        m_cylinderMesh.reset();
        m_planeMesh.reset();
    }

    void PrimitiveRenderer::CreateShaders() {
        std::string vertexShader = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;
            
            uniform mat4 u_mvp;
            uniform mat4 u_model;
            uniform mat3 u_normalMatrix;
            
            out vec3 FragPos;
            out vec3 Normal;
            
            void main() {
                FragPos = vec3(u_model * vec4(aPos, 1.0));
                Normal = u_normalMatrix * aNormal;
                gl_Position = u_mvp * vec4(aPos, 1.0);
            }
        )";

        std::string fragmentShader = R"(
            #version 330 core
            in vec3 FragPos;
            in vec3 Normal;
            
            out vec4 FragColor;
            
            uniform vec4 u_color;
            uniform vec3 u_lightPos;
            uniform vec3 u_lightColor;
            uniform vec3 u_viewPos;
            
            void main() {
                // Ambient
                float ambientStrength = 0.3;
                vec3 ambient = ambientStrength * u_lightColor;
                
                // Diffuse
                vec3 norm = normalize(Normal);
                vec3 lightDir = normalize(u_lightPos - FragPos);
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = diff * u_lightColor;
                
                // Specular
                float specularStrength = 0.5;
                vec3 viewDir = normalize(u_viewPos - FragPos);
                vec3 reflectDir = reflect(-lightDir, norm);
                float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
                vec3 specular = specularStrength * spec * u_lightColor;
                
                vec3 result = (ambient + diffuse + specular) * u_color.rgb;
                FragColor = vec4(result, u_color.a);
            }
        )";

        m_shader = std::make_shared<Shader>();
        m_shader->LoadFromSource(vertexShader, fragmentShader);
    }

    void PrimitiveRenderer::CreatePrimitiveMeshes() {
        m_cubeMesh = CreateCubeMesh();
        m_sphereMesh = CreateSphereMesh();
        m_capsuleMesh = CreateCapsuleMesh();
        m_cylinderMesh = CreateCylinderMesh();
        m_planeMesh = CreatePlaneMesh();
    }

    std::shared_ptr<Mesh> PrimitiveRenderer::CreateCubeMesh() {
        std::vector<Vertex> vertices = {
            // Front face
            {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            
            // Back face
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            
            // Left face
            {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            
            // Right face
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            
            // Bottom face
            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            
            // Top face
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}
        };

        std::vector<uint32_t> indices = {
            0,  1,  2,   2,  3,  0,   // front
            4,  5,  6,   6,  7,  4,   // back
            8,  9,  10,  10, 11, 8,   // left
            12, 13, 14,  14, 15, 12,  // right
            16, 17, 18,  18, 19, 16,  // bottom
            20, 21, 22,  22, 23, 20   // top
        };

        auto mesh = std::make_shared<Mesh>();
        mesh->SetVertices(vertices);
        mesh->SetIndices(indices);
        return mesh;
    }

    std::shared_ptr<Mesh> PrimitiveRenderer::CreateCapsuleMesh(int segments) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        float radius = 0.5f;
        float height = 1.0f; // Half height of cylinder part
        
        // Create cylinder body
        for (int i = 0; i <= segments; ++i) {
            float angle = 2.0f * Math::PI * i / segments;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            
            // Bottom ring
            vertices.push_back({{x, -height, z}, {x/radius, 0.0f, z/radius}, {(float)i/segments, 0.0f}});
            // Top ring
            vertices.push_back({{x, height, z}, {x/radius, 0.0f, z/radius}, {(float)i/segments, 1.0f}});
        }

        // Create cylinder indices
        for (int i = 0; i < segments; ++i) {
            int bottom1 = i * 2;
            int top1 = i * 2 + 1;
            int bottom2 = (i + 1) * 2;
            int top2 = (i + 1) * 2 + 1;
            
            // Two triangles per quad
            indices.insert(indices.end(), {static_cast<uint32_t>(bottom1), static_cast<uint32_t>(bottom2), static_cast<uint32_t>(top1)});
            indices.insert(indices.end(), {static_cast<uint32_t>(top1), static_cast<uint32_t>(bottom2), static_cast<uint32_t>(top2)});
        }

        // Add hemisphere caps (simplified)
        int baseIndex = vertices.size();
        
        // Top hemisphere
        vertices.push_back({{0.0f, height + radius, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 1.0f}});
        for (int i = 0; i <= segments; ++i) {
            float angle = 2.0f * Math::PI * i / segments;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            vertices.push_back({{x, height, z}, {x/radius, 1.0f, z/radius}, {(float)i/segments, 0.5f}});
        }

        // Top hemisphere indices
        for (int i = 0; i < segments; ++i) {
            indices.insert(indices.end(), {static_cast<uint32_t>(baseIndex), static_cast<uint32_t>(baseIndex + 1 + i), static_cast<uint32_t>(baseIndex + 1 + (i + 1) % segments)});
        }

        // Bottom hemisphere
        int bottomBase = static_cast<int>(vertices.size());
        vertices.push_back({{0.0f, -height - radius, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.0f}});
        for (int i = 0; i <= segments; ++i) {
            float angle = 2.0f * Math::PI * i / segments;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            vertices.push_back({{x, -height, z}, {x/radius, -1.0f, z/radius}, {static_cast<float>(i)/segments, 0.5f}});
        }

        // Bottom hemisphere indices
        for (int i = 0; i < segments; ++i) {
            indices.insert(indices.end(), {static_cast<uint32_t>(bottomBase), static_cast<uint32_t>(bottomBase + 1 + (i + 1) % segments), static_cast<uint32_t>(bottomBase + 1 + i)});
        }

        auto mesh = std::make_shared<Mesh>();
        mesh->SetVertices(vertices);
        mesh->SetIndices(indices);
        return mesh;
    }

    std::shared_ptr<Mesh> PrimitiveRenderer::CreateSphereMesh(int segments) {
        // Simplified sphere - will create a basic icosphere
        auto mesh = std::make_shared<Mesh>();
        // For now, use cube as placeholder
        return CreateCubeMesh();
    }

    std::shared_ptr<Mesh> PrimitiveRenderer::CreateCylinderMesh(int segments) {
        // Similar to capsule but without hemispheres
        auto mesh = std::make_shared<Mesh>();
        return CreateCubeMesh(); // Placeholder
    }

    std::shared_ptr<Mesh> PrimitiveRenderer::CreatePlaneMesh() {
        std::vector<Vertex> vertices = {
            {{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}
        };

        std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

        auto mesh = std::make_shared<Mesh>();
        mesh->SetVertices(vertices);
        mesh->SetIndices(indices);
        return mesh;
    }

    void PrimitiveRenderer::SetViewProjectionMatrix(const Math::Mat4& viewProjection) {
        m_viewProjectionMatrix = viewProjection;
    }

    void PrimitiveRenderer::DrawCapsule(const Math::Vec3& position, float radius, float height, const Math::Vec4& color) {
        if (!m_shader || !m_capsuleMesh) return;

        m_shader->Use();
        
        // Create model matrix
        Math::Mat4 model = Math::CreateTransform(position, Math::Quat(1,0,0,0), Math::Vec3(radius, height, radius));
        Math::Mat4 mvp = m_viewProjectionMatrix * model;
        Math::Mat3 normalMatrix = Math::Mat3(glm::transpose(glm::inverse(model)));
        
        // Set uniforms
        m_shader->SetMat4("u_mvp", mvp);
        m_shader->SetMat4("u_model", model);
        m_shader->SetMat3("u_normalMatrix", normalMatrix);
        m_shader->SetVec4("u_color", color);
        m_shader->SetVec3("u_lightPos", Math::Vec3(5.0f, 10.0f, 5.0f));
        m_shader->SetVec3("u_lightColor", Math::Vec3(1.0f, 1.0f, 1.0f));
        m_shader->SetVec3("u_viewPos", Math::Vec3(0.0f, 5.0f, 10.0f));
        
        m_capsuleMesh->Draw();
    }

    void PrimitiveRenderer::DrawCube(const Math::Vec3& position, const Math::Vec3& scale, const Math::Vec4& color) {
        if (!m_shader || !m_cubeMesh) return;

        m_shader->Use();
        
        Math::Mat4 model = Math::CreateTransform(position, Math::Quat(1,0,0,0), scale);
        Math::Mat4 mvp = m_viewProjectionMatrix * model;
        Math::Mat3 normalMatrix = Math::Mat3(glm::transpose(glm::inverse(model)));
        
        m_shader->SetMat4("u_mvp", mvp);
        m_shader->SetMat4("u_model", model);
        m_shader->SetMat3("u_normalMatrix", normalMatrix);
        m_shader->SetVec4("u_color", color);
        m_shader->SetVec3("u_lightPos", Math::Vec3(5.0f, 10.0f, 5.0f));
        m_shader->SetVec3("u_lightColor", Math::Vec3(1.0f, 1.0f, 1.0f));
        m_shader->SetVec3("u_viewPos", Math::Vec3(0.0f, 5.0f, 10.0f));
        
        m_cubeMesh->Draw();
    }

    void PrimitiveRenderer::DrawSphere(const Math::Vec3& position, float radius, const Math::Vec4& color) {
        // Use cube for now
        DrawCube(position, Math::Vec3(radius), color);
    }

    void PrimitiveRenderer::DrawCylinder(const Math::Vec3& position, float radius, float height, const Math::Vec4& color) {
        // Use cube for now
        DrawCube(position, Math::Vec3(radius, height, radius), color);
    }

    void PrimitiveRenderer::DrawPlane(const Math::Vec3& position, const Math::Vec2& size, const Math::Vec4& color) {
        if (!m_shader || !m_planeMesh) return;

        m_shader->Use();
        
        Math::Mat4 model = Math::CreateTransform(position, Math::Quat(1,0,0,0), Math::Vec3(size.x, 1.0f, size.y));
        Math::Mat4 mvp = m_viewProjectionMatrix * model;
        Math::Mat3 normalMatrix = Math::Mat3(glm::transpose(glm::inverse(model)));
        
        m_shader->SetMat4("u_mvp", mvp);
        m_shader->SetMat4("u_model", model);
        m_shader->SetMat3("u_normalMatrix", normalMatrix);
        m_shader->SetVec4("u_color", color);
        m_shader->SetVec3("u_lightPos", Math::Vec3(5.0f, 10.0f, 5.0f));
        m_shader->SetVec3("u_lightColor", Math::Vec3(1.0f, 1.0f, 1.0f));
        m_shader->SetVec3("u_viewPos", Math::Vec3(0.0f, 5.0f, 10.0f));
        
        m_planeMesh->Draw();
    }
}