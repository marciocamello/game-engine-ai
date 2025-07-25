#include "Graphics/Mesh.h"
#include <iostream>

using namespace GameEngine;

int main() {
    std::cout << "=== Testing Direct Mesh Creation ===" << std::endl;
    
    try {
        std::cout << "1. About to create Mesh object directly..." << std::endl;
        
        // Try to create a Mesh object directly (not with make_shared)
        Mesh mesh;
        
        std::cout << "2. Mesh object created successfully!" << std::endl;
        
        std::cout << "3. Direct mesh creation test completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "UNKNOWN EXCEPTION occurred" << std::endl;
        return 1;
    }
    
    return 0;
}