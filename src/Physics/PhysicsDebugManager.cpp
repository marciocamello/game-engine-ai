#include "Physics/PhysicsDebugManager.h"
#include "Physics/PhysicsEngine.h"
#include "Input/InputManager.h"
#include "Core/Logger.h"

namespace GameEngine {
    namespace Physics {
        
        PhysicsDebugManager::PhysicsDebugManager() {
        }
        
        PhysicsDebugManager::~PhysicsDebugManager() {
            Shutdown();
        }
        
        bool PhysicsDebugManager::Initialize(PhysicsEngine* physicsEngine, InputManager* inputManager) {
            if (m_initialized) {
                LOG_WARNING("PhysicsDebugManager already initialized");
                return true;
            }
            
            if (!physicsEngine) {
                LOG_ERROR("PhysicsEngine is null");
                return false;
            }
            
            if (!inputManager) {
                LOG_ERROR("InputManager is null");
                return false;
            }
            
            m_physicsEngine = physicsEngine;
            m_inputManager = inputManager;
            
            // Create debug renderer
            m_debugRenderer = std::make_shared<PhysicsDebugRenderer>();
            if (!m_debugRenderer->Initialize()) {
                LOG_ERROR("Failed to initialize PhysicsDebugRenderer");
                return false;
            }
            
            // Set up input bindings
            UpdateInputBindings();
            
            // Connect debug renderer to physics engine
            m_physicsEngine->SetDebugDrawer(m_debugRenderer);
            
            m_initialized = true;
            LOG_INFO("PhysicsDebugManager initialized successfully");
            return true;
        }
        
        void PhysicsDebugManager::Shutdown() {
            if (!m_initialized) {
                return;
            }
            
            // Disconnect from physics engine
            if (m_physicsEngine) {
                m_physicsEngine->SetDebugDrawer(nullptr);
                m_physicsEngine->EnableDebugDrawing(false);
            }
            
            // Cleanup debug renderer
            if (m_debugRenderer) {
                m_debugRenderer->Shutdown();
                m_debugRenderer.reset();
            }
            
            m_physicsEngine = nullptr;
            m_inputManager = nullptr;
            m_initialized = false;
            
            LOG_INFO("PhysicsDebugManager shut down");
        }
        
        void PhysicsDebugManager::SetConfig(const PhysicsDebugConfig& config) {
            if (m_debugRenderer) {
                m_debugRenderer->SetConfig(config);
            }
        }
        
        const PhysicsDebugConfig& PhysicsDebugManager::GetConfig() const {
            if (m_debugRenderer) {
                return m_debugRenderer->GetConfig();
            }
            
            static PhysicsDebugConfig defaultConfig = PhysicsDebugConfig::Default();
            return defaultConfig;
        }
        
        void PhysicsDebugManager::SetCamera(const Camera* camera) {
            if (m_debugRenderer) {
                m_debugRenderer->SetCamera(camera);
            }
        }
        
        void PhysicsDebugManager::SetDebugMode(PhysicsDebugMode mode) {
            if (!m_initialized || !m_physicsEngine) {
                return;
            }
            
            m_currentMode = mode;
            m_physicsEngine->SetDebugMode(mode);
            
            // Enable/disable debug drawing based on mode
            bool shouldEnable = (mode != PhysicsDebugMode::None) && m_enabled;
            m_physicsEngine->EnableDebugDrawing(shouldEnable);
            
            OnDebugModeChanged();
        }
        
        PhysicsDebugMode PhysicsDebugManager::GetDebugMode() const {
            return m_currentMode;
        }
        
        void PhysicsDebugManager::ToggleDebugMode() {
            PhysicsDebugMode newMode;
            
            switch (m_currentMode) {
                case PhysicsDebugMode::None:
                    newMode = PhysicsDebugMode::Wireframe;
                    break;
                case PhysicsDebugMode::Wireframe:
                    newMode = PhysicsDebugMode::AABB;
                    break;
                case PhysicsDebugMode::AABB:
                    newMode = PhysicsDebugMode::ContactPoints;
                    break;
                case PhysicsDebugMode::ContactPoints:
                    newMode = PhysicsDebugMode::All;
                    break;
                case PhysicsDebugMode::All:
                default:
                    newMode = PhysicsDebugMode::None;
                    break;
            }
            
            SetDebugMode(newMode);
        }
        
        void PhysicsDebugManager::SetEnabled(bool enabled) {
            m_enabled = enabled;
            
            if (m_physicsEngine) {
                bool shouldEnable = enabled && (m_currentMode != PhysicsDebugMode::None);
                m_physicsEngine->EnableDebugDrawing(shouldEnable);
            }
        }
        
        bool PhysicsDebugManager::IsEnabled() const {
            return m_enabled;
        }
        
        void PhysicsDebugManager::BeginFrame() {
            if (!m_initialized || !m_enabled || !m_debugRenderer) {
                return;
            }
            
            m_debugRenderer->BeginFrame();
        }
        
        void PhysicsDebugManager::EndFrame() {
            if (!m_initialized || !m_enabled || !m_debugRenderer) {
                return;
            }
            
            m_debugRenderer->EndFrame();
        }
        
        void PhysicsDebugManager::Render() {
            if (!m_initialized || !m_enabled || !m_debugRenderer) {
                return;
            }
            
            // Physics engine will call debug drawer during DrawDebugWorld()
            if (m_physicsEngine && m_currentMode != PhysicsDebugMode::None) {
                m_physicsEngine->DrawDebugWorld();
            }
        }
        
        void PhysicsDebugManager::HandleInput() {
            if (!m_initialized || !m_inputManager) {
                return;
            }
            
            // Check for debug toggle key (J)
            bool currentDebugKeyState = m_inputManager->IsKeyDown(KeyCode::J);
            
            // Detect key press (not held)
            if (currentDebugKeyState && !m_lastDebugKeyState) {
                OnDebugTogglePressed();
            }
            
            m_lastDebugKeyState = currentDebugKeyState;
        }
        
        const PhysicsDebugRenderer::RenderStats& PhysicsDebugManager::GetRenderStats() const {
            if (m_debugRenderer) {
                return m_debugRenderer->GetRenderStats();
            }
            
            static PhysicsDebugRenderer::RenderStats emptyStats;
            return emptyStats;
        }
        
        void PhysicsDebugManager::PrintDebugInfo() const {
            if (!m_initialized) {
                LOG_INFO("PhysicsDebugManager not initialized");
                return;
            }
            
            std::string modeStr;
            switch (m_currentMode) {
                case PhysicsDebugMode::None: modeStr = "None"; break;
                case PhysicsDebugMode::Wireframe: modeStr = "Wireframe"; break;
                case PhysicsDebugMode::AABB: modeStr = "AABB"; break;
                case PhysicsDebugMode::ContactPoints: modeStr = "ContactPoints"; break;
                case PhysicsDebugMode::All: modeStr = "All"; break;
                default: modeStr = "Unknown"; break;
            }
            
            LOG_INFO("Physics Debug Info:");
            LOG_INFO("  Enabled: " + std::string(m_enabled ? "Yes" : "No"));
            LOG_INFO("  Mode: " + modeStr);
            
            if (m_debugRenderer) {
                const auto& stats = m_debugRenderer->GetRenderStats();
                LOG_INFO("  Render Stats:");
                LOG_INFO("    Lines: " + std::to_string(stats.linesRendered));
                LOG_INFO("    Spheres: " + std::to_string(stats.spheresRendered));
                LOG_INFO("    Boxes: " + std::to_string(stats.boxesRendered));
                LOG_INFO("    Capsules: " + std::to_string(stats.capsulesRendered));
                LOG_INFO("    Contact Points: " + std::to_string(stats.contactPointsRendered));
                LOG_INFO("    Text Items: " + std::to_string(stats.textItemsRendered));
                LOG_INFO("    Total Vertices: " + std::to_string(stats.totalVertices));
                LOG_INFO("    Draw Calls: " + std::to_string(stats.drawCalls));
                LOG_INFO("    Render Time: " + std::to_string(stats.renderTime) + "ms");
            }
        }
        
        void PhysicsDebugManager::UpdateInputBindings() {
            if (!m_inputManager) {
                return;
            }
            
            // Bind the 'J' key for debug toggle
            m_inputManager->BindAction("debug_toggle", KeyCode::J);
        }
        
        void PhysicsDebugManager::OnDebugTogglePressed() {
            if (!m_enabled) {
                // Enable debug rendering and set to wireframe mode
                SetEnabled(true);
                SetDebugMode(PhysicsDebugMode::Wireframe);
                LOG_INFO("Physics debug rendering enabled (Wireframe mode)");
            } else {
                // Cycle through debug modes or disable
                ToggleDebugMode();
                
                std::string modeStr;
                switch (m_currentMode) {
                    case PhysicsDebugMode::None: 
                        modeStr = "disabled";
                        SetEnabled(false);
                        break;
                    case PhysicsDebugMode::Wireframe: modeStr = "Wireframe"; break;
                    case PhysicsDebugMode::AABB: modeStr = "AABB"; break;
                    case PhysicsDebugMode::ContactPoints: modeStr = "Contact Points"; break;
                    case PhysicsDebugMode::All: modeStr = "All"; break;
                    default: modeStr = "Unknown"; break;
                }
                
                LOG_INFO("Physics debug mode: " + modeStr);
            }
        }
        
        void PhysicsDebugManager::OnDebugModeChanged() {
            // Update debug renderer configuration based on mode
            if (!m_debugRenderer) {
                return;
            }
            
            PhysicsDebugConfig config = m_debugRenderer->GetConfig();
            
            // Adjust colors and settings based on debug mode
            switch (m_currentMode) {
                case PhysicsDebugMode::Wireframe:
                    config.wireframeColor = Math::Vec3(0.0f, 1.0f, 0.0f); // Green
                    config.lineWidth = 1.5f;
                    break;
                case PhysicsDebugMode::AABB:
                    config.aabbColor = Math::Vec3(1.0f, 1.0f, 0.0f); // Yellow
                    config.lineWidth = 2.0f;
                    break;
                case PhysicsDebugMode::ContactPoints:
                    config.contactColor = Math::Vec3(1.0f, 0.0f, 0.0f); // Red
                    config.contactPointSize = 0.1f;
                    config.contactNormalLength = 0.3f;
                    break;
                case PhysicsDebugMode::All:
                    config.lineWidth = 1.0f;
                    config.alpha = 0.7f; // More transparent when showing everything
                    break;
                default:
                    break;
            }
            
            m_debugRenderer->SetConfig(config);
        }
        
    } // namespace Physics
} // namespace GameEngine