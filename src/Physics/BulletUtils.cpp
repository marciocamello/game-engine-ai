#include "Physics/BulletUtils.h"

#ifdef GAMEENGINE_HAS_BULLET

#include <glm/gtc/type_ptr.hpp>

namespace GameEngine {
    namespace Physics {
        namespace BulletUtils {
            btVector3 ToBullet(const Math::Vec3& vec) {
                return btVector3(vec.x, vec.y, vec.z);
            }

            btQuaternion ToBullet(const Math::Quat& quat) {
                return btQuaternion(quat.x, quat.y, quat.z, quat.w);
            }

            Math::Vec3 FromBullet(const btVector3& vec) {
                return Math::Vec3(vec.getX(), vec.getY(), vec.getZ());
            }

            Math::Quat FromBullet(const btQuaternion& quat) {
                return Math::Quat(quat.getW(), quat.getX(), quat.getY(), quat.getZ());
            }

            btTransform ToBullet(const Math::Vec3& position, const Math::Quat& rotation) {
                return btTransform(ToBullet(rotation), ToBullet(position));
            }

            void FromBullet(const btTransform& transform, Math::Vec3& position, Math::Quat& rotation) {
                position = FromBullet(transform.getOrigin());
                rotation = FromBullet(transform.getRotation());
            }

            btTransform ToBullet(const Math::Mat4& matrix) {
                btTransform transform;
                transform.setFromOpenGLMatrix(glm::value_ptr(matrix));
                return transform;
            }

            Math::Mat4 FromBullet(const btTransform& transform) {
                Math::Mat4 matrix;
                transform.getOpenGLMatrix(glm::value_ptr(matrix));
                return matrix;
            }
        }
    }
}

#endif // GAMEENGINE_HAS_BULLET