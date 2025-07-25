#include "Graphics/MaterialImporter.h"
#include "Resource/ResourceManager.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <filesystem>
#include <fstream>

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestMaterialImporterInitialization() {
    TestOutput::PrintTestStart("MaterialImporter initialization");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Test that importer is properly initialized
    auto searchPaths = importer.GetTextureSearchPaths();
    EXPECT_TRUE(searchPaths.size() > 0);
    
    TestOutput::PrintTestPass("MaterialImporter initialization");
    return true;
}

bool TestTextureSearchAndFallbackSystem() {
    TestOutput::PrintTestStart("texture search and fallback system");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Test 1: Default texture search paths
    auto searchPaths = importer.GetTextureSearchPaths();
    EXPECT_TRUE(searchPaths.size() > 0);
    TestOutput::PrintInfo("Default search paths count: " + std::to_string(searchPaths.size()));
    
    // Test 2: Add custom search path
    std::string customPath = "test_textures/";
    importer.AddTextureSearchPath(customPath);
    auto updatedPaths = importer.GetTextureSearchPaths();
    EXPECT_TRUE(updatedPaths.size() == searchPaths.size() + 1);
    
    // Test 3: Test supported texture formats
    auto supportedFormats = importer.GetSupportedTextureFormats();
    EXPECT_TRUE(supportedFormats.size() > 0);
    TestOutput::PrintInfo("Supported texture formats count: " + std::to_string(supportedFormats.size()));
    
    // Test 4: Check format support
    EXPECT_TRUE(importer.IsTextureFormatSupported(".png"));
    EXPECT_TRUE(importer.IsTextureFormatSupported(".jpg"));
    EXPECT_TRUE(importer.IsTextureFormatSupported(".jpeg"));
    EXPECT_FALSE(importer.IsTextureFormatSupported(".xyz"));
    
    // Test 5: Create fallback textures for different types
    auto diffuseFallback = importer.CreateFallbackTexture(TextureType::Diffuse, "missing_diffuse.png");
    EXPECT_NOT_NULL(diffuseFallback);
    
    auto normalFallback = importer.CreateFallbackTexture(TextureType::Normal, "missing_normal.png");
    EXPECT_NOT_NULL(normalFallback);
    
    auto metallicFallback = importer.CreateFallbackTexture(TextureType::Metallic, "missing_metallic.png");
    EXPECT_NOT_NULL(metallicFallback);
    
    // Test 6: Create default textures
    auto whiteTexture = importer.CreateDefaultTexture(TextureType::Diffuse);
    EXPECT_NOT_NULL(whiteTexture);
    
    auto normalTexture = importer.CreateDefaultTexture(TextureType::Normal);
    EXPECT_NOT_NULL(normalTexture);
    
    // Test 7: Test statistics
    size_t fallbackCount = importer.GetFallbackTextureCount();
    size_t missingCount = importer.GetMissingTextureCount();
    TestOutput::PrintInfo("Fallback textures created: " + std::to_string(fallbackCount));
    TestOutput::PrintInfo("Missing textures encountered: " + std::to_string(missingCount));
    
    // Test 8: Test texture finding with non-existent file
    auto foundTexture = importer.FindTexture("non_existent_texture.png", "");
    EXPECT_NULL(foundTexture); // Should return null for non-existent texture
    
    // Test 9: Clear cache and verify statistics reset
    importer.ClearCache();
    EXPECT_EQUAL(importer.GetImportedTextureCount(), static_cast<size_t>(0));
    
    TestOutput::PrintTestPass("texture search and fallback system");
    return true;
}

bool TestMaterialImportSettings() {
    TestOutput::PrintTestStart("material import settings");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Test default settings
    auto defaultSettings = importer.GetImportSettings();
    EXPECT_TRUE(defaultSettings.textureSearchPaths.size() > 0);
    EXPECT_TRUE(defaultSettings.generateMissingTextures);
    EXPECT_TRUE(defaultSettings.enableTextureConversion);
    
    // Test custom settings
    MaterialImportSettings customSettings;
    customSettings.conversionMode = MaterialConversionMode::ForcePBR;
    customSettings.textureSearchPaths = {"custom/path1/", "custom/path2/"};
    customSettings.generateMissingTextures = false;
    customSettings.enableTextureConversion = false;
    customSettings.defaultMetallic = 0.2f;
    customSettings.defaultRoughness = 0.8f;
    
    importer.SetImportSettings(customSettings);
    auto updatedSettings = importer.GetImportSettings();
    
    EXPECT_EQUAL(static_cast<int>(updatedSettings.conversionMode), static_cast<int>(MaterialConversionMode::ForcePBR));
    EXPECT_EQUAL(updatedSettings.textureSearchPaths.size(), static_cast<size_t>(2));
    EXPECT_FALSE(updatedSettings.generateMissingTextures);
    EXPECT_FALSE(updatedSettings.enableTextureConversion);
    EXPECT_NEARLY_EQUAL(updatedSettings.defaultMetallic, 0.2f);
    EXPECT_NEARLY_EQUAL(updatedSettings.defaultRoughness, 0.8f);
    
    TestOutput::PrintTestPass("material import settings");
    return true;
}

bool TestTextureValidationAndConversion() {
    TestOutput::PrintTestStart("texture validation and conversion");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Test texture validation with non-existent file
    EXPECT_FALSE(importer.ValidateTexture("non_existent.png"));
    EXPECT_FALSE(importer.ValidateTexture(""));
    
    // Test texture format conversion capabilities
    EXPECT_TRUE(importer.CanConvertTextureFormat(".png", ".jpg"));
    EXPECT_TRUE(importer.CanConvertTextureFormat(".jpg", ".png"));
    EXPECT_FALSE(importer.CanConvertTextureFormat(".xyz", ".png"));
    
    // Test conversion with non-existent files (should fail gracefully)
    EXPECT_FALSE(importer.ConvertTextureFormat("non_existent.png", "output.jpg", TextureFormat::RGB));
    
    TestOutput::PrintTestPass("texture validation and conversion");
    return true;
}

bool TestMaterialImporterStatistics() {
    TestOutput::PrintTestStart("MaterialImporter statistics");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Initial statistics should be zero
    EXPECT_EQUAL(importer.GetImportedMaterialCount(), static_cast<size_t>(0));
    EXPECT_EQUAL(importer.GetImportedTextureCount(), static_cast<size_t>(0));
    EXPECT_EQUAL(importer.GetFallbackTextureCount(), static_cast<size_t>(0));
    EXPECT_EQUAL(importer.GetMissingTextureCount(), static_cast<size_t>(0));
    
    // Create some fallback textures to test statistics
    importer.CreateFallbackTexture(TextureType::Diffuse, "test1.png");
    importer.CreateFallbackTexture(TextureType::Normal, "test2.png");
    
    // Statistics should be updated
    EXPECT_TRUE(importer.GetFallbackTextureCount() > 0);
    
    // Clear cache and verify reset
    importer.ClearCache();
    EXPECT_EQUAL(importer.GetImportedTextureCount(), static_cast<size_t>(0));
    
    TestOutput::PrintTestPass("MaterialImporter statistics");
    return true;
}

int main() {
    TestOutput::PrintHeader("Material Importer Integration");

    bool allPassed = true;

    try {
        // Initialize logger
        Logger::GetInstance().Initialize();
        
        // Create test suite for result tracking
        TestSuite suite("Material Importer Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("MaterialImporter Initialization", TestMaterialImporterInitialization);
        allPassed &= suite.RunTest("Texture Search and Fallback System", TestTextureSearchAndFallbackSystem);
        allPassed &= suite.RunTest("Material Import Settings", TestMaterialImportSettings);
        allPassed &= suite.RunTest("Texture Validation and Conversion", TestTextureValidationAndConversion);
        allPassed &= suite.RunTest("MaterialImporter Statistics", TestMaterialImporterStatistics);

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