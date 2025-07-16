#include "Physics/BulletUtils.h"

#ifdef GAMEENGINE_HAS_BULLET

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
        }
    }
}

#endif // GAMEENGINE_HAS_BULLET