#include "Game/SpringArm.h"
#include "Core/Logger.h"
#include <glm/gtc/constants.hpp>

namespace GameEngine {
    SpringArm::SpringArm() {
        LOG_INFO("SpringArm initialized with default settings");
    }

    void SpringArm::Update(float deltaTime, const Math::Vec3& targetPosition, float inputYaw, float inputPitch) {
        // Update target position
        m_targetPosition = targetPosition;

        // Apply input to desired rotation
        m_targetYaw += inputYaw * m_yawSensitivity;
        m_targetPitch += inputPitch * m_pitchSensitivity;

        // Apply pitch limits
        m_targetPitch = Math::Clamp(m_targetPitch, m_minPitch, m_maxPitch);

        // Normalize yaw to avoid accumulating very large values
        while (m_targetYaw > 360.0f) m_targetYaw -= 360.0f;
        while (m_targetYaw < 0.0f) m_targetYaw += 360.0f;

        // Apply smoothing
        SmoothRotation(deltaTime);
        SmoothPosition(deltaTime);

        // Check collisions (placeholder for now)
        CheckCollisions();
    }

    Math::Vec3 SpringArm::GetCameraPosition() const {
        return m_targetPosition + CalculateCameraOffset();
    }

    Math::Vec3 SpringArm::GetViewDirection() const {
        Math::Vec3 cameraPos = GetCameraPosition();
        Math::Vec3 direction = glm::normalize(m_targetPosition - cameraPos);
        return direction;
    }

    void SpringArm::SmoothRotation(float deltaTime) {
        // Smoothed linear interpolation for rotation
        float rotationSpeed = m_rotationSmoothingSpeed * deltaTime;
        
        // Smooth yaw
        float yawDiff = m_targetYaw - m_currentYaw;
        
        // Handle 0°-360° transition (choose shortest path)
        if (yawDiff > 180.0f) yawDiff -= 360.0f;
        if (yawDiff < -180.0f) yawDiff += 360.0f;
        
        m_currentYaw += yawDiff * rotationSpeed;
        
        // Normalize current yaw
        while (m_currentYaw > 360.0f) m_currentYaw -= 360.0f;
        while (m_currentYaw < 0.0f) m_currentYaw += 360.0f;

        // Smooth pitch
        float pitchDiff = m_targetPitch - m_currentPitch;
        m_currentPitch += pitchDiff * rotationSpeed;
    }

    void SpringArm::SmoothPosition(float deltaTime) {
        // Smoothed linear interpolation for length
        float positionSpeed = m_positionSmoothingSpeed * deltaTime;
        float lengthDiff = m_targetLength - m_currentLength;
        m_currentLength += lengthDiff * positionSpeed;
    }

    void SpringArm::CheckCollisions() {
        // TODO: Implement raycast for collision detection
        // For now, just maintain current length
        // 
        // Pseudocode for future implementation:
        // 1. Raycast from target position to desired camera position
        // 2. If collision detected, adjust m_currentLength to collision distance
        // 3. Apply small offset to prevent camera from sticking to walls
    }

    Math::Vec3 SpringArm::CalculateCameraOffset() const {
        // Convert angles to radians
        float yawRad = Math::ToRadians(m_currentYaw);
        float pitchRad = Math::ToRadians(m_currentPitch);

        // Calculate camera position using spherical coordinates
        // Standard third-person camera positioning
        Math::Vec3 offset;
        offset.x = m_currentLength * cos(pitchRad) * sin(yawRad);   // Left/Right
        offset.y = m_currentLength * sin(pitchRad);                 // Up/Down (positive pitch = camera above)
        offset.z = m_currentLength * cos(pitchRad) * cos(yawRad);   // Forward/Back (positive = camera behind)

        return offset;
    }
}