#include "Graphics/MaterialImporter.h"
#include "Resource/ResourceManager.h"
#include "Graphics/Material.h"
#include "Graphics/Texture.h"
#include "Core/Logger.h"
#include "TestUtils.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test MaterialImporter basic initialization and configuration
 * Requirements: 2.1 (Material import from Assimp aiMaterial structures)
 */
bool TestMaterialImporterInitialization() {
    TestOutput::PrintTestStart("MaterialImporter initialization");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Test default configuration
    auto settings = importer.GetImportSettings();
    EXPECT_TRUE(settings.textureSearchPaths.size() > 0);
    EXPECT_TRUE(settings.generateMissingTextures);
    EXPECT_TRUE(settings.enableTextureConversion);
    EXPECT_EQUAL(static_cast<int>(settings.conversionMode), static_cast<int>(MaterialConversionMode::Auto));
    
    TestOutput::PrintInfo("MaterialImporter initialized with default settings");
    
    TestOutput::PrintTestPass("MaterialImporter initialization");
    return true;
}

/**
 * Test MaterialImporter settings configuration
 * Requirements: 2.1 (Material import configuration and settings)
 */
bool TestMaterialImporterSettings() {
    TestOutput::PrintTestStart("MaterialImporter settings configuration");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Test custom settings
    MaterialImportSettings customSettings;
    customSettings.conversionMode = MaterialConversionMode::ForcePBR;
    customSettings.textureSearchPaths = {"custom/path1/", "custom/path2/"};
    customSettings.generateMissingTextures = false;
    customSettings.enableTextureConversion = false;
    customSettings.defaultMetallic = 0.3f;
    customSettings.defaultRoughness = 0.7f;
    // Note: defaultSpecular and defaultEmissive may not be available in current API
    
    importer.SetImportSettings(customSettings);
    auto updatedSettings = importer.GetImportSettings();
    
    EXPECT_EQUAL(static_cast<int>(updatedSettings.conversionMode), static_cast<int>(MaterialConversionMode::ForcePBR));
    EXPECT_EQUAL(updatedSettings.textureSearchPaths.size(), static_cast<size_t>(2));
    EXPECT_STRING_EQUAL(updatedSettings.textureSearchPaths[0], "custom/path1/");
    EXPECT_STRING_EQUAL(updatedSettings.textureSearchPaths[1], "custom/path2/");
    EXPECT_FALSE(updatedSettings.generateMissingTextures);
    EXPECT_FALSE(updatedSettings.enableTextureConversion);
    EXPECT_NEARLY_EQUAL(updatedSettings.defaultMetallic, 0.3f);
    EXPECT_NEARLY_EQUAL(updatedSettings.defaultRoughness, 0.7f);
    // Note: defaultSpecular and defaultEmissive validation skipped
    
    TestOutput::PrintInfo("Settings configuration working correctly");
    
    TestOutput::PrintTestPass("MaterialImporter settings configuration");
    return true;
}

/**
 * Test texture search path management
 * Requirements: 2.4, 2.6 (Texture path resolution and search directories)
 */
bool TestTextureSearchPaths() {
    TestOutput::PrintTestStart("texture search path management");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Test default search paths
    auto defaultPaths = importer.GetTextureSearchPaths();
    EXPECT_TRUE(defaultPaths.size() > 0);
    TestOutput::PrintInfo("Default search paths: " + std::to_string(defaultPaths.size()));
    
    // Test adding search paths
    importer.AddTextureSearchPath("custom/textures/");
    importer.AddTextureSearchPath("assets/materials/");
    
    auto updatedPaths = importer.GetTextureSearchPaths();
    EXPECT_TRUE(updatedPaths.size() >= defaultPaths.size() + 2);
    
    // Test setting search paths (if available)
    // Note: SetTextureSearchPaths may not be available in current API
    
    TestOutput::PrintInfo("Search path management working correctly");
    
    TestOutput::PrintTestPass("texture search path management");
    return true;
}

/**
 * Test default texture creation for different material types
 * Requirements: 2.4 (Default texture creation for missing textures)
 */
bool TestDefaultTextureCreation() {
    TestOutput::PrintTestStart("default texture creation");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Test creating default textures for different types
    auto diffuseDefault = importer.CreateDefaultTexture(TextureType::Diffuse);
    EXPECT_NOT_NULL(diffuseDefault);
    
    auto normalDefault = importer.CreateDefaultTexture(TextureType::Normal);
    EXPECT_NOT_NULL(normalDefault);
    
    auto metallicDefault = importer.CreateDefaultTexture(TextureType::Metallic);
    EXPECT_NOT_NULL(metallicDefault);
    
    auto roughnessDefault = importer.CreateDefaultTexture(TextureType::Roughness);
    EXPECT_NOT_NULL(roughnessDefault);
    
    auto aoDefault = importer.CreateDefaultTexture(TextureType::AO);
    EXPECT_NOT_NULL(aoDefault);
    
    auto emissiveDefault = importer.CreateDefaultTexture(TextureType::Emissive);
    EXPECT_NOT_NULL(emissiveDefault);
    
    TestOutput::PrintInfo("Default texture creation working for all types");
    
    TestOutput::PrintTestPass("default texture creation");
    return true;
}

/**
 * Test fallback texture creation and management
 * Requirements: 2.4, 2.6 (Fallback texture creation and validation)
 */
bool TestFallbackTextureCreation() {
    TestOutput::PrintTestStart("fallback texture creation");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Test creating fallback textures for missing files
    auto fallbackDiffuse = importer.CreateFallbackTexture(TextureType::Diffuse, "missing_diffuse.png");
    EXPECT_NOT_NULL(fallbackDiffuse);
    
    auto fallbackNormal = importer.CreateFallbackTexture(TextureType::Normal, "missing_normal.png");
    EXPECT_NOT_NULL(fallbackNormal);
    
    auto fallbackMetallic = importer.CreateFallbackTexture(TextureType::Metallic, "missing_metallic.png");
    EXPECT_NOT_NULL(fallbackMetallic);
    
    // Test statistics tracking
    EXPECT_TRUE(importer.GetFallbackTextureCount() >= 3);
    EXPECT_TRUE(importer.GetMissingTextureCount() >= 3);
    
    TestOutput::PrintInfo("Fallback texture creation working correctly");
    
    TestOutput::PrintTestPass("fallback texture creation");
    return true;
}

/**
 * Test texture format support and validation
 * Requirements: 2.6 (Texture format conversion and validation)
 */
bool TestTextureFormatSupport() {
    TestOutput::PrintTestStart("texture format support");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Test supported texture formats
    auto supportedFormats = importer.GetSupportedTextureFormats();
    EXPECT_TRUE(supportedFormats.size() > 0);
    TestOutput::PrintInfo("Supported formats: " + std::to_string(supportedFormats.size()));
    
    // Test common format support
    EXPECT_TRUE(importer.IsTextureFormatSupported(".png"));
    EXPECT_TRUE(importer.IsTextureFormatSupported(".jpg"));
    EXPECT_TRUE(importer.IsTextureFormatSupported(".jpeg"));
    EXPECT_TRUE(importer.IsTextureFormatSupported(".bmp"));
    EXPECT_TRUE(importer.IsTextureFormatSupported(".tga"));
    
    // Test case insensitivity
    EXPECT_TRUE(importer.IsTextureFormatSupported(".PNG"));
    EXPECT_TRUE(importer.IsTextureFormatSupported(".JPG"));
    
    // Test unsupported formats
    EXPECT_FALSE(importer.IsTextureFormatSupported(".xyz"));
    EXPECT_FALSE(importer.IsTextureFormatSupported(".unknown"));
    EXPECT_FALSE(importer.IsTextureFormatSupported(""));
    
    // Test format conversion capabilities
    EXPECT_TRUE(importer.CanConvertTextureFormat(".png", ".jpg"));
    EXPECT_TRUE(importer.CanConvertTextureFormat(".jpg", ".png"));
    EXPECT_FALSE(importer.CanConvertTextureFormat(".xyz", ".png"));
    
    TestOutput::PrintInfo("Texture format support working correctly");
    
    TestOutput::PrintTestPass("texture format support");
    return true;
}

/**
 * Test material conversion modes
 * Requirements: 2.1, 2.3 (Material conversion with different modes)
 */
bool TestMaterialConversionModes() {
    TestOutput::PrintTestStart("material conversion modes");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Test Auto conversion mode
    MaterialImportSettings autoSettings;
    autoSettings.conversionMode = MaterialConversionMode::Auto;
    importer.SetImportSettings(autoSettings);
    
    EXPECT_EQUAL(static_cast<int>(importer.GetImportSettings().conversionMode), 
                static_cast<int>(MaterialConversionMode::Auto));
    
    // Test Force PBR conversion mode
    MaterialImportSettings pbrSettings;
    pbrSettings.conversionMode = MaterialConversionMode::ForcePBR;
    importer.SetImportSettings(pbrSettings);
    
    EXPECT_EQUAL(static_cast<int>(importer.GetImportSettings().conversionMode), 
                static_cast<int>(MaterialConversionMode::ForcePBR));
    
    // Test Force Unlit conversion mode
    MaterialImportSettings unlitSettings;
    unlitSettings.conversionMode = MaterialConversionMode::ForceUnlit;
    importer.SetImportSettings(unlitSettings);
    
    EXPECT_EQUAL(static_cast<int>(importer.GetImportSettings().conversionMode), 
                static_cast<int>(MaterialConversionMode::ForceUnlit));
    
    // Test Preserve Original conversion mode
    MaterialImportSettings preserveSettings;
    preserveSettings.conversionMode = MaterialConversionMode::Preserve;
    importer.SetImportSettings(preserveSettings);
    
    EXPECT_EQUAL(static_cast<int>(importer.GetImportSettings().conversionMode), 
                static_cast<int>(MaterialConversionMode::Preserve));
    
    TestOutput::PrintInfo("Material conversion modes working correctly");
    
    TestOutput::PrintTestPass("material conversion modes");
    return true;
}

/**
 * Test MaterialImporter statistics and cache management
 * Requirements: 2.1 (Material import statistics and caching)
 */
bool TestMaterialImporterStatistics() {
    TestOutput::PrintTestStart("MaterialImporter statistics");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Test initial statistics
    EXPECT_EQUAL(importer.GetImportedMaterialCount(), static_cast<size_t>(0));
    EXPECT_EQUAL(importer.GetImportedTextureCount(), static_cast<size_t>(0));
    EXPECT_EQUAL(importer.GetFallbackTextureCount(), static_cast<size_t>(0));
    EXPECT_EQUAL(importer.GetMissingTextureCount(), static_cast<size_t>(0));
    
    // Create some fallback textures to test statistics
    importer.CreateFallbackTexture(TextureType::Diffuse, "test1.png");
    importer.CreateFallbackTexture(TextureType::Normal, "test2.png");
    importer.CreateFallbackTexture(TextureType::Metallic, "test3.png");
    
    // Check updated statistics
    EXPECT_EQUAL(importer.GetFallbackTextureCount(), static_cast<size_t>(3));
    EXPECT_EQUAL(importer.GetMissingTextureCount(), static_cast<size_t>(3));
    
    // Test cache clearing
    importer.ClearCache();
    EXPECT_EQUAL(importer.GetImportedTextureCount(), static_cast<size_t>(0));
    
    TestOutput::PrintInfo("Statistics and cache management working correctly");
    
    TestOutput::PrintTestPass("MaterialImporter statistics");
    return true;
}

/**
 * Test texture validation and error handling
 * Requirements: 2.4, 2.6 (Texture validation and error handling)
 */
bool TestTextureValidationAndErrorHandling() {
    TestOutput::PrintTestStart("texture validation and error handling");
    
    auto resourceManager = std::make_shared<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());
    
    MaterialImporter importer;
    EXPECT_TRUE(importer.Initialize(resourceManager));
    
    // Test texture validation with non-existent files
    EXPECT_FALSE(importer.ValidateTexture("non_existent.png"));
    EXPECT_FALSE(importer.ValidateTexture(""));
    EXPECT_FALSE(importer.ValidateTexture("invalid/path/texture.png"));
    
    // Test texture finding with non-existent files
    auto foundTexture = importer.FindTexture("non_existent.png", "");
    EXPECT_NULL(foundTexture);
    
    foundTexture = importer.FindTexture("", "");
    EXPECT_NULL(foundTexture);
    
    // Test texture conversion with invalid inputs
    EXPECT_FALSE(importer.ConvertTextureFormat("non_existent.png", "output.jpg", TextureFormat::RGB));
    EXPECT_FALSE(importer.ConvertTextureFormat("", "output.jpg", TextureFormat::RGB));
    EXPECT_FALSE(importer.ConvertTextureFormat("input.png", "", TextureFormat::RGB));
    
    TestOutput::PrintInfo("Texture validation and error handling working correctly");
    
    TestOutput::PrintTestPass("texture validation and error handling");
    return true;
}

int main() {
    TestOutput::PrintHeader("MaterialImporter Unit Tests");

    bool allPassed = true;

    try {
        // Initialize logger for testing
        Logger::GetInstance().SetLogLevel(LogLevel::Info);
        
        // Create test suite for result tracking
        TestSuite suite("MaterialImporter Unit Tests");

        // Run all tests
        allPassed &= suite.RunTest("MaterialImporter Initialization", TestMaterialImporterInitialization);
        allPassed &= suite.RunTest("Settings Configuration", TestMaterialImporterSettings);
        allPassed &= suite.RunTest("Texture Search Paths", TestTextureSearchPaths);
        allPassed &= suite.RunTest("Default Texture Creation", TestDefaultTextureCreation);
        allPassed &= suite.RunTest("Fallback Texture Creation", TestFallbackTextureCreation);
        allPassed &= suite.RunTest("Texture Format Support", TestTextureFormatSupport);
        allPassed &= suite.RunTest("Material Conversion Modes", TestMaterialConversionModes);
        allPassed &= suite.RunTest("Statistics and Cache", TestMaterialImporterStatistics);
        allPassed &= suite.RunTest("Validation and Error Handling", TestTextureValidationAndErrorHandling);

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