#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Physics/PhysicsEngine.h"
#include "Physics/PhysicsDebugDrawer.h"
#include "Physics/PhysicsConsole.h"
#include "Core/Logger.h"
#include <memory>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Physics;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::InSequence;

class MockPhysicsDebugDrawer : public IPhysicsDebugDrawer {
public:
    MOCK_METHOD(void, DrawLine, (const Math::Vec3& from, const Math::Vec3& to, const Math::Vec3& color), (override));
    MOCK_METHOD(void, DrawSphere, (const Math::Vec3& center, float radius, const Math::Vec3& color), (override));
    MOCK_METHOD(void, DrawBox, (const Math::Vec3& center, const Math::Vec3& halfExtents, const Math::Quat& rotation, const Math::Vec3& color), (override));
    MOCK_METHOD(void, DrawCapsule, (const Math::Vec3& center, float radius, float height, const Math::Quat& rotation, const Math::Vec3& color), (override));
    MOCK_METHOD(void, DrawText, (const Math::Vec3& position, const std::string& text, const Math::Vec3& color), (override));
    MOCK_METHOD(void, DrawContactPoint, (const Math::Vec3& point, const Math::Vec3& normal, float distance, const Math::Vec3& color), (override));
    MOCK_METHOD(void, Clear, (), (override));
};

class PhysicsDebugDrawerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize logger for test output
        Logger::GetInstance().Initialize("test_physics_debug_drawer.log");
        Logger::GetInstance().SetLogLevel(LogLevel::Debug);
        
        // Create physics engine
        engine = std::make_shared<PhysicsEngine>();
        ASSERT_TRUE(engine->Initialize()) << "Failed to initialize physics engine";
        
        // Create mock debug drawer
        mockDrawer = std::make_shared<MockPhysicsDebugDrawer>();
        
        // Create console for testing commands
        console = std::make_unique<PhysicsConsole>(engine);
    }
    
    void TearDown() override {
        engine->Shutdown();
    }
    
    std::shared_ptr<PhysicsEngine> engine;
    std::shared_ptr<MockPhysicsDebugDrawer> mockDrawer;
    std::unique_ptr<PhysicsConsole> console;
};

TEST_F(PhysicsDebugDrawerTest, SetDebugDrawerSucceeds) {
    engine->SetDebugDrawer(mockDrawer);
    
    // Verify debug drawing is initially disabled
    EXPECT_FALSE(engine->IsDebugDrawingEnabled());
    
    // Enable debug drawing
    engine->EnableDebugDrawing(true);
    EXPECT_TRUE(engine->IsDebugDrawingEnabled());
}

TEST_F(PhysicsDebugDrawerTest, DrawDebugWorldCallsClear) {
    engine->SetDebugDrawer(mockDrawer);
    engine->EnableDebugDrawing(true);
    
    // Expect Clear to be called when drawing debug world
    EXPECT_CALL(*mockDrawer, Clear())
        .Times(1);
    
    engine->DrawDebugWorld();
}

TEST_F(PhysicsDebugDrawerTest, DebugModeCanBeSet) {
    engine->SetDebugMode(PhysicsDebugMode::Wireframe);
    EXPECT_EQ(PhysicsDebugMode::Wireframe, engine->GetDebugMode());
    
    engine->SetDebugMode(PhysicsDebugMode::AABB);
    EXPECT_EQ(PhysicsDebugMode::AABB, engine->GetDebugMode());
    
    engine->SetDebugMode(PhysicsDebugMode::All);
    EXPECT_EQ(PhysicsDebugMode::All, engine->GetDebugMode());
}

TEST_F(PhysicsDebugDrawerTest, SimpleDebugDrawerStoresCommands) {
    auto simpleDrawer = std::make_shared<SimplePhysicsDebugDrawer>();
    
    // Draw some primitives
    simpleDrawer->DrawLine(Math::Vec3(0, 0, 0), Math::Vec3(1, 1, 1), Math::Vec3(1, 0, 0));
    simpleDrawer->DrawSphere(Math::Vec3(0, 0, 0), 1.0f, Math::Vec3(0, 1, 0));
    simpleDrawer->DrawBox(Math::Vec3(0, 0, 0), Math::Vec3(0.5f, 0.5f, 0.5f), 
                         Math::Quat(1, 0, 0, 0), Math::Vec3(0, 0, 1));
    
    // Verify commands were stored
    EXPECT_EQ(1, simpleDrawer->GetLines().size());
    EXPECT_EQ(1, simpleDrawer->GetSpheres().size());
    EXPECT_EQ(1, simpleDrawer->GetBoxes().size());
    
    // Clear and verify
    simpleDrawer->Clear();
    EXPECT_EQ(0, simpleDrawer->GetLines().size());
    EXPECT_EQ(0, simpleDrawer->GetSpheres().size());
    EXPECT_EQ(0, simpleDrawer->GetBoxes().size());
}

TEST_F(PhysicsDebugDrawerTest, ConsoleCanControlDebugDrawing) {
    engine->SetDebugDrawer(mockDrawer);
    
    // Test enable debug drawing command
    std::string result = console->ExecuteCommand("enable_debug_draw");
    EXPECT_THAT(result, ::testing::HasSubstr("enabled"));
    EXPECT_TRUE(engine->IsDebugDrawingEnabled());
    
    // Test disable debug drawing command
    result = console->ExecuteCommand("disable_debug_draw");
    EXPECT_THAT(result, ::testing::HasSubstr("disabled"));
    EXPECT_FALSE(engine->IsDebugDrawingEnabled());
}

TEST_F(PhysicsDebugDrawerTest, ConsoleCanSetDebugMode) {
    // Test setting wireframe mode
    std::string result = console->ExecuteCommand("set_debug_mode wireframe");
    EXPECT_THAT(result, ::testing::HasSubstr("wireframe"));
    
    // Test setting AABB mode
    result = console->ExecuteCommand("set_debug_mode aabb");
    EXPECT_THAT(result, ::testing::HasSubstr("aabb"));
    
    // Test setting all mode
    result = console->ExecuteCommand("set_debug_mode all");
    EXPECT_THAT(result, ::testing::HasSubstr("all"));
    
    // Test invalid mode
    result = console->ExecuteCommand("set_debug_mode invalid");
    EXPECT_THAT(result, ::testing::HasSubstr("Invalid"));
}

TEST_F(PhysicsDebugDrawerTest, ConsoleDebugInfoCommand) {
    // Create some rigid bodies to show in debug info
    RigidBody bodyDesc;
    bodyDesc.mass = 1.0f;
    bodyDesc.position = Math::Vec3(0, 5, 0);
    
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1, 1, 1);
    
    uint32_t bodyId1 = engine->CreateRigidBody(bodyDesc, shape);
    uint32_t bodyId2 = engine->CreateRigidBody(bodyDesc, shape);
    
    ASSERT_GT(bodyId1, 0);
    ASSERT_GT(bodyId2, 0);
    
    // Test debug info command
    std::string result = console->ExecuteCommand("debug_info");
    EXPECT_THAT(result, ::testing::HasSubstr("Physics Debug Information"));
    EXPECT_THAT(result, ::testing::HasSubstr("Rigid Bodies: 2"));
    EXPECT_THAT(result, ::testing::HasSubstr("World Gravity"));
}

TEST_F(PhysicsDebugDrawerTest, ConsoleHelpCommand) {
    std::string result = console->ExecuteCommand("help");
    EXPECT_THAT(result, ::testing::HasSubstr("Available Physics Console Commands"));
    EXPECT_THAT(result, ::testing::HasSubstr("enable_debug_draw"));
    EXPECT_THAT(result, ::testing::HasSubstr("set_debug_mode"));
    EXPECT_THAT(result, ::testing::HasSubstr("debug_info"));
}

TEST_F(PhysicsDebugDrawerTest, ConsoleParameterTuning) {
    // Test gravity modification
    std::string result = console->ExecuteCommand("set_gravity 0 -20 0");
    EXPECT_THAT(result, ::testing::HasSubstr("Gravity set"));
    
    result = console->ExecuteCommand("get_gravity");
    EXPECT_THAT(result, ::testing::HasSubstr("-20.000"));
    
    // Test timestep modification
    result = console->ExecuteCommand("set_timestep 0.01");
    EXPECT_THAT(result, ::testing::HasSubstr("Timestep set"));
    
    result = console->ExecuteCommand("get_timestep");
    EXPECT_THAT(result, ::testing::HasSubstr("0.010"));
    
    // Test solver iterations
    result = console->ExecuteCommand("set_solver_iterations 20");
    EXPECT_THAT(result, ::testing::HasSubstr("Solver iterations set"));
    
    result = console->ExecuteCommand("get_solver_iterations");
    EXPECT_THAT(result, ::testing::HasSubstr("20"));
}

TEST_F(PhysicsDebugDrawerTest, ConsoleContactThresholds) {
    std::string result = console->ExecuteCommand("set_contact_thresholds 0.01 0.005");
    EXPECT_THAT(result, ::testing::HasSubstr("Contact thresholds set"));
    
    result = console->ExecuteCommand("get_contact_thresholds");
    EXPECT_THAT(result, ::testing::HasSubstr("0.010"));
    EXPECT_THAT(result, ::testing::HasSubstr("0.005"));
}

TEST_F(PhysicsDebugDrawerTest, ConsoleResetCommand) {
    // Modify some parameters
    console->ExecuteCommand("set_gravity 0 -20 0");
    console->ExecuteCommand("set_timestep 0.01");
    
    // Reset to defaults
    std::string result = console->ExecuteCommand("reset");
    EXPECT_THAT(result, ::testing::HasSubstr("reset to defaults"));
    
    // Verify defaults are restored
    const auto& config = engine->GetConfiguration();
    EXPECT_FLOAT_EQ(-9.81f, config.gravity.y);
    EXPECT_FLOAT_EQ(1.0f/60.0f, config.timeStep);
}

TEST_F(PhysicsDebugDrawerTest, ConsoleInvalidCommands) {
    std::string result = console->ExecuteCommand("invalid_command");
    EXPECT_THAT(result, ::testing::HasSubstr("Unknown command"));
    
    result = console->ExecuteCommand("");
    EXPECT_THAT(result, ::testing::HasSubstr("Empty command"));
    
    result = console->ExecuteCommand("set_gravity");
    EXPECT_THAT(result, ::testing::HasSubstr("Usage"));
}

TEST_F(PhysicsDebugDrawerTest, DebugInfoAccuracy) {
    // Create various physics objects
    RigidBody bodyDesc;
    bodyDesc.mass = 1.0f;
    
    CollisionShape boxShape;
    boxShape.type = CollisionShape::Box;
    boxShape.dimensions = Math::Vec3(1, 1, 1);
    
    CollisionShape sphereShape;
    sphereShape.type = CollisionShape::Sphere;
    sphereShape.dimensions = Math::Vec3(0.5f, 0, 0);
    
    // Create rigid bodies
    uint32_t body1 = engine->CreateRigidBody(bodyDesc, boxShape);
    uint32_t body2 = engine->CreateRigidBody(bodyDesc, sphereShape);
    
    // Create ghost object
    uint32_t ghost1 = engine->CreateGhostObject(boxShape, Math::Vec3(0, 0, 0));
    
    ASSERT_GT(body1, 0);
    ASSERT_GT(body2, 0);
    ASSERT_GT(ghost1, 0);
    
    // Get debug info
    auto debugInfo = engine->GetDebugInfo();
    
    EXPECT_EQ(2, debugInfo.numRigidBodies);
    EXPECT_EQ(1, debugInfo.numGhostObjects);
    EXPECT_FLOAT_EQ(-9.81f, debugInfo.worldGravity.y);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}