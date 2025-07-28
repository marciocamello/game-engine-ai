#pragma once

#include "Core/Math.h"
#include <memory>

namespace GameEngine {
    class Shader;
    class Mesh;
    class Texture;

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

        // Set matrices for rendering
        void SetViewProjectionMatrix(const Math::Mat4& viewProjection);

    private:
        void CreatePrimitiveMeshes();
        void CreateShaders();
        void DrawPrimitive(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Vec3& scale, const Math::Vec4& color, std::shared_ptr<Texture> texture = nullptr);
        void DrawPrimitive(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, const Math::Quat& rotation, const Math::Vec3& scale, const Math::Vec4& color, std::shared_ptr<Texture> texture = nullptr);
        
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
    };
}