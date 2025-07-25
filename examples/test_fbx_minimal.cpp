#include "Resource/FBXLoader.h"
#include "Core/Logger.h"
#include <iostream>

using namespace GameEngine;

int main() {
    std::cout << "=== Minimal FBX Test ===" << std::endl;
    
    try {
        FBXLoader loader;
        std::cout << "1. Created FBXLoader" << std::endl;
        
        if (!loader.Initialize()) {
            std::cout << "ERROR: Failed to initialize FBX loader" << std::endl;
            return 1;
        }
        std::cout << "2. Initialized FBXLoader successfully" << std::endl;
        
        // Configure to disable materials and textures for minimal test
        FBXLoader::FBXLoadingConfig config = loader.GetLoadingConfig();
        config.importMaterials = false;
        config.importTextures = false;
        config.optimizeMeshes = false;
        loader.SetLoadingConfig(config);
        std::cout << "3. Configured FBXLoader (materials and textures disabled)" << std::endl;
        
        // Test loading XBot.fbx
        std::cout << "4. Starting to load XBot.fbx..." << std::endl;
        auto result = loader.LoadFBX("assets/meshes/XBot.fbx");
        std::cout << "5. LoadFBX call completed" << std::endl;
        
        if (result.success) {
            std::cout << "SUCCESS: Loaded XBot.fbx" << std::endl;
            std::cout << "  Meshes: " << result.meshes.size() << std::endl;
            std::cout << "  Vertices: " << result.totalVertices << std::endl;
            std::cout << "  Triangles: " << result.totalTriangles << std::endl;
        } else {
            std::cout << "ERROR: Failed to load XBot.fbx: " << result.errorMessage << std::endl;
            return 1;
        }
        
        std::cout << "6. Shutting down loader..." << std::endl;
        loader.Shutdown();
        std::cout << "7. Test completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "UNKNOWN EXCEPTION occurred" << std::endl;
        return 1;
    }
    
    return 0;
}