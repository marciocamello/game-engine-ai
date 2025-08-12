#include "TestUtils.h"
#include "Core/IEngineModule.h"
#include "Core/ModuleRegistry.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

// Mock module implementation for testing
class MockModule : public IEngineModule {
public:
    MockModule(const std::string& name, const std::string& version, ModuleType type,
               const std::vector<std::string>& dependencies = {})
        : m_name(name), m_version(version), m_type(type), m_dependencies(dependencies),
          m_initialized(false), m_enabled(true) {}

    bool Initialize(const ModuleConfig& config) override {
        m_initialized = true;
        m_config = config;
        return true;
    }

    void Update(float deltaTime) override {
        m_lastDeltaTime = deltaTime;
    }

    void Shutdown() override {
        m_initialized = false;
    }

    const char* GetName() const override {
        return m_name.c_str();
    }

    const char* GetVersion() const override {
        return m_version.c_str();
    }

    ModuleType GetType() const override {
        return m_type;
    }

    std::vector<std::string> GetDependencies() const override {
        return m_dependencies;
    }

    bool IsInitialized() const override {
        return m_initialized;
    }

    bool IsEnabled() const override {
        return m_enabled;
    }

    void SetEnabled(bool enabled) override {
        m_enabled = enabled;
    }

    // Test helpers
    float GetLastDeltaTime() const { return m_lastDeltaTime; }
    const ModuleConfig& GetConfig() const { return m_config; }

private:
    std::string m_name;
    std::string m_version;
    ModuleType m_type;
    std::vector<std::string> m_dependencies;
    bool m_initialized;
    bool m_enabled;
    float m_lastDeltaTime = 0.0f;
    ModuleConfig m_config;
};

/**
 * Test basic module interface functionality
 * Requirements: 2.5 (standardized plugin interface)
 */
bool TestModuleInterface() {
    TestOutput::PrintTestStart("module interface functionality");

    // Create a mock module
    MockModule module("TestModule", "1.0.0", ModuleType::Core);

    // Test initial state
    EXPECT_STRING_EQUAL(module.GetName(), "TestModule");
    EXPECT_STRING_EQUAL(module.GetVersion(), "1.0.0");
    EXPECT_TRUE(module.GetType() == ModuleType::Core);
    EXPECT_FALSE(module.IsInitialized());
    EXPECT_TRUE(module.IsEnabled());

    // Test initialization
    ModuleConfig config;
    config.name = "TestModule";
    config.version = "1.0.0";
    config.enabled = true;
    config.parameters["test_param"] = "test_value";

    EXPECT_TRUE(module.Initialize(config));
    EXPECT_TRUE(module.IsInitialized());
    EXPECT_STRING_EQUAL(module.GetConfig().name, "TestModule");
    EXPECT_STRING_EQUAL(module.GetConfig().parameters.at("test_param"), "test_value");

    // Test update
    module.Update(0.016f);
    EXPECT_NEARLY_EQUAL(module.GetLastDeltaTime(), 0.016f);

    // Test enable/disable
    module.SetEnabled(false);
    EXPECT_FALSE(module.IsEnabled());
    module.SetEnabled(true);
    EXPECT_TRUE(module.IsEnabled());

    // Test shutdown
    module.Shutdown();
    EXPECT_FALSE(module.IsInitialized());

    TestOutput::PrintTestPass("module interface functionality");
    return true;
}

/**
 * Test module registry singleton functionality
 * Requirements: 2.6 (runtime module discovery and loading)
 */
bool TestModuleRegistrySingleton() {
    TestOutput::PrintTestStart("module registry singleton");

    // Test singleton pattern
    ModuleRegistry& registry1 = ModuleRegistry::GetInstance();
    ModuleRegistry& registry2 = ModuleRegistry::GetInstance();
    
    EXPECT_EQUAL(&registry1, &registry2);

    TestOutput::PrintTestPass("module registry singleton");
    return true;
}

/**
 * Test module registration and retrieval
 * Requirements: 2.5 (standardized plugin interface), 5.1 (dependency declaration system)
 */
bool TestModuleRegistration() {
    TestOutput::PrintTestStart("module registration and retrieval");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();

    // Clear any existing modules for clean test
    auto existingNames = registry.GetModuleNames();
    for (const auto& name : existingNames) {
        registry.UnregisterModule(name);
    }

    // Test initial state
    EXPECT_EQUAL(registry.GetModuleCount(), 0);

    // Register a module
    auto module = std::make_unique<MockModule>("GraphicsModule", "1.0.0", ModuleType::Graphics);
    MockModule* modulePtr = module.get();
    registry.RegisterModule(std::move(module));

    // Test registration
    EXPECT_EQUAL(registry.GetModuleCount(), 1);
    EXPECT_TRUE(registry.IsModuleRegistered("GraphicsModule"));
    EXPECT_FALSE(registry.IsModuleRegistered("NonExistentModule"));

    // Test retrieval
    IEngineModule* retrievedModule = registry.GetModule("GraphicsModule");
    EXPECT_NOT_NULL(retrievedModule);
    EXPECT_EQUAL(retrievedModule, modulePtr);
    EXPECT_STRING_EQUAL(retrievedModule->GetName(), "GraphicsModule");

    // Test retrieval by type
    auto graphicsModules = registry.GetModulesByType(ModuleType::Graphics);
    EXPECT_EQUAL(graphicsModules.size(), 1);
    EXPECT_EQUAL(graphicsModules[0], modulePtr);

    auto audioModules = registry.GetModulesByType(ModuleType::Audio);
    EXPECT_EQUAL(audioModules.size(), 0);

    // Test module names
    auto moduleNames = registry.GetModuleNames();
    EXPECT_EQUAL(moduleNames.size(), 1);
    EXPECT_STRING_EQUAL(moduleNames[0], "GraphicsModule");

    TestOutput::PrintTestPass("module registration and retrieval");
    return true;
}

/**
 * Test module unregistration
 * Requirements: 2.6 (runtime module discovery and loading)
 */
bool TestModuleUnregistration() {
    TestOutput::PrintTestStart("module unregistration");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();

    // Register a module
    auto module = std::make_unique<MockModule>("TempModule", "1.0.0", ModuleType::Core);
    registry.RegisterModule(std::move(module));

    EXPECT_TRUE(registry.IsModuleRegistered("TempModule"));
    EXPECT_EQUAL(registry.GetModuleCount(), 2); // GraphicsModule from previous test + TempModule

    // Unregister the module
    registry.UnregisterModule("TempModule");

    EXPECT_FALSE(registry.IsModuleRegistered("TempModule"));
    EXPECT_EQUAL(registry.GetModuleCount(), 1);
    EXPECT_NULL(registry.GetModule("TempModule"));

    // Test unregistering non-existent module (should not crash)
    registry.UnregisterModule("NonExistentModule");
    EXPECT_EQUAL(registry.GetModuleCount(), 1);

    TestOutput::PrintTestPass("module unregistration");
    return true;
}

/**
 * Test dependency validation
 * Requirements: 5.3 (dependency compatibility validation), 5.4 (circular dependency detection)
 */
bool TestDependencyValidation() {
    TestOutput::PrintTestStart("dependency validation");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();

    // Clear existing modules
    auto existingNames = registry.GetModuleNames();
    for (const auto& name : existingNames) {
        registry.UnregisterModule(name);
    }

    // Register modules with valid dependencies
    auto coreModule = std::make_unique<MockModule>("Core", "1.0.0", ModuleType::Core);
    auto graphicsModule = std::make_unique<MockModule>("Graphics", "1.0.0", ModuleType::Graphics, 
                                                       std::vector<std::string>{"Core"});
    auto physicsModule = std::make_unique<MockModule>("Physics", "1.0.0", ModuleType::Physics, 
                                                      std::vector<std::string>{"Core"});

    registry.RegisterModule(std::move(coreModule));
    registry.RegisterModule(std::move(graphicsModule));
    registry.RegisterModule(std::move(physicsModule));

    // Test valid dependencies
    EXPECT_TRUE(registry.ValidateDependencies());

    // Test missing dependency
    auto invalidModule = std::make_unique<MockModule>("Invalid", "1.0.0", ModuleType::Audio, 
                                                      std::vector<std::string>{"NonExistent"});
    registry.RegisterModule(std::move(invalidModule));

    EXPECT_FALSE(registry.ValidateDependencies());

    // Remove invalid module
    registry.UnregisterModule("Invalid");
    EXPECT_TRUE(registry.ValidateDependencies());

    TestOutput::PrintTestPass("dependency validation");
    return true;
}

/**
 * Test circular dependency detection
 * Requirements: 5.4 (circular dependency detection)
 */
bool TestCircularDependencyDetection() {
    TestOutput::PrintTestStart("circular dependency detection");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();

    // Clear existing modules
    auto existingNames = registry.GetModuleNames();
    for (const auto& name : existingNames) {
        registry.UnregisterModule(name);
    }

    // Create circular dependency: A -> B -> C -> A
    auto moduleA = std::make_unique<MockModule>("ModuleA", "1.0.0", ModuleType::Core, 
                                                std::vector<std::string>{"ModuleC"});
    auto moduleB = std::make_unique<MockModule>("ModuleB", "1.0.0", ModuleType::Graphics, 
                                                std::vector<std::string>{"ModuleA"});
    auto moduleC = std::make_unique<MockModule>("ModuleC", "1.0.0", ModuleType::Physics, 
                                                std::vector<std::string>{"ModuleB"});

    registry.RegisterModule(std::move(moduleA));
    registry.RegisterModule(std::move(moduleB));
    registry.RegisterModule(std::move(moduleC));

    // Should detect circular dependency
    EXPECT_FALSE(registry.ValidateDependencies());

    TestOutput::PrintTestPass("circular dependency detection");
    return true;
}

/**
 * Test dependency resolution and initialization order
 * Requirements: 5.1 (dependency declaration system), 5.2 (module initialization ordering)
 */
bool TestDependencyResolution() {
    TestOutput::PrintTestStart("dependency resolution and initialization order");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();

    // Clear existing modules
    auto existingNames = registry.GetModuleNames();
    for (const auto& name : existingNames) {
        registry.UnregisterModule(name);
    }

    // Create modules with dependencies: Graphics -> Core, Physics -> Core, Audio -> Graphics
    auto coreModule = std::make_unique<MockModule>("Core", "1.0.0", ModuleType::Core);
    auto graphicsModule = std::make_unique<MockModule>("Graphics", "1.0.0", ModuleType::Graphics, 
                                                       std::vector<std::string>{"Core"});
    auto physicsModule = std::make_unique<MockModule>("Physics", "1.0.0", ModuleType::Physics, 
                                                      std::vector<std::string>{"Core"});
    auto audioModule = std::make_unique<MockModule>("Audio", "1.0.0", ModuleType::Audio, 
                                                    std::vector<std::string>{"Graphics"});

    registry.RegisterModule(std::move(audioModule));    // Register in random order
    registry.RegisterModule(std::move(physicsModule));
    registry.RegisterModule(std::move(graphicsModule));
    registry.RegisterModule(std::move(coreModule));

    // Resolve dependencies
    auto initOrder = registry.ResolveDependencies();
    EXPECT_EQUAL(initOrder.size(), 4);

    // Verify initialization order: Core should be first
    EXPECT_STRING_EQUAL(initOrder[0]->GetName(), "Core");
    
    // Audio should come after Graphics (since Audio depends on Graphics)
    bool foundAudioAfterGraphics = false;
    for (size_t i = 1; i < initOrder.size(); ++i) {
        if (std::string(initOrder[i]->GetName()) == "Audio") {
            // Check that Graphics appears before Audio in the list
            for (size_t j = 0; j < i; ++j) {
                if (std::string(initOrder[j]->GetName()) == "Graphics") {
                    foundAudioAfterGraphics = true;
                    break;
                }
            }
            break;
        }
    }
    EXPECT_TRUE(foundAudioAfterGraphics);

    // Graphics should come before Audio
    int graphicsIndex = -1, audioIndex = -1;
    for (size_t i = 0; i < initOrder.size(); ++i) {
        if (std::string(initOrder[i]->GetName()) == "Graphics") {
            graphicsIndex = static_cast<int>(i);
        }
        if (std::string(initOrder[i]->GetName()) == "Audio") {
            audioIndex = static_cast<int>(i);
        }
    }
    EXPECT_TRUE(graphicsIndex < audioIndex);

    TestOutput::PrintTestPass("dependency resolution and initialization order");
    return true;
}

/**
 * Test module initialization with configuration
 * Requirements: 2.7 (configuration system for enabling/disabling modules)
 */
bool TestModuleInitialization() {
    TestOutput::PrintTestStart("module initialization with configuration");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();

    // Create engine configuration
    EngineConfig config;
    config.configVersion = "1.0";
    config.engineVersion = "1.0.0";

    // Add module configurations
    ModuleConfig coreConfig;
    coreConfig.name = "Core";
    coreConfig.version = "1.0.0";
    coreConfig.enabled = true;
    coreConfig.parameters["debug"] = "true";
    config.modules.push_back(coreConfig);

    ModuleConfig graphicsConfig;
    graphicsConfig.name = "Graphics";
    graphicsConfig.version = "1.0.0";
    graphicsConfig.enabled = true;
    graphicsConfig.parameters["api"] = "OpenGL";
    config.modules.push_back(graphicsConfig);

    ModuleConfig physicsConfig;
    physicsConfig.name = "Physics";
    physicsConfig.version = "1.0.0";
    physicsConfig.enabled = false; // Disabled module
    config.modules.push_back(physicsConfig);

    // Initialize modules
    auto result = registry.InitializeModules(config);
    EXPECT_TRUE(result.success);

    // Check that enabled modules are initialized
    IEngineModule* coreModule = registry.GetModule("Core");
    IEngineModule* graphicsModule = registry.GetModule("Graphics");
    IEngineModule* physicsModule = registry.GetModule("Physics");

    EXPECT_NOT_NULL(coreModule);
    EXPECT_NOT_NULL(graphicsModule);
    EXPECT_NOT_NULL(physicsModule);

    EXPECT_TRUE(coreModule->IsInitialized());
    EXPECT_TRUE(graphicsModule->IsInitialized());
    EXPECT_FALSE(physicsModule->IsInitialized()); // Should not be initialized (disabled)

    // Check configuration was passed correctly
    MockModule* mockCore = static_cast<MockModule*>(coreModule);
    EXPECT_STRING_EQUAL(mockCore->GetConfig().parameters.at("debug"), "true");

    MockModule* mockGraphics = static_cast<MockModule*>(graphicsModule);
    EXPECT_STRING_EQUAL(mockGraphics->GetConfig().parameters.at("api"), "OpenGL");

    TestOutput::PrintTestPass("module initialization with configuration");
    return true;
}

/**
 * Test module update functionality
 * Requirements: 2.5 (standardized plugin interface)
 */
bool TestModuleUpdate() {
    TestOutput::PrintTestStart("module update functionality");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();

    // Update all modules
    float deltaTime = 0.016f;
    registry.UpdateModules(deltaTime);

    // Check that initialized modules received the update
    MockModule* coreModule = static_cast<MockModule*>(registry.GetModule("Core"));
    MockModule* graphicsModule = static_cast<MockModule*>(registry.GetModule("Graphics"));
    MockModule* physicsModule = static_cast<MockModule*>(registry.GetModule("Physics"));

    EXPECT_NEARLY_EQUAL(coreModule->GetLastDeltaTime(), deltaTime);
    EXPECT_NEARLY_EQUAL(graphicsModule->GetLastDeltaTime(), deltaTime);
    EXPECT_NEARLY_EQUAL(physicsModule->GetLastDeltaTime(), 0.0f); // Not initialized, should not update

    TestOutput::PrintTestPass("module update functionality");
    return true;
}

/**
 * Test module shutdown functionality
 * Requirements: 2.5 (standardized plugin interface)
 */
bool TestModuleShutdown() {
    TestOutput::PrintTestStart("module shutdown functionality");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();

    // Shutdown all modules
    registry.ShutdownModules();

    // Check that all modules are shut down
    IEngineModule* coreModule = registry.GetModule("Core");
    IEngineModule* graphicsModule = registry.GetModule("Graphics");
    IEngineModule* physicsModule = registry.GetModule("Physics");
    IEngineModule* audioModule = registry.GetModule("Audio");

    if (coreModule) EXPECT_FALSE(coreModule->IsInitialized());
    if (graphicsModule) EXPECT_FALSE(graphicsModule->IsInitialized());
    if (physicsModule) EXPECT_FALSE(physicsModule->IsInitialized());
    if (audioModule) EXPECT_FALSE(audioModule->IsInitialized());

    TestOutput::PrintTestPass("module shutdown functionality");
    return true;
}

int main() {
    TestOutput::PrintHeader("ModuleRegistry");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ModuleRegistry Tests");

        // Run all tests
        allPassed &= suite.RunTest("Module Interface Functionality", TestModuleInterface);
        allPassed &= suite.RunTest("Module Registry Singleton", TestModuleRegistrySingleton);
        allPassed &= suite.RunTest("Module Registration and Retrieval", TestModuleRegistration);
        allPassed &= suite.RunTest("Module Unregistration", TestModuleUnregistration);
        allPassed &= suite.RunTest("Dependency Validation", TestDependencyValidation);
        allPassed &= suite.RunTest("Circular Dependency Detection", TestCircularDependencyDetection);
        allPassed &= suite.RunTest("Dependency Resolution", TestDependencyResolution);
        allPassed &= suite.RunTest("Module Initialization", TestModuleInitialization);
        allPassed &= suite.RunTest("Module Update", TestModuleUpdate);
        allPassed &= suite.RunTest("Module Shutdown", TestModuleShutdown);

        // Print detailed summary
        suite.PrintSummary();

        TestOutput::PrintFooter(allPassed);
        return allPassed ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
        return 1;
    } catch (...) {
        TestOutput::PrintError("UNKNOWN TEST ERROR!");
        return 1;
    }
}