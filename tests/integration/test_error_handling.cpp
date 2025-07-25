#include "Audio/AudioEngine.h"
#include "Resource/ResourceManager.h"
#include "Graphics/Texture.h"
#include "Graphics/Mesh.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <iostream>
#include <memory>

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestAudioErrorHandling() {
    TestOutput::PrintTestStart("Audio Error Handling");
    
    AudioEngine audioEngine;
    
    // Test initialization (may fail gracefully)
    bool initResult = audioEngine.Initialize();
    LOG_INFO("Audio initialization result: " + std::string(initResult ? "Success" : "Failed (graceful)"));
    LOG_INFO("Audio available: " + std::string(audioEngine.IsAudioAvailable() ? "Yes" : "No"));
    
    // Test loading non-existent audio file
    auto clip = audioEngine.LoadAudioClip("nonexistent_audio.wav");
    EXPECT_TRUE(clip == nullptr); // Should fail gracefully
    
    // Test creating audio source (should work even without audio)
    uint32_t sourceId = audioEngine.CreateAudioSource();
    EXPECT_TRUE(sourceId > 0);
    
    // Test playing on invalid source (should handle gracefully)
    audioEngine.PlayAudioSource(999, clip); // Should not crash
    
    audioEngine.Shutdown();
    
    TestOutput::PrintTestPass("Audio Error Handling");
    return true;
}

bool TestResourceErrorHandling() {
    TestOutput::PrintTestStart("Resource Error Handling");
    
    ResourceManager resourceManager;
    EXPECT_TRUE(resourceManager.Initialize());
    
    // Test loading non-existent texture with fallback enabled
    LOG_INFO("Loading non-existent texture...");
    auto texture = resourceManager.Load<Texture>("nonexistent_texture.png");
    if (texture) {
        LOG_INFO("Texture loaded (fallback): " + std::to_string(texture->GetWidth()) + "x" + std::to_string(texture->GetHeight()));
        EXPECT_TRUE(texture->GetWidth() > 0);
        EXPECT_TRUE(texture->GetHeight() > 0);
    }
    
    // Test loading non-existent mesh with fallback enabled
    LOG_INFO("Loading non-existent mesh...");
    auto mesh = resourceManager.Load<Mesh>("nonexistent_mesh.obj");
    if (mesh) {
        LOG_INFO("Mesh loaded (fallback): " + std::to_string(mesh->GetVertices().size()) + " vertices");
        EXPECT_TRUE(mesh->GetVertices().size() > 0);
    }
    
    // Test memory pressure handling
    LOG_INFO("Testing memory pressure handling...");
    resourceManager.SetMemoryPressureThreshold(1024); // Very low threshold
    resourceManager.CheckMemoryPressure(); // Should not crash
    
    // Test disabling fallback resources
    resourceManager.SetFallbackResourcesEnabled(false);
    auto textureNoFallback = resourceManager.Load<Texture>("another_nonexistent.png");
    EXPECT_TRUE(textureNoFallback == nullptr); // Should fail without fallback
    
    resourceManager.Shutdown();
    
    TestOutput::PrintTestPass("Resource Error Handling");
    return true;
}

bool TestFallbackResources() {
    TestOutput::PrintTestStart("Fallback Resources");
    
    // Test texture fallback directly
    Texture texture;
    bool loadResult = texture.LoadFromFile("nonexistent.png");
    EXPECT_TRUE(!loadResult); // Should fail
    
    texture.CreateDefault();
    EXPECT_TRUE(texture.GetWidth() > 0);
    EXPECT_TRUE(texture.GetHeight() > 0);
    LOG_INFO("Created default texture: " + std::to_string(texture.GetWidth()) + "x" + std::to_string(texture.GetHeight()));
    
    // Test mesh fallback directly
    Mesh mesh;
    loadResult = mesh.LoadFromFile("nonexistent.obj");
    EXPECT_TRUE(!loadResult); // Should fail
    
    mesh.CreateDefault();
    EXPECT_TRUE(mesh.GetVertices().size() > 0);
    LOG_INFO("Created default mesh: " + std::to_string(mesh.GetVertices().size()) + " vertices");
    
    TestOutput::PrintTestPass("Fallback Resources");
    return true;
}

bool TestMemoryPressureHandling() {
    TestOutput::PrintTestStart("Memory Pressure Handling");
    
    ResourceManager resourceManager;
    EXPECT_TRUE(resourceManager.Initialize());
    
    // Load several resources to build up memory usage
    std::vector<std::shared_ptr<Texture>> textures;
    for (int i = 0; i < 5; i++) {
        auto texture = resourceManager.Load<Texture>("test_texture_" + std::to_string(i) + ".png");
        if (texture) {
            textures.push_back(texture);
        }
    }
    
    // Set very low memory threshold to trigger pressure handling
    resourceManager.SetMemoryPressureThreshold(100); // 100 bytes - very low
    
    // This should trigger memory pressure handling
    resourceManager.HandleMemoryPressure();
    
    // Verify system is still functional after memory pressure
    auto newTexture = resourceManager.Load<Texture>("post_pressure_texture.png");
    // Should still work (with fallback if needed)
    
    resourceManager.Shutdown();
    
    TestOutput::PrintTestPass("Memory Pressure Handling");
    return true;
}

int main() {
    TestOutput::PrintHeader("Error Handling Integration");
    
    bool allPassed = true;

    try {
        // Initialize logger
        Logger::GetInstance().Initialize();
        Logger::GetInstance().SetLogLevel(LogLevel::Info);
        
        // Create test suite for result tracking
        TestSuite suite("Error Handling Integration Tests");
        
        // Run all tests
        allPassed &= suite.RunTest("Audio Error Handling", TestAudioErrorHandling);
        allPassed &= suite.RunTest("Resource Error Handling", TestResourceErrorHandling);
        allPassed &= suite.RunTest("Fallback Resources", TestFallbackResources);
        allPassed &= suite.RunTest("Memory Pressure Handling", TestMemoryPressureHandling);
        
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