#include "Resource/FBXLoader.h"
#include "Core/Logger.h"
#include <iostream>

using namespace GameEngine;

int main() {
    std::cout << "=== Debugging FBX Loading Issue ===" << std::endl;
    
    try {
        FBXLoader loader;
        std::cout << "1. Created FBXLoader" << std::endl;
        
        if (!loader.Initialize()) {
            std::cout << "ERROR: Failed to initialize FBX loader" << std::endl;
            return 1;
        }
        std::cout << "2. Initialized FBXLoader successfully" << std::endl;
        
        // Configure to import everything with debug logging
        FBXLoader::FBXLoadingConfig config = loader.GetLoadingConfig();
        config.importMaterials = true;
        config.importTextures = false; // Keep disabled for now
        config.importSkeleton = true;
        config.importAnimations = true;
        config.optimizeMeshes = false; // Disable optimization to isolate issue
        loader.SetLoadingConfig(config);
        std::cout << "3. Configured FBXLoader" << std::endl;
        
        // Test loading XBot.fbx with detailed logging
        std::cout << "4. Starting to load XBot.fbx..." << std::endl;
        std::cout << "   - About to call LoadFBX..." << std::endl;
        
        auto result = loader.LoadFBX("assets/meshes/XBot.fbx");
        
        std::cout << "5. LoadFBX call completed" << std::endl;
        std::cout << "   - Success: " << (result.success ? "true" : "false") << std::endl;
        
        if (result.success) {
            std::cout << "SUCCESS: Loaded XBot.fbx" << std::endl;
            std::cout << "  Meshes: " << result.meshes.size() << std::endl;
            std::cout << "  Materials: " << result.materialCount << std::endl;
            std::cout << "  Vertices: " << result.totalVertices << std::endl;
            std::cout << "  Triangles: " << result.totalTriangles << std::endl;
            std::cout << "  Has Skeleton: " << (result.hasSkeleton ? "Yes" : "No") << std::endl;
            std::cout << "  Bone Count: " << result.boneCount << std::endl;
            std::cout << "  Has Animations: " << (result.hasAnimations ? "Yes" : "No") << std::endl;
            std::cout << "  Animation Count: " << result.animationCount << std::endl;
            std::cout << "  Source App: " << result.sourceApplication << std::endl;
            std::cout << "  Loading Time: " << result.loadingTimeMs << "ms" << std::endl;
        } else {
            std::cout << "ERROR: Failed to load XBot.fbx" << std::endl;
            std::cout << "  Error: " << result.errorMessage << std::endl;
        }
        
        std::cout << "6. Shutting down loader..." << std::endl;
        loader.Shutdown();
        std::cout << "7. Debug test completed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "UNKNOWN EXCEPTION occurred" << std::endl;
        return 1;
    }
    
    return 0;
}