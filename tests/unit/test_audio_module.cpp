#include "TestUtils.h"
#include "../../engine/interfaces/IAudioModule.h"
#include "../../engine/modules/audio-openal/OpenALAudioModule.h"
#include "Core/Math.h"

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Audio;

/**
 * Test OpenAL audio module creation and basic properties
 * Requirements: 2.3, 2.5 (Audio module interface and lifecycle)
 */
bool TestOpenALAudioModuleCreation() {
    TestOutput::PrintTestStart("OpenAL audio module creation and basic properties");

    auto audioModule = std::make_unique<OpenALAudioModule>();
    
    // Test basic module properties
    EXPECT_TRUE(audioModule != nullptr);
    EXPECT_EQUAL(std::string(audioModule->GetName()), std::string("OpenALAudioModule"));
    EXPECT_EQUAL(std::string(audioModule->GetVersion()), std::string("1.0.0"));
    EXPECT_TRUE(audioModule->GetType() == ModuleType::Audio);
    EXPECT_FALSE(audioModule->IsInitialized());
    EXPECT_TRUE(audioModule->IsEnabled());

    TestOutput::PrintTestPass("OpenAL audio module creation and basic properties");
    return true;
}

/**
 * Test audio module dependencies
 * Requirements: 2.5 (Module dependency management)
 */
bool TestAudioModuleDependencies() {
    TestOutput::PrintTestStart("audio module dependencies");

    auto audioModule = std::make_unique<OpenALAudioModule>();
    auto dependencies = audioModule->GetDependencies();
    
    // Audio module should depend on Core module
    EXPECT_TRUE(dependencies.size() >= 1);
    bool foundCore = false;
    for (const auto& dep : dependencies) {
        if (dep == "Core") {
            foundCore = true;
            break;
        }
    }
    EXPECT_TRUE(foundCore);

    TestOutput::PrintTestPass("audio module dependencies");
    return true;
}

/**
 * Test audio module initialization and shutdown lifecycle
 * Requirements: 2.3, 2.5 (Audio module lifecycle management)
 */
bool TestAudioModuleLifecycle() {
    TestOutput::PrintTestStart("audio module initialization and shutdown lifecycle");

    auto audioModule = std::make_unique<OpenALAudioModule>();
    
    // Test initial state
    EXPECT_FALSE(audioModule->IsInitialized());
    
    // Test initialization
    ModuleConfig config;
    config.name = "OpenALAudioModule";
    config.version = "1.0.0";
    config.enabled = true;
    
    bool initResult = audioModule->Initialize(config);
    // Note: Initialization may fail if OpenAL is not available, which is acceptable
    if (initResult) {
        EXPECT_TRUE(audioModule->IsInitialized());
        
        // Test that we can get the audio engine
        AudioEngine* engine = audioModule->GetAudioEngine();
        EXPECT_TRUE(engine != nullptr);
        
        // Test shutdown
        audioModule->Shutdown();
        EXPECT_FALSE(audioModule->IsInitialized());
    } else {
        // If initialization failed, it should still be in a valid state
        EXPECT_FALSE(audioModule->IsInitialized());
    }

    TestOutput::PrintTestPass("audio module initialization and shutdown lifecycle");
    return true;
}

/**
 * Test audio module enable/disable functionality
 * Requirements: 2.5 (Module state management)
 */
bool TestAudioModuleEnableDisable() {
    TestOutput::PrintTestStart("audio module enable/disable functionality");

    auto audioModule = std::make_unique<OpenALAudioModule>();
    
    // Test initial enabled state
    EXPECT_TRUE(audioModule->IsEnabled());
    
    // Test disabling
    audioModule->SetEnabled(false);
    EXPECT_FALSE(audioModule->IsEnabled());
    
    // Test re-enabling
    audioModule->SetEnabled(true);
    EXPECT_TRUE(audioModule->IsEnabled());

    TestOutput::PrintTestPass("audio module enable/disable functionality");
    return true;
}

/**
 * Test audio format support capabilities
 * Requirements: 2.3 (Audio system capabilities)
 */
bool TestAudioFormatSupport() {
    TestOutput::PrintTestStart("audio format support capabilities");

    auto audioModule = std::make_unique<OpenALAudioModule>();
    
    // Test supported formats
    EXPECT_TRUE(audioModule->SupportsFormat("wav"));
    EXPECT_TRUE(audioModule->SupportsFormat("WAV"));
    EXPECT_TRUE(audioModule->SupportsFormat("ogg"));
    EXPECT_TRUE(audioModule->SupportsFormat("OGG"));
    EXPECT_TRUE(audioModule->SupportsFormat("mp3"));
    EXPECT_TRUE(audioModule->SupportsFormat("MP3"));
    
    // Test unsupported format
    EXPECT_FALSE(audioModule->SupportsFormat("flac"));
    EXPECT_FALSE(audioModule->SupportsFormat("unknown"));
    
    // Test 3D audio support
    EXPECT_TRUE(audioModule->Supports3DAudio());
    
    // Test streaming support (current implementation doesn't support it)
    EXPECT_FALSE(audioModule->SupportsStreaming());

    TestOutput::PrintTestPass("audio format support capabilities");
    return true;
}

/**
 * Test audio backend information
 * Requirements: 2.3 (Audio system identification)
 */
bool TestAudioBackendInfo() {
    TestOutput::PrintTestStart("audio backend information");

    auto audioModule = std::make_unique<OpenALAudioModule>();
    
    // Test backend name
    std::string backendName = audioModule->GetAudioBackendName();
    EXPECT_EQUAL(backendName, std::string("OpenAL"));
    
    // Test device name (should return something meaningful even if not initialized)
    std::string deviceName = audioModule->GetAudioDeviceName();
    EXPECT_TRUE(!deviceName.empty());

    TestOutput::PrintTestPass("audio backend information");
    return true;
}

/**
 * Test audio module configuration handling
 * Requirements: 2.5 (Module configuration system)
 */
bool TestAudioModuleConfiguration() {
    TestOutput::PrintTestStart("audio module configuration handling");

    auto audioModule = std::make_unique<OpenALAudioModule>();
    
    // Create configuration with custom parameters
    ModuleConfig config;
    config.name = "OpenALAudioModule";
    config.version = "1.0.0";
    config.enabled = true;
    config.parameters["masterVolume"] = "0.8";
    config.parameters["musicVolume"] = "0.7";
    config.parameters["sfxVolume"] = "0.9";
    config.parameters["enableBufferPooling"] = "true";
    config.parameters["enableSourcePooling"] = "false";
    
    // Initialize with configuration
    bool initResult = audioModule->Initialize(config);
    
    if (initResult) {
        // Test that configuration was applied (volume getters should reflect config)
        EXPECT_NEARLY_EQUAL(audioModule->GetMasterVolume(), 0.8f);
        EXPECT_NEARLY_EQUAL(audioModule->GetMusicVolume(), 0.7f);
        EXPECT_NEARLY_EQUAL(audioModule->GetSFXVolume(), 0.9f);
        
        audioModule->Shutdown();
    }
    
    // Test should pass regardless of initialization success

    TestOutput::PrintTestPass("audio module configuration handling");
    return true;
}

/**
 * Test audio module update functionality
 * Requirements: 2.5 (Module lifecycle management)
 */
bool TestAudioModuleUpdate() {
    TestOutput::PrintTestStart("audio module update functionality");

    auto audioModule = std::make_unique<OpenALAudioModule>();
    
    // Test update when not initialized (should not crash)
    audioModule->Update(0.016f);
    
    // Test update when disabled (should not crash)
    audioModule->SetEnabled(false);
    audioModule->Update(0.016f);
    
    // Re-enable for potential initialization test
    audioModule->SetEnabled(true);
    
    ModuleConfig config;
    config.name = "OpenALAudioModule";
    config.version = "1.0.0";
    config.enabled = true;
    
    bool initResult = audioModule->Initialize(config);
    if (initResult) {
        // Test update when initialized (should not crash)
        audioModule->Update(0.016f);
        audioModule->Update(0.033f);
        
        audioModule->Shutdown();
    }

    TestOutput::PrintTestPass("audio module update functionality");
    return true;
}

int main() {
    TestOutput::PrintHeader("AudioModule");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("AudioModule Tests");

        // Run all tests
        allPassed &= suite.RunTest("OpenAL Audio Module Creation", TestOpenALAudioModuleCreation);
        allPassed &= suite.RunTest("Audio Module Dependencies", TestAudioModuleDependencies);
        allPassed &= suite.RunTest("Audio Module Lifecycle", TestAudioModuleLifecycle);
        allPassed &= suite.RunTest("Audio Module Enable/Disable", TestAudioModuleEnableDisable);
        allPassed &= suite.RunTest("Audio Format Support", TestAudioFormatSupport);
        allPassed &= suite.RunTest("Audio Backend Info", TestAudioBackendInfo);
        allPassed &= suite.RunTest("Audio Module Configuration", TestAudioModuleConfiguration);
        allPassed &= suite.RunTest("Audio Module Update", TestAudioModuleUpdate);

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