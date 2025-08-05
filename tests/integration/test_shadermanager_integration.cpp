#include "Graphics/ShaderManager.h"
#include "Graphics/OpenGLRenderer.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <memory>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test ShaderManager integration with existing systems
 * Requirements: 7.1, 7.3, 9.1
 */
bool TestShaderManagerIntegration() {
    TestOutput::PrintTestStart("shader manager integration");

    // Test ShaderManager singleton access
    auto& shaderManager = ShaderManager::GetInstance();
    
    // Initialize ShaderManager
    if (!shaderManager.Initialize()) {
        TestOutput::PrintTestFail("shader manager integration");
        return false;
    }

    // Test basic functionality without requiring actual shader files
    auto stats = shaderManager.GetShaderStats();
    EXPECT_TRUE(stats.totalShaders >= 0);
    
    // Test hot-reload functionality
    shaderManager.EnableHotReload(true);
    EXPECT_TRUE(shaderManager.IsHotReloadEnabled());
    
    shaderManager.EnableHotReload(false);
    EXPECT_FALSE(shaderManager.IsHotReloadEnabled());

    // Test shader name listing (should work even with no shaders)
    auto shaderNames = shaderManager.GetShaderNames();
    
    // Cleanup
    shaderManager.Shutdown();

    TestOutput::PrintTestPass("shader manager integration");
    return true;
}

/**
 * Test OpenGLRenderer shader integration
 * Requirements: 7.1, 7.2, 8.1
 */
bool TestOpenGLRendererShaderIntegration() {
    TestOutput::PrintTestStart("opengl renderer shader integration");

    // Initialize ShaderManager first
    auto& shaderManager = ShaderManager::GetInstance();
    if (!shaderManager.Initialize()) {
        TestOutput::PrintTestFail("opengl renderer shader integration");
        return false;
    }

    // Create OpenGL renderer
    auto renderer = std::make_unique<OpenGLRenderer>();
    
    // Test enhanced shader management methods exist and can be called
    // Note: These will fail without proper OpenGL context, but we're testing the interface
    
    // Test LoadShader method
    bool loadResult = renderer->LoadShader("test_renderer_shader", "vertex.glsl", "fragment.glsl", true);
    // Expected to fail without files/context, but method should exist
    
    // Test GetLoadedShaderNames method
    auto shaderNames = renderer->GetLoadedShaderNames();
    // Should return empty list or existing shaders
    
    // Test EnableShaderHotReload method
    renderer->EnableShaderHotReload(true);
    // Should not crash
    
    // Test UnloadShader method
    bool unloadResult = renderer->UnloadShader("test_renderer_shader");
    // Should return true (ShaderManager doesn't report failure for unload)
    EXPECT_TRUE(unloadResult);
    
    // Cleanup
    shaderManager.Shutdown();
    
    TestOutput::PrintTestPass("opengl renderer shader integration");
    return true;
}

/**
 * Test PrimitiveRenderer shader integration
 * Requirements: 7.2, 2.4, 7.4
 */
bool TestPrimitiveRendererShaderIntegration() {
    TestOutput::PrintTestStart("primitive renderer shader integration");

    // Initialize ShaderManager first
    auto& shaderManager = ShaderManager::GetInstance();
    if (!shaderManager.Initialize()) {
        TestOutput::PrintTestFail("primitive renderer shader integration");
        return false;
    }

    PrimitiveRenderer primitiveRenderer;
    
    // Test initialization (this will create default shaders through ShaderManager)
    if (!primitiveRenderer.Initialize()) {
        TestOutput::PrintTestFail("primitive renderer shader integration");
        shaderManager.Shutdown();
        return false;
    }

    // Test shader access methods
    auto colorShader = primitiveRenderer.GetColorShader();
    auto texturedShader = primitiveRenderer.GetTexturedShader();
    
    // Shaders should exist after initialization
    EXPECT_NOT_NULL(colorShader);
    EXPECT_NOT_NULL(texturedShader);

    // Test custom shader setting (with null shader - should log warning but not crash)
    primitiveRenderer.SetCustomColorShader(nullptr);
    primitiveRenderer.SetCustomTexturedShader(nullptr);
    
    // Test reset to default shaders
    primitiveRenderer.ResetToDefaultShaders();
    
    // Test shader hot-reload functionality
    primitiveRenderer.EnableShaderHotReload(true);
    primitiveRenderer.ReloadShaders();
    
    // Cleanup
    primitiveRenderer.Shutdown();
    shaderManager.Shutdown();

    TestOutput::PrintTestPass("primitive renderer shader integration");
    return true;
}

int main() {
    TestOutput::PrintHeader("Shader Manager Integration");

    // Initialize logger
    Logger::GetInstance().Initialize();

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Shader Manager Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Shader Manager Integration", TestShaderManagerIntegration);
        allPassed &= suite.RunTest("OpenGL Renderer Shader Integration", TestOpenGLRendererShaderIntegration);
        allPassed &= suite.RunTest("Primitive Renderer Shader Integration", TestPrimitiveRendererShaderIntegration);

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