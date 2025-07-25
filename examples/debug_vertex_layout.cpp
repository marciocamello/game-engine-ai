#include "Graphics/Mesh.h"
#include <iostream>

using namespace GameEngine;

int main() {
    std::cout << "=== Testing VertexLayout Creation ===" << std::endl;
    
    try {
        std::cout << "1. About to create VertexLayout object..." << std::endl;
        
        // Try to create just a VertexLayout object
        VertexLayout layout;
        
        std::cout << "2. VertexLayout object created successfully!" << std::endl;
        std::cout << "   Stride: " << layout.stride << std::endl;
        std::cout << "   Attributes: " << layout.attributes.size() << std::endl;
        
        std::cout << "3. VertexLayout test completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "UNKNOWN EXCEPTION occurred" << std::endl;
        return 1;
    }
    
    return 0;
}