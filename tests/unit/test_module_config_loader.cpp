#include "TestUtils.h"
#include "Core/ModuleConfigLoader.h"
#include "Core/IEngineModule.h"
#include <fstream>
#include <filesystem>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test basic module config creation and validation
 * Requirements: 2.7, 7.2 (module configuration system with validation)
 */
bool TestModuleConfigCreation() {
    TestOutput::PrintTestStart("module config creation and validation");

    // Test default module config creation
    ModuleConfig config = ModuleConfigLoader::CreateDefaultModuleConfig("TestModule", ModuleType::Graphics);
    
    EXPECT_EQUAL(config.name, "TestModule");
    EXPECT_EQUAL(config.version, "1.0.0");
    EXPECT_TRUE(config.enabled);
    EXPECT_TRUE(config.parameters.find("renderer") != config.parameters.end());
    EXPECT_EQUAL(config.parameters.at("renderer"), "OpenGL");

    // Test module config validation
    ConfigValidationResult result = ModuleConfigLoader::ValidateModuleConfig(config);
    EXPECT_TRUE(result.isValid);

    TestOutput::PrintTestPass("module config creation and validation");
    return true;
}

/**
 * Test engine config creation and validation
 * Requirements: 2.7, 7.2 (engine configuration system with validation)
 */
bool TestEngineConfigCreation() {
    TestOutput::PrintTestStart("engine config creation and validation");

    // Test default engine config creation
    EngineConfig config = ModuleConfigLoader::CreateDefaultConfig();
    
    EXPECT_EQUAL(config.engineVersion, "1.0.0");
    EXPECT_EQUAL(config.configVersion, "1.0.0");
    EXPECT_TRUE(config.modules.size() > 0);
    EXPECT_EQUAL(config.modules[0].name, "Core");

    // Test engine config validation
    ConfigValidationResult result = ModuleConfigLoader::ValidateConfig(config);
    EXPECT_TRUE(result.isValid);

    TestOutput::PrintTestPass("engine config creation and validation");
    return true;
}

/**
 * Test JSON serialization and deserialization
 * Requirements: 2.7, 7.2 (JSON-based configuration loading)
 */
bool TestJsonSerialization() {
    TestOutput::PrintTestStart("JSON serialization and deserialization");

    // Create test config
    EngineConfig originalConfig = ModuleConfigLoader::CreateDefaultConfig();
    
    // Add a graphics module
    ModuleConfig graphicsModule = ModuleConfigLoader::CreateDefaultModuleConfig("Graphics", ModuleType::Graphics);
    graphicsModule.parameters["vsync"] = "false";
    graphicsModule.parameters["fullscreen"] = "true";
    originalConfig.modules.push_back(graphicsModule);

    // Serialize to JSON string
    std::string jsonString = ModuleConfigLoader::SaveToString(originalConfig);
    EXPECT_TRUE(!jsonString.empty());

    // Deserialize from JSON string
    auto loadedConfig = ModuleConfigLoader::LoadFromString(jsonString);
    EXPECT_TRUE(loadedConfig.has_value());

    // Verify loaded config matches original
    EXPECT_EQUAL(loadedConfig->engineVersion, originalConfig.engineVersion);
    EXPECT_EQUAL(loadedConfig->configVersion, originalConfig.configVersion);
    EXPECT_EQUAL(loadedConfig->modules.size(), originalConfig.modules.size());
    
    // Check graphics module
    bool foundGraphicsModule = false;
    for (const auto& module : loadedConfig->modules) {
        if (module.name == "Graphics") {
            foundGraphicsModule = true;
            EXPECT_EQUAL(module.version, "1.0.0");
            EXPECT_TRUE(module.enabled);
            EXPECT_EQUAL(module.parameters.at("vsync"), "false");
            EXPECT_EQUAL(module.parameters.at("fullscreen"), "true");
            break;
        }
    }
    EXPECT_TRUE(foundGraphicsModule);

    TestOutput::PrintTestPass("JSON serialization and deserialization");
    return true;
}

/**
 * Test file I/O operations
 * Requirements: 2.7, 7.2 (JSON-based configuration loading from files)
 */
bool TestFileOperations() {
    TestOutput::PrintTestStart("file I/O operations");

    // Create test config
    EngineConfig config = ModuleConfigLoader::CreateDefaultConfig();
    ModuleConfig audioModule = ModuleConfigLoader::CreateDefaultModuleConfig("Audio", ModuleType::Audio);
    config.modules.push_back(audioModule);

    // Test file path
    std::string testFilePath = "test_config.json";

    // Save to file
    bool saveResult = ModuleConfigLoader::SaveToFile(config, testFilePath);
    EXPECT_TRUE(saveResult);
    EXPECT_TRUE(std::filesystem::exists(testFilePath));

    // Load from file
    auto loadedConfig = ModuleConfigLoader::LoadFromFile(testFilePath);
    EXPECT_TRUE(loadedConfig.has_value());
    EXPECT_EQUAL(loadedConfig->modules.size(), config.modules.size());

    // Cleanup
    std::filesystem::remove(testFilePath);

    TestOutput::PrintTestPass("file I/O operations");
    return true;
}

/**
 * Test configuration validation with invalid data
 * Requirements: 7.3 (configuration error handling with descriptive error messages)
 */
bool TestConfigValidationErrors() {
    TestOutput::PrintTestStart("configuration validation with errors");

    // Test invalid module name
    ModuleConfig invalidModule;
    invalidModule.name = "123InvalidName"; // Should start with letter
    invalidModule.version = "1.0.0";
    invalidModule.enabled = true;

    ConfigValidationResult result = ModuleConfigLoader::ValidateModuleConfig(invalidModule);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQUAL(static_cast<int>(result.errorType), static_cast<int>(ConfigError::InvalidModuleName));

    // Test missing version
    ModuleConfig missingVersionModule;
    missingVersionModule.name = "ValidName";
    missingVersionModule.version = ""; // Empty version
    missingVersionModule.enabled = true;

    result = ModuleConfigLoader::ValidateModuleConfig(missingVersionModule);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQUAL(static_cast<int>(result.errorType), static_cast<int>(ConfigError::MissingRequiredField));

    // Test invalid version format
    ModuleConfig invalidVersionModule;
    invalidVersionModule.name = "ValidName";
    invalidVersionModule.version = "1.0"; // Should be major.minor.patch
    invalidVersionModule.enabled = true;

    result = ModuleConfigLoader::ValidateModuleConfig(invalidVersionModule);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQUAL(static_cast<int>(result.errorType), static_cast<int>(ConfigError::InvalidVersion));

    // Test duplicate modules in engine config
    EngineConfig duplicateConfig;
    duplicateConfig.engineVersion = "1.0.0";
    duplicateConfig.configVersion = "1.0.0";
    
    ModuleConfig module1;
    module1.name = "TestModule";
    module1.version = "1.0.0";
    module1.enabled = true;
    
    ModuleConfig module2 = module1; // Same name
    
    duplicateConfig.modules.push_back(module1);
    duplicateConfig.modules.push_back(module2);

    result = ModuleConfigLoader::ValidateConfig(duplicateConfig);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQUAL(static_cast<int>(result.errorType), static_cast<int>(ConfigError::DuplicateModule));

    TestOutput::PrintTestPass("configuration validation with errors");
    return true;
}

/**
 * Test error message generation
 * Requirements: 7.3 (descriptive error messages)
 */
bool TestErrorMessages() {
    TestOutput::PrintTestStart("error message generation");

    // Test basic error messages
    std::string fileNotFoundMsg = ModuleConfigLoader::GetErrorMessage(ConfigError::FileNotFound);
    EXPECT_TRUE(!fileNotFoundMsg.empty());
    EXPECT_TRUE(fileNotFoundMsg.find("not found") != std::string::npos);

    std::string invalidJsonMsg = ModuleConfigLoader::GetErrorMessage(ConfigError::InvalidJson);
    EXPECT_TRUE(!invalidJsonMsg.empty());
    EXPECT_TRUE(invalidJsonMsg.find("JSON") != std::string::npos);

    // Test detailed error messages
    ConfigValidationResult result;
    result.isValid = false;
    result.errorType = ConfigError::InvalidModuleName;
    result.errorMessage = "Module name contains invalid characters";
    result.fieldName = "modules.TestModule.name";
    result.lineNumber = 5;

    std::string detailedMsg = ModuleConfigLoader::GetDetailedErrorMessage(result);
    EXPECT_TRUE(!detailedMsg.empty());
    EXPECT_TRUE(detailedMsg.find("TestModule") != std::string::npos);
    EXPECT_TRUE(detailedMsg.find("line 5") != std::string::npos);

    TestOutput::PrintTestPass("error message generation");
    return true;
}

/**
 * Test invalid JSON parsing
 * Requirements: 7.3 (configuration error handling)
 */
bool TestInvalidJsonParsing() {
    TestOutput::PrintTestStart("invalid JSON parsing");

    // Test malformed JSON
    std::string malformedJson = R"({
        "engineVersion": "1.0.0",
        "configVersion": "1.0.0"
        "modules": [
            {
                "name": "TestModule",
                "version": "1.0.0"
            }
        ]
    })"; // Missing comma after configVersion

    auto result = ModuleConfigLoader::LoadFromString(malformedJson);
    EXPECT_FALSE(result.has_value());

    // Test JSON with wrong field types
    std::string wrongTypesJson = R"({
        "engineVersion": 1.0,
        "configVersion": "1.0.0",
        "modules": [
            {
                "name": "TestModule",
                "version": "1.0.0",
                "enabled": "true"
            }
        ]
    })"; // engineVersion should be string, enabled should be boolean

    result = ModuleConfigLoader::LoadFromString(wrongTypesJson);
    EXPECT_FALSE(result.has_value());

    TestOutput::PrintTestPass("invalid JSON parsing");
    return true;
}

int main() {
    TestOutput::PrintHeader("ModuleConfigLoader");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ModuleConfigLoader Tests");

        // Run all tests
        allPassed &= suite.RunTest("Module Config Creation", TestModuleConfigCreation);
        allPassed &= suite.RunTest("Engine Config Creation", TestEngineConfigCreation);
        allPassed &= suite.RunTest("JSON Serialization", TestJsonSerialization);
        allPassed &= suite.RunTest("File Operations", TestFileOperations);
        allPassed &= suite.RunTest("Config Validation Errors", TestConfigValidationErrors);
        allPassed &= suite.RunTest("Error Messages", TestErrorMessages);
        allPassed &= suite.RunTest("Invalid JSON Parsing", TestInvalidJsonParsing);

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