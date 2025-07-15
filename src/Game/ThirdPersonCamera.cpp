#include "Game/ThirdPersonCamera.h"
#include "Game/Character.h"
#include "Input/InputManager.h"
#include "Core/Math.h"
#include "Core/Logger.h"

namespace GameEngine {
    ThirdPersonCamera::ThirdPersonCamera() : Camera(CameraType::Perspective) {
        SetPerspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
        
        // Better default values for third person
        m_distance = 8.0f;  // More distance from character
        m_heightOffset = 1.5f;  // Look at character's chest level
        m_pitch = -15.0f;  // Slight downward angle
        m_yaw = 0.0f;
        m_mouseSensitivity = 0.15f;  // Slower mouse movement
    }

    void ThirdPersonCamera::Update(float deltaTime, InputManager* input) {
        if (!m_target) return;

        HandleMouseInput(deltaTime, input);
        UpdateCameraPosition();
    }

    void ThirdPersonCamera::HandleMouseInput(float deltaTime, InputManager* input) {
        Math::Vec2 mouseDelta = input->GetMouseDelta();
        
        if (glm::length(mouseDelta) > 0.0f) {
            m_yaw += mouseDelta.x * m_mouseSensitivity;
            m_pitch -= mouseDelta.y * m_mouseSensitivity;
            
            // Clamp pitch
            m_pitch = Math::Clamp(m_pitch, m_minPitch, m_maxPitch);
            
            LOG_INFO("Camera angles - Yaw: " + std::to_string(m_yaw) + ", Pitch: " + std::to_string(m_pitch));
        }

        // Handle mouse scroll for distance
        float scrollDelta = input->GetMouseScrollDelta();
        if (scrollDelta != 0.0f) {
            m_distance -= scrollDelta * 0.5f;
            m_distance = Math::Clamp(m_distance, m_minDistance, m_maxDistance);
            LOG_INFO("Camera distance: " + std::to_string(m_distance));
        }
    }

    void ThirdPersonCamera::UpdateCameraPosition() {
        if (!m_target) return;

        // Get target position
        Math::Vec3 targetPos = m_target->GetPosition();
        targetPos.y += m_heightOffset;

        // Calculate camera position based on spherical coordinates
        float yawRad = Math::ToRadians(m_yaw);
        float pitchRad = Math::ToRadians(m_pitch);

        Math::Vec3 offset;
        offset.x = m_distance * cos(pitchRad) * sin(yawRad);
        offset.y = m_distance * sin(pitchRad);
        offset.z = m_distance * cos(pitchRad) * cos(yawRad);

        Math::Vec3 cameraPos = targetPos + offset;
        
        // Set camera transform
        SetPosition(cameraPos);
        LookAt(targetPos);
    }

    Math::Vec3 ThirdPersonCamera::GetForwardDirection() const {
        float yawRad = Math::ToRadians(m_yaw);
        return Math::Vec3(-sin(yawRad), 0.0f, -cos(yawRad));
    }

    Math::Vec3 ThirdPersonCamera::GetRightDirection() const {
        float yawRad = Math::ToRadians(m_yaw);
        return Math::Vec3(cos(yawRad), 0.0f, -sin(yawRad));
    }
}