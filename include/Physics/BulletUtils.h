#pragma once

#ifdef GAMEENGINE_HAS_BULLET

#include "Core/Math.h"
#include <btBulletDynamicsCommon.h>

namespace GameEngine {
    namespace Physics {
        namespace BulletUtils {
            // Convert from engine math types to Bullet types
            btVector3 ToBullet(const Math::Vec3& vec);
            btQuaternion ToBullet(const Math::Quat& quat);
            
            // Convert from Bullet types to engine math types
            Math::Vec3 FromBullet(const btVector3& vec);
            Math::Quat FromBullet(const btQuaternion& quat);
        }
    }
}

#endif // GAMEENGINE_HAS_BULLET