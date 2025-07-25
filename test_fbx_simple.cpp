#include "Resource/FBXLoader.h"
#include "Core/Logger.h"
#include <iostream>

using namespace GameEngine;

int main() {
    std::cout << "Testing FBX Loader..." << std::endl;
    
    FBXLoader loader;
    if (!loader.Initialize()) {
        std::cout << "Failed to initialize FBX loader" << std::endl;
        return 1;
    }
    
    std::cout << "FBX loader initialized successfully" << std::endl;
    
    // Test loading XBot.fbx
    auto result = loader.LoadFBX("assets/meshes/XBot.fbx");
    
    if (result.success) {
        std::cout << "Successfully loaded XBot.fbx:" << std::endl;
        std::cout << "  Meshes: " << result.meshes.size() << std::endl;
        std::cout << "  Materials: " << result.materialCount << std::endl;
        std::cout << "  Vertices: " << result.totalVertices << std::endl;
        std::cout << "  Triangles: " << result.totalTriangles << std::endl;
        std::cout << "  Loading time: " << result.loadingTimeMs << "ms" << std::endl;
        std::cout << "  Source app: " << result.sourceApplication << std::endl;
        std::cout << "  Has skeleton: " << (result.hasSkeleton ? "Yes" : "No") << std::endl;
        std::cout << "  Has animations: " << (result.hasAnimations ? "Yes" : "No") << std::endl;
    } else {
        std::cout << "Failed to load XBot.fbx: " << result.errorMessage << std::endl;
        return 1;
    }
    
    loader.Shutdown();
    std::cout << "Test completed successfully!" << std::endl;
    
    return 0;
}