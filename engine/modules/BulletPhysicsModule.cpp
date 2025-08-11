#include "BulletPhysicsModule.h"
#include "../core/Logger.h"

namespace GameEngine {
    namespace Physics {
        
        BulletPhysicsModule::BulletPhysicsModule() {
            // Set default physics settings
            m_physicsSettings.api = PhysicsAPI::Bullet;
            m_physicsSettings.configuration = PhysicsConfiguration::Default();
            m_physicsSettings.enableDebugDrawing = false;
            m_physicsSettings.enableCCD = true;
            m_physicsSettings.maxRigidBodies = 10000;
            m_physicsSettings.maxGhostObjects = 1000;
        }

        BulletPhysicsModule::~BulletPhysicsModule() {
            Shutdown();
        }

        bool BulletPhysicsModule::Initialize(const ModuleConfig& config) {
            if (m_initialized) {
                LOG_WARNING("Bullet Physics Module already initialized");
                return true;
            }

            LOG_INFO("Initializing Bullet Physics Module...");

            // Parse configuration parameters
            auto it = config.parameters.find("gravity_x");
            if (it != config.parameters.end()) {
                m_physicsSettings.configuration.gravity.x = std::stof(it->second);
            }

            it = config.parameters.find("gravity_y");
            if (it != config.parameters.end()) {
                m_physicsSettings.configuration.gravity.y = std::stof(it->second);
            }

            it = config.parameters.find("gravity_z");
            if (it != config.parameters.end()) {
                m_physicsSettings.configuration.gravity.z = std::stof(it->second);
            }

            it = config.parameters.find("timeStep");
            if (it != config.parameters.end()) {
                m_physicsSettings.configuration.timeStep = std::stof(it->second);
            }

            it = config.parameters.find("maxSubSteps");
            if (it != config.parameters.end()) {
                m_physicsSettings.configuration.maxSubSteps = std::stoi(it->second);
            }

            it = config.parameters.find("solverIterations");
            if (it != config.parameters.end()) {
                m_physicsSettings.configuration.solverIterations = std::stoi(it->second);
            }

            it = config.parameters.find("enableCCD");
            if (it != config.parameters.end()) {
                m_physicsSettings.enableCCD = (it->second == "true");
                m_physicsSettings.configuration.enableCCD = m_physicsSettings.enableCCD;
            }

            it = config.parameters.find("enableDebugDrawing");
            if (it != config.parameters.end()) {
                m_physicsSettings.enableDebugDrawing = (it->second == "true");
            }

            it = config.parameters.find("maxRigidBodies");
            if (it != config.parameters.end()) {
                m_physicsSettings.maxRigidBodies = std::stoi(it->second);
            }

            it = config.parameters.find("maxGhostObjects");
            if (it != config.parameters.end()) {
                m_physicsSettings.maxGhostObjects = std::stoi(it->second);
            }

            it = config.parameters.find("linearDamping");
            if (it != config.parameters.end()) {
                m_physicsSettings.configuration.linearDamping = std::stof(it->second);
            }

            it = config.parameters.find("angularDamping");
            if (it != config.parameters.end()) {
                m_physicsSettings.configuration.angularDamping = std::stof(it->second);
            }

            // Initialize the physics engine
            if (!InitializePhysicsEngine()) {
                LOG_ERROR("Failed to initialize Bullet Physics engine");
                return false;
            }

            m_initialized = true;
            LOG_INFO("Bullet Physics Module initialized successfully");
            return true;
        }

        void BulletPhysicsModule::Update(float deltaTime) {
            if (!m_initialized || !m_enabled || !m_physicsEngine) {
                return;
            }

            // Update the physics simulation
            m_physicsEngine->Update(deltaTime);
        }

        void BulletPhysicsModule::Shutdown() {
            if (!m_initialized) {
                return;
            }

            LOG_INFO("Shutting down Bullet Physics Module...");
            
            ShutdownPhysicsEngine();
            
            m_initialized = false;
            LOG_INFO("Bullet Physics Module shutdown complete");
        }

        std::vector<std::string> BulletPhysicsModule::GetDependencies() const {
            // Physics module has no dependencies on other engine modules
            return {};
        }

        PhysicsEngine* BulletPhysicsModule::GetPhysicsEngine() {
            return m_physicsEngine.get();
        }

        bool BulletPhysicsModule::SupportsAPI(PhysicsAPI api) {
            return api == PhysicsAPI::Bullet;
        }

        bool BulletPhysicsModule::SupportsFeature(PhysicsFeature feature) {
            switch (feature) {
                case PhysicsFeature::RigidBodies:
                case PhysicsFeature::CharacterController:
                case PhysicsFeature::Constraints:
                case PhysicsFeature::Triggers:
                    return true;
                case PhysicsFeature::SoftBodies:
                case PhysicsFeature::Cloth:
                    return true; // Bullet supports these
                case PhysicsFeature::Fluids:
                case PhysicsFeature::Vehicles:
                    return false; // Not implemented in our wrapper yet
                default:
                    return false;
            }
        }

        void BulletPhysicsModule::SetPhysicsSettings(const PhysicsSettings& settings) {
            if (settings.api != PhysicsAPI::Bullet) {
                LOG_WARNING("Bullet Physics Module does not support the requested API");
                return;
            }

            m_physicsSettings = settings;
            
            // Apply configuration to the physics engine if already initialized
            if (m_initialized && m_physicsEngine) {
                ApplyConfiguration();
                LOG_INFO("Physics settings updated");
            }
        }

        PhysicsSettings BulletPhysicsModule::GetPhysicsSettings() const {
            return m_physicsSettings;
        }

        std::shared_ptr<PhysicsWorld> BulletPhysicsModule::CreateWorld(const PhysicsConfiguration& config) {
            if (!m_physicsEngine) {
                LOG_ERROR("Cannot create physics world: Physics engine not initialized");
                return nullptr;
            }

            return m_physicsEngine->CreateWorld(config);
        }

        void BulletPhysicsModule::SetActiveWorld(std::shared_ptr<PhysicsWorld> world) {
            if (!m_physicsEngine) {
                LOG_ERROR("Cannot set active world: Physics engine not initialized");
                return;
            }

            m_physicsEngine->SetActiveWorld(world);
        }

        std::shared_ptr<PhysicsWorld> BulletPhysicsModule::GetActiveWorld() {
            if (!m_physicsEngine) {
                return nullptr;
            }

            // Note: PhysicsEngine doesn't currently expose GetActiveWorld()
            // This would need to be added to the PhysicsEngine interface
            LOG_WARNING("GetActiveWorld not implemented in PhysicsEngine interface");
            return nullptr;
        }

        void BulletPhysicsModule::EnableDebugDrawing(bool enabled) {
            m_physicsSettings.enableDebugDrawing = enabled;
            
            if (m_physicsEngine) {
                m_physicsEngine->EnableDebugDrawing(enabled);
            }
        }

        bool BulletPhysicsModule::IsDebugDrawingEnabled() const {
            if (m_physicsEngine) {
                return m_physicsEngine->IsDebugDrawingEnabled();
            }
            return m_physicsSettings.enableDebugDrawing;
        }

        PhysicsEngine::PhysicsDebugInfo BulletPhysicsModule::GetDebugInfo() const {
            if (m_physicsEngine) {
                return m_physicsEngine->GetDebugInfo();
            }
            return PhysicsEngine::PhysicsDebugInfo{};
        }

        bool BulletPhysicsModule::InitializePhysicsEngine() {
            try {
                m_physicsEngine = std::make_unique<PhysicsEngine>();
                if (!m_physicsEngine) {
                    LOG_ERROR("Failed to create Bullet Physics engine");
                    return false;
                }

                if (!m_physicsEngine->Initialize(m_physicsSettings.configuration)) {
                    LOG_ERROR("Failed to initialize Bullet Physics engine");
                    m_physicsEngine.reset();
                    return false;
                }

                // Apply additional settings
                ApplyConfiguration();

                return true;
            }
            catch (const std::exception& e) {
                LOG_ERROR("Exception during physics engine initialization: " + std::string(e.what()));
                m_physicsEngine.reset();
                return false;
            }
        }

        void BulletPhysicsModule::ShutdownPhysicsEngine() {
            if (m_physicsEngine) {
                m_physicsEngine->Shutdown();
                m_physicsEngine.reset();
            }
        }

        void BulletPhysicsModule::ApplyConfiguration() {
            if (!m_physicsEngine) {
                return;
            }

            // Apply the current configuration to the physics engine
            m_physicsEngine->SetConfiguration(m_physicsSettings.configuration);
            
            // Apply debug drawing setting
            m_physicsEngine->EnableDebugDrawing(m_physicsSettings.enableDebugDrawing);
            
            LOG_DEBUG("Applied physics configuration to engine");
        }
    }
}