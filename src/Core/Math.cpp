#include "Core/Math.h"

namespace GameEngine {
    namespace Math {
        Mat4 CreateTransform(const Vec3& position, const Quat& rotation, const Vec3& scale) {
            Mat4 translation = glm::translate(Mat4(1.0f), position);
            Mat4 rotationMat = glm::mat4_cast(rotation);
            Mat4 scaleMat = glm::scale(Mat4(1.0f), scale);
            return translation * rotationMat * scaleMat;
        }

        Mat4 CreateViewMatrix(const Vec3& position, const Vec3& target, const Vec3& up) {
            return glm::lookAt(position, target, up);
        }

        Mat4 CreatePerspectiveMatrix(float fov, float aspect, float nearPlane, float farPlane) {
            return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
        }

        Mat4 CreateOrthographicMatrix(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
            return glm::ortho(left, right, bottom, top, nearPlane, farPlane);
        }
    }
}