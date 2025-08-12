#include <iostream>
#include "../TestUtils.h"
#include "Core/ModuleError.h"
#include "Core/ModuleRegistry.h"
#include "Core/IEngineModule.h"
#include <memory>

using namespace GameEngine;
using namespace GameEngine::Testing;

// Helper function to safely clear all modules from registry
void ClearModuleRegistry(ModuleRegistry& registry) {
    // Enable graceful fallbacks to allow forced cleanup
    registry.EnableGracefulFallbacks(true);
    
    // Shutdown all modules first
    registry.ShutdownModules();
    
    // Get all module names
    auto moduleNames = registry.GetModuleNames();
    
    // Try to unregister modules in reverse dependency order
    // Start with modules that have no dependents
    int maxAttempts = static_cast<int>(moduleNames.size()) * 3; // Safety limit
    int attempts = 0;
    
    while (!moduleNames.empty() && attempts < maxAttempts) {
        size_t initialCount = moduleNames.size();
        
        // Try to find modules with no dependents first
        for (auto it = moduleNames.begin(); it != moduleNames.end(); ) {
            ModuleErrorCollector errors;
            bool removed = registry.UnregisterModule(*it, &errors);
            if (removed) {
                it = moduleNames.erase(it);
            } else {
                ++it;
            }
        }
        
        // If no progress was made, we might have circular dependencies
        // In that case, just accept the warnings and continue
        if (moduleNames.size() == initialCount) {
            // Force remove by ignoring dependency warnings
            if (!moduleNames.empty()) {
                ModuleErrorCollector errors;
                registry.UnregisterModule(moduleNames[0], &errors);
                // Remove from our list regardless of success
                moduleNames.erase(moduleNames.begin());
            }
        }
        
        attempts++;
    }
    
    // Clear any remaining error state
    registry.ClearErrorState();
}

// Complex mock module for integration testing
class IntegrationMockModule : public IEngineModule {
private:
    std::string m_name;
    std::string m_version;
    ModuleType m_type;
    std::vector<std::string> m_dependencies;
    bool m_initialized = false;
    bool m_enabled = true;
    bool m_shouldFailInit = false;
    bool m_shouldThrowException = false;
    int m_initAttempts = 0;

public:
    IntegrationMockModule(const std::string& name, ModuleType type, 
                         const std::vector<std::string>& deps = {})
        : m_name(name), m_version("1.0.0"), m_type(type), m_dependencies(deps) {}

    void SetShouldFailInit(bool fail) { m_shouldFailInit = fail; }
    void SetShouldThrowException(bool throwEx) { m_shouldThrowException = throwEx; }
    int GetInitAttempts() const { return m_initAttempts; }

    bool Initialize(const ModuleConfig& config) override {
        m_initAttempts++;
        
        if (m_shouldThrowException) {
            throw std::runtime_error("Integration test exception in " + m_name);
        }
        
        if (m_shouldFailInit) {
            return false;
        }
        
        m_initialized = true;
        return true;
    }

    void Update(float deltaTime) override {
        if (m_shouldThrowException && m_initialized) {
            throw std::runtime_error("Runtime exception in " + m_name);
        }
    }

    void Shutdown() override {
        if (m_shouldThrowException) {
            throw std::runtime_error("Shutdown exception in " + m_name);
        }
        m_initialized = false;
    }

    const char* GetName() const override { return m_name.c_str(); }
    const char* GetVersion() const override { return m_version.c_str(); }
    ModuleType GetType() const override { return m_type; }
    std::vector<std::string> GetDependencies() const override { return m_dependencies; }

    bool IsInitialized() const override { return m_initialized; }
    bool IsEnabled() const override { return m_enabled; }
    void SetEnabled(bool enabled) override { m_enabled = enabled; }
};

/**
 * Test complete error handling workflow with multiple modules
 * Requirements: 5.3, 5.4 (comprehensive error handling and validation)
 */
bool TestCompleteErrorHandlingWorkflow() {
    TestOutput::PrintTestStart("complete error handling workflow");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();
    registry.ClearErrorState();
    registry.EnableGracefulFallbacks(true);
    
    // Clear any existing modules from previous tests
    ClearModuleRegistry(registry);

    // Create a complex module dependency scenario
    auto coreModule = std::make_unique<IntegrationMockModule>("core", ModuleType::Core);
    auto graphicsModule = std::make_unique<IntegrationMockModule>("graphics", ModuleType::Graphics, 
                                                                 std::vector<std::string>{"core"});
    auto physicsModule = std::make_unique<IntegrationMockModule>("physics", ModuleType::Physics, 
                                                                std::vector<std::string>{"core"});
    auto audioModule = std::make_unique<IntegrationMockModule>("audio", ModuleType::Audio, 
                                                              std::vector<std::string>{"core"});
    auto gameModule = std::make_unique<IntegrationMockModule>("game", ModuleType::Core, 
                                                             std::vector<std::string>{"graphics", "physics", "audio"});

    // Make some modules fail
    physicsModule->SetShouldFailInit(true);
    audioModule->SetShouldThrowException(true);

    // Register modules
    ModuleErrorCollector registrationErrors;
    EXPECT_TRUE(registry.RegisterModule(std::move(coreModule), &registrationErrors));
    EXPECT_TRUE(registry.RegisterModule(std::move(graphicsModule), &registrationErrors));
    EXPECT_TRUE(registry.RegisterModule(std::move(physicsModule), &registrationErrors));
    EXPECT_TRUE(registry.RegisterModule(std::move(audioModule), &registrationErrors));
    EXPECT_TRUE(registry.RegisterModule(std::move(gameModule), &registrationErrors));

    EXPECT_FALSE(registrationErrors.HasErrors());

    // Create engine configuration
    EngineConfig config;
    config.configVersion = "1.0";
    config.engineVersion = "1.0.0";

    std::vector<std::string> testModuleNames = {"core", "graphics", "physics", "audio", "game"};
    for (const auto& name : testModuleNames) {
        ModuleConfig moduleConfig;
        moduleConfig.name = name;
        moduleConfig.version = "1.0.0";
        moduleConfig.enabled = true;
        config.modules.push_back(moduleConfig);
    }

    // Validate configuration
    auto configValidation = registry.ValidateConfiguration(config);
    EXPECT_TRUE(configValidation.IsValid());

    // Validate dependencies
    ModuleErrorCollector dependencyErrors;
    bool depsValid = registry.ValidateDependencies(&dependencyErrors);
    EXPECT_TRUE(depsValid);
    EXPECT_FALSE(dependencyErrors.HasErrors());

    // Initialize modules with error handling
    auto initResult = registry.InitializeModules(config);

    // Should have errors but still succeed with fallbacks
    EXPECT_TRUE(initResult.errors.HasErrors());
    EXPECT_TRUE(initResult.success || !initResult.errors.HasCriticalErrors());

    // Check that some modules were skipped or had fallbacks
    EXPECT_TRUE(initResult.skippedModules.size() > 0 || initResult.fallbackModules.size() > 0);

    // Verify error types
    auto initErrors = initResult.errors.GetErrorsByType(ModuleErrorType::InitializationFailed);
    EXPECT_TRUE(initErrors.size() > 0);

    // Check error summary
    std::string summary = initResult.GetSummary();
    EXPECT_TRUE(summary.find("Initialization Summary") != std::string::npos);

    TestOutput::PrintTestPass("complete error handling workflow");
    return true;
}

/**
 * Test circular dependency detection in complex scenarios
 * Requirements: 5.3, 5.4 (detailed error messages and validation)
 */
bool TestComplexCircularDependencyDetection() {
    TestOutput::PrintTestStart("complex circular dependency detection");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();
    registry.ClearErrorState();
    
    // Clear any existing modules from previous tests
    ClearModuleRegistry(registry);

    // Create a complex circular dependency: A -> B -> C -> A
    auto moduleA = std::make_unique<IntegrationMockModule>("moduleA", ModuleType::Core, 
                                                          std::vector<std::string>{"moduleB"});
    auto moduleB = std::make_unique<IntegrationMockModule>("moduleB", ModuleType::Graphics, 
                                                          std::vector<std::string>{"moduleC"});
    auto moduleC = std::make_unique<IntegrationMockModule>("moduleC", ModuleType::Physics, 
                                                          std::vector<std::string>{"moduleA"});

    ModuleErrorCollector errors;
    registry.RegisterModule(std::move(moduleA), &errors);
    registry.RegisterModule(std::move(moduleB), &errors);
    registry.RegisterModule(std::move(moduleC), &errors);

    // Validate dependencies - should detect circular dependency
    bool isValid = registry.ValidateDependencies(&errors);
    EXPECT_FALSE(isValid);
    EXPECT_TRUE(errors.HasCriticalErrors());

    auto circularErrors = errors.GetErrorsByType(ModuleErrorType::CircularDependency);
    EXPECT_TRUE(circularErrors.size() > 0);

    // Check that error details include the dependency chain
    bool foundChainDetails = false;
    for (const auto& error : circularErrors) {
        if (error.details.find("Dependency chain:") != std::string::npos) {
            foundChainDetails = true;
            break;
        }
    }
    EXPECT_TRUE(foundChainDetails);

    TestOutput::PrintTestPass("complex circular dependency detection");
    return true;
}

/**
 * Test missing dependency handling with detailed error reporting
 * Requirements: 5.3 (detailed error messages for module loading failures)
 */
bool TestMissingDependencyHandling() {
    TestOutput::PrintTestStart("missing dependency handling");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();
    registry.ClearErrorState();
    
    // Clear any existing modules from previous tests
    ClearModuleRegistry(registry);

    // Create modules with missing dependencies
    auto moduleWithMissingDep = std::make_unique<IntegrationMockModule>("dependent-module", 
                                                                       ModuleType::Audio, 
                                                                       std::vector<std::string>{"missing-module1", "missing-module2"});

    ModuleErrorCollector errors;
    registry.RegisterModule(std::move(moduleWithMissingDep), &errors);

    // Validate dependencies
    bool isValid = registry.ValidateDependencies(&errors);
    EXPECT_FALSE(isValid);
    EXPECT_TRUE(errors.HasErrors());

    auto missingErrors = errors.GetErrorsByType(ModuleErrorType::DependencyMissing);
    EXPECT_EQUAL(missingErrors.size(), 2); // Should have 2 missing dependency errors

    // Check that missing dependencies are correctly identified
    auto missingDeps = registry.GetMissingDependencies();
    EXPECT_EQUAL(missingDeps.size(), 2);
    EXPECT_TRUE(std::find(missingDeps.begin(), missingDeps.end(), "missing-module1") != missingDeps.end());
    EXPECT_TRUE(std::find(missingDeps.begin(), missingDeps.end(), "missing-module2") != missingDeps.end());

    TestOutput::PrintTestPass("missing dependency handling");
    return true;
}

/**
 * Test configuration validation with invalid configurations
 * Requirements: 5.4 (validation system for module and project configurations)
 */
bool TestInvalidConfigurationHandling() {
    TestOutput::PrintTestStart("invalid configuration handling");

    // Test invalid engine configuration
    EngineConfig invalidConfig;
    // Leave config version and engine version empty
    
    // Add invalid module configurations
    ModuleConfig invalidModuleConfig1;
    invalidModuleConfig1.name = ""; // Empty name
    invalidModuleConfig1.version = "invalid-version-format!@#";
    
    ModuleConfig invalidModuleConfig2;
    invalidModuleConfig2.name = "valid-module";
    invalidModuleConfig2.version = "1.0.0";
    invalidModuleConfig2.parameters[""] = "empty-key"; // Empty parameter key
    invalidModuleConfig2.parameters["valid-key"] = std::string(2000, 'x'); // Too long value
    
    // Add duplicate module
    ModuleConfig duplicateConfig;
    duplicateConfig.name = "valid-module"; // Same name as invalidModuleConfig2
    duplicateConfig.version = "2.0.0";
    
    invalidConfig.modules.push_back(invalidModuleConfig1);
    invalidConfig.modules.push_back(invalidModuleConfig2);
    invalidConfig.modules.push_back(duplicateConfig);

    // Validate configuration
    auto validation = ConfigurationValidator::ValidateEngineConfig(invalidConfig);
    EXPECT_FALSE(validation.IsValid());
    EXPECT_TRUE(validation.hasErrors);
    EXPECT_TRUE(validation.hasCriticalErrors);

    // Check that all issues are detected
    EXPECT_TRUE(validation.issues.size() >= 4); // At least: empty name, duplicate module, empty param key, long value

    // Check validation summary
    std::string summary = validation.GetSummary();
    EXPECT_TRUE(summary.find("Critical Issues:") != std::string::npos);
    EXPECT_TRUE(summary.find("Errors:") != std::string::npos);

    TestOutput::PrintTestPass("invalid configuration handling");
    return true;
}

/**
 * Test module recovery and error recovery mechanisms
 * Requirements: 5.4 (graceful fallback mechanisms for missing modules)
 */
bool TestModuleRecoveryMechanisms() {
    TestOutput::PrintTestStart("module recovery mechanisms");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();
    registry.ClearErrorState();
    
    // Clear any existing modules from previous tests
    ClearModuleRegistry(registry);

    // Create a module that initially fails but can be recovered
    auto recoverableModule = std::make_unique<IntegrationMockModule>("recoverable", ModuleType::Scripting);
    recoverableModule->SetShouldFailInit(true);
    
    std::string moduleName = recoverableModule->GetName();
    registry.RegisterModule(std::move(recoverableModule));

    // Try to initialize - should fail
    EngineConfig config;
    config.configVersion = "1.0";
    config.engineVersion = "1.0.0";
    
    ModuleConfig moduleConfig;
    moduleConfig.name = moduleName;
    moduleConfig.version = "1.0.0";
    moduleConfig.enabled = true;
    config.modules.push_back(moduleConfig);

    auto initResult = registry.InitializeModules(config);
    EXPECT_TRUE(initResult.errors.HasErrors());

    // Get the module and verify it failed to initialize
    auto* module = dynamic_cast<IntegrationMockModule*>(registry.GetModule(moduleName));
    EXPECT_TRUE(module != nullptr);
    EXPECT_FALSE(module->IsInitialized()); // Should have failed
    EXPECT_TRUE(module->GetInitAttempts() >= 1); // Should have at least one attempt
    
    int initialAttempts = module->GetInitAttempts();
    
    // Now fix the module and attempt recovery
    module->SetShouldFailInit(false);

    ModuleErrorCollector recoveryErrors;
    bool recovered = registry.AttemptModuleRecovery(moduleName, &recoveryErrors);
    EXPECT_TRUE(recovered);
    EXPECT_FALSE(recoveryErrors.HasErrors());

    // Verify module is now initialized and has more attempts
    EXPECT_TRUE(module->IsInitialized());
    EXPECT_TRUE(module->GetInitAttempts() > initialAttempts); // Should have at least one more attempt

    TestOutput::PrintTestPass("module recovery mechanisms");
    return true;
}

/**
 * Test fallback provider integration
 * Requirements: 5.4 (graceful fallback mechanisms for missing modules)
 */
bool TestFallbackProviderIntegration() {
    TestOutput::PrintTestStart("fallback provider integration");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();
    registry.ClearErrorState();
    registry.EnableGracefulFallbacks(true);
    
    // Clear any existing modules from previous tests
    ClearModuleRegistry(registry);

    // Set up a fallback provider
    bool fallbackCalled = false;
    std::string fallbackModuleName;
    ModuleType fallbackModuleType;
    
    registry.SetFallbackProvider([&](const std::string& name, ModuleType type) -> std::unique_ptr<IEngineModule> {
        fallbackCalled = true;
        fallbackModuleName = name;
        fallbackModuleType = type;
        
        // Return a working fallback module
        return std::make_unique<IntegrationMockModule>("fallback-" + name, type);
    });

    // Create a module that will fail initialization (no dependencies to avoid validation issues)
    auto failingModule = std::make_unique<IntegrationMockModule>("failing-module", ModuleType::Network);
    failingModule->SetShouldFailInit(true);
    registry.RegisterModule(std::move(failingModule));

    // Initialize modules
    EngineConfig config;
    config.configVersion = "1.0";
    config.engineVersion = "1.0.0";
    
    ModuleConfig moduleConfig;
    moduleConfig.name = "failing-module";
    moduleConfig.version = "1.0.0";
    moduleConfig.enabled = true;
    config.modules.push_back(moduleConfig);

    auto initResult = registry.InitializeModules(config);

    // The fallback should have been called during initialization failure
    // Even if the overall initialization succeeds with graceful fallbacks
    EXPECT_TRUE(fallbackCalled);
    EXPECT_EQUAL(fallbackModuleName, "failing-module");
    EXPECT_TRUE(fallbackModuleType == ModuleType::Network);

    TestOutput::PrintTestPass("fallback provider integration");
    return true;
}

int main() {
    TestOutput::PrintHeader("Module Error Handling Integration");

    bool allPassed = true;

    try {
        TestSuite suite("Module Error Handling Integration Tests");

        allPassed &= suite.RunTest("Complete Error Handling Workflow", TestCompleteErrorHandlingWorkflow);
        allPassed &= suite.RunTest("Complex Circular Dependency Detection", TestComplexCircularDependencyDetection);
        allPassed &= suite.RunTest("Missing Dependency Handling", TestMissingDependencyHandling);
        allPassed &= suite.RunTest("Invalid Configuration Handling", TestInvalidConfigurationHandling);
        allPassed &= suite.RunTest("Module Recovery Mechanisms", TestModuleRecoveryMechanisms);
        allPassed &= suite.RunTest("Fallback Provider Integration", TestFallbackProviderIntegration);

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