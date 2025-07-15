#pragma once

#include "Core/Math.h"

namespace GameEngine {
    class PrimitiveRenderer;
    class InputManager;
    class PhysicsEngine;

    class Character {
    public:
        Character();
        ~Character();

        bool Initialize();
        void Update(float deltaTime, InputManager* input, class ThirdPersonCameraSystem* camera = nullptr);
        void Render(PrimitiveRenderer* renderer);

        // Transform
        void SetPosition(const Math::Vec3& position) { m_position = position; }
        const Math::Vec3& GetPosition() const { return m_position; }
        
        void SetRotation(float yaw) { m_yaw = yaw; }
        float GetRotation() const { return m_yaw; }

        // Movement
        void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
        float GetMoveSpeed() const { return m_moveSpeed; }

        // Character properties
        float GetHeight() const { return m_height; }
        float GetRadius() const { return m_radius; }

    private:
        void HandleMovementInput(float deltaTime, InputManager* input, class ThirdPersonCameraSystem* camera);
        void UpdatePhysics(float deltaTime);

        // Transform
        Math::Vec3 m_position{0.0f, 0.9f, 0.0f}; // Height/2 to sit on ground (1.8/2 = 0.9)
        Math::Vec3 m_velocity{0.0f};
        float m_yaw = 0.0f;

        // Character properties (human proportions)
        float m_height = 1.8f;  // Height of a person
        float m_radius = 0.3f;  // Thicker capsule like a barrel
        float m_moveSpeed = 3.0f;  // Slower, more realistic speed
        float m_jumpSpeed = 6.0f;
        float m_gravity = -15.0f;

        // State
        bool m_isGrounded = true;
        bool m_isJumping = false;

        // Rendering
        Math::Vec4 m_color{0.2f, 0.6f, 1.0f, 1.0f}; // Blue capsule
    };
}