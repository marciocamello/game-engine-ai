#include "TestUtils.h"
#include "../../engine/modules/OpenGLGraphicsModule.h"
#include "../../engine/modules/GraphicsModuleFactory.h"

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Graphics;

/**
 * Test graphics module factory creation
 * Requirements: 2.1, 2.4, 2.5
 */
bool TestGraphicsModuleFactory() {
    TestOutput::PrintTestStart("graphics module factory creation");

    // Test supported APIs
    auto supportedAPIs = GraphicsModuleFactory::GetSupportedAPIs();
    EXPECT_TRUE(!supportedAPIs.empty());
    EXPECT_TRUE(GraphicsModuleFactory::IsAPISupported(GraphicsAPI::OpenGL));

    // Test OpenGL module creation
    auto module = GraphicsModuleFactory::CreateModule(GraphicsAPI::OpenGL);
    EXPECT_TRUE(module != nullptr);
    
    if (module) {
        EXPECT_TRUE(module->GetName() == std::string("OpenGLGraphics"));
        EXPECT_TRUE(module->GetType() == ModuleType::Graphics);
        EXPECT_TRUE(module->SupportsAPI(GraphicsAPI::OpenGL));
        EXPECT_FALSE(module->SupportsAPI(GraphicsAPI::Vulkan));
    }

    TestOutput::PrintTestPass("graphics module factory creation");
    return true;
}

/**
 * Test graphics module configuration
 * Requirements: 2.4, 2.7
 */
bool TestGraphicsModuleConfiguration() {
    TestOutput::PrintTestStart("graphics module configuration");

    auto module = GraphicsModuleFactory::CreateModule(GraphicsAPI::OpenGL);
    EXPECT_TRUE(module != nullptr);

    if (module) {
        // Test default render settings
        RenderSettings defaultSettings = module->GetRenderSettings();
        EXPECT_EQUAL(defaultSettings.windowWidth, 1920);
        EXPECT_EQUAL(defaultSettings.windowHeight, 1080);
        EXPECT_FALSE(defaultSettings.fullscreen);
        EXPECT_TRUE(defaultSettings.vsync);
        EXPECT_EQUAL(defaultSettings.msaaSamples, 4);
        EXPECT_TRUE(defaultSettings.api == GraphicsAPI::OpenGL);

        // Test setting new render settings
        RenderSettings newSettings;
        newSettings.windowWidth = 1280;
        newSettings.windowHeight = 720;
        newSettings.fullscreen = true;
        newSettings.vsync = false;
        newSettings.msaaSamples = 8;
        newSettings.api = GraphicsAPI::OpenGL;

        module->SetRenderSettings(newSettings);
        RenderSettings retrievedSettings = module->GetRenderSettings();
        
        EXPECT_EQUAL(retrievedSettings.windowWidth, 1280);
        EXPECT_EQUAL(retrievedSettings.windowHeight, 720);
        EXPECT_TRUE(retrievedSettings.fullscreen);
        EXPECT_FALSE(retrievedSettings.vsync);
        EXPECT_EQUAL(retrievedSettings.msaaSamples, 8);
    }

    TestOutput::PrintTestPass("graphics module configuration");
    return true;
}

/**
 * Test graphics module lifecycle
 * Requirements: 2.5
 */
bool TestGraphicsModuleLifecycle() {
    TestOutput::PrintTestStart("graphics module lifecycle");

    auto module = GraphicsModuleFactory::CreateModule(GraphicsAPI::OpenGL);
    EXPECT_TRUE(module != nullptr);

    if (module) {
        // Test initial state
        EXPECT_FALSE(module->IsInitialized());
        EXPECT_TRUE(module->IsEnabled());

        // Test enable/disable
        module->SetEnabled(false);
        EXPECT_FALSE(module->IsEnabled());
        module->SetEnabled(true);
        EXPECT_TRUE(module->IsEnabled());

        // Test dependencies
        auto dependencies = module->GetDependencies();
        EXPECT_TRUE(dependencies.empty()); // Graphics module should have no dependencies

        // Note: We can't test actual initialization here because it requires
        // a valid OpenGL context, which is not available in unit tests
        // This would be tested in integration tests
    }

    TestOutput::PrintTestPass("graphics module lifecycle");
    return true;
}

/**
 * Test graphics module interface compliance
 * Requirements: 2.1, 2.5
 */
bool TestGraphicsModuleInterface() {
    TestOutput::PrintTestStart("graphics module interface compliance");

    auto module = GraphicsModuleFactory::CreateModule(GraphicsAPI::OpenGL);
    EXPECT_TRUE(module != nullptr);

    if (module) {
        // Test IEngineModule interface
        EXPECT_TRUE(module->GetName() != nullptr);
        EXPECT_TRUE(module->GetVersion() != nullptr);
        EXPECT_TRUE(module->GetType() == ModuleType::Graphics);

        // Test IGraphicsModule interface
        EXPECT_TRUE(module->SupportsAPI(GraphicsAPI::OpenGL));
        
        // GetRenderer() will return nullptr until initialized
        EXPECT_TRUE(module->GetRenderer() == nullptr);
        
        // GetWindow() will return nullptr until initialized
        EXPECT_TRUE(module->GetWindow() == nullptr);
    }

    TestOutput::PrintTestPass("graphics module interface compliance");
    return true;
}

int main() {
    TestOutput::PrintHeader("GraphicsModule");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("GraphicsModule Tests");

        // Run all tests
        allPassed &= suite.RunTest("Graphics Module Factory", TestGraphicsModuleFactory);
        allPassed &= suite.RunTest("Graphics Module Configuration", TestGraphicsModuleConfiguration);
        allPassed &= suite.RunTest("Graphics Module Lifecycle", TestGraphicsModuleLifecycle);
        allPassed &= suite.RunTest("Graphics Module Interface", TestGraphicsModuleInterface);

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