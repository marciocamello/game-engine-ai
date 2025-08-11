#pragma once

#include "../interfaces/IPhysicsModule.h"
#include <memory>

namespace GameEngine {
    namespace Physics {
        class PhysicsModuleFactory {
        public:
            static std::unique_ptr<IPhysicsModule> CreateModule(PhysicsAPI api);
            static std::vector<PhysicsAPI> GetSupportedAPIs();
            static const char* GetAPIName(PhysicsAPI api);
        };
    }
}