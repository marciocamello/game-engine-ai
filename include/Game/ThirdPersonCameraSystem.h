#pragma once

#include "Graphics/Camera.h"
#include "Game/SpringArm.h"
#include "Core/Math.h"

namespace GameEngine {
    class InputManager;
    class Character;

    /**
     * Complete third-person camera system
     * Combines SpringArm + Camera to create a professional third-person camera
     */
    class ThirdPersonCameraSystem : public Camera {
    public:
        ThirdPersonCameraSystem();
        ~ThirdPersonCameraSystem() = default;

        /**
         * Updates the entire camera system
         * @param deltaTime Time elapsed since last frame
         * @param input Input manager to capture mouse movement
         */
        void Update(float deltaTime, InputManager* input);

        /**
         * Sets the target that the camera should follow
         * @param target Pointer to the target character
         */
        void SetTarget(Character* target) { m_target = target; }

        /**
         * SpringArm configuration methods
         */
        void SetArmLength(float length) { m_springArm.SetLength(length); }
        void SetRotationLimits(float minPitch, float maxPitch) { 
            m_springArm.SetRotationLimits(minPitch, maxPitch); 
        }
        void SetSensitivity(float yawSensitivity, float pitchSensitivity) {
            m_springArm.SetSensitivity(yawSensitivity, pitchSensitivity);
        }
        void SetSmoothingSpeed(float rotationSpeed, float positionSpeed) {
            m_springArm.SetSmoothingSpeed(rotationSpeed, positionSpeed);
        }

        /**
         * Behavior configuration methods
         */
        void SetFollowCameraMode(bool enabled) { m_followCameraMode = enabled; }
        void SetMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }

        /**
         * Camera information getters
         */
        Math::Vec3 GetForwardDirection() const;
        Math::Vec3 GetRightDirection() const;
        float GetCameraYaw() const { return m_springArm.GetYaw(); }

        /**
         * Gets movement direction based on camera (for character movement)
         * @param inputForward W/S input (-1 to 1)
         * @param inputRight A/D input (-1 to 1)
         * @return World movement direction
         */
        Math::Vec3 GetMovementDirection(float inputForward, float inputRight) const;

    private:
        /**
         * Processes mouse input for camera rotation
         */
        void HandleMouseInput(float deltaTime, InputManager* input);

        /**
         * Updates camera position and orientation based on SpringArm
         */
        void UpdateCameraTransform();

        /**
         * Updates character rotation to follow camera (if enabled)
         */
        void UpdateCharacterRotation();

        // Main components
        SpringArm m_springArm;          // Camera articulated arm
        Character* m_target = nullptr;   // Target character

        // Behavior settings
        bool m_followCameraMode = true;  // Whether character should rotate with camera
        float m_mouseSensitivity = 1.0f; // General mouse sensitivity

        // Internal state
        Math::Vec3 m_lastTargetPosition{0.0f}; // To detect target movement
    };
}