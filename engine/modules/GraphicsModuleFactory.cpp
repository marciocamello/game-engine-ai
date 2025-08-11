#include "GraphicsModuleFactory.h"
#include "OpenGLGraphicsModule.h"
#include "../core/Logger.h"
#include <algorithm>

namespace GameEngine {
    namespace Graphics {
        
        std::unique_ptr<IGraphicsModule> GraphicsModuleFactory::CreateModule(GraphicsAPI api) {
            switch (api) {
                case GraphicsAPI::OpenGL:
                    LOG_INFO("Creating OpenGL Graphics Module");
                    return std::make_unique<OpenGLGraphicsModule>();
                    
                case GraphicsAPI::Vulkan:
                    LOG_WARNING("Vulkan Graphics Module not yet implemented");
                    return nullptr;
                    
                default:
                    LOG_ERROR("Unknown graphics API requested");
                    return nullptr;
            }
        }

        std::vector<GraphicsAPI> GraphicsModuleFactory::GetSupportedAPIs() {
            return {
                GraphicsAPI::OpenGL
                // Add other APIs as they are implemented
            };
        }

        bool GraphicsModuleFactory::IsAPISupported(GraphicsAPI api) {
            auto supportedAPIs = GetSupportedAPIs();
            return std::find(supportedAPIs.begin(), supportedAPIs.end(), api) != supportedAPIs.end();
        }
    }
}