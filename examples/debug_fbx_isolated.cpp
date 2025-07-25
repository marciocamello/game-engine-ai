#include "Resource/FBXLoader.h"
#include "Core/Logger.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

using namespace GameEngine;

bool testWithConfig(const std::string& testName, FBXLoader::FBXLoadingConfig config) {
    std::cout << "\n=== " << testName << " ===" << std::endl;
    
    FBXLoader loader;
    if (!loader.Initialize()) {
        std::cout << "ERROR: Failed to initialize FBX loader" << std::endl;
        return false;
    }
    
    loader.SetLoadingConfig(config);
    
    // Use timeout to detect infinite loops
    std::atomic<bool> done = false;
    std::atomic<bool> success = false;
    
    std::thread loadThread([&] {
        try {
            std::cout << "Starting LoadFBX call..." << std::endl;
            auto result = loader.LoadFBX("assets/meshes/XBot.fbx");
            std::cout << "LoadFBX call completed" << std::endl;
            
            success = result.success;
            if (result.success) {
                std::cout << "SUCCESS: " << testName << std::endl;
                std::cout << "  Meshes: " << result.meshes.size() << std::endl;
                std::cout << "  Materials: " << result.materialCount << std::endl;
                std::cout << "  Vertices: " << result.totalVertices << std::endl;
                std::cout << "  Triangles: " << result.totalTriangles << std::endl;
                std::cout << "  Has Skeleton: " << (result.hasSkeleton ? "Yes" : "No") << std::endl;
                std::cout << "  Has Animations: " << (result.hasAnimations ? "Yes" : "No") << std::endl;
            } else {
                std::cout << "FAILED: " << testName << " - " << result.errorMessage << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "EXCEPTION in " << testName << ": " << e.what() << std::endl;
        }
        done = true;
    });
    
    // Wait with timeout (10 seconds)
    int timeout = 100; // 10 seconds (100 * 100ms)
    for (int i = 0; i < timeout && !done; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (i % 10 == 0) { // Print every second
            std::cout << "  Waiting... (" << (i/10 + 1) << "s)" << std::endl;
        }
    }
    
    if (!done) {
        std::cout << "ERROR: " << testName << " TIMED OUT after 10 seconds!" << std::endl;
        // Note: In a real application, you'd want to properly terminate the thread
        // For this debug test, we'll just detach it
        loadThread.detach();
        loader.Shutdown();
        return false;
    }
    
    loadThread.join();
    loader.Shutdown();
    
    return success;
}

int main() {
    std::cout << "=== FBX Loading Isolation Test ===" << std::endl;
    
    // Test 1: Only materials (baseline)
    FBXLoader::FBXLoadingConfig config1;
    config1.importMaterials = true;
    config1.importTextures = false;
    config1.importSkeleton = false;
    config1.importAnimations = false;
    config1.optimizeMeshes = false;
    
    if (!testWithConfig("Materials Only", config1)) {
        std::cout << "CRITICAL: Even basic material import is failing!" << std::endl;
        return 1;
    }
    
    // Test 2: Materials + Meshes (no skeleton/animation)
    FBXLoader::FBXLoadingConfig config2;
    config2.importMaterials = true;
    config2.importTextures = false;
    config2.importSkeleton = false;
    config2.importAnimations = false;
    config2.optimizeMeshes = false;
    
    if (!testWithConfig("Materials + Meshes", config2)) {
        std::cout << "PROBLEM FOUND: Mesh processing is causing the hang!" << std::endl;
        return 1;
    }
    
    // Test 3: Materials + Meshes + Skeleton (no animation)
    FBXLoader::FBXLoadingConfig config3;
    config3.importMaterials = true;
    config3.importTextures = false;
    config3.importSkeleton = true;
    config3.importAnimations = false;
    config3.optimizeMeshes = false;
    
    if (!testWithConfig("Materials + Meshes + Skeleton", config3)) {
        std::cout << "PROBLEM FOUND: Skeleton processing is causing the hang!" << std::endl;
        return 1;
    }
    
    // Test 4: Materials + Meshes + Skeleton + Animations
    FBXLoader::FBXLoadingConfig config4;
    config4.importMaterials = true;
    config4.importTextures = false;
    config4.importSkeleton = true;
    config4.importAnimations = true;
    config4.optimizeMeshes = false;
    
    if (!testWithConfig("Materials + Meshes + Skeleton + Animations", config4)) {
        std::cout << "PROBLEM FOUND: Animation processing is causing the hang!" << std::endl;
        return 1;
    }
    
    // Test 5: Full import with optimizations
    FBXLoader::FBXLoadingConfig config5;
    config5.importMaterials = true;
    config5.importTextures = false;
    config5.importSkeleton = true;
    config5.importAnimations = true;
    config5.optimizeMeshes = true;
    
    if (!testWithConfig("Full Import", config5)) {
        std::cout << "PROBLEM FOUND: Mesh optimization is causing the hang!" << std::endl;
        return 1;
    }
    
    std::cout << "\n=== All tests completed successfully! ===" << std::endl;
    return 0;
}