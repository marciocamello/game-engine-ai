#pragma once

#include "Core/Math.h"

#ifdef GAMEENGINE_HAS_BULLET
#include <btBulletDynamicsCommon.h>

namespace GameEngine {
    namespace Physics {
        namespace BulletUtils {
            
            // Vec3 conversions
            btVector3 ToBullet(const Math::Vec3& vec);
            Math::Vec3 FromBullet(const btVector3& vec);
            
            // Quaternion conversions
            btQuaternion ToBullet(const Math::Quat& quat);
            Math::Quat FromBullet(const btQuaternion& quat);
            
            // Transform conversions
            btTransform ToBullet(const Math::Vec3& position, const Math::Quat& rotation);
            void FromBullet(const btTransform& transform, Math::Vec3& position, Math::Quat& rotation);
            
            // Matrix conversions (4x4 transform matrix)
            btTransform ToBullet(const Math::Mat4& matrix);
            Math::Mat4 FromBullet(const btTransform& transform);
            
        } // namespace BulletUtils
    } // namespace Physics
} // namespace GameEngine

#endif // GAMEENGINE_HAS_BULLET