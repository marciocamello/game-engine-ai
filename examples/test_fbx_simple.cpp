#include "Resource/FBXLoader.h"
#include "Resource/ModelLoader.h"
#include "Core/Logger.h"
#include <iostream>

using namespace GameEngine;

int main() {
    std::cout << "Testing FBX Loader..." << std::endl;
    
    // Test 1: Direct FBX loader
    std::cout << "\n=== Testing Direct FBX Loader ===" << std::endl;
    FBXLoader fbxLoader;
    if (!fbxLoader.Initialize()) {
        std::cout << "Failed to initialize FBX loader" << std::endl;
        return 1;
    }
    
    std::cout << "FBX loader initialized successfully" << std::endl;
    
    // Test loading XBot.fbx
    auto fbxResult = fbxLoader.LoadFBX("assets/meshes/XBot.fbx");
    
    if (fbxResult.success) {
        std::cout << "Successfully loaded XBot.fbx via FBXLoader:" << std::endl;
        std::cout << "  Meshes: " << fbxResult.meshes.size() << std::endl;
        std::cout << "  Materials: " << fbxResult.materialCount << std::endl;
        std::cout << "  Vertices: " << fbxResult.totalVertices << std::endl;
        std::cout << "  Triangles: " << fbxResult.totalTriangles << std::endl;
        std::cout << "  Loading time: " << fbxResult.loadingTimeMs << "ms" << std::endl;
        std::cout << "  Source app: " << fbxResult.sourceApplication << std::endl;
        std::cout << "  Has skeleton: " << (fbxResult.hasSkeleton ? "Yes" : "No") << std::endl;
        std::cout << "  Has animations: " << (fbxResult.hasAnimations ? "Yes" : "No") << std::endl;
    } else {
        std::cout << "Failed to load XBot.fbx via FBXLoader: " << fbxResult.errorMessage << std::endl;
        return 1;
    }
    
    fbxLoader.Shutdown();
    
    // Test 2: ModelLoader with FBX
    std::cout << "\n=== Testing ModelLoader with FBX ===" << std::endl;
    ModelLoader modelLoader;
    if (!modelLoader.Initialize()) {
        std::cout << "Failed to initialize ModelLoader" << std::endl;
        return 1;
    }
    
    std::cout << "ModelLoader initialized successfully" << std::endl;
    
    // Test loading XBot.fbx through ModelLoader
    auto modelResult = modelLoader.LoadModel("assets/meshes/XBot.fbx");
    
    if (modelResult.success) {
        std::cout << "Successfully loaded XBot.fbx via ModelLoader:" << std::endl;
        std::cout << "  Meshes: " << modelResult.meshes.size() << std::endl;
        std::cout << "  Vertices: " << modelResult.totalVertices << std::endl;
        std::cout << "  Triangles: " << modelResult.totalTriangles << std::endl;
        std::cout << "  Loading time: " << modelResult.loadingTimeMs << "ms" << std::endl;
        std::cout << "  Format used: " << modelResult.formatUsed << std::endl;
    } else {
        std::cout << "Failed to load XBot.fbx via ModelLoader: " << modelResult.errorMessage << std::endl;
        return 1;
    }
    
    modelLoader.Shutdown();
    
    // Test 3: Test Idle.fbx
    std::cout << "\n=== Testing Idle.fbx ===" << std::endl;
    FBXLoader idleLoader;
    if (!idleLoader.Initialize()) {
        std::cout << "Failed to initialize FBX loader for Idle test" << std::endl;
        return 1;
    }
    
    auto idleResult = idleLoader.LoadFBX("assets/meshes/Idle.fbx");
    
    if (idleResult.success) {
        std::cout << "Successfully loaded Idle.fbx:" << std::endl;
        std::cout << "  Meshes: " << idleResult.meshes.size() << std::endl;
        std::cout << "  Materials: " << idleResult.materialCount << std::endl;
        std::cout << "  Vertices: " << idleResult.totalVertices << std::endl;
        std::cout << "  Triangles: " << idleResult.totalTriangles << std::endl;
        std::cout << "  Loading time: " << idleResult.loadingTimeMs << "ms" << std::endl;
        std::cout << "  Source app: " << idleResult.sourceApplication << std::endl;
        std::cout << "  Has skeleton: " << (idleResult.hasSkeleton ? "Yes" : "No") << std::endl;
        std::cout << "  Has animations: " << (idleResult.hasAnimations ? "Yes" : "No") << std::endl;
    } else {
        std::cout << "Failed to load Idle.fbx: " << idleResult.errorMessage << std::endl;
        return 1;
    }
    
    idleLoader.Shutdown();
    
    std::cout << "\nAll FBX tests completed successfully!" << std::endl;
    
    return 0;
}