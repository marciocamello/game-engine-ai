#include "OpenGLGraphicsModule.h"
#include "../../include/Graphics/GraphicsRenderer.h"
#include "../core/Logger.h"

namespace GameEngine {
    namespace Graphics {
        
        OpenGLGraphicsModule::OpenGLGraphicsModule() {
            // Set default render settings
            m_renderSettings.windowWidth = 1920;
            m_renderSettings.windowHeight = 1080;
            m_renderSettings.fullscreen = false;
            m_renderSettings.vsync = true;
            m_renderSettings.msaaSamples = 4;
            m_renderSettings.api = GraphicsAPI::OpenGL;
        }

        OpenGLGraphicsModule::~OpenGLGraphicsModule() {
            Shutdown();
        }

        bool OpenGLGraphicsModule::Initialize(const ModuleConfig& config) {
            if (m_initialized) {
                LOG_WARNING("OpenGL Graphics Module already initialized");
                return true;
            }

            LOG_INFO("Initializing OpenGL Graphics Module...");

            // Parse configuration parameters
            auto it = config.parameters.find("windowWidth");
            if (it != config.parameters.end()) {
                m_renderSettings.windowWidth = std::stoi(it->second);
            }

            it = config.parameters.find("windowHeight");
            if (it != config.parameters.end()) {
                m_renderSettings.windowHeight = std::stoi(it->second);
            }

            it = config.parameters.find("fullscreen");
            if (it != config.parameters.end()) {
                m_renderSettings.fullscreen = (it->second == "true");
            }

            it = config.parameters.find("vsync");
            if (it != config.parameters.end()) {
                m_renderSettings.vsync = (it->second == "true");
            }

            it = config.parameters.find("msaaSamples");
            if (it != config.parameters.end()) {
                m_renderSettings.msaaSamples = std::stoi(it->second);
            }

            // Initialize the renderer
            if (!InitializeRenderer()) {
                LOG_ERROR("Failed to initialize OpenGL renderer");
                return false;
            }

            m_initialized = true;
            LOG_INFO("OpenGL Graphics Module initialized successfully");
            return true;
        }

        void OpenGLGraphicsModule::Update(float deltaTime) {
            if (!m_initialized || !m_enabled) {
                return;
            }

            // Graphics modules typically don't need per-frame updates
            // The renderer handles its own frame management
        }

        void OpenGLGraphicsModule::Shutdown() {
            if (!m_initialized) {
                return;
            }

            LOG_INFO("Shutting down OpenGL Graphics Module...");
            
            ShutdownRenderer();
            
            m_initialized = false;
            LOG_INFO("OpenGL Graphics Module shutdown complete");
        }

        std::vector<std::string> OpenGLGraphicsModule::GetDependencies() const {
            // Graphics module has no dependencies on other engine modules
            return {};
        }

        GraphicsRenderer* OpenGLGraphicsModule::GetRenderer() {
            return m_renderer.get();
        }

        bool OpenGLGraphicsModule::SupportsAPI(GraphicsAPI api) {
            return api == GraphicsAPI::OpenGL;
        }

        void OpenGLGraphicsModule::SetRenderSettings(const RenderSettings& settings) {
            if (settings.api != GraphicsAPI::OpenGL) {
                LOG_WARNING("OpenGL Graphics Module does not support the requested API");
                return;
            }

            m_renderSettings = settings;
            
            // If already initialized, we might need to recreate the renderer
            // For now, just log that settings have changed
            if (m_initialized) {
                LOG_INFO("Render settings updated - restart may be required for some changes");
            }
        }

        RenderSettings OpenGLGraphicsModule::GetRenderSettings() const {
            return m_renderSettings;
        }

        void* OpenGLGraphicsModule::GetWindow() {
            if (m_renderer) {
                return m_renderer->GetWindow();
            }
            return nullptr;
        }

        void OpenGLGraphicsModule::SwapBuffers() {
            if (m_renderer) {
                m_renderer->Present();
            }
        }

        bool OpenGLGraphicsModule::ShouldClose() {
            if (m_renderer) {
                // This would need to be implemented in the renderer
                // For now, return false
                return false;
            }
            return true;
        }

        bool OpenGLGraphicsModule::InitializeRenderer() {
            try {
                m_renderer = GraphicsRenderer::Create(GraphicsAPI::OpenGL);
                if (!m_renderer) {
                    LOG_ERROR("Failed to create OpenGL renderer");
                    return false;
                }

                if (!m_renderer->Initialize(m_renderSettings)) {
                    LOG_ERROR("Failed to initialize OpenGL renderer");
                    m_renderer.reset();
                    return false;
                }

                return true;
            }
            catch (const std::exception& e) {
                LOG_ERROR("Exception during renderer initialization: " + std::string(e.what()));
                m_renderer.reset();
                return false;
            }
        }

        void OpenGLGraphicsModule::ShutdownRenderer() {
            if (m_renderer) {
                // The renderer should handle its own cleanup in the destructor
                m_renderer.reset();
            }
        }
    }
}