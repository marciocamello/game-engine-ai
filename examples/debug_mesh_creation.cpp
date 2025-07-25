#include "Graphics/Mesh.h"
#include "Core/Logger.h"
#include <iostream>
#include <memory>

using namespace GameEngine;

int main() {
    std::cout << "=== Testing Mesh Creation ===" << std::endl;
    
    try {
        std::cout << "1. About to create Mesh object..." << std::endl;
        
        // Try to create a Mesh object
        auto mesh = std::make_shared<Mesh>();
        
        std::cout << "2. Mesh object created successfully!" << std::endl;
        
        // Try to set some basic data
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        
        std::cout << "3. About to set vertices..." << std::endl;
        mesh->SetVertices(vertices);
        
        std::cout << "4. About to set indices..." << std::endl;
        mesh->SetIndices(indices);
        
        std::cout << "5. Mesh creation test completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "UNKNOWN EXCEPTION occurred" << std::endl;
        return 1;
    }
    
    return 0;
}