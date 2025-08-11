#include "TestUtils.h"
#include "Core/ProjectConfigLoader.h"
#include <filesystem>
#include <fstream>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test project configuration creation and validation
 * Requirements: 4.5, 7.1, 7.2 (project configuration system)
 */
bool TestProjectConfigCreation() {
    TestOutput::PrintTestStart("project configuration creation");

    // Test default configuration creation
    ProjectConfig defaultConfig = ProjectConfigLoader::CreateDefaultConfig("TestProject");
    
    EXPECT_EQUAL(defaultConfig.projectName, "TestProject");
    EXPECT_EQUAL(defaultConfig.projectVersion, "1.0.0");
    EXPECT_FALSE(defaultConfig.requiredModules.empty());
    EXPECT_EQUAL(defaultConfig.assetPath, "assets/");
    EXPECT_EQUAL(defaultConfig.configPath, "config/");
    EXPECT_EQUAL(defaultConfig.buildPath, "build/");

    // Test basic game configuration creation
    ProjectConfig gameConfig = ProjectConfigLoader::CreateBasicGameConfig("GameProject");
    
    EXPECT_EQUAL(gameConfig.projectName, "GameProject");
    EXPECT_TRUE(gameConfig.requiredModules.size() >= 4); // Should include Core, Graphics, Input, Physics, Audio
    
    // Test minimal configuration creation
    ProjectConfig minimalConfig = ProjectConfigLoader::CreateMinimalConfig("MinimalProject");
    
    EXPECT_EQUAL(minimalConfig.projectName, "MinimalProject");
    EXPECT_TRUE(minimalConfig.requiredModules.size() >= 2); // Should include Core, Graphics
    EXPECT_TRUE(minimalConfig.optionalModules.empty());

    TestOutput::PrintTestPass("project configuration creation");
    return true;
}

/**
 * Test project configuration validation
 * Requirements: 4.5, 7.2 (project configuration validation)
 */
bool TestProjectConfigValidation() {
    TestOutput::PrintTestStart("project configuration validation");

    // Test valid configuration
    ProjectConfig validConfig = ProjectConfigLoader::CreateDefaultConfig("ValidProject");
    ProjectConfigValidationResult result = ProjectConfigLoader::ValidateConfig(validConfig);
    
    EXPECT_TRUE(result.isValid);

    // Test invalid project name
    ProjectConfig invalidNameConfig = validConfig;
    invalidNameConfig.projectName = ""; // Empty name
    result = ProjectConfigLoader::ValidateConfig(invalidNameConfig);
    
    EXPECT_FALSE(result.isValid);
    EXPECT_EQUAL(static_cast<int>(result.errorType), static_cast<int>(ProjectConfigError::MissingRequiredField));

    // Test invalid version
    ProjectConfig invalidVersionConfig = validConfig;
    invalidVersionConfig.projectVersion = "invalid.version"; // Invalid format
    result = ProjectConfigLoader::ValidateConfig(invalidVersionConfig);
    
    EXPECT_FALSE(result.isValid);
    EXPECT_EQUAL(static_cast<int>(result.errorType), static_cast<int>(ProjectConfigError::InvalidVersion));

    // Test duplicate modules
    ProjectConfig duplicateModuleConfig = validConfig;
    duplicateModuleConfig.requiredModules.push_back("Core");
    duplicateModuleConfig.optionalModules.push_back("Core"); // Duplicate in both lists
    result = ProjectConfigLoader::ValidateModuleDependencies(
        duplicateModuleConfig.requiredModules, duplicateModuleConfig.optionalModules);
    
    EXPECT_FALSE(result.isValid);
    EXPECT_EQUAL(static_cast<int>(result.errorType), static_cast<int>(ProjectConfigError::DuplicateModule));

    TestOutput::PrintTestPass("project configuration validation");
    return true;
}

/**
 * Test project configuration JSON serialization and deserialization
 * Requirements: 7.1, 7.2 (project configuration loading and validation)
 */
bool TestProjectConfigSerialization() {
    TestOutput::PrintTestStart("project configuration serialization");

    // Create a test configuration
    ProjectConfig originalConfig = ProjectConfigLoader::CreateBasicGameConfig("SerializationTest");
    originalConfig.description = "Test project for serialization";
    originalConfig.author = "Test Author";
    originalConfig.minEngineVersion = "1.0.0";
    originalConfig.maxEngineVersion = "2.0.0";
    originalConfig.projectSettings["customSetting"] = "customValue";

    // Serialize to JSON string
    std::string jsonString = ProjectConfigLoader::SaveToString(originalConfig);
    EXPECT_FALSE(jsonString.empty());

    // Deserialize from JSON string
    auto loadedConfigOpt = ProjectConfigLoader::LoadFromString(jsonString);
    EXPECT_TRUE(loadedConfigOpt.has_value());

    if (loadedConfigOpt) {
        ProjectConfig loadedConfig = *loadedConfigOpt;
        
        // Verify all fields are preserved
        EXPECT_EQUAL(loadedConfig.projectName, originalConfig.projectName);
        EXPECT_EQUAL(loadedConfig.projectVersion, originalConfig.projectVersion);
        EXPECT_EQUAL(loadedConfig.description, originalConfig.description);
        EXPECT_EQUAL(loadedConfig.author, originalConfig.author);
        EXPECT_EQUAL(loadedConfig.minEngineVersion, originalConfig.minEngineVersion);
        EXPECT_EQUAL(loadedConfig.maxEngineVersion, originalConfig.maxEngineVersion);
        EXPECT_EQUAL(loadedConfig.assetPath, originalConfig.assetPath);
        EXPECT_EQUAL(loadedConfig.configPath, originalConfig.configPath);
        EXPECT_EQUAL(loadedConfig.buildPath, originalConfig.buildPath);
        
        // Verify module dependencies
        EXPECT_EQUAL(loadedConfig.requiredModules.size(), originalConfig.requiredModules.size());
        EXPECT_EQUAL(loadedConfig.optionalModules.size(), originalConfig.optionalModules.size());
        
        // Verify project settings
        EXPECT_EQUAL(loadedConfig.projectSettings.size(), originalConfig.projectSettings.size());
        if (loadedConfig.projectSettings.find("customSetting") != loadedConfig.projectSettings.end()) {
            EXPECT_EQUAL(loadedConfig.projectSettings.at("customSetting"), "customValue");
        }
    }

    TestOutput::PrintTestPass("project configuration serialization");
    return true;
}

/**
 * Test project configuration file operations
 * Requirements: 7.1 (project configuration file management)
 */
bool TestProjectConfigFileOperations() {
    TestOutput::PrintTestStart("project configuration file operations");

    // Create a test configuration
    ProjectConfig testConfig = ProjectConfigLoader::CreateDefaultConfig("FileTestProject");
    testConfig.description = "Test project for file operations";
    
    // Test file path
    std::string testFilePath = "test_project_config.json";
    
    // Clean up any existing test file
    if (std::filesystem::exists(testFilePath)) {
        std::filesystem::remove(testFilePath);
    }

    // Save configuration to file
    bool saveResult = ProjectConfigLoader::SaveToFile(testConfig, testFilePath);
    EXPECT_TRUE(saveResult);
    EXPECT_TRUE(std::filesystem::exists(testFilePath));

    // Load configuration from file
    auto loadedConfigOpt = ProjectConfigLoader::LoadFromFile(testFilePath);
    EXPECT_TRUE(loadedConfigOpt.has_value());

    if (loadedConfigOpt) {
        ProjectConfig loadedConfig = *loadedConfigOpt;
        EXPECT_EQUAL(loadedConfig.projectName, testConfig.projectName);
        EXPECT_EQUAL(loadedConfig.description, testConfig.description);
    }

    // Test loading non-existent file
    auto nonExistentConfigOpt = ProjectConfigLoader::LoadFromFile("non_existent_config.json");
    EXPECT_FALSE(nonExistentConfigOpt.has_value());

    // Clean up test file
    if (std::filesystem::exists(testFilePath)) {
        std::filesystem::remove(testFilePath);
    }

    TestOutput::PrintTestPass("project configuration file operations");
    return true;
}

/**
 * Test module name validation
 * Requirements: 4.5 (module dependency declaration)
 */
bool TestModuleNameValidation() {
    TestOutput::PrintTestStart("module name validation");

    // Test valid module names
    EXPECT_TRUE(ProjectConfigLoader::IsValidModuleName("Core"));
    EXPECT_TRUE(ProjectConfigLoader::IsValidModuleName("Graphics"));
    EXPECT_TRUE(ProjectConfigLoader::IsValidModuleName("Physics_Engine"));
    EXPECT_TRUE(ProjectConfigLoader::IsValidModuleName("Audio3D"));

    // Test invalid module names
    EXPECT_FALSE(ProjectConfigLoader::IsValidModuleName("")); // Empty
    EXPECT_FALSE(ProjectConfigLoader::IsValidModuleName("123Invalid")); // Starts with number
    EXPECT_FALSE(ProjectConfigLoader::IsValidModuleName("Invalid-Name")); // Contains hyphen
    EXPECT_FALSE(ProjectConfigLoader::IsValidModuleName("Invalid Name")); // Contains space
    EXPECT_FALSE(ProjectConfigLoader::IsValidModuleName("Invalid.Name")); // Contains dot

    TestOutput::PrintTestPass("module name validation");
    return true;
}

/**
 * Test engine version compatibility checking
 * Requirements: 7.2 (configuration validation)
 */
bool TestEngineVersionCompatibility() {
    TestOutput::PrintTestStart("engine version compatibility");

    // Test compatible versions
    EXPECT_TRUE(ProjectConfigLoader::IsCompatibleEngineVersion("1.5.0", "1.0.0", "2.0.0"));
    EXPECT_TRUE(ProjectConfigLoader::IsCompatibleEngineVersion("1.0.0", "1.0.0", "2.0.0")); // Exact min
    EXPECT_TRUE(ProjectConfigLoader::IsCompatibleEngineVersion("2.0.0", "1.0.0", "2.0.0")); // Exact max

    // Test incompatible versions
    EXPECT_FALSE(ProjectConfigLoader::IsCompatibleEngineVersion("0.9.0", "1.0.0", "2.0.0")); // Below min
    EXPECT_FALSE(ProjectConfigLoader::IsCompatibleEngineVersion("2.1.0", "1.0.0", "2.0.0")); // Above max

    // Test with empty constraints
    EXPECT_TRUE(ProjectConfigLoader::IsCompatibleEngineVersion("1.5.0", "", "")); // No constraints
    EXPECT_TRUE(ProjectConfigLoader::IsCompatibleEngineVersion("1.5.0", "1.0.0", "")); // Only min
    EXPECT_TRUE(ProjectConfigLoader::IsCompatibleEngineVersion("1.5.0", "", "2.0.0")); // Only max

    TestOutput::PrintTestPass("engine version compatibility");
    return true;
}

/**
 * Test error message generation
 * Requirements: 7.2 (configuration error handling)
 */
bool TestErrorMessageGeneration() {
    TestOutput::PrintTestStart("error message generation");

    // Test basic error messages
    std::string fileNotFoundMsg = ProjectConfigLoader::GetErrorMessage(ProjectConfigError::FileNotFound);
    EXPECT_FALSE(fileNotFoundMsg.empty());

    std::string invalidJsonMsg = ProjectConfigLoader::GetErrorMessage(ProjectConfigError::InvalidJson);
    EXPECT_FALSE(invalidJsonMsg.empty());

    // Test detailed error message
    ProjectConfigValidationResult result;
    result.isValid = false;
    result.errorType = ProjectConfigError::InvalidProjectName;
    result.errorMessage = "Test error message";
    result.fieldName = "projectName";
    result.lineNumber = 5;

    std::string detailedMsg = ProjectConfigLoader::GetDetailedErrorMessage(result);
    EXPECT_FALSE(detailedMsg.empty());
    EXPECT_TRUE(detailedMsg.find("projectName") != std::string::npos);
    EXPECT_TRUE(detailedMsg.find("Test error message") != std::string::npos);

    TestOutput::PrintTestPass("error message generation");
    return true;
}

int main() {
    TestOutput::PrintHeader("ProjectConfig");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ProjectConfig Tests");

        // Run all tests
        allPassed &= suite.RunTest("Project Configuration Creation", TestProjectConfigCreation);
        allPassed &= suite.RunTest("Project Configuration Validation", TestProjectConfigValidation);
        allPassed &= suite.RunTest("Project Configuration Serialization", TestProjectConfigSerialization);
        allPassed &= suite.RunTest("Project Configuration File Operations", TestProjectConfigFileOperations);
        allPassed &= suite.RunTest("Module Name Validation", TestModuleNameValidation);
        allPassed &= suite.RunTest("Engine Version Compatibility", TestEngineVersionCompatibility);
        allPassed &= suite.RunTest("Error Message Generation", TestErrorMessageGeneration);

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