#pragma once

#include "Core/Math.h"
#include <memory>
#include <string>
#include <vector>

namespace GameEngine {
    class Shader;
    class Mesh;
    class Texture;
    class Material;
    class PBRMaterial;

    enum class PrimitiveType {
        Cube,
        Sphere,
        Capsule,
        Cylinder,
        Plane
    };

    class PrimitiveRenderer {
    public:
        PrimitiveRenderer();
        ~PrimitiveRenderer();

        bool Initialize();
        void Shutdown();

        // Render primitives with color
        void DrawCube(const Math::Vec3& position, const Math::Vec3& scale = Math::Vec3(1.0f), const Math::Vec4& color = Math::Vec4(1.0f));
        void DrawSphere(const Math::Vec3& position, float radius = 1.0f, const Math::Vec4& color = Math::Vec4(1.0f));
        void DrawCapsule(const Math::Vec3& position, float radius = 0.5f, float height = 2.0f, const Math::Vec4& color = Math::Vec4(1.0f));
        void DrawCylinder(const Math::Vec3& position, float radius = 0.5f, float height = 2.0f, const Math::Vec4& color = Math::Vec4(1.0f));
        void DrawPlane(const Math::Vec3& position, const Math::Vec2& size = Math::Vec2(10.0f), const Math::Vec4& color = Math::Vec4(1.0f));

        // Render primitives with texture
        void DrawCube(const Math::Vec3& position, const Math::Vec3& scale, std::shared_ptr<Texture> texture);
        void DrawSphere(const Math::Vec3& position, float radius, std::shared_ptr<Texture> texture);
        void DrawCapsule(const Math::Vec3& position, float radius, float height, std::shared_ptr<Texture> texture);
        void DrawCylinder(const Math::Vec3& position, float radius, float height, std::shared_ptr<Texture> texture);
        void DrawPlane(const Math::Vec3& position, const Math::Vec2& size, std::shared_ptr<Texture> texture);

        // Render loaded meshes
        void DrawMesh(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Vec3& scale = Math::Vec3(1.0f), const Math::Vec4& color = Math::Vec4(1.0f));
        void DrawMesh(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Vec3& scale, std::shared_ptr<Texture> texture);
        
        // Render loaded meshes with rotation
        void DrawMesh(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Quat& rotation, const Math::Vec3& scale = Math::Vec3(1.0f), const Math::Vec4& color = Math::Vec4(1.0f));
        void DrawMesh(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Quat& rotation, const Math::Vec3& scale, std::shared_ptr<Texture> texture);

        // Material-aware rendering methods
        void DrawCube(const Math::Vec3& position, const Math::Vec3& scale, std::shared_ptr<Material> material);
        void DrawSphere(const Math::Vec3& position, float radius, std::shared_ptr<Material> material);
        void DrawCapsule(const Math::Vec3& position, float radius, float height, std::shared_ptr<Material> material);
        void DrawCylinder(const Math::Vec3& position, float radius, float height, std::shared_ptr<Material> material);
        void DrawPlane(const Math::Vec3& position, const Math::Vec2& size, std::shared_ptr<Material> material);
        void DrawMesh(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Vec3& scale, std::shared_ptr<Material> material);
        void DrawMesh(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Quat& rotation, const Math::Vec3& scale, std::shared_ptr<Material> material);

        // Set matrices for rendering
        void SetViewProjectionMatrix(const Math::Mat4& viewProjection);
        void SetViewMatrix(const Math::Mat4& viewMatrix);
        void SetProjectionMatrix(const Math::Mat4& projectionMatrix);

        // Lighting system integration
        void SetCameraPosition(const Math::Vec3& position);
        void SetDirectionalLight(const Math::Vec3& direction, const Math::Vec3& color, float intensity = 1.0f);
        void AddPointLight(const Math::Vec3& position, const Math::Vec3& color, float intensity = 1.0f, float radius = 10.0f);
        void ClearPointLights();

        // Enhanced shader management integration
        void SetCustomColorShader(std::shared_ptr<Shader> shader);
        void SetCustomTexturedShader(std::shared_ptr<Shader> shader);
        void ResetToDefaultShaders();
        std::shared_ptr<Shader> GetColorShader() const { return m_colorShader; }
        std::shared_ptr<Shader> GetTexturedShader() const { return m_texturedShader; }
        
        // Shader hot-reload support
        void ReloadShaders();
        void EnableShaderHotReload(bool enable);

    private:
        void CreatePrimitiveMeshes();
        void CreateShaders();
        void DrawPrimitive(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Vec3& scale, const Math::Vec4& color, std::shared_ptr<Texture> texture = nullptr);
        void DrawPrimitive(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Quat& rotation, const Math::Vec3& scale, const Math::Vec4& color, std::shared_ptr<Texture> texture = nullptr);
        void DrawPrimitive(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Quat& rotation, const Math::Vec3& scale, std::shared_ptr<Material> material);
        void ApplyLightingUniforms(std::shared_ptr<Shader> shader);
        
        std::shared_ptr<Mesh> CreateCubeMesh();
        std::shared_ptr<Mesh> CreateSphereMesh(int segments = 16);
        std::shared_ptr<Mesh> CreateCapsuleMesh(int segments = 16);
        std::shared_ptr<Mesh> CreateCylinderMesh(int segments = 16);
        std::shared_ptr<Mesh> CreatePlaneMesh();

        std::shared_ptr<Shader> m_colorShader;
        std::shared_ptr<Shader> m_texturedShader;
        std::shared_ptr<Mesh> m_cubeMesh;
        std::shared_ptr<Mesh> m_sphereMesh;
        std::shared_ptr<Mesh> m_capsuleMesh;
        std::shared_ptr<Mesh> m_cylinderMesh;
        std::shared_ptr<Mesh> m_planeMesh;

        Math::Mat4 m_viewProjectionMatrix{1.0f};
        Math::Mat4 m_viewMatrix{1.0f};
        Math::Mat4 m_projectionMatrix{1.0f};

        // Lighting system
        Math::Vec3 m_cameraPosition{0.0f, 5.0f, 10.0f};
        
        struct DirectionalLight {
            Math::Vec3 direction = Math::Vec3(0.0f, -1.0f, 0.0f);
            Math::Vec3 color = Math::Vec3(1.0f);
            float intensity = 1.0f;
        } m_directionalLight;

        struct PointLight {
            Math::Vec3 position = Math::Vec3(0.0f);
            Math::Vec3 color = Math::Vec3(1.0f);
            float intensity = 1.0f;
            float radius = 10.0f;
        };
        std::vector<PointLight> m_pointLights;
        static const int MAX_POINT_LIGHTS = 8;

        // Shader management
        bool m_usingCustomColorShader = false;
        bool m_usingCustomTexturedShader = false;
        bool m_hotReloadEnabled = false;
        static const std::string DEFAULT_COLOR_SHADER_NAME;
        static const std::string DEFAULT_TEXTURED_SHADER_NAME;
    };
}