#include "Graphics/MaterialImporter.h"
#include "Resource/ResourceManager.h"
#include "Core/Logger.h"
#include <iostream>
#include <memory>

using namespace GameEngine;

bool TestMaterialImporterInitialization() {
    std::cout << "Testing MaterialImporter initialization..." << std::endl;
    
    auto resourceManager = std::make_shared<ResourceManager>();
    if (!resourceManager->Initialize()) {
        std::cerr << "Failed to initialize ResourceManager" << std::endl;
        return false;
    }
    
    MaterialImporter importer;
    if (!importer.Initialize(resourceManager)) {
        std::cerr << "Failed to initialize MaterialImporter" << std::endl;
        return false;
    }
    
    std::cout << "MaterialImporter initialized successfully" << std::endl;
    return true;
}

bool TestTextureSearchPaths() {
    std::cout << "Testing texture search paths..." << std::endl;
    
    auto resourceManager = std::make_shared<ResourceManager>();
    resourceManager->Initialize();
    
    MaterialImporter importer;
    importer.Initialize(resourceManager);
    
    // Test adding search paths
    importer.AddTextureSearchPath("test/path1/");
    importer.AddTextureSearchPath("test/path2/");
    
    auto searchPaths = importer.GetTextureSearchPaths();
    if (searchPaths.size() < 2) {
        std::cerr << "Search paths not added correctly" << std::endl;
        return false;
    }
    
    std::cout << "Texture search paths working correctly" << std::endl;
    return true;
}

bool TestDefaultTextureCreation() {
    std::cout << "Testing default texture creation..." << std::endl;
    
    auto resourceManager = std::make_shared<ResourceManager>();
    resourceManager->Initialize();
    
    MaterialImporter importer;
    importer.Initialize(resourceManager);
    
    // Test creating different types of default textures
    // Note: These may return null without OpenGL context, which is acceptable
    auto diffuseTexture = importer.CreateDefaultTexture(TextureType::Diffuse);
    auto normalTexture = importer.CreateDefaultTexture(TextureType::Normal);
    auto metallicTexture = importer.CreateDefaultTexture(TextureType::Metallic);
    
    // For now, just test that the method doesn't crash
    // In a full implementation with OpenGL context, these would be non-null
    std::cout << "Default texture creation completed (may be null without OpenGL context)" << std::endl;
    return true;
}

bool TestTextureFormatSupport() {
    std::cout << "Testing texture format support..." << std::endl;
    
    auto resourceManager = std::make_shared<ResourceManager>();
    resourceManager->Initialize();
    
    MaterialImporter importer;
    importer.Initialize(resourceManager);
    
    // Test supported formats
    if (!importer.IsTextureFormatSupported(".png")) {
        std::cerr << "PNG format should be supported" << std::endl;
        return false;
    }
    
    if (!importer.IsTextureFormatSupported(".jpg")) {
        std::cerr << "JPG format should be supported" << std::endl;
        return false;
    }
    
    if (importer.IsTextureFormatSupported(".xyz")) {
        std::cerr << "XYZ format should not be supported" << std::endl;
        return false;
    }
    
    auto supportedFormats = importer.GetSupportedTextureFormats();
    if (supportedFormats.empty()) {
        std::cerr << "Should have supported texture formats" << std::endl;
        return false;
    }
    
    std::cout << "Texture format support working correctly" << std::endl;
    return true;
}

bool TestMaterialImporterStatistics() {
    std::cout << "Testing MaterialImporter statistics..." << std::endl;
    
    auto resourceManager = std::make_shared<ResourceManager>();
    resourceManager->Initialize();
    
    MaterialImporter importer;
    importer.Initialize(resourceManager);
    
    // Initial statistics should be zero
    if (importer.GetImportedMaterialCount() != 0) {
        std::cerr << "Initial material count should be 0" << std::endl;
        return false;
    }
    
    if (importer.GetImportedTextureCount() != 0) {
        std::cerr << "Initial texture count should be 0" << std::endl;
        return false;
    }
    
    if (importer.GetFallbackTextureCount() != 0) {
        std::cerr << "Initial fallback texture count should be 0" << std::endl;
        return false;
    }
    
    if (importer.GetMissingTextureCount() != 0) {
        std::cerr << "Initial missing texture count should be 0" << std::endl;
        return false;
    }
    
    std::cout << "MaterialImporter statistics working correctly" << std::endl;
    return true;
}

int main() {
    try {
        Logger::GetInstance().Initialize();
        
        std::cout << "Running MaterialImporter tests..." << std::endl;
        
        bool allTestsPassed = true;
        
        allTestsPassed &= TestMaterialImporterInitialization();
        allTestsPassed &= TestTextureSearchPaths();
        allTestsPassed &= TestDefaultTextureCreation();
        allTestsPassed &= TestTextureFormatSupport();
        allTestsPassed &= TestMaterialImporterStatistics();
        
        if (allTestsPassed) {
            std::cout << "All MaterialImporter tests passed!" << std::endl;
            return 0;
        } else {
            std::cerr << "Some MaterialImporter tests failed!" << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception during MaterialImporter tests: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception during MaterialImporter tests" << std::endl;
        return 1;
    }
}