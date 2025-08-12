#include "TestUtils.h"
#include "Core/ModuleError.h"
#include "Core/ModuleRegistry.h"
#include "Core/IEngineModule.h"
#include <memory>

using namespace GameEngine;
using namespace GameEngine::Testing;

// Mock module for testing
class MockModule : public IEngineModule {
private:
    std::string m_name;
    std::string m_version;
    ModuleType m_type;
    std::vector<std::string> m_dependencies;
    bool m_initialized = false;
    bool m_enabled = true;
    bool m_shouldFailInit = false;
    bool m_shouldThrowException = false;

public:
    MockModule(const std::string& name, ModuleType type, const std::vector<std::string>& deps = {})
        : m_name(name), m_version("1.0.0"), m_type(type), m_dependencies(deps) {}

    void SetShouldFailInit(bool fail) { m_shouldFailInit = fail; }
    void SetShouldThrowException(bool throwEx) { m_shouldThrowException = throwEx; }

    bool Initialize(const ModuleConfig& config) override {
        if (m_shouldThrowException) {
            throw std::runtime_error("Mock initialization exception");
        }
        if (m_shouldFailInit) {
            return false;
        }
        m_initialized = true;
        return true;
    }

    void Update(float deltaTime) override {}

    void Shutdown() override {
        if (m_shouldThrowException) {
            throw std::runtime_error("Mock shutdown exception");
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
 * Test ModuleError basic functionality
 * Requirements: 5.3 (detailed error messages for module loading failures)
 */
bool TestModuleErrorBasics() {
    TestOutput::PrintTestStart("module error basic functionality");

    ModuleError error(ModuleErrorType::InitializationFailed, "test-module", 
                     "Test error message", "Additional details");

    EXPECT_TRUE(error.HasError());
    EXPECT_TRUE(error.type == ModuleErrorType::InitializationFailed);
    EXPECT_EQUAL(error.moduleName, "test-module");
    EXPECT_EQUAL(error.message, "Test error message");
    EXPECT_EQUAL(error.details, "Additional details");

    std::string formatted = error.GetFormattedMessage();
    EXPECT_TRUE(formatted.find("[INITIALIZATION FAILED]") != std::string::npos);
    EXPECT_TRUE(formatted.find("test-module") != std::string::npos);
    EXPECT_TRUE(formatted.find("Test error message") != std::string::npos);

    TestOutput::PrintTestPass("module error basic functionality");
    return true;
}

/**
 * Test ModuleErrorCollector functionality
 * Requirements: 5.3 (detailed error messages for module loading failures)
 */
bool TestModuleErrorCollector() {
    TestOutput::PrintTestStart("module error collector");

    ModuleErrorCollector collector;
    EXPECT_FALSE(collector.HasErrors());
    EXPECT_EQUAL(collector.GetErrorCount(), 0);

    collector.AddError(ModuleErrorType::ModuleNotFound, "module1", "Not found", "Details");
    collector.AddError(ModuleErrorType::CircularDependency, "module2", "Circular dep", "More details");

    EXPECT_TRUE(collector.HasErrors());
    EXPECT_EQUAL(collector.GetErrorCount(), 2);
    EXPECT_TRUE(collector.HasCriticalErrors()); // CircularDependency is critical

    auto errors = collector.GetErrorsByType(ModuleErrorType::ModuleNotFound);
    EXPECT_EQUAL(errors.size(), 1);
    EXPECT_EQUAL(errors[0].moduleName, "module1");

    std::string summary = collector.GetSummary();
    EXPECT_TRUE(summary.find("2 errors") != std::string::npos);

    TestOutput::PrintTestPass("module error collector");
    return true;
}

/**
 * Test configuration validation
 * Requirements: 5.4 (validation system for module and project configurations)
 */
bool TestConfigurationValidation() {
    TestOutput::PrintTestStart("configuration validation");

    // Test valid module config
    ModuleConfig validConfig;
    validConfig.name = "test-module";
    validConfig.version = "1.0.0";
    validConfig.enabled = true;
    validConfig.parameters["param1"] = "value1";

    auto validation = ConfigurationValidator::ValidateModuleConfig(validConfig);
    EXPECT_TRUE(validation.IsValid());
    EXPECT_FALSE(validation.hasErrors);

    // Test invalid module config
    ModuleConfig invalidConfig;
    invalidConfig.name = ""; // Empty name should be critical
    invalidConfig.version = "";

    validation = ConfigurationValidator::ValidateModuleConfig(invalidConfig);
    EXPECT_FALSE(validation.IsValid());
    EXPECT_TRUE(validation.hasCriticalErrors);

    // Test engine config validation
    EngineConfig engineConfig;
    engineConfig.configVersion = "1.0";
    engineConfig.engineVersion = "1.0.0";
    engineConfig.modules.push_back(validConfig);

    auto engineValidation = ConfigurationValidator::ValidateEngineConfig(engineConfig);
    EXPECT_TRUE(engineValidation.IsValid());

    TestOutput::PrintTestPass("configuration validation");
    return true;
}

/**
 * Test module registration error handling
 * Requirements: 5.3 (detailed error messages for module loading failures)
 */
bool TestModuleRegistrationErrors() {
    TestOutput::PrintTestStart("module registration error handling");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();
    ModuleErrorCollector errors;

    // Test registering null module
    bool result = registry.RegisterModule(nullptr, &errors);
    EXPECT_FALSE(result);
    EXPECT_TRUE(errors.HasErrors());
    EXPECT_EQUAL(errors.GetErrorCount(), 1);

    errors.Clear();

    // Test registering module with empty name
    auto emptyNameModule = std::make_unique<MockModule>("", ModuleType::Core);
    result = registry.RegisterModule(std::move(emptyNameModule), &errors);
    EXPECT_FALSE(result);
    EXPECT_TRUE(errors.HasErrors());

    errors.Clear();

    // Test registering module with invalid dependencies
    auto invalidDepsModule = std::make_unique<MockModule>("test-module", ModuleType::Core, 
                                                         std::vector<std::string>{"", "invalid-dep"});
    result = registry.RegisterModule(std::move(invalidDepsModule), &errors);
    EXPECT_TRUE(errors.HasErrors()); // Should have validation errors

    TestOutput::PrintTestPass("module registration error handling");
    return true;
}

/**
 * Test dependency validation errors
 * Requirements: 5.3, 5.4 (detailed error messages and validation)
 */
bool TestDependencyValidationErrors() {
    TestOutput::PrintTestStart("dependency validation errors");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();
    ModuleErrorCollector errors;

    // Clear any existing modules
    registry.ClearErrorState();
    auto moduleNames = registry.GetModuleNames();
    for (const auto& name : moduleNames) {
        registry.UnregisterModule(name);
    }

    // Create modules with circular dependencies
    auto module1 = std::make_unique<MockModule>("module1", ModuleType::Core, 
                                               std::vector<std::string>{"module2"});
    auto module2 = std::make_unique<MockModule>("module2", ModuleType::Graphics, 
                                               std::vector<std::string>{"module1"});

    registry.RegisterModule(std::move(module1), &errors);
    registry.RegisterModule(std::move(module2), &errors);

    // Validate dependencies - should detect circular dependency
    bool isValid = registry.ValidateDependencies(&errors);
    EXPECT_FALSE(isValid);
    EXPECT_TRUE(errors.HasErrors());
    EXPECT_TRUE(errors.HasCriticalErrors());

    auto circularErrors = errors.GetErrorsByType(ModuleErrorType::CircularDependency);
    EXPECT_TRUE(circularErrors.size() > 0);

    TestOutput::PrintTestPass("dependency validation errors");
    return true;
}

/**
 * Test module initialization with fallbacks
 * Requirements: 5.4 (graceful fallback mechanisms for missing modules)
 */
bool TestModuleInitializationFallbacks() {
    TestOutput::PrintTestStart("module initialization fallbacks");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();
    registry.ClearErrorState();
    
    // Clear any existing modules from previous tests
    auto moduleNames = registry.GetModuleNames();
    for (const auto& name : moduleNames) {
        registry.UnregisterModule(name);
    }

    // Enable graceful fallbacks
    registry.EnableGracefulFallbacks(true);

    // Create a module that will fail initialization
    auto failingModule = std::make_unique<MockModule>("failing-module", ModuleType::Audio);
    failingModule->SetShouldFailInit(true);
    registry.RegisterModule(std::move(failingModule));

    // Create engine config
    EngineConfig config;
    config.configVersion = "1.0";
    config.engineVersion = "1.0.0";
    
    ModuleConfig moduleConfig;
    moduleConfig.name = "failing-module";
    moduleConfig.version = "1.0.0";
    moduleConfig.enabled = true;
    config.modules.push_back(moduleConfig);

    // Initialize modules - should handle failure gracefully
    auto result = registry.InitializeModules(config);
    
    // With graceful fallbacks, initialization should succeed even with failures
    EXPECT_TRUE(result.success || !result.errors.HasCriticalErrors());
    EXPECT_TRUE(result.errors.HasErrors()); // Should have recorded the failure
    EXPECT_TRUE(result.skippedModules.size() > 0 || result.fallbackModules.size() > 0);

    TestOutput::PrintTestPass("module initialization fallbacks");
    return true;
}

/**
 * Test module recovery mechanisms
 * Requirements: 5.4 (graceful fallback mechanisms for missing modules)
 */
bool TestModuleRecovery() {
    TestOutput::PrintTestStart("module recovery mechanisms");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();
    ModuleErrorCollector errors;

    // Create a module for recovery testing
    auto recoverableModule = std::make_unique<MockModule>("recoverable-module", ModuleType::Physics);
    std::string moduleName = recoverableModule->GetName();
    registry.RegisterModule(std::move(recoverableModule));

    // Attempt recovery on non-existent module
    bool recovered = registry.AttemptModuleRecovery("non-existent", &errors);
    EXPECT_FALSE(recovered);
    EXPECT_TRUE(errors.HasErrors());

    errors.Clear();

    // Attempt recovery on existing module
    recovered = registry.AttemptModuleRecovery(moduleName, &errors);
    EXPECT_TRUE(recovered);
    EXPECT_FALSE(errors.HasErrors());

    TestOutput::PrintTestPass("module recovery mechanisms");
    return true;
}

/**
 * Test exception handling during module operations
 * Requirements: 5.3 (detailed error messages for module loading failures)
 */
bool TestExceptionHandling() {
    TestOutput::PrintTestStart("exception handling during module operations");

    ModuleRegistry& registry = ModuleRegistry::GetInstance();
    registry.ClearErrorState();
    
    // Clear any existing modules from previous tests
    auto moduleNames = registry.GetModuleNames();
    for (const auto& name : moduleNames) {
        registry.UnregisterModule(name);
    }

    // Create a module that throws exceptions
    auto throwingModule = std::make_unique<MockModule>("throwing-module", ModuleType::Input);
    throwingModule->SetShouldThrowException(true);
    registry.RegisterModule(std::move(throwingModule));

    // Create engine config
    EngineConfig config;
    config.configVersion = "1.0";
    config.engineVersion = "1.0.0";
    
    ModuleConfig moduleConfig;
    moduleConfig.name = "throwing-module";
    moduleConfig.version = "1.0.0";
    moduleConfig.enabled = true;
    config.modules.push_back(moduleConfig);

    // Initialize modules - should handle exceptions gracefully
    auto result = registry.InitializeModules(config);
    
    // Should have errors from the exception
    EXPECT_TRUE(result.errors.HasErrors());
    
    // Should have either skipped the module or used a fallback
    EXPECT_TRUE(result.skippedModules.size() > 0 || result.fallbackModules.size() > 0);

    // Check that exception details are captured in the errors
    auto initErrors = result.errors.GetErrorsByType(ModuleErrorType::InitializationFailed);
    EXPECT_TRUE(initErrors.size() > 0);
    
    bool foundExceptionDetails = false;
    for (const auto& error : initErrors) {
        if (error.details.find("Exception:") != std::string::npos) {
            foundExceptionDetails = true;
            break;
        }
    }
    EXPECT_TRUE(foundExceptionDetails);

    TestOutput::PrintTestPass("exception handling during module operations");
    return true;
}

int main() {
    TestOutput::PrintHeader("Module Error Handling");

    bool allPassed = true;

    try {
        TestSuite suite("Module Error Handling Tests");

        allPassed &= suite.RunTest("Module Error Basics", TestModuleErrorBasics);
        allPassed &= suite.RunTest("Module Error Collector", TestModuleErrorCollector);
        allPassed &= suite.RunTest("Configuration Validation", TestConfigurationValidation);
        allPassed &= suite.RunTest("Module Registration Errors", TestModuleRegistrationErrors);
        allPassed &= suite.RunTest("Dependency Validation Errors", TestDependencyValidationErrors);
        allPassed &= suite.RunTest("Module Initialization Fallbacks", TestModuleInitializationFallbacks);
        allPassed &= suite.RunTest("Module Recovery", TestModuleRecovery);
        allPassed &= suite.RunTest("Exception Handling", TestExceptionHandling);

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