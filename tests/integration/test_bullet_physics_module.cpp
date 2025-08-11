#include <iostream>
#include "../TestUtils.h"
#include "../../engine/modules/BulletPhysicsModule.h"
#include "../../engine/modules/PhysicsModuleFactory.h"

using namespace GameEngine::Testing;
using namespace GameEngine::Physics;
using namespace GameEngine;

/**
 * Test Bullet physics module initialization and basic functionality
 * Requirements: 2.1, 2.2, 2.5
 */
bool TestBulletPhysicsModuleIntegration() {
    TestOutput::PrintTestStart("Bullet physics module integration");

#ifdef GAMEENGINE_HAS_BULLET
    auto module = PhysicsModuleFactory::CreateModule(PhysicsAPI::Bullet);
    EXPECT_TRUE(module != nullptr);

    if (module) {
        // Initialize the module
        ModuleConfig config;
        config.name = "BulletPhysics";
        config.version = "1.0.0";
        config.enabled = true;
        config.parameters["gravity_y"] = "-9.81";
        config.parameters["timeStep"] = "0.016667";

        bool initResult = module->Initialize(config);
        EXPECT_TRUE(initResult);
        EXPECT_TRUE(module->IsInitialized());

        // Get the physics engine
        PhysicsEngine* engine = module->GetPhysicsEngine();
        EXPECT_TRUE(engine != nullptr);

        if (engine) {
            // Test creating a rigid body
            RigidBody bodyDesc;
            bodyDesc.position = Math::Vec3(0.0f, 10.0f, 0.0f);
            bodyDesc.mass = 1.0f;
            bodyDesc.restitution = 0.5f;
            bodyDesc.friction = 0.7f;

            CollisionShape shape;
            shape.type = CollisionShape::Box;
            shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);

            uint32_t bodyId = engine->CreateRigidBody(bodyDesc, shape);
            EXPECT_TRUE(bodyId != 0);

            // Test getting rigid body transform
            Math::Vec3 position;
            Math::Quat rotation;
            bool transformResult = engine->GetRigidBodyTransform(bodyId, position, rotation);
            EXPECT_TRUE(transformResult);
            EXPECT_VEC3_NEARLY_EQUAL(position, Math::Vec3(0.0f, 10.0f, 0.0f));

            // Test applying force
            engine->ApplyForce(bodyId, Math::Vec3(0.0f, 100.0f, 0.0f));

            // Test physics simulation step
            module->Update(0.016667f); // One frame at 60 FPS

            // Test raycast
            RaycastHit hit = engine->Raycast(Math::Vec3(0.0f, 15.0f, 0.0f), Math::Vec3(0.0f, -1.0f, 0.0f), 20.0f);
            EXPECT_TRUE(hit.hasHit);
            if (hit.hasHit) {
                EXPECT_EQUAL(hit.bodyId, bodyId);
            }

            // Test overlap sphere
            std::vector<OverlapResult> overlaps = engine->OverlapSphere(Math::Vec3(0.0f, 10.0f, 0.0f), 2.0f);
            EXPECT_TRUE(overlaps.size() > 0);

            // Clean up
            engine->DestroyRigidBody(bodyId);
        }

        module->Shutdown();
        EXPECT_FALSE(module->IsInitialized());
    }
#else
    LOG_INFO("Bullet Physics not available - skipping integration test");
#endif

    TestOutput::PrintTestPass("Bullet physics module integration");
    return true;
}

/**
 * Test physics world management through module
 * Requirements: 2.1, 2.5
 */
bool TestPhysicsWorldManagement() {
    TestOutput::PrintTestStart("physics world management");

#ifdef GAMEENGINE_HAS_BULLET
    auto module = PhysicsModuleFactory::CreateModule(PhysicsAPI::Bullet);
    EXPECT_TRUE(module != nullptr);

    if (module) {
        // Initialize the module
        ModuleConfig config;
        config.name = "BulletPhysics";
        config.enabled = true;

        bool initResult = module->Initialize(config);
        EXPECT_TRUE(initResult);

        // Test creating a custom physics world
        PhysicsConfiguration worldConfig = PhysicsConfiguration::ForCharacterMovement();
        auto customWorld = module->CreateWorld(worldConfig);
        EXPECT_TRUE(customWorld != nullptr);

        if (customWorld) {
            // Test setting active world
            module->SetActiveWorld(customWorld);

            // Verify the world has the correct gravity
            EXPECT_VEC3_NEARLY_EQUAL(customWorld->GetGravity(), worldConfig.gravity);
        }

        module->Shutdown();
    }
#else
    LOG_INFO("Bullet Physics not available - skipping world management test");
#endif

    TestOutput::PrintTestPass("physics world management");
    return true;
}

/**
 * Test physics module debug functionality
 * Requirements: 2.5
 */
bool TestPhysicsModuleDebug() {
    TestOutput::PrintTestStart("physics module debug functionality");

#ifdef GAMEENGINE_HAS_BULLET
    auto module = PhysicsModuleFactory::CreateModule(PhysicsAPI::Bullet);
    EXPECT_TRUE(module != nullptr);

    if (module) {
        // Initialize the module
        ModuleConfig config;
        config.name = "BulletPhysics";
        config.enabled = true;
        config.parameters["enableDebugDrawing"] = "true";

        bool initResult = module->Initialize(config);
        EXPECT_TRUE(initResult);

        // Test debug drawing
        EXPECT_TRUE(module->IsDebugDrawingEnabled());

        module->EnableDebugDrawing(false);
        EXPECT_FALSE(module->IsDebugDrawingEnabled());

        module->EnableDebugDrawing(true);
        EXPECT_TRUE(module->IsDebugDrawingEnabled());

        // Test debug info
        auto debugInfo = module->GetDebugInfo();
        EXPECT_TRUE(debugInfo.numRigidBodies >= 0);
        EXPECT_TRUE(debugInfo.numGhostObjects >= 0);
        EXPECT_TRUE(debugInfo.numActiveObjects >= 0);
        EXPECT_TRUE(debugInfo.numSleepingObjects >= 0);

        module->Shutdown();
    }
#else
    LOG_INFO("Bullet Physics not available - skipping debug test");
#endif

    TestOutput::PrintTestPass("physics module debug functionality");
    return true;
}

/**
 * Test physics module configuration updates at runtime
 * Requirements: 2.7
 */
bool TestPhysicsModuleRuntimeConfiguration() {
    TestOutput::PrintTestStart("physics module runtime configuration");

#ifdef GAMEENGINE_HAS_BULLET
    auto module = PhysicsModuleFactory::CreateModule(PhysicsAPI::Bullet);
    EXPECT_TRUE(module != nullptr);

    if (module) {
        // Initialize with default settings
        ModuleConfig config;
        config.name = "BulletPhysics";
        config.enabled = true;

        bool initResult = module->Initialize(config);
        EXPECT_TRUE(initResult);

        PhysicsEngine* engine = module->GetPhysicsEngine();
        EXPECT_TRUE(engine != nullptr);

        if (engine) {
            // Test runtime configuration changes
            PhysicsSettings newSettings;
            newSettings.api = PhysicsAPI::Bullet;
            newSettings.configuration = PhysicsConfiguration::HighPrecision();
            newSettings.enableDebugDrawing = true;

            module->SetPhysicsSettings(newSettings);

            // Verify settings were applied
            PhysicsSettings retrievedSettings = module->GetPhysicsSettings();
            EXPECT_NEARLY_EQUAL(retrievedSettings.configuration.timeStep, 1.0f / 120.0f);
            EXPECT_EQUAL(retrievedSettings.configuration.solverIterations, 20);
            EXPECT_TRUE(retrievedSettings.enableDebugDrawing);

            // Test that the physics engine configuration was updated
            const PhysicsConfiguration& engineConfig = engine->GetConfiguration();
            EXPECT_NEARLY_EQUAL(engineConfig.timeStep, 1.0f / 120.0f);
            EXPECT_EQUAL(engineConfig.solverIterations, 20);
        }

        module->Shutdown();
    }
#else
    LOG_INFO("Bullet Physics not available - skipping runtime configuration test");
#endif

    TestOutput::PrintTestPass("physics module runtime configuration");
    return true;
}

int main() {
    TestOutput::PrintHeader("BulletPhysicsModule Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("BulletPhysicsModule Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Bullet Physics Module Integration", TestBulletPhysicsModuleIntegration);
        allPassed &= suite.RunTest("Physics World Management", TestPhysicsWorldManagement);
        allPassed &= suite.RunTest("Physics Module Debug", TestPhysicsModuleDebug);
        allPassed &= suite.RunTest("Physics Module Runtime Config", TestPhysicsModuleRuntimeConfiguration);

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