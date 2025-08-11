#pragma once

#include "../interfaces/IGraphicsModule.h"
#include "../../include/Graphics/GraphicsRenderer.h"
#include <memory>

namespace GameEngine {
    namespace Graphics {
        class OpenGLGraphicsModule : public IGraphicsModule {
        public:
            OpenGLGraphicsModule();
            virtual ~OpenGLGraphicsModule();

            // IEngineModule interface
            bool Initialize(const ModuleConfig& config) override;
            void Update(float deltaTime) override;
            void Shutdown() override;
            
            const char* GetName() const override { return "OpenGLGraphics"; }
            const char* GetVersion() const override { return "1.0.0"; }
            ModuleType GetType() const override { return ModuleType::Graphics; }
            std::vector<std::string> GetDependencies() const override;
            
            bool IsInitialized() const override { return m_initialized; }
            bool IsEnabled() const override { return m_enabled; }
            void SetEnabled(bool enabled) override { m_enabled = enabled; }

            // IGraphicsModule interface
            GraphicsRenderer* GetRenderer() override;
            bool SupportsAPI(GraphicsAPI api) override;
            void SetRenderSettings(const RenderSettings& settings) override;
            RenderSettings GetRenderSettings() const override;
            
            // Window management
            void* GetWindow() override;
            void SwapBuffers() override;
            bool ShouldClose() override;

        private:
            bool InitializeRenderer();
            void ShutdownRenderer();
            
            std::unique_ptr<GraphicsRenderer> m_renderer;
            RenderSettings m_renderSettings;
            bool m_initialized = false;
            bool m_enabled = true;
        };
    }
}