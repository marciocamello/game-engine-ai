#pragma once

#include "../interfaces/IGraphicsModule.h"
#include <memory>

namespace GameEngine {
    namespace Graphics {
        class GraphicsModuleFactory {
        public:
            static std::unique_ptr<IGraphicsModule> CreateModule(GraphicsAPI api);
            static std::vector<GraphicsAPI> GetSupportedAPIs();
            static bool IsAPISupported(GraphicsAPI api);
        };
    }
}