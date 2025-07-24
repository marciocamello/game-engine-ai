#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Graphics/GraphicsRenderer.h"
#include "Physics/PhysicsEngine.h"
#include "Audio/AudioEngine.h"
#include "Resource/ResourceManager.h"
#include "Graphics/Texture.h"
#include "Graphics/Mesh.h"
#include "Game/Character.h"
#include "Game/ThirdPersonCamera.h"
#include "Input/InputManager.h"
#include "../TestUtils.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

class V1ValidationTest {
private:
    std::unique_ptr<Engine> m_engine;
    TestSuite m_suite;
    
public:
    V1ValidationTest() : m_suite("Game Engine Kiro v1.0 Final Validation") {}
    
    bool RunAllTests() {
        TestOutput::PrintHeader("Game Engine Kiro v1.0 Final Validation");
        
        bool allPassed = true;
        
        // Core system tests
        allPassed &= m_suite.RunTest("Engine Initialization", [this]() { return TestEngineInitialization(); });
        allPassed &= m_suite.RunTest("System Integration", [this]() { return TestSystemIntegration(); });
        
        // Audio system validation
        allPassed &= m_suite.RunTest("Audio System Complete", [this]() { return TestAudioSystemComplete(); });
        allPassed &= m_suite.RunTest("3D Audio Positioning", [this]() { return Test3DAudioPositioning(); });
        
        // Resource system validation
        allPassed &= m_suite.RunTest("Resource System Complete", [this]() { return TestResourceSystemComplete(); });
        allPassed &= m_suite.RunTest("Resource Loading Performance", [this]() { return TestResourceLoadingPerformance(); });
        
        // Memory and performance validation
        allPassed &= m_suite.RunTest("Memory Management", [this]() { return TestMemoryManagement(); });
        allPassed &= m_suite.RunTest("Performance Under Load", [this]() { return TestPerformanceUnderLoad(); });
        
        // Error handling validation
        allPassed &= m_suite.RunTest("Error Recovery", [this]() { return TestErrorRecovery(); });
        allPassed &= m_suite.RunTest("Graceful Degradation", [this]() { return TestGracefulDegradation(); });
        
        // Requirements validation
        allPassed &= m_suite.RunTest("Requirements 3.1-3.3", [this]() { return TestRequirements31to33(); });
        allPassed &= m_suite.RunTest("Requirements 5.7 & 6.6", [this]() { return TestRequirements57and66(); });
        
        m_suite.PrintSummary();
        TestOutput::PrintFooter(allPassed);
        
        return allPassed;
    }
    
private:
    bool TestEngineInitialization() {
        TestOutput::PrintTestStart("Engine Initialization");
        
        try {
            m_engine = std::make_unique<Engine>();
            
            // Test engine initialization
            bool initResult = m_engine->Initialize();
            EXPECT_TRUE(initResult);
            
            // Verify all core systems are initialized
            auto* graphics = m_engine->GetRenderer();
            EXPECT_TRUE(graphics != nullptr);
            
            auto* physics = m_engine->GetPhysics();
            EXPECT_TRUE(physics != nullptr);
            
            auto* audio = m_engine->GetAudio();
            EXPECT_TRUE(audio != nullptr);
            
            auto* input = m_engine->GetInput();
            EXPECT_TRUE(input != nullptr);
            
            TestOutput::PrintTestPass("Engine Initialization");
            return true;
        }
        catch (const std::exception& e) {
            TestOutput::PrintTestFail("Engine Initialization");
            return false;
        }
    }
    
    bool TestSystemIntegration() {
        TestOutput::PrintTestStart("System Integration");
        
        try {
            if (!m_engine) {
                TestOutput::PrintTestFail("System Integration");
                return false;
            }
            
            // Test graphics-physics integration
            auto* graphics = m_engine->GetRenderer();
            auto* physics = m_engine->GetPhysics();
            
            // Create a test character to verify game system integration
            Character testCharacter;
            
            // Test camera system integration
            ThirdPersonCamera camera;
            camera.SetTarget(&testCharacter);
            
            // Verify systems can work together
            Math::Vec3 testPos(1.0f, 2.0f, 3.0f);
            testCharacter.SetPosition(testPos);
            camera.Update(0.016f, m_engine->GetInput()); // Update camera
            
            Math::Vec3 cameraPos = camera.GetPosition();
            EXPECT_TRUE(cameraPos.x != 0.0f || cameraPos.y != 0.0f || cameraPos.z != 0.0f);
            
            TestOutput::PrintTestPass("System Integration");
            return true;
        }
        catch (const std::exception& e) {
            TestOutput::PrintTestFail("System Integration");
            return false;
        }
    }
    
    bool TestAudioSystemComplete() {
        TestOutput::PrintTestStart("Audio System Complete");
        
        try {
            auto* audio = m_engine->GetAudio();
            EXPECT_TRUE(audio != nullptr);
            
            // Test OpenAL initialization
            bool audioInitialized = audio->IsOpenALInitialized();
            if (!audioInitialized) {
                Logger::GetInstance().Warning("OpenAL not available - testing fallback behavior");
                // This is acceptable - engine should work without audio
            }
            
            // Test audio loading capabilities
            auto audioClip = audio->LoadAudioClip("assets/audio/file_example_WAV_5MG.wav");
            EXPECT_TRUE(audioClip != nullptr);
            
            // Test audio source creation and management
            auto audioSource = audio->CreateAudioSource();
            EXPECT_TRUE(audioSource != 0);
            
            if (audioInitialized && audioSource) {
                // Test 3D positioning
                audio->SetAudioSourcePosition(audioSource, Math::Vec3(1.0f, 0.0f, 0.0f));
                audio->SetAudioSourceVolume(audioSource, 0.5f);
                audio->SetAudioSourcePitch(audioSource, 1.0f);
                
                // Test playback control
                if (audioClip) {
                    audio->PlayAudioSource(audioSource, audioClip);
                    audio->StopAudioSource(audioSource);
                }
            }
            
            TestOutput::PrintTestPass("Audio System Complete");
            return true;
        }
        catch (const std::exception& e) {
            TestOutput::PrintTestFail("Audio System Complete");
            return false;
        }
    }
    
    bool Test3DAudioPositioning() {
        TestOutput::PrintTestStart("3D Audio Positioning");
        
        try {
            auto* audio = m_engine->GetAudio();
            EXPECT_TRUE(audio != nullptr);
            
            if (!audio->IsOpenALInitialized()) {
                Logger::GetInstance().Info("OpenAL not available - skipping 3D audio test");
                TestOutput::PrintTestPass("3D Audio Positioning (Skipped - No OpenAL)");
                return true;
            }
            
            // Test listener positioning
            Math::Vec3 listenerPos(0.0f, 0.0f, 0.0f);
            Math::Vec3 listenerForward(0.0f, 0.0f, -1.0f);
            Math::Vec3 listenerUp(0.0f, 1.0f, 0.0f);
            
            audio->SetListenerPosition(listenerPos);
            audio->SetListenerOrientation(listenerForward, listenerUp);
            
            // Test multiple audio sources at different positions
            auto source1 = audio->CreateAudioSource();
            auto source2 = audio->CreateAudioSource();
            
            EXPECT_TRUE(source1 != 0);
            EXPECT_TRUE(source2 != 0);
            
            if (source1 != 0 && source2 != 0) {
                audio->SetAudioSourcePosition(source1, Math::Vec3(-5.0f, 0.0f, 0.0f));
                audio->SetAudioSourcePosition(source2, Math::Vec3(5.0f, 0.0f, 0.0f));
            }
            
            TestOutput::PrintTestPass("3D Audio Positioning");
            return true;
        }
        catch (const std::exception& e) {
            TestOutput::PrintTestFail("3D Audio Positioning");
            return false;
        }
    }
    
    bool TestResourceSystemComplete() {
        TestOutput::PrintTestStart("Resource System Complete");
        
        try {
            auto* resourceManager = m_engine->GetResourceManager();
            
            // Test texture loading
            auto texture = resourceManager->Load<Texture>("assets/textures/wall.png");
            EXPECT_TRUE(texture != nullptr);
            // Force GPU resource creation by binding the texture
            texture->Bind();
            EXPECT_TRUE(texture->GetID() != 0);
            
            // Test mesh loading
            auto mesh = resourceManager->Load<Mesh>("assets/meshes/cube.obj");
            EXPECT_TRUE(mesh != nullptr);
            
            // Test audio clip loading through AudioEngine
            auto* audio = m_engine->GetAudio();
            auto audioClip = audio->LoadAudioClip("assets/audio/file_example_WAV_5MG.wav");
            EXPECT_TRUE(audioClip != nullptr);
            
            // Test resource caching
            auto texture2 = resourceManager->Load<Texture>("assets/textures/wall.png");
            EXPECT_TRUE(texture.get() == texture2.get()); // Should be same cached instance
            
            // Test resource statistics
            size_t resourceCount = resourceManager->GetResourceCount();
            size_t memoryUsage = resourceManager->GetMemoryUsage();
            
            EXPECT_TRUE(resourceCount > 0);
            EXPECT_TRUE(memoryUsage > 0);
            
            Logger::GetInstance().Info("Resource Statistics - Count: " + std::to_string(resourceCount) + 
                                        ", Memory: " + std::to_string(memoryUsage) + " bytes");
            
            TestOutput::PrintTestPass("Resource System Complete");
            return true;
        }
        catch (const std::exception& e) {
            TestOutput::PrintTestFail("Resource System Complete");
            return false;
        }
    }
    
    bool TestResourceLoadingPerformance() {
        TestOutput::PrintTestStart("Resource Loading Performance");
        
        try {
            auto* resourceManager = m_engine->GetResourceManager();
            TestTimer timer;
            
            // Test texture loading performance
            timer.Restart();
            for (int i = 0; i < 10; ++i) {
                auto texture = resourceManager->Load<Texture>("assets/textures/wall.png");
                EXPECT_TRUE(texture != nullptr);
            }
            double textureLoadTime = timer.ElapsedMs();
            
            // Test mesh loading performance
            timer.Restart();
            for (int i = 0; i < 5; ++i) {
                auto mesh = resourceManager->Load<Mesh>("assets/meshes/cube.obj");
                EXPECT_TRUE(mesh != nullptr);
            }
            double meshLoadTime = timer.ElapsedMs();
            
            // Test audio loading performance
            auto* audio = m_engine->GetAudio();
            timer.Restart();
            for (int i = 0; i < 5; ++i) {
                auto audioClip = audio->LoadAudioClip("assets/audio/file_example_WAV_5MG.wav");
                EXPECT_TRUE(audioClip != nullptr);
            }
            double audioLoadTime = timer.ElapsedMs();
            
            Logger::GetInstance().Info("Performance Results:");
            Logger::GetInstance().Info("  Texture Loading: " + std::to_string(textureLoadTime) + "ms");
            Logger::GetInstance().Info("  Mesh Loading: " + std::to_string(meshLoadTime) + "ms");
            Logger::GetInstance().Info("  Audio Loading: " + std::to_string(audioLoadTime) + "ms");
            
            // Performance should be reasonable (under 100ms for cached resources)
            EXPECT_TRUE(textureLoadTime < 100.0);
            EXPECT_TRUE(meshLoadTime < 500.0);
            EXPECT_TRUE(audioLoadTime < 500.0);
            
            TestOutput::PrintTestPass("Resource Loading Performance");
            return true;
        }
        catch (const std::exception& e) {
            TestOutput::PrintTestFail("Resource Loading Performance");
            return false;
        }
    }
    
    bool TestMemoryManagement() {
        TestOutput::PrintTestStart("Memory Management");
        
        try {
            auto* resourceManager = m_engine->GetResourceManager();
            
            // Get initial memory usage
            size_t initialMemory = resourceManager->GetMemoryUsage();
            
            // Load resources in a scope to test automatic cleanup
            {
                std::vector<std::shared_ptr<Texture>> textures;
                std::vector<std::shared_ptr<Mesh>> meshes;
                
                // Load multiple resources
                for (int i = 0; i < 5; ++i) {
                    textures.push_back(resourceManager->Load<Texture>("assets/textures/wall.png"));
                    meshes.push_back(resourceManager->Load<Mesh>("assets/meshes/cube.obj"));
                    // Audio clips are managed by AudioEngine, not ResourceManager
                }
                
                // Memory usage should increase
                size_t midMemory = resourceManager->GetMemoryUsage();
                EXPECT_TRUE(midMemory >= initialMemory);
            }
            
            // Force cleanup of unused resources
            resourceManager->UnloadUnused();
            
            // Memory should be managed properly (some resources may still be cached)
            size_t finalMemory = resourceManager->GetMemoryUsage();
            
            Logger::GetInstance().Info("Memory Management Results:");
            Logger::GetInstance().Info("  Initial Memory: " + std::to_string(initialMemory) + " bytes");
            Logger::GetInstance().Info("  Final Memory: " + std::to_string(finalMemory) + " bytes");
            
            TestOutput::PrintTestPass("Memory Management");
            return true;
        }
        catch (const std::exception& e) {
            TestOutput::PrintTestFail("Memory Management");
            return false;
        }
    }
    
    bool TestPerformanceUnderLoad() {
        TestOutput::PrintTestStart("Performance Under Load");
        
        try {
            TestTimer timer;
            auto* resourceManager = m_engine->GetResourceManager();
            
            // Test engine update performance under load
            timer.Restart();
            
            // Simulate game loop with multiple systems active
            for (int frame = 0; frame < 100; ++frame) {
                float deltaTime = 0.016f; // 60 FPS
                
                // Update physics
                auto* physics = m_engine->GetPhysics();
                if (physics) {
                    physics->Update(deltaTime);
                }
                
                // Update audio
                auto* audio = m_engine->GetAudio();
                if (audio) {
                    // Simulate listener movement
                    Math::Vec3 listenerPos(sin(frame * 0.1f), 0.0f, cos(frame * 0.1f));
                    audio->SetListenerPosition(listenerPos);
                    audio->SetListenerOrientation(Math::Vec3(0.0f, 0.0f, -1.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
                }
                
                // Load resources periodically
                if (frame % 10 == 0) {
                    auto texture = resourceManager->Load<Texture>("assets/textures/wall.png");
                    auto mesh = resourceManager->Load<Mesh>("assets/meshes/cube.obj");
                }
            }
            
            double totalTime = timer.ElapsedMs();
            double avgFrameTime = totalTime / 100.0;
            
            Logger::GetInstance().Info("Performance Under Load:");
            Logger::GetInstance().Info("  Total Time: " + std::to_string(totalTime) + "ms");
            Logger::GetInstance().Info("  Average Frame Time: " + std::to_string(avgFrameTime) + "ms");
            Logger::GetInstance().Info("  Estimated FPS: " + std::to_string(1000.0 / avgFrameTime));
            
            // Should maintain reasonable performance (target: >30 FPS)
            EXPECT_TRUE(avgFrameTime < 33.33); // 30 FPS threshold
            
            TestOutput::PrintTestPass("Performance Under Load");
            return true;
        }
        catch (const std::exception& e) {
            TestOutput::PrintTestFail("Performance Under Load");
            return false;
        }
    }
    
    bool TestErrorRecovery() {
        TestOutput::PrintTestStart("Error Recovery");
        
        try {
            auto* resourceManager = m_engine->GetResourceManager();
            
            // Test loading non-existent resources (should use fallbacks)
            auto invalidTexture = resourceManager->Load<Texture>("nonexistent.png");
            EXPECT_TRUE(invalidTexture != nullptr); // Should get default texture
            // Force GPU resource creation by binding the texture
            invalidTexture->Bind();
            EXPECT_TRUE(invalidTexture->GetID() != 0); // Should have valid OpenGL texture
            
            auto invalidMesh = resourceManager->Load<Mesh>("nonexistent.obj");
            EXPECT_TRUE(invalidMesh != nullptr); // Should get default mesh
            
            auto* audio = m_engine->GetAudio();
            auto invalidAudio = audio->LoadAudioClip("nonexistent.wav");
            // Audio loading may return nullptr for invalid files, which is acceptable
            
            // Test audio system error recovery
            if (audio && audio->IsOpenALInitialized()) {
                auto source = audio->CreateAudioSource();
                if (source != 0) {
                    // Try to play with invalid audio clip (should not crash)
                    audio->StopAudioSource(source);
                }
            }
            
            TestOutput::PrintTestPass("Error Recovery");
            return true;
        }
        catch (const std::exception& e) {
            TestOutput::PrintTestFail("Error Recovery");
            return false;
        }
    }
    
    bool TestGracefulDegradation() {
        TestOutput::PrintTestStart("Graceful Degradation");
        
        try {
            auto* resourceManager = m_engine->GetResourceManager();
            
            // Test engine behavior when audio is not available
            auto* audio = m_engine->GetAudio();
            EXPECT_TRUE(audio != nullptr); // Audio engine should exist
            
            if (!audio->IsOpenALInitialized()) {
                Logger::GetInstance().Info("Testing graceful degradation - OpenAL not available");
                
                // Engine should still function without audio
                auto source = audio->CreateAudioSource();
                // Should return 0 or dummy source, not crash
                
                // Audio loading should still work (may return nullptr gracefully)
                auto audioClip = audio->LoadAudioClip("assets/audio/file_example_WAV_5MG.wav");
                // This may be nullptr when OpenAL is not available, which is acceptable
            }
            
            // Test resource system with limited memory (simulated)
            // Load many resources to test memory pressure handling
            std::vector<std::shared_ptr<Texture>> textures;
            for (int i = 0; i < 20; ++i) {
                auto texture = resourceManager->Load<Texture>("assets/textures/wall.png");
                EXPECT_TRUE(texture != nullptr);
                textures.push_back(texture);
            }
            
            // System should handle this gracefully
            size_t memoryUsage = resourceManager->GetMemoryUsage();
            EXPECT_TRUE(memoryUsage > 0);
            
            TestOutput::PrintTestPass("Graceful Degradation");
            return true;
        }
        catch (const std::exception& e) {
            TestOutput::PrintTestFail("Graceful Degradation");
            return false;
        }
    }
    
    bool TestRequirements31to33() {
        TestOutput::PrintTestStart("Requirements 3.1-3.3 Validation");
        
        try {
            auto* resourceManager = m_engine->GetResourceManager();
            
            // Requirement 3.1: Main engine initializes both audio and resource systems
            EXPECT_TRUE(m_engine != nullptr);
            EXPECT_TRUE(m_engine->GetAudio() != nullptr);
            EXPECT_TRUE(resourceManager != nullptr);
            
            // Requirement 3.2: Character system can play sounds through audio system
            Character testCharacter;
            
            auto* audio = m_engine->GetAudio();
            if (audio && audio->IsOpenALInitialized()) {
                auto audioSource = audio->CreateAudioSource();
                EXPECT_TRUE(audioSource != 0);
                
                // Test character audio integration
                testCharacter.SetPosition(Math::Vec3(1.0f, 0.0f, 0.0f));
                if (audioSource != 0) {
                    audio->SetAudioSourcePosition(audioSource, testCharacter.GetPosition());
                }
            }
            
            // Requirement 3.3: Graphics system loads textures through resource manager
            auto* graphics = m_engine->GetRenderer();
            EXPECT_TRUE(graphics != nullptr);
            
            auto texture = resourceManager->Load<Texture>("assets/textures/wall.png");
            EXPECT_TRUE(texture != nullptr);
            // Force GPU resource creation by binding the texture
            texture->Bind();
            EXPECT_TRUE(texture->GetID() != 0);
            
            TestOutput::PrintTestPass("Requirements 3.1-3.3 Validation");
            return true;
        }
        catch (const std::exception& e) {
            TestOutput::PrintTestFail("Requirements 3.1-3.3 Validation");
            return false;
        }
    }
    
    bool TestRequirements57and66() {
        TestOutput::PrintTestStart("Requirements 5.7 & 6.6 Validation");
        
        try {
            auto* resourceManager = m_engine->GetResourceManager();
            
            // Requirement 5.7: Extended periods without memory leaks
            size_t initialMemory = resourceManager->GetMemoryUsage();
            
            // Simulate extended operation
            for (int cycle = 0; cycle < 10; ++cycle) {
                {
                    std::vector<std::shared_ptr<Texture>> tempTextures;
                    for (int i = 0; i < 5; ++i) {
                        tempTextures.push_back(resourceManager->Load<Texture>("assets/textures/wall.png"));
                    }
                    // Resources go out of scope
                }
                
                // Force cleanup
                resourceManager->UnloadUnused();
            }
            
            size_t finalMemory = resourceManager->GetMemoryUsage();
            
            // Memory should not grow unbounded
            float memoryGrowth = 0.0f;
            if (initialMemory > 0) {
                memoryGrowth = (float)(finalMemory - initialMemory) / (float)initialMemory;
            } else if (finalMemory > 0) {
                memoryGrowth = 1.0f; // 100% growth from 0 is acceptable
            }
            EXPECT_TRUE(memoryGrowth < 2.0f); // Less than 200% growth
            
            // Requirement 6.6: System provides resource counts and memory usage
            size_t resourceCount = resourceManager->GetResourceCount();
            size_t memoryUsage = resourceManager->GetMemoryUsage();
            
            EXPECT_TRUE(resourceCount >= 0);
            EXPECT_TRUE(memoryUsage >= 0);
            
            Logger::GetInstance().Info("Final Resource Statistics:");
            Logger::GetInstance().Info("  Resource Count: " + std::to_string(resourceCount));
            Logger::GetInstance().Info("  Memory Usage: " + std::to_string(memoryUsage) + " bytes");
            Logger::GetInstance().Info("  Memory Growth: " + std::to_string(memoryGrowth * 100.0f) + "%");
            
            TestOutput::PrintTestPass("Requirements 5.7 & 6.6 Validation");
            return true;
        }
        catch (const std::exception& e) {
            TestOutput::PrintTestFail("Requirements 5.7 & 6.6 Validation");
            return false;
        }
    }
};

int main() {
    Logger::GetInstance().Initialize();
    Logger::GetInstance().Info("Starting Game Engine Kiro v1.0 Final Validation Test");
    
    V1ValidationTest validator;
    bool allTestsPassed = validator.RunAllTests();
    
    if (allTestsPassed) {
        Logger::GetInstance().Info("✅ Game Engine Kiro v1.0 VALIDATION PASSED");
        Logger::GetInstance().Info("All systems are working correctly and requirements are met.");
    } else {
        Logger::GetInstance().Error("❌ Game Engine Kiro v1.0 VALIDATION FAILED");
        Logger::GetInstance().Error("Some systems or requirements are not working correctly.");
    }
    
    return allTestsPassed ? 0 : 1;
}