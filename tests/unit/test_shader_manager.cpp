#include "../TestUtils.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Shader.h"
#include <memory>

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestShaderManagerInitialization() {
    TestOutput::PrintTestStart("shader manager initialization");

    ShaderManager& manager = ShaderManager::GetInstance();
    
    // Test initialization
    EXPECT_TRUE(manager.Initialize());
    
    // Test stats after initialization
    ShaderStats stats = manager.GetShaderStats();
    EXPECT_EQUAL(stats.totalShaders, 0);
    EXPECT_EQUAL(stats.loadedShaders, 0);
    EXPECT_EQUAL(stats.compilationErrors, 0);
    
    // Test shutdown
    manager.Shutdown();
    
    TestOutput::PrintTestPass("shader manager initialization");
    return true;
}

bool TestShaderManagerLogicOnly() {
    TestOutput::PrintTestStart("shader manager logic only");

    ShaderManager& manager = ShaderManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());

    // Test that no shaders exist initially
    EXPECT_FALSE(manager.HasShader("test_shader"));
    EXPECT_FALSE(manager.HasShader("nonexistent_shader"));

    // Test shader names list is empty
    auto names = manager.GetShaderNames();
    EXPECT_EQUAL(names.size(), 0);

    // Test initial stats
    ShaderStats stats = manager.GetShaderStats();
    EXPECT_EQUAL(stats.totalShaders, 0);
    EXPECT_EQUAL(stats.loadedShaders, 0);

    // Test GetShader returns null for non-existent shader
    auto nonExistentShader = manager.GetShader("nonexistent");
    EXPECT_NULL(nonExistentShader);

    // Test unloading non-existent shader doesn't crash
    manager.UnloadShader("nonexistent");
    
    // Stats should remain the same
    stats = manager.GetShaderStats();
    EXPECT_EQUAL(stats.totalShaders, 0);

    manager.Shutdown();
    TestOutput::PrintTestPass("shader manager logic only");
    return true;
}

bool TestShaderManagerRegistration() {
    TestOutput::PrintTestStart("shader manager registration");

    ShaderManager& manager = ShaderManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());

    // Create a shader manually
    auto shader = std::make_shared<Shader>();
    
    // Test registration
    EXPECT_TRUE(manager.RegisterShader("registered_shader", shader));
    EXPECT_TRUE(manager.HasShader("registered_shader"));
    
    // Test retrieval
    auto retrievedShader = manager.GetShader("registered_shader");
    EXPECT_NOT_NULL(retrievedShader);
    EXPECT_EQUAL(shader.get(), retrievedShader.get());

    // Test shader names includes registered shader
    auto names = manager.GetShaderNames();
    EXPECT_EQUAL(names.size(), 1);
    EXPECT_EQUAL(names[0], "registered_shader");

    // Test stats update
    ShaderStats stats = manager.GetShaderStats();
    EXPECT_EQUAL(stats.totalShaders, 1);

    // Test duplicate registration
    auto anotherShader = std::make_shared<Shader>();
    EXPECT_TRUE(manager.RegisterShader("registered_shader", anotherShader)); // Should replace
    
    auto newRetrievedShader = manager.GetShader("registered_shader");
    EXPECT_EQUAL(anotherShader.get(), newRetrievedShader.get());

    // Stats should still be 1 (replaced, not added)
    stats = manager.GetShaderStats();
    EXPECT_EQUAL(stats.totalShaders, 1);

    // Test null shader registration
    EXPECT_FALSE(manager.RegisterShader("null_shader", nullptr));

    // Test unloading registered shader
    manager.UnloadShader("registered_shader");
    EXPECT_FALSE(manager.HasShader("registered_shader"));
    
    stats = manager.GetShaderStats();
    EXPECT_EQUAL(stats.totalShaders, 0);

    manager.Shutdown();
    TestOutput::PrintTestPass("shader manager registration");
    return true;
}

bool TestShaderManagerHotReload() {
    TestOutput::PrintTestStart("shader manager hot reload");

    ShaderManager& manager = ShaderManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());

    // Test hot reload enable/disable
    EXPECT_FALSE(manager.IsHotReloadEnabled());
    
    manager.EnableHotReload(true);
    EXPECT_TRUE(manager.IsHotReloadEnabled());
    
    manager.EnableHotReload(false);
    EXPECT_FALSE(manager.IsHotReloadEnabled());

    // Test hot reload callback
    bool callbackCalled = false;
    std::string callbackShaderName;
    
    manager.SetHotReloadCallback([&](const std::string& name) {
        callbackCalled = true;
        callbackShaderName = name;
    });

    // Test update method (should not crash)
    manager.Update(0.016f); // 16ms delta time

    manager.Shutdown();
    TestOutput::PrintTestPass("shader manager hot reload");
    return true;
}

bool TestShaderManagerMultipleShaders() {
    TestOutput::PrintTestStart("shader manager multiple shaders");

    ShaderManager& manager = ShaderManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());

    // Register multiple shaders
    auto shader1 = std::make_shared<Shader>();
    auto shader2 = std::make_shared<Shader>();
    auto shader3 = std::make_shared<Shader>();

    EXPECT_TRUE(manager.RegisterShader("shader1", shader1));
    EXPECT_TRUE(manager.RegisterShader("shader2", shader2));
    EXPECT_TRUE(manager.RegisterShader("shader3", shader3));

    // Test all shaders exist
    EXPECT_TRUE(manager.HasShader("shader1"));
    EXPECT_TRUE(manager.HasShader("shader2"));
    EXPECT_TRUE(manager.HasShader("shader3"));

    // Test shader names
    auto names = manager.GetShaderNames();
    EXPECT_EQUAL(names.size(), 3);

    // Test stats
    ShaderStats stats = manager.GetShaderStats();
    EXPECT_EQUAL(stats.totalShaders, 3);

    // Test unloading one shader
    manager.UnloadShader("shader2");
    EXPECT_FALSE(manager.HasShader("shader2"));
    EXPECT_TRUE(manager.HasShader("shader1"));
    EXPECT_TRUE(manager.HasShader("shader3"));

    stats = manager.GetShaderStats();
    EXPECT_EQUAL(stats.totalShaders, 2);

    // Test unloading all shaders
    manager.UnloadAllShaders();
    EXPECT_FALSE(manager.HasShader("shader1"));
    EXPECT_FALSE(manager.HasShader("shader3"));

    stats = manager.GetShaderStats();
    EXPECT_EQUAL(stats.totalShaders, 0);

    names = manager.GetShaderNames();
    EXPECT_EQUAL(names.size(), 0);

    manager.Shutdown();
    TestOutput::PrintTestPass("shader manager multiple shaders");
    return true;
}

bool TestShaderManagerDebugMode() {
    TestOutput::PrintTestStart("shader manager debug mode");

    ShaderManager& manager = ShaderManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());

    // Test debug mode
    EXPECT_FALSE(manager.IsDebugMode());
    
    manager.SetDebugMode(true);
    EXPECT_TRUE(manager.IsDebugMode());
    
    manager.SetDebugMode(false);
    EXPECT_FALSE(manager.IsDebugMode());

    manager.Shutdown();
    TestOutput::PrintTestPass("shader manager debug mode");
    return true;
}

int main() {
    TestOutput::PrintHeader("ShaderManager");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ShaderManager Tests");

        // Run all tests
        allPassed &= suite.RunTest("Shader Manager Initialization", TestShaderManagerInitialization);
        allPassed &= suite.RunTest("Shader Manager Logic Only", TestShaderManagerLogicOnly);
        allPassed &= suite.RunTest("Shader Manager Registration", TestShaderManagerRegistration);
        allPassed &= suite.RunTest("Shader Manager Multiple Shaders", TestShaderManagerMultipleShaders);
        allPassed &= suite.RunTest("Shader Manager Hot Reload", TestShaderManagerHotReload);
        allPassed &= suite.RunTest("Shader Manager Debug Mode", TestShaderManagerDebugMode);

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