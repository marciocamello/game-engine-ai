#pragma once

#include "Core/Math.h"
#include <memory>

namespace GameEngine {
    class Shader;
    class Mesh;

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

        // Render primitives
        void DrawCube(const Math::Vec3& position, const Math::Vec3& scale = Math::Vec3(1.0f), const Math::Vec4& color = Math::Vec4(1.0f));
        void DrawSphere(const Math::Vec3& position, float radius = 1.0f, const Math::Vec4& color = Math::Vec4(1.0f));
        void DrawCapsule(const Math::Vec3& position, float radius = 0.5f, float height = 2.0f, const Math::Vec4& color = Math::Vec4(1.0f));
        void DrawCylinder(const Math::Vec3& position, float radius = 0.5f, float height = 2.0f, const Math::Vec4& color = Math::Vec4(1.0f));
        void DrawPlane(const Math::Vec3& position, const Math::Vec2& size = Math::Vec2(10.0f), const Math::Vec4& color = Math::Vec4(1.0f));

        // Set matrices for rendering
        void SetViewProjectionMatrix(const Math::Mat4& viewProjection);

    private:
        void CreatePrimitiveMeshes();
        void CreateShaders();
        
        std::shared_ptr<Mesh> CreateCubeMesh();
        std::shared_ptr<Mesh> CreateSphereMesh(int segments = 16);
        std::shared_ptr<Mesh> CreateCapsuleMesh(int segments = 16);
        std::shared_ptr<Mesh> CreateCylinderMesh(int segments = 16);
        std::shared_ptr<Mesh> CreatePlaneMesh();

        std::shared_ptr<Shader> m_shader;
        std::shared_ptr<Mesh> m_cubeMesh;
        std::shared_ptr<Mesh> m_sphereMesh;
        std::shared_ptr<Mesh> m_capsuleMesh;
        std::shared_ptr<Mesh> m_cylinderMesh;
        std::shared_ptr<Mesh> m_planeMesh;

        Math::Mat4 m_viewProjectionMatrix{1.0f};
    };
}