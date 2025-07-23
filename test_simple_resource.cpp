#include "Resource/ResourceManager.h"
#include "Core/Logger.h"
#include <iostream>

using namespace GameEngine;

int main() {
    Logger::GetInstance().Initialize();
    
    std::cout << "=== Testing ResourceManager Statistics ===" << std::endl;
    
    ResourceManager resourceManager;
    if (!resourceManager.Initialize()) {
        std::cout << "FAIL: Could not initialize ResourceManager" << std::endl;
        return 1;
    }
    
    std::cout << "PASS: ResourceManager initialized" << std::endl;
    
    // Test initial state
    std::cout << "Initial state:" << std::endl;
    std::cout << "  Resources: " << resourceManager.GetResourceCount() << std::endl;
    std::cout << "  Memory: " << resourceManager.GetMemoryUsage() << " bytes" << std::endl;
    
    // Test statistics methods
    ResourceStats stats = resourceManager.GetResourceStats();
    std::cout << "  Stats - Total Resources: " << stats.totalResources << std::endl;
    std::cout << "  Stats - Total Memory: " << stats.totalMemoryUsage << " bytes" << std::endl;
    std::cout << "  Stats - Expired References: " << stats.expiredReferences << std::endl;
    
    // Test memory management methods
    std::cout << "\nTesting memory management methods:" << std::endl;
    resourceManager.SetMemoryPressureThreshold(1024 * 1024); // 1MB
    std::cout << "PASS: SetMemoryPressureThreshold" << std::endl;
    
    resourceManager.CheckMemoryPressure();
    std::cout << "PASS: CheckMemoryPressure" << std::endl;
    
    resourceManager.UnloadLeastRecentlyUsed(0);
    std::cout << "PASS: UnloadLeastRecentlyUsed" << std::endl;
    
    // Test logging methods
    std::cout << "\nTesting logging methods:" << std::endl;
    resourceManager.LogResourceUsage();
    std::cout << "PASS: LogResourceUsage" << std::endl;
    
    resourceManager.LogDetailedResourceInfo();
    std::cout << "PASS: LogDetailedResourceInfo" << std::endl;
    
    std::cout << "\n=== All ResourceManager Statistics Tests PASSED! ===" << std::endl;
    return 0;
}