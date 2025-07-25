#include "Resource/ModelLoader.h"
#include "../TestUtils.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test ModelLoader initialization with Assimp
 * Requirements: 1.5, 1.6, 10.1
 */
bool TestModelLoaderInitialization() {
    TestOutput::PrintTestStart("ModelLoader initialization with Assimp");
    
    ModelLoader loader;
    
    // Test initialization
    bool initResult = loader.Initialize();
    
#ifdef GAMEENGINE_HAS_ASSIMP
    EXPECT_TRUE(initResult);
    EXPECT_TRUE(loader.IsInitialized());
    TestOutput::PrintInfo("ModelLoader initialized successfully with Assimp support");
#else
    EXPECT_FALSE(initResult);
    EXPECT_FALSE(loader.IsInitialized());
    TestOutput::PrintWarning("ModelLoader correctly reports Assimp unavailable");
#endif
    
    loader.Shutdown();
    
    TestOutput::PrintTestPass("ModelLoader initialization with Assimp");
    return true;
}

/**
 * Test format detection and enumeration
 * Requirements: 1.5, 1.6
 */
bool TestFormatDetectionAndEnumeration() {
    TestOutput::PrintTestStart("format detection and enumeration");
    
    ModelLoader loader;
    loader.Initialize();
    
#ifdef GAMEENGINE_HAS_ASSIMP
    // Test supported extensions
    auto extensions = loader.GetSupportedExtensions();
    EXPECT_TRUE(!extensions.empty());
    
    TestOutput::PrintInfo("Found " + std::to_string(extensions.size()) + " supported extensions");
    
    // Test some common formats
    EXPECT_TRUE(loader.IsFormatSupported("obj"));
    EXPECT_TRUE(loader.IsFormatSupported("fbx"));
    EXPECT_TRUE(loader.IsFormatSupported("gltf"));
    
    TestOutput::PrintInfo("Common formats (OBJ, FBX, GLTF) are supported");
    
    // Test format info
    auto formats = loader.GetSupportedFormats();
    EXPECT_TRUE(!formats.empty());
    
    TestOutput::PrintInfo("Format information available for " + std::to_string(formats.size()) + " formats");
    
    // Test format detection
    EXPECT_STRING_EQUAL(loader.DetectFormat("model.obj"), "obj");
    EXPECT_STRING_EQUAL(loader.DetectFormat("model.FBX"), "fbx");
    EXPECT_STRING_EQUAL(loader.DetectFormat("model.gltf"), "gltf");
    
    TestOutput::PrintInfo("Format detection working correctly");
    
    // Test utility methods
    EXPECT_TRUE(ModelLoader::IsModelFile("test.obj"));
    EXPECT_TRUE(ModelLoader::IsModelFile("test.fbx"));
    EXPECT_FALSE(ModelLoader::IsModelFile("test.txt"));
    
    TestOutput::PrintInfo("Model file detection working correctly");
    
    // Test file extension extraction
    EXPECT_STRING_EQUAL(ModelLoader::GetFileExtension("path/to/model.obj"), "obj");
    EXPECT_STRING_EQUAL(ModelLoader::GetFileExtension("model.FBX"), "FBX");
    EXPECT_STRING_EQUAL(ModelLoader::GetFileExtension("noextension"), "");
    
    TestOutput::PrintInfo("File extension extraction working correctly");
#else
    auto extensions = loader.GetSupportedExtensions();
    EXPECT_TRUE(extensions.empty());
    TestOutput::PrintInfo("No extensions reported without Assimp (expected)");
#endif
    
    loader.Shutdown();
    
    TestOutput::PrintTestPass("format detection and enumeration");
    return true;
}

/**
 * Test loading flags configuration
 * Requirements: 1.5
 */
bool TestLoadingFlagsConfiguration() {
    TestOutput::PrintTestStart("loading flags configuration");
    
    ModelLoader loader;
    loader.Initialize();
    
    // Test default flags
    auto defaultFlags = loader.GetLoadingFlags();
    TestOutput::PrintInfo("Default loading flags: " + std::to_string(static_cast<uint32_t>(defaultFlags)));
    
    // Test setting flags
    auto newFlags = ModelLoader::LoadingFlags::Triangulate | 
                   ModelLoader::LoadingFlags::GenerateNormals |
                   ModelLoader::LoadingFlags::FlipUVs;
    
    loader.SetLoadingFlags(newFlags);
    EXPECT_TRUE(loader.GetLoadingFlags() == newFlags);
    
    TestOutput::PrintInfo("Loading flags configuration working");
    
    // Test import scale
    loader.SetImportScale(2.0f);
    EXPECT_NEARLY_EQUAL(loader.GetImportScale(), 2.0f);
    
    // Test invalid scale (should be ignored)
    loader.SetImportScale(-1.0f);
    EXPECT_NEARLY_EQUAL(loader.GetImportScale(), 2.0f);
    
    TestOutput::PrintInfo("Import scale configuration working");
    
    loader.Shutdown();
    
    TestOutput::PrintTestPass("loading flags configuration");
    return true;
}

/**
 * Test bitwise operators for LoadingFlags
 * Requirements: 1.5
 */
bool TestBitwiseOperators() {
    TestOutput::PrintTestStart("bitwise operators for LoadingFlags");
    
    using Flags = ModelLoader::LoadingFlags;
    
    // Test OR operator
    auto combined = Flags::Triangulate | Flags::GenerateNormals;
    EXPECT_TRUE(HasFlag(combined, Flags::Triangulate));
    EXPECT_TRUE(HasFlag(combined, Flags::GenerateNormals));
    EXPECT_FALSE(HasFlag(combined, Flags::FlipUVs));
    
    // Test AND operator
    auto masked = combined & Flags::Triangulate;
    EXPECT_TRUE(masked == Flags::Triangulate);
    
    // Test XOR operator
    auto xored = combined ^ Flags::Triangulate;
    EXPECT_TRUE(HasFlag(xored, Flags::GenerateNormals));
    EXPECT_FALSE(HasFlag(xored, Flags::Triangulate));
    
    // Test NOT operator
    auto inverted = ~Flags::None;
    EXPECT_TRUE(inverted != Flags::None);
    
    TestOutput::PrintInfo("Bitwise operators working correctly");
    
    TestOutput::PrintTestPass("bitwise operators for LoadingFlags");
    return true;
}

/**
 * Test error handling for invalid operations
 * Requirements: 10.1
 */
bool TestErrorHandling() {
    TestOutput::PrintTestStart("error handling for invalid operations");
    
    ModelLoader loader;
    
    // Test loading without initialization
    auto result = loader.LoadModel("nonexistent.obj");
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
    
    TestOutput::PrintInfo("Proper error handling for uninitialized loader");
    
    // Initialize for further tests
    loader.Initialize();
    
    // Test loading non-existent file
    result = loader.LoadModel("definitely_does_not_exist.obj");
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
    
    TestOutput::PrintInfo("Proper error handling for non-existent files");
    
    // Test loading from empty memory buffer
    std::vector<uint8_t> emptyData;
    result = loader.LoadModelFromMemory(emptyData, "obj");
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
    
    TestOutput::PrintInfo("Proper error handling for empty memory buffer");
    
    loader.Shutdown();
    
    TestOutput::PrintTestPass("error handling for invalid operations");
    return true;
}

int main() {
    TestOutput::PrintHeader("Model Loader Assimp Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Model Loader Assimp Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("ModelLoader Initialization", TestModelLoaderInitialization);
        allPassed &= suite.RunTest("Format Detection", TestFormatDetectionAndEnumeration);
        allPassed &= suite.RunTest("Loading Flags", TestLoadingFlagsConfiguration);
        allPassed &= suite.RunTest("Bitwise Operators", TestBitwiseOperators);
        allPassed &= suite.RunTest("Error Handling", TestErrorHandling);

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