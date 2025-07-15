#include "Graphics/Camera.h"

namespace GameEngine {
    Camera::Camera(CameraType type) : m_type(type) {
        UpdateViewMatrix();
        UpdateProjectionMatrix();
    }

    void Camera::LookAt(const Math::Vec3& target, const Math::Vec3& up) {
        m_target = target;
        Math::Vec3 forward = glm::normalize(target - m_position);
        Math::Vec3 right = glm::normalize(glm::cross(forward, up));
        Math::Vec3 actualUp = glm::cross(right, forward);
        
        m_rotation = glm::quatLookAt(forward, actualUp);
        UpdateViewMatrix();
    }

    void Camera::SetPerspective(float fov, float aspect, float nearPlane, float farPlane) {
        m_type = CameraType::Perspective;
        m_fov = fov;
        m_aspect = aspect;
        m_nearPlane = nearPlane;
        m_farPlane = farPlane;
        UpdateProjectionMatrix();
    }

    void Camera::SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
        m_type = CameraType::Orthographic;
        m_left = left;
        m_right = right;
        m_bottom = bottom;
        m_top = top;
        m_nearPlane = nearPlane;
        m_farPlane = farPlane;
        UpdateProjectionMatrix();
    }

    void Camera::Orbit(const Math::Vec3& target, float deltaYaw, float deltaPitch, float distance) {
        m_target = target;
        
        // Create rotation from yaw and pitch
        Math::Quat yawRotation = glm::angleAxis(deltaYaw, Math::Vec3(0, 1, 0));
        Math::Quat pitchRotation = glm::angleAxis(deltaPitch, Math::Vec3(1, 0, 0));
        m_rotation = yawRotation * pitchRotation;
        
        // Position camera at distance from target
        Math::Vec3 offset = m_rotation * Math::Vec3(0, 0, distance);
        m_position = target + offset;
        
        UpdateViewMatrix();
    }

    void Camera::UpdateViewMatrix() {
        Math::Mat4 translation = glm::translate(Math::Mat4(1.0f), -m_position);
        Math::Mat4 rotation = glm::mat4_cast(glm::conjugate(m_rotation));
        m_viewMatrix = rotation * translation;
    }

    void Camera::UpdateProjectionMatrix() {
        if (m_type == CameraType::Perspective) {
            m_projectionMatrix = Math::CreatePerspectiveMatrix(m_fov, m_aspect, m_nearPlane, m_farPlane);
        } else {
            m_projectionMatrix = Math::CreateOrthographicMatrix(m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane);
        }
    }
}