#include "TestUtils.h"
#include "Graphics/SkinningShaderManager.h"
#include "Graphics/Material.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Graphics;

/**
 * Test SkinningShaderManager initialization and cleanup
 * Requirements: 7.1, 7.2
 */
bool TestSkinningShaderManagerInitialization() {
    TestOutput::PrintTestStart("skinning shader manager initialization");

    SkinningShaderManager manager;
    
    // Test initial state
    EXPECT_FALSE(manager.IsInitialized());
    EXPECT_EQUAL(manager.GetShaderProgram(), 0u);
    EXPECT_EQUAL(manager.GetShaderBinds(), 0u);
    EXPECT_EQUAL(manager.GetUniformUpdates(), 0u);
    
    // Note: Shader loading requires OpenGL context and shader files
    // Focus on testing the initialization logic without OpenGL
    
    TestOutput::PrintTestPass("skinning shader manager initialization");
    return true;
}

/**
 * Test performance counter functionality
 * Requirements: 7.2, 7.5
 */
bool TestPerformanceCounters() {
    TestOutput::PrintTestStart("performance counters");

    SkinningShaderManager manager;
    
    // Initial state
    EXPECT_EQUAL(manager.GetShaderBinds(), 0u);
    EXPECT_EQUAL(manager.GetUniformUpdates(), 0u);
    
    // Test counter reset
    manager.ResetPerformanceCounters();
    EXPECT_EQUAL(manager.GetShaderBinds(), 0u);
    EXPECT_EQUAL(manager.GetUniformUpdates(), 0u);

    TestOutput::PrintTestPass("performance counters");
    return true;
}

/**
 * Test shader binding without OpenGL context
 * Requirements: 7.1, 7.2
 */
bool TestShaderBinding() {
    TestOutput::PrintTestStart("shader binding");

    try {
        SkinningShaderManager manager;
        
        // Test binding without shader program (should handle gracefully)
        manager.BindSkinningShader(); // Should log error but not crash
        manager.UnbindShader(); // Should work regardless
        
        // If we reach here, no crashes occurred
        TestOutput::PrintTestPass("shader binding");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception in shader binding test: " + std::string(e.what()));
        return false;
    } catch (...) {
        TestOutput::PrintError("Unknown exception in shader binding test");
        return false;
    }
}

/**
 * Test uniform setting without OpenGL context
 * Requirements: 7.1, 7.2
 */
bool TestUniformSetting() {
    TestOutput::PrintTestStart("uniform setting");

    SkinningShaderManager manager;
    
    // Test bone matrices setting
    std::vector<glm::mat4> matrices(128, glm::mat4(1.0f));
    manager.SetBoneMatrices(matrices); // Should handle gracefully without shader
    
    // Test transform uniforms
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 16.0f/9.0f, 0.1f, 100.0f);
    
    manager.SetTransformUniforms(model, view, projection); // Should handle gracefully
    
    // Test material uniforms
    Material material;
    manager.SetMaterialUniforms(material); // Should handle gracefully
    
    // Verify no crashes occurred
    EXPECT_TRUE(true);

    TestOutput::PrintTestPass("uniform setting");
    return true;
}

/**
 * Test shader validation without OpenGL context
 * Requirements: 7.2, 7.4
 */
bool TestShaderValidation() {
    TestOutput::PrintTestStart("shader validation");

    SkinningShaderManager manager;
    
    // Test validation without shader program
    bool result = manager.ValidateShaderProgram();
    EXPECT_FALSE(result); // Should return false without shader program
    
    // Test shader info logging
    manager.LogShaderInfo(); // Should handle gracefully without shader

    TestOutput::PrintTestPass("shader validation");
    return true;
}

/**
 * Test shader reloading functionality
 * Requirements: 7.3, 7.5
 */
bool TestShaderReloading() {
    TestOutput::PrintTestStart("shader reloading");

    SkinningShaderManager manager;
    
    // Test reloading without initial load
    bool result = manager.ReloadShaders();
    EXPECT_FALSE(result); // Should fail without shader files or OpenGL context
    
    // Verify manager state remains consistent
    EXPECT_FALSE(manager.IsInitialized());

    TestOutput::PrintTestPass("shader reloading");
    return true;
}

/**
 * Test resource management
 * Requirements: 7.2, 7.5
 */
bool TestResourceManagement() {
    TestOutput::PrintTestStart("resource management");

    SkinningShaderManager manager;
    
    // Test shutdown without initialization
    manager.Shutdown(); // Should handle gracefully
    
    // Test multiple shutdowns
    manager.Shutdown();
    manager.Shutdown();
    
    // Verify no crashes occurred
    EXPECT_TRUE(true);

    TestOutput::PrintTestPass("resource management");
    return true;
}

/**
 * Test error handling with invalid operations
 * Requirements: 6.1, 6.2, 7.2
 */
bool TestErrorHandling() {
    TestOutput::PrintTestStart("error handling");

    SkinningShaderManager manager;
    
    // Test operations without initialization
    manager.BindSkinningShader(); // Should log error
    
    std::vector<glm::mat4> matrices(200, glm::mat4(1.0f)); // Too many matrices
    manager.SetBoneMatrices(matrices); // Should handle gracefully
    
    // Test with empty matrices
    std::vector<glm::mat4> emptyMatrices;
    manager.SetBoneMatrices(emptyMatrices); // Should handle gracefully
    
    // Verify no crashes occurred
    EXPECT_TRUE(true);

    TestOutput::PrintTestPass("error handling");
    return true;
}

int main() {
    TestOutput::PrintHeader("Skinning Shader Manager");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Skinning Shader Manager Tests");

        // Run all tests
        allPassed &= suite.RunTest("Initialization", TestSkinningShaderManagerInitialization);
        allPassed &= suite.RunTest("Performance Counters", TestPerformanceCounters);
        allPassed &= suite.RunTest("Shader Binding", TestShaderBinding);
        allPassed &= suite.RunTest("Uniform Setting", TestUniformSetting);
        allPassed &= suite.RunTest("Shader Validation", TestShaderValidation);
        allPassed &= suite.RunTest("Shader Reloading", TestShaderReloading);
        allPassed &= suite.RunTest("Resource Management", TestResourceManagement);
        allPassed &= suite.RunTest("Error Handling", TestErrorHandling);

        // Print detailed summary
        suite.PrintSummary();

        TestOutput::PrintFooter(allPassed);
        return allPassed ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
        return 1;
    } catch (...) {
        TestOutput::PrintError("UNKNOWN TEST ERROR!");
        return 1;
    }
}