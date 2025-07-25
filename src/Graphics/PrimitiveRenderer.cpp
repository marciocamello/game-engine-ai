#include "Graphics/PrimitiveRenderer.h"
#include "Graphics/Mesh.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
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
        m_colorShader.reset();
        m_texturedShader.reset();
        m_cubeMesh.reset();
        m_sphereMesh.reset();
        m_capsuleMesh.reset();
        m_cylinderMesh.reset();
        m_planeMesh.reset();
    }

    void PrimitiveRenderer::CreateShaders() {
        // Color-only shader
        std::string colorVertexShader = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;
            layout (location = 2) in vec2 aTexCoords;
            
            uniform mat4 u_mvp;
            uniform mat4 u_model;
            uniform mat3 u_normalMatrix;
            
            out vec3 FragPos;
            out vec3 Normal;
            out vec2 TexCoords;
            
            void main() {
                FragPos = vec3(u_model * vec4(aPos, 1.0));
                Normal = u_normalMatrix * aNormal;
                TexCoords = aTexCoords;
                gl_Position = u_mvp * vec4(aPos, 1.0);
            }
        )";

        std::string colorFragmentShader = R"(
            #version 330 core
            in vec3 FragPos;
            in vec3 Normal;
            in vec2 TexCoords;
            
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

        // Textured shader
        std::string texturedVertexShader = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;
            layout (location = 2) in vec2 aTexCoords;
            
            uniform mat4 u_mvp;
            uniform mat4 u_model;
            uniform mat3 u_normalMatrix;
            
            out vec3 FragPos;
            out vec3 Normal;
            out vec2 TexCoords;
            
            void main() {
                FragPos = vec3(u_model * vec4(aPos, 1.0));
                Normal = u_normalMatrix * aNormal;
                TexCoords = aTexCoords;
                gl_Position = u_mvp * vec4(aPos, 1.0);
            }
        )";

        std::string texturedFragmentShader = R"(
            #version 330 core
            in vec3 FragPos;
            in vec3 Normal;
            in vec2 TexCoords;
            
            out vec4 FragColor;
            
            uniform sampler2D u_texture;
            uniform vec3 u_lightPos;
            uniform vec3 u_lightColor;
            uniform vec3 u_viewPos;
            
            void main() {
                // Sample texture
                vec4 texColor = texture(u_texture, TexCoords);
                
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
                
                vec3 result = (ambient + diffuse + specular) * texColor.rgb;
                FragColor = vec4(result, texColor.a);
            }
        )";

        m_colorShader = std::make_shared<Shader>();
        m_colorShader->LoadFromSource(colorVertexShader, colorFragmentShader);

        m_texturedShader = std::make_shared<Shader>();
        m_texturedShader->LoadFromSource(texturedVertexShader, texturedFragmentShader);
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

    void PrimitiveRenderer::DrawPrimitive(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Vec3& scale, const Math::Vec4& color, std::shared_ptr<Texture> texture) {
        if (!mesh) return;

        std::shared_ptr<Shader> shader = texture ? m_texturedShader : m_colorShader;
        if (!shader) return;

        shader->Use();
        
        // Create model matrix
        Math::Mat4 model = Math::CreateTransform(position, Math::Quat(1,0,0,0), scale);
        Math::Mat4 mvp = m_viewProjectionMatrix * model;
        Math::Mat3 normalMatrix = Math::Mat3(glm::transpose(glm::inverse(model)));
        
        // Set common uniforms
        shader->SetMat4("u_mvp", mvp);
        shader->SetMat4("u_model", model);
        shader->SetMat3("u_normalMatrix", normalMatrix);
        shader->SetVec3("u_lightPos", Math::Vec3(5.0f, 10.0f, 5.0f));
        shader->SetVec3("u_lightColor", Math::Vec3(1.0f, 1.0f, 1.0f));
        shader->SetVec3("u_viewPos", Math::Vec3(0.0f, 5.0f, 10.0f));
        
        // Set texture or color
        if (texture) {
            texture->Bind(0);
            shader->SetInt("u_texture", 0);
        } else {
            shader->SetVec4("u_color", color);
        }
        
        mesh->Draw();
        
        if (texture) {
            texture->Unbind();
        }
    }

    // Color-based primitive drawing methods
    void PrimitiveRenderer::DrawCube(const Math::Vec3& position, const Math::Vec3& scale, const Math::Vec4& color) {
        DrawPrimitive(m_cubeMesh, position, scale, color);
    }

    void PrimitiveRenderer::DrawSphere(const Math::Vec3& position, float radius, const Math::Vec4& color) {
        DrawPrimitive(m_sphereMesh, position, Math::Vec3(radius), color);
    }

    void PrimitiveRenderer::DrawCapsule(const Math::Vec3& position, float radius, float height, const Math::Vec4& color) {
        DrawPrimitive(m_capsuleMesh, position, Math::Vec3(radius, height, radius), color);
    }

    void PrimitiveRenderer::DrawCylinder(const Math::Vec3& position, float radius, float height, const Math::Vec4& color) {
        DrawPrimitive(m_cylinderMesh, position, Math::Vec3(radius, height, radius), color);
    }

    void PrimitiveRenderer::DrawPlane(const Math::Vec3& position, const Math::Vec2& size, const Math::Vec4& color) {
        DrawPrimitive(m_planeMesh, position, Math::Vec3(size.x, 1.0f, size.y), color);
    }

    // Texture-based primitive drawing methods
    void PrimitiveRenderer::DrawCube(const Math::Vec3& position, const Math::Vec3& scale, std::shared_ptr<Texture> texture) {
        DrawPrimitive(m_cubeMesh, position, scale, Math::Vec4(1.0f), texture);
    }

    void PrimitiveRenderer::DrawSphere(const Math::Vec3& position, float radius, std::shared_ptr<Texture> texture) {
        DrawPrimitive(m_sphereMesh, position, Math::Vec3(radius), Math::Vec4(1.0f), texture);
    }

    void PrimitiveRenderer::DrawCapsule(const Math::Vec3& position, float radius, float height, std::shared_ptr<Texture> texture) {
        DrawPrimitive(m_capsuleMesh, position, Math::Vec3(radius, height, radius), Math::Vec4(1.0f), texture);
    }

    void PrimitiveRenderer::DrawCylinder(const Math::Vec3& position, float radius, float height, std::shared_ptr<Texture> texture) {
        DrawPrimitive(m_cylinderMesh, position, Math::Vec3(radius, height, radius), Math::Vec4(1.0f), texture);
    }

    void PrimitiveRenderer::DrawPlane(const Math::Vec3& position, const Math::Vec2& size, std::shared_ptr<Texture> texture) {
        DrawPrimitive(m_planeMesh, position, Math::Vec3(size.x, 1.0f, size.y), Math::Vec4(1.0f), texture);
    }

    // Mesh drawing methods
    void PrimitiveRenderer::DrawMesh(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Vec3& scale, const Math::Vec4& color) {
        DrawPrimitive(mesh, position, scale, color);
    }

    void PrimitiveRenderer::DrawMesh(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Vec3& scale, std::shared_ptr<Texture> texture) {
        DrawPrimitive(mesh, position, scale, Math::Vec4(1.0f), texture);
    }
}