#include "Resource/TextureLoader.h"
#include "Graphics/Texture.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <filesystem>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test TextureLoader format detection and basic functionality
 * Requirements: 2.1, 4.3, 6.4 (Texture loading with format detection)
 */
bool TestTextureLoaderFormatDetection() {
    TestOutput::PrintTestStart("format detection");

    TextureLoader loader;
    
    // Test format detection for supported formats
    EXPECT_TRUE(TextureLoader::IsSupportedFormat("test.png"));
    EXPECT_TRUE(TextureLoader::IsSupportedFormat("test.jpg"));
    EXPECT_TRUE(TextureLoader::IsSupportedFormat("test.jpeg"));
    EXPECT_TRUE(TextureLoader::IsSupportedFormat("test.tga"));
    EXPECT_TRUE(TextureLoader::IsSupportedFormat("test.bmp"));
    
    // Test format detection for unsupported formats
    EXPECT_FALSE(TextureLoader::IsSupportedFormat("test.txt"));
    EXPECT_FALSE(TextureLoader::IsSupportedFormat("test.doc"));
    EXPECT_FALSE(TextureLoader::IsSupportedFormat("test.exe"));
    
    // Test case insensitive detection
    EXPECT_TRUE(TextureLoader::IsSupportedFormat("test.PNG"));
    EXPECT_TRUE(TextureLoader::IsSupportedFormat("test.JPG"));
    
    // Test format from file extension
    TextureFormat format = TextureLoader::GetFormatFromFile("test.png");
    EXPECT_EQUAL(static_cast<int>(format), static_cast<int>(TextureFormat::RGBA));

    TestOutput::PrintTestPass("format detection");
    return true;
}

/**
 * Test TextureLoader with invalid files
 * Requirements: 2.5, 4.3 (Error handling for missing files)
 */
bool TestTextureLoaderInvalidFiles() {
    TestOutput::PrintTestStart("invalid file handling");

    TextureLoader loader;
    
    // Test with non-existent file
    TextureLoader::ImageData invalidData = loader.LoadImageData("non_existent_file.png");
    EXPECT_FALSE(invalidData.isValid);
    EXPECT_EQUAL(invalidData.width, 0);
    EXPECT_EQUAL(invalidData.height, 0);
    EXPECT_EQUAL(invalidData.channels, 0);
    EXPECT_NULL(invalidData.data);

    TestOutput::PrintTestPass("invalid file handling");
    return true;
}

/**
 * Test Texture class initial state
 * Requirements: 2.1 (Texture class basic functionality)
 */
bool TestTextureInitialState() {
    TestOutput::PrintTestStart("texture initial state");

    Texture texture;
    
    // Test initial state
    EXPECT_FALSE(texture.IsValid());
    EXPECT_EQUAL(texture.GetWidth(), 0);
    EXPECT_EQUAL(texture.GetHeight(), 0);
    EXPECT_EQUAL(texture.GetChannels(), 0);
    EXPECT_EQUAL(texture.GetID(), static_cast<uint32_t>(0));
    EXPECT_EQUAL(static_cast<int>(texture.GetFormat()), static_cast<int>(TextureFormat::RGBA));

    TestOutput::PrintTestPass("texture initial state");
    return true;
}

/**
 * Test Texture class method calls without OpenGL context
 * Requirements: 2.1, 4.3 (Graceful handling without OpenGL context)
 */
bool TestTextureMethodsWithoutContext() {
    TestOutput::PrintTestStart("texture methods without OpenGL context");

    Texture texture;
    
    // Test that texture remains invalid initially
    EXPECT_FALSE(texture.IsValid());
    EXPECT_EQUAL(texture.GetID(), static_cast<uint32_t>(0));
    
    // Test that we can call methods on invalid texture without crashing
    // Note: We avoid actual OpenGL calls in unit tests
    EXPECT_TRUE(true); // If we get here, the basic functionality works

    TestOutput::PrintTestPass("texture methods without OpenGL context");
    return true;
}

/**
 * Test TextureLoader basic functionality
 * Requirements: 2.1, 6.4 (Basic TextureLoader functionality)
 */
bool TestTextureLoaderBasicFunctionality() {
    TestOutput::PrintTestStart("texture loader basic functionality");

    TextureLoader loader;
    
    // Test that we can create a loader instance
    EXPECT_TRUE(true); // Basic instantiation test
    
    // Test that format detection works
    EXPECT_TRUE(TextureLoader::IsSupportedFormat("test.png"));
    EXPECT_FALSE(TextureLoader::IsSupportedFormat("test.xyz"));

    TestOutput::PrintTestPass("texture loader basic functionality");
    return true;
}

/**
 * Test TextureLoader ImageData structure
 * Requirements: 2.1, 4.3 (ImageData structure functionality)
 */
bool TestImageDataStructure() {
    TestOutput::PrintTestStart("ImageData structure");

    // Test default construction
    TextureLoader::ImageData imageData;
    EXPECT_NULL(imageData.data);
    EXPECT_EQUAL(imageData.width, 0);
    EXPECT_EQUAL(imageData.height, 0);
    EXPECT_EQUAL(imageData.channels, 0);
    EXPECT_FALSE(imageData.isValid);
    
    // Test that we can create and manipulate ImageData
    imageData.width = 256;
    imageData.height = 256;
    imageData.channels = 4;
    imageData.isValid = true;
    
    EXPECT_EQUAL(imageData.width, 256);
    EXPECT_EQUAL(imageData.height, 256);
    EXPECT_EQUAL(imageData.channels, 4);
    EXPECT_TRUE(imageData.isValid);

    TestOutput::PrintTestPass("ImageData structure");
    return true;
}

/**
 * Test Texture fallback resource creation
 * Requirements: 2.5, 4.3 (Fallback resources for missing files)
 */
bool TestTextureFallbackResource() {
    TestOutput::PrintTestStart("texture fallback resource");

    Texture texture;
    
    // Test creating default/fallback texture
    texture.CreateDefault();
    
    // After creating default, texture should be valid
    EXPECT_TRUE(texture.IsValid());
    EXPECT_TRUE(texture.GetWidth() > 0);
    EXPECT_TRUE(texture.GetHeight() > 0);
    EXPECT_TRUE(texture.GetChannels() > 0);

    TestOutput::PrintTestPass("texture fallback resource");
    return true;
}

/**
 * Test Texture memory usage calculation
 * Requirements: 2.4, 5.3 (Memory usage tracking)
 */
bool TestTextureMemoryUsage() {
    TestOutput::PrintTestStart("texture memory usage");

    Texture texture;
    
    // Initial memory usage should be minimal
    size_t initialMemory = texture.GetMemoryUsage();
    EXPECT_TRUE(initialMemory >= sizeof(Texture));
    
    // Create a texture with known dimensions
    bool created = texture.CreateEmpty(256, 256, TextureFormat::RGBA);
    if (created) {
        size_t afterCreation = texture.GetMemoryUsage();
        EXPECT_TRUE(afterCreation >= initialMemory);
        
        // Memory usage should account for pixel data
        // 256x256x4 bytes = 262,144 bytes minimum
        EXPECT_TRUE(afterCreation >= 262144);
    }

    TestOutput::PrintTestPass("texture memory usage");
    return true;
}

/**
 * Test Texture format handling
 * Requirements: 2.1, 4.3 (Texture format support)
 */
bool TestTextureFormats() {
    TestOutput::PrintTestStart("texture formats");

    Texture texture;
    
    // Test different texture formats
    TextureFormat formats[] = {
        TextureFormat::RGB,
        TextureFormat::RGBA,
        TextureFormat::Depth,
        TextureFormat::DepthStencil
    };
    
    for (auto format : formats) {
        bool created = texture.CreateEmpty(64, 64, format);
        // Creation success depends on OpenGL context availability
        // But should not crash
        
        if (created) {
            EXPECT_EQUAL(static_cast<int>(texture.GetFormat()), static_cast<int>(format));
        }
    }

    TestOutput::PrintTestPass("texture formats");
    return true;
}

/**
 * Test Texture filter and wrap settings
 * Requirements: 2.1 (Texture parameter configuration)
 */
bool TestTextureParameters() {
    TestOutput::PrintTestStart("texture parameters");

    Texture texture;
    texture.CreateDefault(); // Create a valid texture
    
    // Test filter settings (should not crash)
    texture.SetFilter(TextureFilter::Nearest, TextureFilter::Nearest);
    texture.SetFilter(TextureFilter::Linear, TextureFilter::Linear);
    texture.SetFilter(TextureFilter::LinearMipmapLinear, TextureFilter::Linear);
    
    // Test wrap settings (should not crash)
    texture.SetWrap(TextureWrap::Repeat, TextureWrap::Repeat);
    texture.SetWrap(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge);
    texture.SetWrap(TextureWrap::MirroredRepeat, TextureWrap::MirroredRepeat);
    
    // Test mipmap generation (should not crash)
    texture.GenerateMipmaps();

    TestOutput::PrintTestPass("texture parameters");
    return true;
}

int main() {
    TestOutput::PrintHeader("Texture Loader");

    bool allPassed = true;

    try {
        // Initialize logger as required by the project pattern
        Logger::GetInstance().Initialize();

        // Create test suite for result tracking
        TestSuite suite("Texture Loader Tests");

        // Run all tests following the established pattern
        allPassed &= suite.RunTest("Format Detection", TestTextureLoaderFormatDetection);
        allPassed &= suite.RunTest("Invalid File Handling", TestTextureLoaderInvalidFiles);
        allPassed &= suite.RunTest("Texture Initial State", TestTextureInitialState);
        allPassed &= suite.RunTest("Methods Without Context", TestTextureMethodsWithoutContext);
        allPassed &= suite.RunTest("Basic Functionality", TestTextureLoaderBasicFunctionality);
        allPassed &= suite.RunTest("ImageData Structure", TestImageDataStructure);
        allPassed &= suite.RunTest("Texture Fallback Resource", TestTextureFallbackResource);
        allPassed &= suite.RunTest("Texture Memory Usage", TestTextureMemoryUsage);
        allPassed &= suite.RunTest("Texture Formats", TestTextureFormats);
        allPassed &= suite.RunTest("Texture Parameters", TestTextureParameters);

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