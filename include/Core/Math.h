#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace GameEngine {
    namespace Math {
        // Common types
        using Vec2 = glm::vec2;
        using Vec3 = glm::vec3;
        using Vec4 = glm::vec4;
        using Mat3 = glm::mat3;
        using Mat4 = glm::mat4;
        using Quat = glm::quat;

        // Constants
        constexpr float PI = 3.14159265359f;
        constexpr float TWO_PI = 2.0f * PI;
        constexpr float HALF_PI = PI * 0.5f;
        constexpr float DEG_TO_RAD = PI / 180.0f;
        constexpr float RAD_TO_DEG = 180.0f / PI;

        // Utility functions
        inline float ToRadians(float degrees) { return degrees * DEG_TO_RAD; }
        inline float ToDegrees(float radians) { return radians * RAD_TO_DEG; }
        
        inline float Lerp(float a, float b, float t) { return a + t * (b - a); }
        inline Vec3 Lerp(const Vec3& a, const Vec3& b, float t) { return glm::mix(a, b, t); }
        
        inline float Clamp(float value, float min, float max) { return glm::clamp(value, min, max); }
        inline Vec3 Clamp(const Vec3& value, const Vec3& min, const Vec3& max) { return glm::clamp(value, min, max); }

        // Transform utilities
        Mat4 CreateTransform(const Vec3& position, const Quat& rotation, const Vec3& scale);
        Mat4 CreateViewMatrix(const Vec3& position, const Vec3& target, const Vec3& up);
        Mat4 CreatePerspectiveMatrix(float fov, float aspect, float nearPlane, float farPlane);
        Mat4 CreateOrthographicMatrix(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    }
}