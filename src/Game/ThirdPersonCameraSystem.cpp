#include "Game/ThirdPersonCameraSystem.h"
#include "Game/Character.h"
#include "Input/InputManager.h"
#include "Core/Logger.h"

namespace GameEngine {
    ThirdPersonCameraSystem::ThirdPersonCameraSystem() : Camera(CameraType::Perspective) {
        // Initialize camera with standard perspective settings
        SetPerspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
        
        // Configure SpringArm with good default values
        m_springArm.SetLength(10.0f);  // Good distance from character
        m_springArm.SetRotationLimits(-45.0f, 75.0f);  // Proper pitch limits: -45° up, 75° down
        m_springArm.SetSensitivity(2.0f, 1.0f);  // Higher yaw sensitivity for 360° rotation
        m_springArm.SetSmoothingSpeed(6.0f, 8.0f);  // Smooth but responsive
        
        // Set mouse sensitivity
        m_mouseSensitivity = 2.0f;  // Better sensitivity for responsive control
        
        LOG_INFO("ThirdPersonCameraSystem initialized");
    }

    void ThirdPersonCameraSystem::Update(float deltaTime, InputManager* input) {
        if (!m_target) {
            LOG_WARNING("ThirdPersonCameraSystem: No target set");
            return;
        }

        // Handle mouse input for camera rotation
        HandleMouseInput(deltaTime, input);
        
        // Update SpringArm with target position and input
        Math::Vec3 targetPos = m_target->GetPosition();
        m_springArm.Update(deltaTime, targetPos, 0.0f, 0.0f);  // Input handled separately
        
        // Update camera transform based on SpringArm
        UpdateCameraTransform();
        
        // Update character rotation if follow mode is enabled
        if (m_followCameraMode) {
            UpdateCharacterRotation();
        }
        
        // Store target position for next frame
        m_lastTargetPosition = targetPos;
    }

    Math::Vec3 ThirdPersonCameraSystem::GetForwardDirection() const {
        // Get forward direction based on camera yaw (ignore pitch for movement)
        // Forward is the direction the camera is looking at (towards target)
        float yawRad = Math::ToRadians(m_springArm.GetYaw());
        return Math::Vec3(-sin(yawRad), 0.0f, -cos(yawRad));
    }

    Math::Vec3 ThirdPersonCameraSystem::GetRightDirection() const {
        // Get right direction based on camera yaw (90 degrees from forward)
        float yawRad = Math::ToRadians(m_springArm.GetYaw());
        return Math::Vec3(cos(yawRad), 0.0f, -sin(yawRad));
    }

    Math::Vec3 ThirdPersonCameraSystem::GetMovementDirection(float inputForward, float inputRight) const {
        Math::Vec3 forward = GetForwardDirection();
        Math::Vec3 right = GetRightDirection();
        
        Math::Vec3 movement = forward * inputForward + right * inputRight;
        
        // Normalize if there's any movement
        if (glm::length(movement) > 0.0f) {
            movement = glm::normalize(movement);
        }
        
        return movement;
    }

    void ThirdPersonCameraSystem::HandleMouseInput(float deltaTime, InputManager* input) {
        Math::Vec2 mouseDelta = input->GetMouseDelta();
        
        if (glm::length(mouseDelta) > 0.0f) {
            // Apply mouse sensitivity (fix inverted horizontal movement)
            float yawInput = -mouseDelta.x * m_mouseSensitivity;  // Invert X for correct left/right
            float pitchInput = -mouseDelta.y * m_mouseSensitivity;  // Invert Y for natural feel
            
            // Update SpringArm directly with processed input
            Math::Vec3 targetPos = m_target ? m_target->GetPosition() : Math::Vec3(0.0f);
            m_springArm.Update(deltaTime, targetPos, yawInput, pitchInput);
        }
        
        // Handle mouse scroll for distance adjustment
        float scrollDelta = input->GetMouseScrollDelta();
        if (scrollDelta != 0.0f) {
            float currentLength = m_springArm.GetLength();
            float newLength = currentLength - scrollDelta * 2.0f;  // Scroll in = closer, increased sensitivity
            newLength = Math::Clamp(newLength, 2.0f, 25.0f);  // Better zoom limits
            m_springArm.SetLength(newLength);
            LOG_INFO("Scroll detected: " + std::to_string(scrollDelta) + ", New length: " + std::to_string(newLength));
        }
    }

    void ThirdPersonCameraSystem::UpdateCameraTransform() {
        // Get camera position from SpringArm
        Math::Vec3 cameraPos = m_springArm.GetCameraPosition();
        Math::Vec3 targetPos = m_springArm.GetTargetPosition();
        
        // Set camera position
        SetPosition(cameraPos);
        
        // Make camera look at target
        LookAt(targetPos);
    }

    void ThirdPersonCameraSystem::UpdateCharacterRotation() {
        if (!m_target) return;
        
        // Only rotate character when moving
        // This prevents the character from constantly spinning with camera movement
        // The character movement system should handle this based on movement input
        
        // For now, we'll let the Character class handle its own rotation
        // based on movement direction in HandleMovementInput
    }
}