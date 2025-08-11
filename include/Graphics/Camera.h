#pragma once

#include "../../engine/core/Math.h"

namespace GameEngine {
    enum class CameraType {
        Perspective,
        Orthographic
    };

    class Camera {
    public:
        Camera(CameraType type = CameraType::Perspective);
        ~Camera() = default;

        // Transform
        void SetPosition(const Math::Vec3& position) { m_position = position; UpdateViewMatrix(); }
        void SetRotation(const Math::Quat& rotation) { m_rotation = rotation; UpdateViewMatrix(); }
        void LookAt(const Math::Vec3& target, const Math::Vec3& up = Math::Vec3(0, 1, 0));

        const Math::Vec3& GetPosition() const { return m_position; }
        const Math::Quat& GetRotation() const { return m_rotation; }
        Math::Vec3 GetForward() const { return m_rotation * Math::Vec3(0, 0, -1); }
        Math::Vec3 GetRight() const { return m_rotation * Math::Vec3(1, 0, 0); }
        Math::Vec3 GetUp() const { return m_rotation * Math::Vec3(0, 1, 0); }
        Math::Vec3 GetVelocity() const { return m_velocity; }
        
        // Update velocity tracking (should be called each frame)
        void UpdateVelocity(float deltaTime);

        // Projection settings
        void SetPerspective(float fov, float aspect, float nearPlane, float farPlane);
        void SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

        // Matrices
        const Math::Mat4& GetViewMatrix() const { return m_viewMatrix; }
        const Math::Mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
        Math::Mat4 GetViewProjectionMatrix() const { return m_projectionMatrix * m_viewMatrix; }

        // Movement (for third-person camera)
        void Orbit(const Math::Vec3& target, float deltaYaw, float deltaPitch, float distance);
        void SetTarget(const Math::Vec3& target) { m_target = target; }
        const Math::Vec3& GetTarget() const { return m_target; }

    private:
        void UpdateViewMatrix();
        void UpdateProjectionMatrix();

        CameraType m_type;
        Math::Vec3 m_position{0.0f, 0.0f, 5.0f};
        Math::Quat m_rotation{1.0f, 0.0f, 0.0f, 0.0f};
        Math::Vec3 m_target{0.0f, 0.0f, 0.0f};

        // Projection parameters
        float m_fov = 45.0f;
        float m_aspect = 16.0f / 9.0f;
        float m_nearPlane = 0.1f;
        float m_farPlane = 1000.0f;
        
        // Orthographic parameters
        float m_left = -10.0f, m_right = 10.0f;
        float m_bottom = -10.0f, m_top = 10.0f;

        Math::Mat4 m_viewMatrix{1.0f};
        Math::Mat4 m_projectionMatrix{1.0f};
        
        // Velocity tracking for audio Doppler effect
        Math::Vec3 m_previousPosition{0.0f, 0.0f, 5.0f};
        Math::Vec3 m_velocity{0.0f};
    };
}