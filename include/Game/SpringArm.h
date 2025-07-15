#pragma once

#include "Core/Math.h"

namespace GameEngine {
    /**
     * SpringArm Component - Articulated arm for third-person camera
     * Based on standard game industry practices
     * 
     * Coordinate system:
     * - X: Forward/Backward
     * - Y: Left/Right  
     * - Z: Up/Down
     */
    class SpringArm {
    public:
        SpringArm();
        ~SpringArm() = default;

        /**
         * Updates the SpringArm position and rotation
         * @param deltaTime Time elapsed since last frame
         * @param targetPosition Character/target position
         * @param inputYaw Horizontal mouse input (horizontal rotation)
         * @param inputPitch Vertical mouse input (vertical rotation)
         */
        void Update(float deltaTime, const Math::Vec3& targetPosition, float inputYaw, float inputPitch);

        /**
         * Calculates the final camera position based on rotation and distance
         * @return Position where the camera should be placed
         */
        Math::Vec3 GetCameraPosition() const;

        /**
         * Gets the direction the camera should look at (always towards target)
         * @return Normalized direction vector
         */
        Math::Vec3 GetViewDirection() const;

        // SpringArm configuration methods
        void SetLength(float length) { m_targetLength = length; }
        void SetRotationLimits(float minPitch, float maxPitch) { 
            m_minPitch = minPitch; 
            m_maxPitch = maxPitch; 
        }
        void SetSensitivity(float yawSensitivity, float pitchSensitivity) {
            m_yawSensitivity = yawSensitivity;
            m_pitchSensitivity = pitchSensitivity;
        }
        void SetSmoothingSpeed(float rotationSpeed, float positionSpeed) {
            m_rotationSmoothingSpeed = rotationSpeed;
            m_positionSmoothingSpeed = positionSpeed;
        }

        // Getters
        float GetLength() const { return m_currentLength; }
        float GetYaw() const { return m_currentYaw; }
        float GetPitch() const { return m_currentPitch; }
        const Math::Vec3& GetTargetPosition() const { return m_targetPosition; }

    private:
        /**
         * Applies smoothing to rotation using interpolation
         */
        void SmoothRotation(float deltaTime);

        /**
         * Applies smoothing to position/length
         */
        void SmoothPosition(float deltaTime);

        /**
         * Checks for collisions and adjusts arm length
         * TODO: Implement raycast for collision detection
         */
        void CheckCollisions();

        /**
         * Calculates camera offset based on current rotation
         */
        Math::Vec3 CalculateCameraOffset() const;

        // Target position (character)
        Math::Vec3 m_targetPosition{0.0f};

        // Current rotation (smoothed)
        float m_currentYaw = 0.0f;      // Horizontal rotation in degrees
        float m_currentPitch = 20.0f;   // Vertical rotation in degrees (positive = looking down)

        // Desired rotation (player input)
        float m_targetYaw = 0.0f;
        float m_targetPitch = 20.0f;

        // Arm length
        float m_currentLength = 8.0f;   // Current length (smoothed)
        float m_targetLength = 8.0f;    // Desired length

        // Rotation limits
        float m_minPitch = -30.0f;      // Lower limit (looking up)
        float m_maxPitch = 80.0f;       // Upper limit (looking down - prevent ground clipping)

        // Input sensitivity
        float m_yawSensitivity = 1.0f;
        float m_pitchSensitivity = 1.0f;

        // Smoothing speeds
        float m_rotationSmoothingSpeed = 8.0f;  // Rotation interpolation speed
        float m_positionSmoothingSpeed = 10.0f; // Position interpolation speed

        // Calculated camera offset
        mutable Math::Vec3 m_cameraOffset{0.0f};
    };
}