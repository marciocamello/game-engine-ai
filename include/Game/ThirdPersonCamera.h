#pragma once

#include "Graphics/Camera.h"

namespace GameEngine {
    class Character;
    class InputManager;

    class ThirdPersonCamera : public Camera {
    public:
        ThirdPersonCamera();
        ~ThirdPersonCamera() = default;

        void SetTarget(Character* target) { m_target = target; }
        void Update(float deltaTime, InputManager* input);

        // Camera settings
        void SetDistance(float distance) { m_distance = distance; }
        void SetHeight(float height) { m_heightOffset = height; }
        void SetSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }

        float GetDistance() const { return m_distance; }
        float GetHeight() const { return m_heightOffset; }
        
        // Get camera direction for character movement
        Math::Vec3 GetForwardDirection() const;
        Math::Vec3 GetRightDirection() const;
        float GetYaw() const { return m_yaw; }

    private:
        void HandleMouseInput(float deltaTime, InputManager* input);
        void UpdateCameraPosition();

        Character* m_target = nullptr;
        
        // Camera parameters
        float m_distance = 4.0f;
        float m_heightOffset = 1.5f;
        float m_mouseSensitivity = 0.05f;  // Very low sensitivity
        
        // Camera angles
        float m_yaw = 0.0f;
        float m_pitch = 20.0f;
        
        // Constraints
        float m_minPitch = -30.0f;
        float m_maxPitch = 60.0f;
        float m_minDistance = 2.0f;
        float m_maxDistance = 10.0f;
    };
}