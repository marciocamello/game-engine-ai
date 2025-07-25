#include "Resource/FBXLoader.h"
#include "Resource/ModelLoader.h"
#include "Core/Logger.h"
#include "TestUtils.h"
#include <filesystem>
#include <iostream>

using namespace GameEngine;

bool TestFBXLoaderInitialization() {
    TestOutput::PrintTestStart("FBX loader initialization");

    FBXLoader loader;
    EXPECT_TRUE(loader.Initialize());
    EXPECT_TRUE(loader.IsInitialized());

    loader.Shutdown();
    EXPECT_FALSE(loader.IsInitialized());

    TestOutput::PrintTestPass("FBX loader initialization");
    return true;
}

bool TestFBXFileDetection() {
    TestOutput::PrintTestStart("FBX file detection");

    EXPECT_TRUE(FBXLoader::IsFBXFile("test.fbx"));
    EXPECT_TRUE(FBXLoader::IsFBXFile("model.FBX"));
    EXPECT_FALSE(FBXLoader::IsFBXFile("test.obj"));
    EXPECT_FALSE(FBXLoader::IsFBXFile("model.gltf"));

    TestOutput::PrintTestPass("FBX file detection");
    return true;
}

bool TestFBXLoadingConfiguration() {
    TestOutput::PrintTestStart("FBX loading configuration");

    FBXLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test default configuration
    auto config = loader.GetLoadingConfig();
    EXPECT_TRUE(config.convertToOpenGLCoordinates);
    EXPECT_TRUE(config.importMaterials);
    EXPECT_TRUE(config.importTextures);
    EXPECT_TRUE(config.optimizeMeshes);
    EXPECT_EQUAL(config.importScale, 1.0f);

    // Test custom configuration
    FBXLoader::FBXLoadingConfig customConfig;
    customConfig.convertToOpenGLCoordinates = false;
    customConfig.importMaterials = false;
    customConfig.importScale = 2.0f;
    
    loader.SetLoadingConfig(customConfig);
    auto retrievedConfig = loader.GetLoadingConfig();
    
    EXPECT_FALSE(retrievedConfig.convertToOpenGLCoordinates);
    EXPECT_FALSE(retrievedConfig.importMaterials);
    EXPECT_EQUAL(retrievedConfig.importScale, 2.0f);

    TestOutput::PrintTestPass("FBX loading configuration");
    return true;
}

bool TestFBXModelLoading() {
    TestOutput::PrintTestStart("FBX model loading");

    // Check if test FBX files exist
    if (!std::filesystem::exists("assets/meshes/XBot.fbx")) {
        TestOutput::PrintInfo("Skipping test - XBot.fbx not found");
        TestOutput::PrintTestPass("FBX model loading");
        return true;
    }

    FBXLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Load XBot.fbx (T-Poser from Mixamo)
    auto result = loader.LoadFBX("assets/meshes/XBot.fbx");
    
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.meshes.size() > 0);
    EXPECT_TRUE(result.totalVertices > 0);
    EXPECT_TRUE(result.totalTriangles > 0);
    EXPECT_TRUE(result.loadingTimeMs > 0.0f);

    // Verify mesh properties
    for (const auto& mesh : result.meshes) {
        EXPECT_NOT_NULL(mesh);
        EXPECT_TRUE(mesh->GetVertexCount() > 0);
        EXPECT_TRUE(mesh->GetTriangleCount() > 0);
        EXPECT_TRUE(mesh->Validate());
    }

    TestOutput::PrintInfo("Loaded FBX model: " + std::to_string(result.meshes.size()) + " meshes, " +
                         std::to_string(result.totalVertices) + " vertices, " +
                         std::to_string(result.totalTriangles) + " triangles");

    TestOutput::PrintTestPass("FBX model loading");
    return true;
}

bool TestFBXModelLoadingWithMaterials() {
    TestOutput::PrintTestStart("FBX model loading with materials");

    // Check if test FBX files exist
    if (!std::filesystem::exists("assets/meshes/XBot.fbx")) {
        TestOutput::PrintInfo("Skipping test - XBot.fbx not found");
        TestOutput::PrintTestPass("FBX model loading with materials");
        return true;
    }

    FBXLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Configure to import materials
    FBXLoader::FBXLoadingConfig config = loader.GetLoadingConfig();
    config.importMaterials = true;
    config.importTextures = true;
    loader.SetLoadingConfig(config);

    // Load XBot.fbx
    auto result = loader.LoadFBX("assets/meshes/XBot.fbx");
    
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.meshes.size() > 0);

    // Check if materials were imported
    if (result.materialCount > 0) {
        EXPECT_TRUE(result.materials.size() > 0);
        TestOutput::PrintInfo("Imported " + std::to_string(result.materialCount) + " materials");
        
        // Verify material associations
        for (const auto& mesh : result.meshes) {
            if (mesh && mesh->GetMaterial()) {
                TestOutput::PrintInfo("Mesh '" + mesh->GetName() + "' has associated material");
            }
        }
    } else {
        TestOutput::PrintInfo("No materials found in FBX file");
    }

    TestOutput::PrintTestPass("FBX model loading with materials");
    return true;
}

bool TestFBXModelLoadingThroughModelLoader() {
    TestOutput::PrintTestStart("FBX model loading through ModelLoader");

    // Check if test FBX files exist
    if (!std::filesystem::exists("assets/meshes/XBot.fbx")) {
        TestOutput::PrintInfo("Skipping test - XBot.fbx not found");
        TestOutput::PrintTestPass("FBX model loading through ModelLoader");
        return true;
    }

    ModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Verify FBX format is supported
    EXPECT_TRUE(loader.IsFormatSupported("fbx"));

    // Load FBX through ModelLoader
    auto result = loader.LoadModel("assets/meshes/XBot.fbx");
    
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.meshes.size() > 0);
    EXPECT_TRUE(result.totalVertices > 0);
    EXPECT_TRUE(result.totalTriangles > 0);
    EXPECT_EQUAL(result.formatUsed, "fbx");

    TestOutput::PrintInfo("Loaded FBX through ModelLoader: " + std::to_string(result.meshes.size()) + " meshes");

    TestOutput::PrintTestPass("FBX model loading through ModelLoader");
    return true;
}

bool TestFBXIdleAnimationModel() {
    TestOutput::PrintTestStart("FBX Idle animation model loading");

    // Check if test FBX files exist
    if (!std::filesystem::exists("assets/meshes/Idle.fbx")) {
        TestOutput::PrintInfo("Skipping test - Idle.fbx not found");
        TestOutput::PrintTestPass("FBX Idle animation model loading");
        return true;
    }

    FBXLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Load Idle.fbx (Idle animation from Mixamo)
    auto result = loader.LoadFBX("assets/meshes/Idle.fbx");
    
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.meshes.size() > 0);

    // Check for animation data
    if (result.hasAnimations) {
        TestOutput::PrintInfo("FBX file contains animation data");
    } else {
        TestOutput::PrintInfo("FBX file does not contain animation data (expected for basic mesh loading)");
    }

    // Check for skeleton data
    if (result.hasSkeleton) {
        TestOutput::PrintInfo("FBX file contains skeleton data");
    } else {
        TestOutput::PrintInfo("FBX file does not contain skeleton data");
    }

    TestOutput::PrintInfo("Source application: " + result.sourceApplication);

    TestOutput::PrintTestPass("FBX Idle animation model loading");
    return true;
}

bool TestFBXErrorHandling() {
    TestOutput::PrintTestStart("FBX error handling");

    FBXLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test loading non-existent file
    auto result = loader.LoadFBX("non_existent_file.fbx");
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());

    // Test loading non-FBX file
    if (std::filesystem::exists("assets/meshes/cube.obj")) {
        auto result2 = loader.LoadFBX("assets/meshes/cube.obj");
        EXPECT_FALSE(result2.success);
        EXPECT_FALSE(result2.errorMessage.empty());
    }

    TestOutput::PrintTestPass("FBX error handling");
    return true;
}

int main() {
    TestOutput::PrintHeader("FBX Loader Integration Tests");

    bool allTestsPassed = true;

    allTestsPassed &= TestFBXLoaderInitialization();
    allTestsPassed &= TestFBXFileDetection();
    allTestsPassed &= TestFBXLoadingConfiguration();
    allTestsPassed &= TestFBXModelLoading();
    allTestsPassed &= TestFBXModelLoadingWithMaterials();
    allTestsPassed &= TestFBXModelLoadingThroughModelLoader();
    allTestsPassed &= TestFBXIdleAnimationModel();
    allTestsPassed &= TestFBXErrorHandling();

    TestOutput::PrintSummary(allTestsPassed);

    return allTestsPassed ? 0 : 1;
}