#include "Resource/ModelLoader.h"
#include "Core/Logger.h"
#include <iostream>

using namespace GameEngine;

int main() {
    std::cout << "=== Testing FBX via ModelLoader ===" << std::endl;
    
    try {
        ModelLoader loader;
        std::cout << "1. Created ModelLoader" << std::endl;
        
        if (!loader.Initialize()) {
            std::cout << "ERROR: Failed to initialize ModelLoader" << std::endl;
            return 1;
        }
        std::cout << "2. Initialized ModelLoader successfully" << std::endl;
        
        // Test loading XBot.fbx
        std::cout << "3. Starting to load XBot.fbx via ModelLoader..." << std::endl;
        auto result = loader.LoadModel("assets/meshes/XBot.fbx");
        std::cout << "4. LoadModel call completed" << std::endl;
        
        if (result.success) {
            std::cout << "SUCCESS: Loaded XBot.fbx via ModelLoader" << std::endl;
            std::cout << "  Meshes: " << result.meshes.size() << std::endl;
            std::cout << "  Vertices: " << result.totalVertices << std::endl;
            std::cout << "  Triangles: " << result.totalTriangles << std::endl;
            std::cout << "  Format: " << result.formatUsed << std::endl;
        } else {
            std::cout << "ERROR: Failed to load XBot.fbx via ModelLoader: " << result.errorMessage << std::endl;
            return 1;
        }
        
        std::cout << "5. Shutting down loader..." << std::endl;
        loader.Shutdown();
        std::cout << "6. Test completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "UNKNOWN EXCEPTION occurred" << std::endl;
        return 1;
    }
    
    return 0;
}