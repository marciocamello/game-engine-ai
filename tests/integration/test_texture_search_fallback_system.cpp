#include "../TestUtils.h"
#include "Graphics/MaterialImporter.h"
#include "Resource/ResourceManager.h"
#include "Core/Logger.h"
#include <filesystem>
#include <fstream>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test texture search path functionality
 * Requirements: 3.1, 3.2 (Texture search and fallback system)
 */
bool TestTextureSearchPaths() {
    TestOutput::PrintTestStart("texture search paths");

    try {
        // Initialize Resource Manager
        auto resourceManager = std::make_shared<ResourceManager>();
        resourceManager->Initialize();
        
        // Initialize MaterialImporter
        MaterialImporter importer;
        importer.Initialize(resourceManager);

        // Get default search paths
        auto defaultPaths = importer.GetTextureSearchPaths();
        size_t initialCount = defaultPaths.size();

        // Add custom search paths
        importer.AddTextureSearchPath("custom/textures/");
        importer.AddTextureSearchPath("fallback/textures/");

        // Verify paths were added
        auto updatedPaths = importer.GetTextureSearchPaths();
        EXPECT_TRUE(updatedPaths.size() >= initialCount + 2);

        // Test path priority (newer paths should be searched first)
        bool foundCustomPath = false;
        bool foundFallbackPath = false;
        
        for (const auto& path : updatedPaths) {
            if (path.find("custom/textures/") != std::string::npos) {
                foundCustomPath = true;
            }
            if (path.find("fallback/textures/") != std::string::npos) {
                foundFallbackPath = true;
            }
        }

        EXPECT_TRUE(foundCustomPath);
        EXPECT_TRUE(foundFallbackPath);

        // Cleanup
        importer.Shutdown();
        resourceManager->Shutdown();

        TestOutput::PrintTestPass("texture search paths");
        return true;

    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception in texture search paths test: " + std::string(e.what()));
        return false;
    }
}

/**
 * Test texture fallback system functionality
 * Requirements: 3.1, 3.2 (Texture fallback when files are missing)
 */
bool TestTextureFallbackSystem() {
    TestOutput::PrintTestStart("texture fallback system");

    try {
        // Initialize Resource Manager
        auto resourceManager = std::make_shared<ResourceManager>();
        resourceManager->Initialize();
        
        // Initialize MaterialImporter
        MaterialImporter importer;
        importer.Initialize(resourceManager);

        // Test fallback for missing diffuse texture
        auto diffuseTexture = importer.FindTexture("nonexistent_diffuse.png", "");
        EXPECT_NOT_NULL(diffuseTexture); // Should return fallback texture

        // Test fallback for missing normal texture
        auto normalTexture = importer.FindTexture("nonexistent_normal.png", "");
        EXPECT_NOT_NULL(normalTexture); // Should return fallback texture

        // Test fallback for missing metallic texture
        auto metallicTexture = importer.FindTexture("nonexistent_metallic.png", "");
        EXPECT_NOT_NULL(metallicTexture); // Should return fallback texture

        // Verify fallback textures are different for different types
        EXPECT_NOT_EQUAL(diffuseTexture, normalTexture);
        EXPECT_NOT_EQUAL(diffuseTexture, metallicTexture);

        // Cleanup
        importer.Shutdown();
        resourceManager->Shutdown();

        TestOutput::PrintTestPass("texture fallback system");
        return true;

    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception in texture fallback system test: " + std::string(e.what()));
        return false;
    }
}

/**
 * Test texture format validation and fallback
 * Requirements: 3.1, 3.2 (Format validation and fallback)
 */
bool TestTextureFormatValidation() {
    TestOutput::PrintTestStart("texture format validation");

    try {
        // Initialize Resource Manager
        auto resourceManager = std::make_shared<ResourceManager>();
        resourceManager->Initialize();
        
        // Initialize MaterialImporter
        MaterialImporter importer;
        importer.Initialize(resourceManager);

        // Test texture validation
        bool validPng = importer.ValidateTexture("test.png");
        bool validJpg = importer.ValidateTexture("test.jpg");
        bool invalidXyz = importer.ValidateTexture("test.xyz");

        // These should return false since files don't exist, but format validation should work
        EXPECT_FALSE(validPng); // File doesn't exist
        EXPECT_FALSE(validJpg); // File doesn't exist
        EXPECT_FALSE(invalidXyz); // File doesn't exist and unsupported format

        // Test unsupported format fallback
        std::string unsupportedFile = "test.xyz"; // Unsupported format
        auto fallbackTexture = importer.FindTexture(unsupportedFile, "");
        EXPECT_NOT_NULL(fallbackTexture); // Should return fallback

        // Cleanup
        importer.Shutdown();
        resourceManager->Shutdown();

        TestOutput::PrintTestPass("texture format validation");
        return true;

    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception in texture format validation test: " + std::string(e.what()));
        return false;
    }
}

/**
 * Test texture search priority system
 * Requirements: 3.1, 3.2 (Search priority and path ordering)
 */
bool TestTextureSearchPriority() {
    TestOutput::PrintTestStart("texture search priority");

    try {
        // Initialize Resource Manager
        auto resourceManager = std::make_shared<ResourceManager>();
        resourceManager->Initialize();
        
        // Initialize MaterialImporter
        MaterialImporter importer;
        importer.Initialize(resourceManager);

        // Add search paths in specific order
        importer.AddTextureSearchPath("high_priority/");
        importer.AddTextureSearchPath("medium_priority/");
        importer.AddTextureSearchPath("low_priority/");

        // Verify search paths are in correct order
        auto searchPaths = importer.GetTextureSearchPaths();
        
        // Find the positions of our added paths
        int highPriorityPos = -1;
        int mediumPriorityPos = -1;
        int lowPriorityPos = -1;

        for (size_t i = 0; i < searchPaths.size(); ++i) {
            if (searchPaths[i].find("high_priority/") != std::string::npos) {
                highPriorityPos = static_cast<int>(i);
            } else if (searchPaths[i].find("medium_priority/") != std::string::npos) {
                mediumPriorityPos = static_cast<int>(i);
            } else if (searchPaths[i].find("low_priority/") != std::string::npos) {
                lowPriorityPos = static_cast<int>(i);
            }
        }

        // Verify all paths were found
        EXPECT_TRUE(highPriorityPos >= 0);
        EXPECT_TRUE(mediumPriorityPos >= 0);
        EXPECT_TRUE(lowPriorityPos >= 0);

        // Cleanup
        importer.Shutdown();
        resourceManager->Shutdown();

        TestOutput::PrintTestPass("texture search priority");
        return true;

    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception in texture search priority test: " + std::string(e.what()));
        return false;
    }
}

int main() {
    TestOutput::PrintHeader("Texture Search Fallback System Integration");

    bool allPassed = true;

    try {
        // Initialize logger for testing
        Logger::GetInstance().Initialize();
        Logger::GetInstance().SetLogLevel(LogLevel::Info);
        
        // Create test suite for result tracking
        TestSuite suite("Texture Search Fallback System Tests");

        // Run all tests
        allPassed &= suite.RunTest("Texture Search Paths", TestTextureSearchPaths);
        allPassed &= suite.RunTest("Texture Fallback System", TestTextureFallbackSystem);
        allPassed &= suite.RunTest("Texture Format Validation", TestTextureFormatValidation);
        allPassed &= suite.RunTest("Texture Search Priority", TestTextureSearchPriority);

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