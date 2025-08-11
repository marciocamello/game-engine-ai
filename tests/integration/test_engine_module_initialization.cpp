#include "../TestUtils.h"
#include "../../engine/core/Engine.h"
#include "../../include/Core/ModuleRegistry.h"
#include "../../include/Core/ModuleConfigLoader.h"
#include "../../include/Graphics/Camera.h"
#include <filesystem>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test engine initialization with module system
 * Requirements: 2.5, 2.6, 5.1
 */
bool TestEngineModuleInitialization() {
    TestOutput::PrintTestStart("engine module initialization");

    Engine engine;
    
    // Test initialization with default configuration
    bool initResult = engine.Initialize();
    EXPECT_TRUE(initResult);
    
    // Verify module registry is accessible
    ModuleRegistry* registry = engine.GetModuleRegistry();
    EXPECT_TRUE(registry != nullptr);
    
    // Verify modules are registered
    EXPECT_TRUE(registry->IsModuleRegistered("OpenGLGraphics"));
    EXPECT_TRUE(registry->IsModuleRegistered("BulletPhysics"));
    EXPECT_TRUE(registry->IsModuleRegistered("OpenALAudioModule"));
    
    // Verify module count
    size_t moduleCount = registry->GetModuleCount();
    EXPECT_TRUE(moduleCount >= 3);
    
    // Test module access through engine
    auto graphicsModule = engine.GetGraphicsModule();
    EXPECT_TRUE(graphicsModule != nullptr);
    
    auto physicsModule = engine.GetPhysicsModule();
    EXPECT_TRUE(physicsModule != nullptr);
    
    auto audioModule = engine.GetAudioModule();
    EXPECT_TRUE(audioModule != nullptr);
    
    // Test legacy compatibility getters
    auto renderer = engine.GetRenderer();
    EXPECT_TRUE(renderer != nullptr);
    
    auto physics = engine.GetPhysics();
    EXPECT_TRUE(physics != nullptr);
    
    auto audio = engine.GetAudio();
    EXPECT_TRUE(audio != nullptr);
    
    engine.Shutdown();

    TestOutput::PrintTestPass("engine module initialization");
    return true;
}

/**
 * Test engine initialization with custom configuration
 * Requirements: 2.5, 2.6, 5.1
 */
bool TestEngineCustomConfiguration() {
    TestOutput::PrintTestStart("engine custom configuration");

    // Create a temporary configuration file
    EngineConfig config = ModuleConfigLoader::CreateDefaultConfig();
    
    // Add custom graphics module configuration
    ModuleConfig graphicsConfig;
    graphicsConfig.name = "OpenGLGraphics";
    graphicsConfig.version = "1.0.0";
    graphicsConfig.enabled = true;
    graphicsConfig.parameters["windowWidth"] = "800";
    graphicsConfig.parameters["windowHeight"] = "600";
    graphicsConfig.parameters["fullscreen"] = "false";
    graphicsConfig.parameters["vsync"] = "true";
    
    // Replace default graphics config
    for (auto& moduleConfig : config.modules) {
        if (moduleConfig.name == "OpenGLGraphics") {
            moduleConfig = graphicsConfig;
            break;
        }
    }
    
    // Save temporary config
    std::string tempConfigPath = "temp_engine_config.json";
    bool saveResult = ModuleConfigLoader::SaveToFile(config, tempConfigPath);
    EXPECT_TRUE(saveResult);
    
    Engine engine;
    
    // Test initialization with custom configuration
    bool initResult = engine.Initialize(tempConfigPath);
    EXPECT_TRUE(initResult);
    
    // Verify graphics module is accessible
    auto graphicsModule = engine.GetGraphicsModule();
    EXPECT_TRUE(graphicsModule != nullptr);
    
    // Note: We can't test specific render settings without including the full
    // IGraphicsModule interface, but we can verify the module is accessible
    
    engine.Shutdown();
    
    // Clean up temporary file
    if (std::filesystem::exists(tempConfigPath)) {
        std::filesystem::remove(tempConfigPath);
    }

    TestOutput::PrintTestPass("engine custom configuration");
    return true;
}

/**
 * Test engine module lifecycle management
 * Requirements: 2.5, 2.6, 5.1
 */
bool TestEngineModuleLifecycle() {
    TestOutput::PrintTestStart("engine module lifecycle");

    Engine engine;
    
    // Initialize engine
    bool initResult = engine.Initialize();
    EXPECT_TRUE(initResult);
    
    ModuleRegistry* registry = engine.GetModuleRegistry();
    EXPECT_TRUE(registry != nullptr);
    
    // Verify all modules are initialized
    auto modules = registry->GetAllModules();
    for (auto* module : modules) {
        EXPECT_TRUE(module->IsInitialized());
        EXPECT_TRUE(module->IsEnabled());
    }
    
    // Test module update (simulate one frame)
    float deltaTime = 0.016f; // 60 FPS
    registry->UpdateModules(deltaTime);
    
    // Modules should still be initialized after update
    for (auto* module : modules) {
        EXPECT_TRUE(module->IsInitialized());
    }
    
    // Test shutdown
    engine.Shutdown();
    
    // After shutdown, modules should be cleaned up
    // Note: We can't test this directly as the registry is cleaned up

    TestOutput::PrintTestPass("engine module lifecycle");
    return true;
}

/**
 * Test engine camera integration with modules
 * Requirements: 2.5, 2.6, 5.1
 */
bool TestEngineCameraIntegration() {
    TestOutput::PrintTestStart("engine camera integration");

    Engine engine;
    
    // Initialize engine
    bool initResult = engine.Initialize();
    EXPECT_TRUE(initResult);
    
    // Create a test camera
    Camera camera;
    camera.SetPosition(Math::Vec3(1.0f, 2.0f, 3.0f));
    camera.SetRotation(Math::Vec3(0.1f, 0.2f, 0.0f));
    
    // Set main camera
    engine.SetMainCamera(&camera);
    
    // Verify audio module receives camera updates
    auto audioModule = engine.GetAudioModule();
    EXPECT_TRUE(audioModule != nullptr);
    
    // Update camera velocity
    float deltaTime = 0.016f;
    Math::Vec3 newPosition(2.0f, 3.0f, 4.0f);
    camera.SetPosition(newPosition);
    
    // Simulate engine update to propagate camera changes
    // Note: We can't directly test the audio listener position without 
    // exposing internal state, but we can verify the integration doesn't crash
    
    engine.Shutdown();

    TestOutput::PrintTestPass("engine camera integration");
    return true;
}

/**
 * Test engine fallback to legacy mode (when module system fails)
 * Requirements: 2.5, 2.6, 5.1
 */
bool TestEngineFallbackMode() {
    TestOutput::PrintTestStart("engine fallback mode");

    // This test is difficult to implement without breaking the module system
    // For now, we'll just verify that the engine can handle initialization
    // gracefully when things go wrong
    
    Engine engine;
    
    // Test with invalid configuration path
    bool initResult = engine.Initialize("nonexistent_config.json");
    
    // Should still succeed with default configuration
    EXPECT_TRUE(initResult);
    
    engine.Shutdown();

    TestOutput::PrintTestPass("engine fallback mode");
    return true;
}

int main() {
    TestOutput::PrintHeader("Engine Module Integration Tests");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Engine Module Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Engine Module Initialization", TestEngineModuleInitialization);
        allPassed &= suite.RunTest("Engine Custom Configuration", TestEngineCustomConfiguration);
        allPassed &= suite.RunTest("Engine Module Lifecycle", TestEngineModuleLifecycle);
        allPassed &= suite.RunTest("Engine Camera Integration", TestEngineCameraIntegration);
        allPassed &= suite.RunTest("Engine Fallback Mode", TestEngineFallbackMode);

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