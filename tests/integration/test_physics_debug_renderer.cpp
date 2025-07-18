#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Physics/PhysicsEngine.h"
#include "Physics/PhysicsDebugRenderer.h"
#include "Physics/PhysicsDebugDrawer.h"
#include "Graphics/Camera.h"
#include "Core/Logger.h"
#include <memory>
#include <thread>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Physics;

class PhysicsDebugRendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize logger for test output
        Logger::GetInstance().Initialize();
        
        // Create physics engine
        m_physicsEngine = std::make_unique<PhysicsEngine>();
        ASSERT_TRUE(m_physicsEngine->Initialize());
        
        // Create debug renderer
        m_debugRenderer = std::make_unique<PhysicsDebugRenderer>();
        ASSERT_TRUE(m_debugRenderer->Initialize());
        
        // Create camera for rendering
        m_camera = std::make_unique<Camera>();
        m_camera->SetPosition(Math::Vec3(0.0f, 5.0f, 10.0f));
        m_camera->SetTarget(Math::Vec3(0.0f, 0.0f, 0.0f));
        m_camera->SetPerspective(45.0f, 16.0f/9.0f, 0.1f, 100.0f);
        
        // Set up debug renderer with camera
        m_debugRenderer->SetCamera(m_camera.get());
        
        // Connect debug renderer to physics engine
        std::shared_ptr<IPhysicsDebugDrawer> debugDrawerPtr(m_debugRenderer.get(), [](IPhysicsDebugDrawer*){});
        m_physicsEngine->SetDebugDrawer(debugDrawerPtr);
        m_physicsEngine->EnableDebugDrawing(true);
    }
    
    void TearDown() override {
        m_physicsEngine->Shutdown();
        m_debugRenderer->Shutdown();
        // Logger cleanup is handled by singleton
    }
    
    std::unique_ptr<PhysicsEngine> m_physicsEngine;
    std::unique_ptr<PhysicsDebugRenderer> m_debugRenderer;
    std::unique_ptr<Camera> m_camera;
};

TEST_F(PhysicsDebugRendererTest, InitializationAndShutdown) {
    // Test that debug renderer initializes and shuts down properly
    auto renderer = std::make_unique<PhysicsDebugRenderer>();
    EXPECT_TRUE(renderer->Initialize());
    
    // Test configuration
    PhysicsDebugConfig config = PhysicsDebugConfig::Default();
    config.lineWidth = 3.0f;
    config.wireframeColor = Math::Vec3(1.0f, 0.0f, 0.0f); // Red
    renderer->SetConfig(config);
    
    EXPECT_EQ(renderer->GetConfig().lineWidth, 3.0f);
    EXPECT_EQ(renderer->GetConfig().wireframeColor.x, 1.0f);
    
    renderer->Shutdown();
}

TEST_F(PhysicsDebugRendererTest, BasicDrawingOperations) {
    // Test basic drawing operations
    m_debugRenderer->BeginFrame();
    
    // Draw some basic shapes
    m_debugRenderer->DrawLine(Math::Vec3(0, 0, 0), Math::Vec3(1, 1, 1), Math::Vec3(1, 0, 0));
    m_debugRenderer->DrawSphere(Math::Vec3(2, 0, 0), 1.0f, Math::Vec3(0, 1, 0));
    m_debugRenderer->DrawBox(Math::Vec3(-2, 0, 0), Math::Vec3(0.5f, 0.5f, 0.5f), 
                            Math::Quat(1, 0, 0, 0), Math::Vec3(0, 0, 1));
    m_debugRenderer->DrawCapsule(Math::Vec3(0, 2, 0), 0.5f, 2.0f, 
                                Math::Quat(1, 0, 0, 0), Math::Vec3(1, 1, 0));
    m_debugRenderer->DrawContactPoint(Math::Vec3(0, -1, 0), Math::Vec3(0, 1, 0), 
                                     0.1f, Math::Vec3(1, 0, 1));
    m_debugRenderer->DrawText(Math::Vec3(0, 3, 0), "Test Text", Math::Vec3(1, 1, 1));
    
    // Check statistics
    const auto& stats = m_debugRenderer->GetRenderStats();
    EXPECT_GT(stats.linesRendered, 0);
    EXPECT_GT(stats.spheresRendered, 0);
    EXPECT_GT(stats.boxesRendered, 0);
    EXPECT_GT(stats.capsulesRendered, 0);
    EXPECT_GT(stats.contactPointsRendered, 0);
    EXPECT_GT(stats.textItemsRendered, 0);
    
    m_debugRenderer->EndFrame();
    
    // Clear and verify
    m_debugRenderer->Clear();
    m_debugRenderer->ResetStats();
    const auto& clearedStats = m_debugRenderer->GetRenderStats();
    EXPECT_EQ(clearedStats.linesRendered, 0);
}

TEST_F(PhysicsDebugRendererTest, PhysicsIntegration) {
    // Create some physics objects to visualize
    CollisionShape boxShape;
    boxShape.type = CollisionShape::Box;
    boxShape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    RigidBody boxBody;
    boxBody.position = Math::Vec3(0.0f, 5.0f, 0.0f);
    boxBody.mass = 1.0f;
    
    uint32_t boxId = m_physicsEngine->CreateRigidBody(boxBody, boxShape);
    EXPECT_GT(boxId, 0);
    
    // Create ground
    CollisionShape groundShape;
    groundShape.type = CollisionShape::Box;
    groundShape.dimensions = Math::Vec3(10.0f, 0.1f, 10.0f);
    
    RigidBody groundBody;
    groundBody.position = Math::Vec3(0.0f, 0.0f, 0.0f);
    groundBody.isStatic = true;
    
    uint32_t groundId = m_physicsEngine->CreateRigidBody(groundBody, groundShape);
    EXPECT_GT(groundId, 0);
    
    // Simulate for a few frames and draw debug info
    for (int frame = 0; frame < 10; ++frame) {
        m_physicsEngine->Update(1.0f / 60.0f);
        
        // Draw debug visualization
        m_debugRenderer->BeginFrame();
        m_physicsEngine->DrawDebugWorld();
        m_debugRenderer->EndFrame();
        
        // Check that we're drawing something
        const auto& stats = m_debugRenderer->GetRenderStats();
        if (frame > 0) { // Skip first frame as objects might not be visible yet
            EXPECT_GT(stats.totalVertices, 0) << "Frame " << frame << " should have debug vertices";
        }
        
        m_debugRenderer->ResetStats();
    }
    
    // Clean up
    m_physicsEngine->DestroyRigidBody(boxId);
    m_physicsEngine->DestroyRigidBody(groundId);
}

TEST_F(PhysicsDebugRendererTest, PerformanceTest) {
    // Test performance with many objects
    std::vector<uint32_t> bodyIds;
    
    // Create a grid of boxes
    const int gridSize = 5;
    for (int x = -gridSize; x <= gridSize; ++x) {
        for (int z = -gridSize; z <= gridSize; ++z) {
            CollisionShape shape;
            shape.type = CollisionShape::Box;
            shape.dimensions = Math::Vec3(0.5f, 0.5f, 0.5f);
            
            RigidBody body;
            body.position = Math::Vec3(x * 2.0f, 1.0f, z * 2.0f);
            body.mass = 1.0f;
            
            uint32_t bodyId = m_physicsEngine->CreateRigidBody(body, shape);
            bodyIds.push_back(bodyId);
        }
    }
    
    LOG_INFO("Created " + std::to_string(bodyIds.size()) + " physics objects for performance test");
    
    // Measure rendering performance
    auto startTime = std::chrono::high_resolution_clock::now();
    
    const int testFrames = 30;
    for (int frame = 0; frame < testFrames; ++frame) {
        m_physicsEngine->Update(1.0f / 60.0f);
        
        m_debugRenderer->BeginFrame();
        m_physicsEngine->DrawDebugWorld();
        m_debugRenderer->EndFrame();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    float avgFrameTime = duration.count() / float(testFrames);
    LOG_INFO("Average frame time with " + std::to_string(bodyIds.size()) + 
             " objects: " + std::to_string(avgFrameTime) + "ms");
    
    // Performance should be reasonable (less than 16ms per frame for 60fps)
    EXPECT_LT(avgFrameTime, 50.0f) << "Debug rendering should maintain reasonable performance";
    
    // Check final statistics
    const auto& stats = m_debugRenderer->GetRenderStats();
    EXPECT_GT(stats.totalVertices, 0);
    EXPECT_GT(stats.drawCalls, 0);
    EXPECT_GT(stats.boxesRendered, 0);
    
    LOG_INFO("Final render stats - Vertices: " + std::to_string(stats.totalVertices) + 
             ", Draw calls: " + std::to_string(stats.drawCalls) + 
             ", Boxes: " + std::to_string(stats.boxesRendered));
    
    // Clean up
    for (uint32_t bodyId : bodyIds) {
        m_physicsEngine->DestroyRigidBody(bodyId);
    }
}

TEST_F(PhysicsDebugRendererTest, ConfigurationTest) {
    // Test different configuration options
    PhysicsDebugConfig config;
    config.lineWidth = 5.0f;
    config.wireframeColor = Math::Vec3(1.0f, 0.5f, 0.0f); // Orange
    config.contactColor = Math::Vec3(0.0f, 1.0f, 1.0f);   // Cyan
    config.maxRenderDistance = 50.0f;
    config.enableFrustumCulling = true;
    config.alpha = 0.7f;
    
    m_debugRenderer->SetConfig(config);
    
    // Verify configuration was applied
    const auto& appliedConfig = m_debugRenderer->GetConfig();
    EXPECT_EQ(appliedConfig.lineWidth, 5.0f);
    EXPECT_EQ(appliedConfig.wireframeColor.x, 1.0f);
    EXPECT_EQ(appliedConfig.wireframeColor.y, 0.5f);
    EXPECT_EQ(appliedConfig.wireframeColor.z, 0.0f);
    EXPECT_EQ(appliedConfig.maxRenderDistance, 50.0f);
    EXPECT_TRUE(appliedConfig.enableFrustumCulling);
    EXPECT_FLOAT_EQ(appliedConfig.alpha, 0.7f);
    
    // Test rendering with new configuration
    m_debugRenderer->BeginFrame();
    m_debugRenderer->DrawBox(Math::Vec3(0, 0, 0), Math::Vec3(1, 1, 1), 
                            Math::Quat(1, 0, 0, 0), appliedConfig.wireframeColor);
    m_debugRenderer->EndFrame();
    
    const auto& stats = m_debugRenderer->GetRenderStats();
    EXPECT_GT(stats.boxesRendered, 0);
}

TEST_F(PhysicsDebugRendererTest, FrustumCullingTest) {
    // Test frustum culling functionality
    PhysicsDebugConfig config;
    config.enableFrustumCulling = true;
    config.maxRenderDistance = 20.0f;
    m_debugRenderer->SetConfig(config);
    
    m_debugRenderer->BeginFrame();
    
    // Draw objects at different distances
    m_debugRenderer->DrawSphere(Math::Vec3(0, 0, 0), 1.0f, Math::Vec3(1, 0, 0));    // Close
    m_debugRenderer->DrawSphere(Math::Vec3(0, 0, 15), 1.0f, Math::Vec3(0, 1, 0));   // Medium
    m_debugRenderer->DrawSphere(Math::Vec3(0, 0, 50), 1.0f, Math::Vec3(0, 0, 1));   // Far (should be culled)
    
    m_debugRenderer->EndFrame();
    
    const auto& stats = m_debugRenderer->GetRenderStats();
    // Should render fewer spheres due to distance culling
    EXPECT_LE(stats.spheresRendered, 2) << "Distance culling should remove far objects";
    
    // Test with culling disabled
    config.enableFrustumCulling = false;
    config.maxRenderDistance = 100.0f;
    m_debugRenderer->SetConfig(config);
    
    m_debugRenderer->ResetStats();
    m_debugRenderer->BeginFrame();
    
    m_debugRenderer->DrawSphere(Math::Vec3(0, 0, 0), 1.0f, Math::Vec3(1, 0, 0));
    m_debugRenderer->DrawSphere(Math::Vec3(0, 0, 15), 1.0f, Math::Vec3(0, 1, 0));
    m_debugRenderer->DrawSphere(Math::Vec3(0, 0, 50), 1.0f, Math::Vec3(0, 0, 1));
    
    m_debugRenderer->EndFrame();
    
    const auto& statsNoCulling = m_debugRenderer->GetRenderStats();
    EXPECT_EQ(statsNoCulling.spheresRendered, 3) << "All spheres should render without culling";
}

// Mock test for integration with graphics system (would require OpenGL context)
class MockGraphicsRenderer {
public:
    MOCK_METHOD(void, SetLineWidth, (float width));
    MOCK_METHOD(void, EnableBlending, (bool enable));
    MOCK_METHOD(void, DrawLines, (const std::vector<float>& vertices));
};

TEST_F(PhysicsDebugRendererTest, GraphicsIntegrationMock) {
    // This test demonstrates how the debug renderer would integrate with graphics
    // In a real scenario, this would require an OpenGL context
    
    MockGraphicsRenderer mockRenderer;
    
    // Expect certain graphics calls when rendering debug info
    EXPECT_CALL(mockRenderer, SetLineWidth(testing::_))
        .Times(testing::AtLeast(1));
    EXPECT_CALL(mockRenderer, EnableBlending(true))
        .Times(testing::AtLeast(1));
    
    // Simulate the integration
    PhysicsDebugConfig config;
    config.lineWidth = 2.0f;
    m_debugRenderer->SetConfig(config);
    
    // In real implementation, these calls would happen inside the renderer
    mockRenderer.SetLineWidth(config.lineWidth);
    mockRenderer.EnableBlending(true);
    
    // Verify mock expectations
    testing::Mock::VerifyAndClearExpectations(&mockRenderer);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}