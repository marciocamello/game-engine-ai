#include "PhysicsModuleFactory.h"
#include "BulletPhysicsModule.h"
#include "../core/Logger.h"

namespace GameEngine {
    namespace Physics {
        
        std::unique_ptr<IPhysicsModule> PhysicsModuleFactory::CreateModule(PhysicsAPI api) {
            switch (api) {
                case PhysicsAPI::Bullet:
#ifdef GAMEENGINE_HAS_BULLET
                    LOG_INFO("Creating Bullet Physics module");
                    return std::make_unique<BulletPhysicsModule>();
#else
                    LOG_ERROR("Bullet Physics not available - engine compiled without Bullet support");
                    return nullptr;
#endif
                
                case PhysicsAPI::PhysX:
                    LOG_ERROR("PhysX Physics module not implemented yet");
                    return nullptr;
                
                default:
                    LOG_ERROR("Unknown physics API requested");
                    return nullptr;
            }
        }

        std::vector<PhysicsAPI> PhysicsModuleFactory::GetSupportedAPIs() {
            std::vector<PhysicsAPI> apis;
            
#ifdef GAMEENGINE_HAS_BULLET
            apis.push_back(PhysicsAPI::Bullet);
#endif
            
            // PhysX would be added here when implemented
            // #ifdef GAMEENGINE_HAS_PHYSX
            //     apis.push_back(PhysicsAPI::PhysX);
            // #endif
            
            return apis;
        }

        const char* PhysicsModuleFactory::GetAPIName(PhysicsAPI api) {
            switch (api) {
                case PhysicsAPI::Bullet:
                    return "Bullet Physics";
                case PhysicsAPI::PhysX:
                    return "NVIDIA PhysX";
                default:
                    return "Unknown";
            }
        }
    }
}