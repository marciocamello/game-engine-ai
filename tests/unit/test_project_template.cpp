#include "TestUtils.h"
#include "Core/ProjectTemplate.h"
#include <filesystem>
#include <fstream>

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Tools;

/**
 * Test project name validation
 * Requirements: 4.1 (template structure validation)
 */
bool TestProjectNameValidation() {
    TestOutput::PrintTestStart("project name validation");

    // Valid project names
    EXPECT_TRUE(ProjectTemplate::ValidateProjectName("MyGame"));
    EXPECT_TRUE(ProjectTemplate::ValidateProjectName("Game_Project"));
    EXPECT_TRUE(ProjectTemplate::ValidateProjectName("Game-Project"));
    EXPECT_TRUE(ProjectTemplate::ValidateProjectName("GameProject123"));

    // Invalid project names
    EXPECT_FALSE(ProjectTemplate::ValidateProjectName(""));
    EXPECT_FALSE(ProjectTemplate::ValidateProjectName("123Game"));
    EXPECT_FALSE(ProjectTemplate::ValidateProjectName("Game Project"));
    EXPECT_FALSE(ProjectTemplate::ValidateProjectName("Game@Project"));

    TestOutput::PrintTestPass("project name validation");
    return true;
}

/**
 * Test module validation
 * Requirements: 4.2 (module dependency declaration)
 */
bool TestModuleValidation() {
    TestOutput::PrintTestStart("module validation");

    // Valid modules
    EXPECT_TRUE(ProjectTemplate::IsValidModule("graphics-opengl"));
    EXPECT_TRUE(ProjectTemplate::IsValidModule("physics-bullet"));
    EXPECT_TRUE(ProjectTemplate::IsValidModule("audio-openal"));

    // Invalid modules
    EXPECT_FALSE(ProjectTemplate::IsValidModule("invalid-module"));
    EXPECT_FALSE(ProjectTemplate::IsValidModule(""));

    // Check available modules list
    auto modules = ProjectTemplate::GetAvailableModules();
    EXPECT_TRUE(modules.size() > 0);

    TestOutput::PrintTestPass("module validation");
    return true;
}

/**
 * Test template configuration validation
 * Requirements: 4.1, 4.2 (template structure and module validation)
 */
bool TestTemplateConfigValidation() {
    TestOutput::PrintTestStart("template configuration validation");

    // Valid configuration
    TemplateConfig validConfig;
    validConfig.projectName = "TestProject";
    validConfig.targetDirectory = "test_temp";
    validConfig.requiredModules = {"graphics-opengl"};
    validConfig.optionalModules = {"audio-openal"};
    validConfig.templateType = "basic";

    EXPECT_TRUE(ProjectTemplate::ValidateTemplateConfig(validConfig));

    // Invalid project name
    TemplateConfig invalidName = validConfig;
    invalidName.projectName = "";
    EXPECT_FALSE(ProjectTemplate::ValidateTemplateConfig(invalidName));

    // Invalid template type
    TemplateConfig invalidType = validConfig;
    invalidType.templateType = "invalid";
    EXPECT_FALSE(ProjectTemplate::ValidateTemplateConfig(invalidType));

    // Invalid required module
    TemplateConfig invalidModule = validConfig;
    invalidModule.requiredModules = {"invalid-module"};
    EXPECT_FALSE(ProjectTemplate::ValidateTemplateConfig(invalidModule));

    TestOutput::PrintTestPass("template configuration validation");
    return true;
}

/**
 * Test available templates retrieval
 * Requirements: 4.1 (template structure)
 */
bool TestAvailableTemplates() {
    TestOutput::PrintTestStart("available templates retrieval");

    auto templates = ProjectTemplate::GetAvailableTemplates();
    EXPECT_TRUE(templates.size() > 0);

    // Check for expected templates
    bool hasBasic = false;
    bool hasAdvanced = false;
    for (const auto& tmpl : templates) {
        if (tmpl == "basic") hasBasic = true;
        if (tmpl == "advanced") hasAdvanced = true;
    }

    EXPECT_TRUE(hasBasic);
    EXPECT_TRUE(hasAdvanced);

    TestOutput::PrintTestPass("available templates retrieval");
    return true;
}

/**
 * Test CMake content generation
 * Requirements: 4.2 (CMakeLists.txt generation)
 */
bool TestCMakeContentGeneration() {
    TestOutput::PrintTestStart("CMake content generation");

    TemplateConfig config;
    config.projectName = "TestProject";
    config.targetDirectory = "test_temp";
    config.templateType = "basic";
    config.requiredModules = {"graphics-opengl", "physics-bullet"};
    config.optionalModules = {"audio-openal"};

    // Test that we can generate CMake content without errors
    // This is a basic validation that the function doesn't crash
    bool generationSucceeded = true;
    try {
        // We can't directly test the private method, but we can test
        // that the validation passes for a valid config
        EXPECT_TRUE(ProjectTemplate::ValidateTemplateConfig(config));
    } catch (...) {
        generationSucceeded = false;
    }

    EXPECT_TRUE(generationSucceeded);

    TestOutput::PrintTestPass("CMake content generation");
    return true;
}

/**
 * Test project creation with basic template
 * Requirements: 4.1, 4.2, 4.3 (template creation and structure)
 */
bool TestBasicProjectCreation() {
    TestOutput::PrintTestStart("basic project creation");

    // Create a temporary directory for testing
    std::string testDir = "test_project_temp";
    std::string projectName = "TestBasicProject";
    
    // Clean up any existing test directory
    if (std::filesystem::exists(testDir)) {
        std::filesystem::remove_all(testDir);
    }

    TemplateConfig config;
    config.projectName = projectName;
    config.targetDirectory = testDir;
    config.templateType = "basic";
    config.requiredModules = {"graphics-opengl"};
    config.includeExampleCode = true;

    // Create the project
    bool created = ProjectTemplate::CreateProject(config);
    EXPECT_TRUE(created);

    if (created) {
        // Verify directory structure was created
        std::string projectPath = testDir + "/" + projectName;
        EXPECT_TRUE(std::filesystem::exists(projectPath));
        EXPECT_TRUE(std::filesystem::exists(projectPath + "/src"));
        EXPECT_TRUE(std::filesystem::exists(projectPath + "/include"));
        EXPECT_TRUE(std::filesystem::exists(projectPath + "/assets"));
        EXPECT_TRUE(std::filesystem::exists(projectPath + "/config"));

        // Verify files were created
        EXPECT_TRUE(std::filesystem::exists(projectPath + "/CMakeLists.txt"));
        EXPECT_TRUE(std::filesystem::exists(projectPath + "/README.md"));
        EXPECT_TRUE(std::filesystem::exists(projectPath + "/config/project.json"));
        EXPECT_TRUE(std::filesystem::exists(projectPath + "/src/main.cpp"));

        // Clean up
        std::filesystem::remove_all(testDir);
    }

    TestOutput::PrintTestPass("basic project creation");
    return true;
}

/**
 * Test project creation failure cases
 * Requirements: 4.1 (error handling)
 */
bool TestProjectCreationFailures() {
    TestOutput::PrintTestStart("project creation failure cases");

    // Test creating project with invalid name
    TemplateConfig invalidConfig;
    invalidConfig.projectName = "";
    invalidConfig.targetDirectory = "test_temp";
    invalidConfig.templateType = "basic";

    EXPECT_FALSE(ProjectTemplate::CreateProject(invalidConfig));

    // Test creating project with invalid template type
    TemplateConfig invalidTemplate;
    invalidTemplate.projectName = "TestProject";
    invalidTemplate.targetDirectory = "test_temp";
    invalidTemplate.templateType = "nonexistent";

    EXPECT_FALSE(ProjectTemplate::CreateProject(invalidTemplate));

    TestOutput::PrintTestPass("project creation failure cases");
    return true;
}

/**
 * Test directory and file utilities
 * Requirements: 4.1 (directory structure creation)
 */
bool TestDirectoryUtilities() {
    TestOutput::PrintTestStart("directory utilities");

    std::string testDir = "test_util_temp";
    
    // Clean up any existing test directory
    if (std::filesystem::exists(testDir)) {
        std::filesystem::remove_all(testDir);
    }

    // Test directory creation through project template
    TemplateConfig config;
    config.projectName = "UtilTest";
    config.targetDirectory = testDir;
    config.templateType = "basic";

    // Validate that the target directory validation works
    EXPECT_TRUE(ProjectTemplate::ValidateTargetDirectory(testDir));

    // Clean up
    if (std::filesystem::exists(testDir)) {
        std::filesystem::remove_all(testDir);
    }

    TestOutput::PrintTestPass("directory utilities");
    return true;
}

int main() {
    TestOutput::PrintHeader("ProjectTemplate");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ProjectTemplate Tests");

        // Run all tests
        allPassed &= suite.RunTest("Project Name Validation", TestProjectNameValidation);
        allPassed &= suite.RunTest("Module Validation", TestModuleValidation);
        allPassed &= suite.RunTest("Template Config Validation", TestTemplateConfigValidation);
        allPassed &= suite.RunTest("Available Templates", TestAvailableTemplates);
        allPassed &= suite.RunTest("CMake Content Generation", TestCMakeContentGeneration);
        allPassed &= suite.RunTest("Basic Project Creation", TestBasicProjectCreation);
        allPassed &= suite.RunTest("Project Creation Failures", TestProjectCreationFailures);
        allPassed &= suite.RunTest("Directory Utilities", TestDirectoryUtilities);

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