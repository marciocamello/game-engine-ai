#include "Resource/ResourceManager.h"
#include "Graphics/Texture.h"
#include "Core/Logger.h"
#include <iostream>

using namespace GameEngine;

int main() {
    Logger::GetInstance().Initialize();
    
    std::cout << "Testing ResourceManager statistics..." << std::endl;
    
    ResourceManager resourceManager;
    if (!resourceManager.Initialize()) {
        std::cout << "Failed to initialize ResourceManager" << std::endl;
        return 1;
    }
    
    std::cout << "Initial state:" << std::endl;
    std::cout << "  Resources: " << resourceManager.GetResourceCount() << std::endl;
    std::cout << "  Memory: " << resourceManager.GetMemoryUsage() << " bytes" << std::endl;
    
    // Load a texture
    auto texture = resourceManager.Load<Texture>("textures/wall.png");
    if (texture) {
        std::cout << "Loaded texture successfully" << std::endl;
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    
    std::cout << "After loading texture:" << std::endl;
    std::cout << "  Resources: " << resourceManager.GetResourceCount() << std::endl;
    std::cout << "  Memory: " << resourceManager.GetMemoryUsage() << " bytes" << std::endl;
    
    // Test logging
    resourceManager.LogResourceUsage();
    resourceManager.LogDetailedResourceInfo();
    
    std::cout << "Test completed successfully!" << std::endl;
    return 0;
}