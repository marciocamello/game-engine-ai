#include "TestUtils.h"
#include "../../engine/modules/BulletPhysicsModule.h"
#include "../../engine/modules/PhysicsModuleFactory.h"

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Physics;

/**
 * Test physics module factory creation
 * Requirements: 2.1, 2.2, 2.5
 */
bool TestPhysicsModuleFactory() {
    TestOutput::PrintTestStart("physics module factory creation");

    // Test supported APIs
    auto supportedAPIs = PhysicsModuleFactory::GetSupportedAPIs();
    EXPECT_TRUE(!supportedAPIs.empty());

    // Test API name retrieval
    const char* bulletName = PhysicsModuleFactory::GetAPIName(PhysicsAPI::Bullet);
    EXPECT_TRUE(bulletName != nullptr);
    EXPECT_TRUE(std::string(bulletName) == "Bullet Physics");

    const char* physxName = PhysicsModuleFactory::GetAPIName(PhysicsAPI::PhysX);
    EXPECT_TRUE(physxName != nullptr);
    EXPECT_TRUE(std::string(physxName) == "NVIDIA PhysX");

#ifdef GAMEENGINE_HAS_BULLET
    // Test Bullet module creation
    auto module = PhysicsModuleFactory::CreateModule(PhysicsAPI::Bullet);
    EXPECT_TRUE(module != nullptr);
    
    if (module) {
        EXPECT_TRUE(module->GetName() == std::string("BulletPhysics"));
        EXPECT_TRUE(module->GetType() == ModuleType::Physics);
        EXPECT_TRUE(module->SupportsAPI(PhysicsAPI::Bullet));
        EXPECT_FALSE(module->SupportsAPI(PhysicsAPI::PhysX));
    }
#endif

    TestOutput::PrintTestPass("physics module factory creation");
    return true;
}

/**
 * Test physics module configuration
 * Requirements: 2.2, 2.7
 */
bool TestPhysicsModuleConfiguration() {
    TestOutput::PrintTestStart("physics module configuration");

#ifdef GAMEENGINE_HAS_BULLET
    auto module = PhysicsModuleFactory::CreateModule(PhysicsAPI::Bullet);
    EXPECT_TRUE(module != nullptr);

    if (module) {
        // Test default physics settings
        PhysicsSettings defaultSettings = module->GetPhysicsSettings();
        EXPECT_TRUE(defaultSettings.api == PhysicsAPI::Bullet);
        EXPECT_NEARLY_EQUAL(defaultSettings.configuration.gravity.y, -9.81f);
        EXPECT_NEARLY_EQUAL(defaultSettings.configuration.timeStep, 1.0f / 60.0f);
        EXPECT_EQUAL(defaultSettings.configuration.maxSubSteps, 10);
        EXPECT_EQUAL(defaultSettings.configuration.solverIterations, 10);
        EXPECT_TRUE(defaultSettings.enableCCD);
        EXPECT_FALSE(defaultSettings.enableDebugDrawing);
        EXPECT_EQUAL(defaultSettings.maxRigidBodies, 10000);
        EXPECT_EQUAL(defaultSettings.maxGhostObjects, 1000);

        // Test setting new physics settings
        PhysicsSettings newSettings;
        newSettings.api = PhysicsAPI::Bullet;
        newSettings.configuration.gravity = Math::Vec3(0.0f, -19.62f, 0.0f); // Double gravity
        newSettings.configuration.timeStep = 1.0f / 120.0f; // Higher frequency
        newSettings.configuration.maxSubSteps = 20;
        newSettings.configuration.solverIterations = 15;
        newSettings.enableCCD = false;
        newSettings.enableDebugDrawing = true;
        newSettings.maxRigidBodies = 5000;
        newSettings.maxGhostObjects = 500;

        module->SetPhysicsSettings(newSettings);
        PhysicsSettings retrievedSettings = module->GetPhysicsSettings();
        
        EXPECT_NEARLY_EQUAL(retrievedSettings.configuration.gravity.y, -19.62f);
        EXPECT_NEARLY_EQUAL(retrievedSettings.configuration.timeStep, 1.0f / 120.0f);
        EXPECT_EQUAL(retrievedSettings.configuration.maxSubSteps, 20);
        EXPECT_EQUAL(retrievedSettings.configuration.solverIterations, 15);
        EXPECT_FALSE(retrievedSettings.enableCCD);
        EXPECT_TRUE(retrievedSettings.enableDebugDrawing);
        EXPECT_EQUAL(retrievedSettings.maxRigidBodies, 5000);
        EXPECT_EQUAL(retrievedSettings.maxGhostObjects, 500);
    }
#endif

    TestOutput::PrintTestPass("physics module configuration");
    return true;
}

/**
 * Test physics module feature support
 * Requirements: 2.1, 2.2
 */
bool TestPhysicsModuleFeatures() {
    TestOutput::PrintTestStart("physics module feature support");

#ifdef GAMEENGINE_HAS_BULLET
    auto module = PhysicsModuleFactory::CreateModule(PhysicsAPI::Bullet);
    EXPECT_TRUE(module != nullptr);

    if (module) {
        // Test supported features
        EXPECT_TRUE(module->SupportsFeature(PhysicsFeature::RigidBodies));
        EXPECT_TRUE(module->SupportsFeature(PhysicsFeature::CharacterController));
        EXPECT_TRUE(module->SupportsFeature(PhysicsFeature::Constraints));
        EXPECT_TRUE(module->SupportsFeature(PhysicsFeature::Triggers));
        EXPECT_TRUE(module->SupportsFeature(PhysicsFeature::SoftBodies));
        EXPECT_TRUE(module->SupportsFeature(PhysicsFeature::Cloth));

        // Test unsupported features (not implemented yet)
        EXPECT_FALSE(module->SupportsFeature(PhysicsFeature::Fluids));
        EXPECT_FALSE(module->SupportsFeature(PhysicsFeature::Vehicles));
    }
#endif

    TestOutput::PrintTestPass("physics module feature support");
    return true;
}

/**
 * Test physics module lifecycle
 * Requirements: 2.5
 */
bool TestPhysicsModuleLifecycle() {
    TestOutput::PrintTestStart("physics module lifecycle");

#ifdef GAMEENGINE_HAS_BULLET
    auto module = PhysicsModuleFactory::CreateModule(PhysicsAPI::Bullet);
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
        EXPECT_TRUE(dependencies.empty()); // Physics module should have no dependencies

        // Test module configuration initialization
        ModuleConfig config;
        config.name = "BulletPhysics";
        config.version = "1.0.0";
        config.enabled = true;
        config.parameters["gravity_y"] = "-9.81";
        config.parameters["timeStep"] = "0.016667";
        config.parameters["enableCCD"] = "true";

        bool initResult = module->Initialize(config);
        EXPECT_TRUE(initResult);
        EXPECT_TRUE(module->IsInitialized());

        // Test physics engine access
        PhysicsEngine* engine = module->GetPhysicsEngine();
        EXPECT_TRUE(engine != nullptr);

        // Test debug drawing
        module->EnableDebugDrawing(true);
        EXPECT_TRUE(module->IsDebugDrawingEnabled());
        module->EnableDebugDrawing(false);
        EXPECT_FALSE(module->IsDebugDrawingEnabled());

        // Test debug info
        auto debugInfo = module->GetDebugInfo();
        // Debug info should be valid (even if empty initially)
        EXPECT_TRUE(debugInfo.numRigidBodies >= 0);
        EXPECT_TRUE(debugInfo.numGhostObjects >= 0);

        // Test shutdown
        module->Shutdown();
        EXPECT_FALSE(module->IsInitialized());
    }
#endif

    TestOutput::PrintTestPass("physics module lifecycle");
    return true;
}

/**
 * Test physics module interface compliance
 * Requirements: 2.1, 2.5
 */
bool TestPhysicsModuleInterface() {
    TestOutput::PrintTestStart("physics module interface compliance");

#ifdef GAMEENGINE_HAS_BULLET
    auto module = PhysicsModuleFactory::CreateModule(PhysicsAPI::Bullet);
    EXPECT_TRUE(module != nullptr);

    if (module) {
        // Test IEngineModule interface
        EXPECT_TRUE(module->GetName() != nullptr);
        EXPECT_TRUE(module->GetVersion() != nullptr);
        EXPECT_TRUE(module->GetType() == ModuleType::Physics);

        // Test IPhysicsModule interface
        EXPECT_TRUE(module->SupportsAPI(PhysicsAPI::Bullet));
        
        // GetPhysicsEngine() will return nullptr until initialized
        EXPECT_TRUE(module->GetPhysicsEngine() == nullptr);
        
        // GetActiveWorld() will return nullptr until initialized
        EXPECT_TRUE(module->GetActiveWorld() == nullptr);
    }
#endif

    TestOutput::PrintTestPass("physics module interface compliance");
    return true;
}

/**
 * Test physics module configuration parsing
 * Requirements: 2.7
 */
bool TestPhysicsModuleConfigurationParsing() {
    TestOutput::PrintTestStart("physics module configuration parsing");

#ifdef GAMEENGINE_HAS_BULLET
    auto module = PhysicsModuleFactory::CreateModule(PhysicsAPI::Bullet);
    EXPECT_TRUE(module != nullptr);

    if (module) {
        // Test comprehensive configuration parsing
        ModuleConfig config;
        config.name = "BulletPhysics";
        config.version = "1.0.0";
        config.enabled = true;
        config.parameters["gravity_x"] = "1.0";
        config.parameters["gravity_y"] = "-19.62";
        config.parameters["gravity_z"] = "2.0";
        config.parameters["timeStep"] = "0.008333"; // 120 FPS
        config.parameters["maxSubSteps"] = "15";
        config.parameters["solverIterations"] = "20";
        config.parameters["enableCCD"] = "false";
        config.parameters["enableDebugDrawing"] = "true";
        config.parameters["maxRigidBodies"] = "8000";
        config.parameters["maxGhostObjects"] = "800";
        config.parameters["linearDamping"] = "0.2";
        config.parameters["angularDamping"] = "0.3";

        bool initResult = module->Initialize(config);
        EXPECT_TRUE(initResult);

        // Verify configuration was parsed correctly
        PhysicsSettings settings = module->GetPhysicsSettings();
        EXPECT_NEARLY_EQUAL(settings.configuration.gravity.x, 1.0f);
        EXPECT_NEARLY_EQUAL(settings.configuration.gravity.y, -19.62f);
        EXPECT_NEARLY_EQUAL(settings.configuration.gravity.z, 2.0f);
        EXPECT_NEARLY_EQUAL(settings.configuration.timeStep, 0.008333f);
        EXPECT_EQUAL(settings.configuration.maxSubSteps, 15);
        EXPECT_EQUAL(settings.configuration.solverIterations, 20);
        EXPECT_FALSE(settings.enableCCD);
        EXPECT_TRUE(settings.enableDebugDrawing);
        EXPECT_EQUAL(settings.maxRigidBodies, 8000);
        EXPECT_EQUAL(settings.maxGhostObjects, 800);
        EXPECT_NEARLY_EQUAL(settings.configuration.linearDamping, 0.2f);
        EXPECT_NEARLY_EQUAL(settings.configuration.angularDamping, 0.3f);

        module->Shutdown();
    }
#endif

    TestOutput::PrintTestPass("physics module configuration parsing");
    return true;
}

int main() {
    TestOutput::PrintHeader("PhysicsModule");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("PhysicsModule Tests");

        // Run all tests
        allPassed &= suite.RunTest("Physics Module Factory", TestPhysicsModuleFactory);
        allPassed &= suite.RunTest("Physics Module Configuration", TestPhysicsModuleConfiguration);
        allPassed &= suite.RunTest("Physics Module Features", TestPhysicsModuleFeatures);
        allPassed &= suite.RunTest("Physics Module Lifecycle", TestPhysicsModuleLifecycle);
        allPassed &= suite.RunTest("Physics Module Interface", TestPhysicsModuleInterface);
        allPassed &= suite.RunTest("Physics Module Config Parsing", TestPhysicsModuleConfigurationParsing);

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