#pragma once

#include "IEngineModule.h"

// Forward declarations
namespace GameEngine {
    class GraphicsRenderer;
    enum class GraphicsAPI;
    struct RenderSettings;
    
    namespace Graphics {
        class IGraphicsModule : public IEngineModule {
        public:
            virtual ~IGraphicsModule() = default;
            
            // Graphics-specific methods
            virtual GraphicsRenderer* GetRenderer() = 0;
            virtual bool SupportsAPI(GraphicsAPI api) = 0;
            virtual void SetRenderSettings(const RenderSettings& settings) = 0;
            virtual RenderSettings GetRenderSettings() const = 0;
            
            // Window management
            virtual void* GetWindow() = 0;
            virtual void SwapBuffers() = 0;
            virtual bool ShouldClose() = 0;
        };
    }
}