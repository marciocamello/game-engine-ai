#include "Core/Engine.h"
#include "Physics/PhysicsEngine.h"
#include "Physics/PhysicsDebugRenderer.h"
#include "Graphics/Camera.h"
#include "Graphics/GraphicsRenderer.h"
#include "Graphics/OpenGLRenderer.h"
#include "Input/InputManager.h"
#include "Core/Logger.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <chrono>

using namespace GameEngine;

class PhysicsDebugExample {
public:
    bool Initialize() {
        // Initialize logger
        Logger::GetInstance().Initialize();
        LOG_INFO("Starting Physics Debug Renderer Example");
        
        // Initialize engine
        m_engine = std::make_unique<Engine>();
        if (!m_engine->Initialize()) {
            LOG_ERROR("Failed to initialize engine");
            return false;
        }
        
        // Get systems
        m_physicsEngine = m_engine->GetPhysics();
        m_renderer = m_engine->GetRenderer();
        m_inputManager = m_engine->GetInput();
        
        if (!m_physicsEngine || !m_renderer || !m_inputManager) {
            LOG_ERROR("Failed to get engine systems");
            return false;
        }
        
        // Initialize debug renderer
        m_debugRenderer = std::make_unique<Physics::PhysicsDebugRenderer>();
        if (!m_debugRenderer->Initialize()) {
            LOG_ERROR("Failed to initialize physics debug renderer");
            return false;
        }
        
        // Configure debug renderer
        Physics::PhysicsDebugConfig debugConfig;
        debugConfig.lineWidth = 2.0f;
        debugConfig.wireframeColor = Math::Vec3(0.0f, 1.0f, 0.0f);  // Green wireframes
        debugConfig.contactColor = Math::Vec3(1.0f, 0.0f, 0.0f);    // Red contact points
        debugConfig.aabbColor = Math::Vec3(1.0f, 1.0f, 0.0f);       // Yellow AABBs
        debugConfig.enableFrustumCulling = true;
        debugConfig.maxRenderDistance = 50.0f;
        debugConfig.alpha = 0.8f;
        m_debugRenderer->SetConfig(debugConfig);
        
        // Set up camera
        m_camera = std::make_unique<Camera>();
        m_camera->SetPosition(Math::Vec3(0.0f, 10.0f, 15.0f));
        m_camera->SetTarget(Math::Vec3(0.0f, 0.0f, 0.0f));
        m_camera->SetPerspective(45.0f, 16.0f/9.0f, 0.1f, 100.0f);
        m_debugRenderer->SetCamera(m_camera.get());
        
        // Connect debug renderer to physics engine
        std::shared_ptr<Physics::IPhysicsDebugDrawer> debugDrawerPtr(m_debugRenderer.get(), [](Physics::IPhysicsDebugDrawer*){});
        m_physicsEngine->SetDebugDrawer(debugDrawerPtr);
        m_physicsEngine->EnableDebugDrawing(true);
        
        // Create physics scene
        CreatePhysicsScene();
        
        LOG_INFO("Physics Debug Example initialized successfully");
        LOG_INFO("Controls:");
        LOG_INFO("  J - Toggle debug rendering");
        LOG_INFO("  R - Reset scene");
        LOG_INFO("  ESC - Exit");
        
        return true;
    }
    
    void CreatePhysicsScene() {
        // Clear existing objects
        for (uint32_t bodyId : m_physicsObjects) {
            m_physicsEngine->DestroyRigidBody(bodyId);
        }
        m_physicsObjects.clear();
        
        // Create ground plane
        CollisionShape groundShape;
        groundShape.type = CollisionShape::Box;
        groundShape.dimensions = Math::Vec3(20.0f, 0.5f, 20.0f);
        
        RigidBody groundBody;
        groundBody.position = Math::Vec3(0.0f, -0.5f, 0.0f);
        groundBody.isStatic = true;
        groundBody.friction = 0.8f;
        
        uint32_t groundId = m_physicsEngine->CreateRigidBody(groundBody, groundShape);
        m_physicsObjects.push_back(groundId);
        
        // Create a tower of boxes
        for (int i = 0; i < 5; ++i) {
            CollisionShape boxShape;
            boxShape.type = CollisionShape::Box;
            boxShape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
            
            RigidBody boxBody;
            boxBody.position = Math::Vec3(0.0f, 1.0f + i * 2.2f, 0.0f);
            boxBody.mass = 1.0f;
            boxBody.restitution = 0.3f;
            boxBody.friction = 0.7f;
            
            uint32_t boxId = m_physicsEngine->CreateRigidBody(boxBody, boxShape);
            m_physicsObjects.push_back(boxId);
        }
        
        // Create some spheres
        for (int i = 0; i < 3; ++i) {
            CollisionShape sphereShape;
            sphereShape.type = CollisionShape::Sphere;
            sphereShape.dimensions = Math::Vec3(0.8f, 0.0f, 0.0f); // radius in x component
            
            RigidBody sphereBody;
            sphereBody.position = Math::Vec3(-5.0f + i * 2.5f, 8.0f, 3.0f);
            sphereBody.mass = 0.8f;
            sphereBody.restitution = 0.6f;
            sphereBody.friction = 0.4f;
            
            uint32_t sphereId = m_physicsEngine->CreateRigidBody(sphereBody, sphereShape);
            m_physicsObjects.push_back(sphereId);
        }
        
        // Create some capsules
        for (int i = 0; i < 2; ++i) {
            CollisionShape capsuleShape;
            capsuleShape.type = CollisionShape::Capsule;
            capsuleShape.dimensions = Math::Vec3(0.6f, 2.0f, 0.0f); // radius, height
            
            RigidBody capsuleBody;
            capsuleBody.position = Math::Vec3(5.0f, 6.0f + i * 3.0f, -2.0f);
            capsuleBody.mass = 1.2f;
            capsuleBody.restitution = 0.4f;
            capsuleBody.friction = 0.6f;
            
            uint32_t capsuleId = m_physicsEngine->CreateRigidBody(capsuleBody, capsuleShape);
            m_physicsObjects.push_back(capsuleId);
        }
        
        LOG_INFO("Created physics scene with " + std::to_string(m_physicsObjects.size()) + " objects");
    }
    
    void Run() {
        if (!Initialize()) {
            return;
        }
        
        bool running = true;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        while (running && m_engine->IsRunning()) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - startTime).count();
            startTime = currentTime;
            
            // Handle input
            m_inputManager->Update();
            
            if (m_inputManager->IsKeyPressed(KeyCode::Escape)) {
                running = false;
            }
            
            if (m_inputManager->IsKeyPressed(KeyCode::J)) {
                m_debugRenderingEnabled = !m_debugRenderingEnabled;
                m_physicsEngine->EnableDebugDrawing(m_debugRenderingEnabled);
                LOG_INFO("Debug rendering " + std::string(m_debugRenderingEnabled ? "enabled" : "disabled"));
            }
            
            if (m_inputManager->IsKeyPressed(KeyCode::R)) {
                LOG_INFO("Resetting physics scene");
                CreatePhysicsScene();
            }
            
            // Update physics
            m_physicsEngine->Update(deltaTime);
            
            // Render
            m_renderer->BeginFrame();
            m_renderer->Clear();
            
            // Set camera
            m_renderer->SetCamera(m_camera.get());
            
            // Render debug physics if enabled
            if (m_debugRenderingEnabled) {
                m_debugRenderer->BeginFrame();
                m_physicsEngine->DrawDebugWorld();
                m_debugRenderer->EndFrame();
                
                // Print debug stats occasionally
                static int frameCount = 0;
                if (++frameCount % 60 == 0) {
                    const auto& stats = m_debugRenderer->GetRenderStats();
                    LOG_DEBUG("Debug render stats - Lines: " + std::to_string(stats.linesRendered) + 
                             ", Boxes: " + std::to_string(stats.boxesRendered) + 
                             ", Spheres: " + std::to_string(stats.spheresRendered) + 
                             ", Capsules: " + std::to_string(stats.capsulesRendered) + 
                             ", Vertices: " + std::to_string(stats.totalVertices) + 
                             ", Draw calls: " + std::to_string(stats.drawCalls) + 
                             ", Render time: " + std::to_string(stats.renderTime) + "ms");
                }
            }
            
            m_renderer->EndFrame();
            m_renderer->Present();
        }
        
        Shutdown();
    }
    
    void Shutdown() {
        LOG_INFO("Shutting down Physics Debug Example");
        
        // Clean up physics objects
        for (uint32_t bodyId : m_physicsObjects) {
            m_physicsEngine->DestroyRigidBody(bodyId);
        }
        m_physicsObjects.clear();
        
        // Shutdown debug renderer
        if (m_debugRenderer) {
            m_debugRenderer->Shutdown();
        }
        
        // Shutdown engine
        if (m_engine) {
            m_engine->Shutdown();
        }
        
        // Logger cleanup is handled by singleton
    }
    
private:
    std::unique_ptr<Engine> m_engine;
    std::unique_ptr<Physics::PhysicsDebugRenderer> m_debugRenderer;
    std::unique_ptr<Camera> m_camera;
    
    PhysicsEngine* m_physicsEngine = nullptr;
    GraphicsRenderer* m_renderer = nullptr;
    InputManager* m_inputManager = nullptr;
    
    std::vector<uint32_t> m_physicsObjects;
    bool m_debugRenderingEnabled = true;
};

int main() {
    try {
        PhysicsDebugExample example;
        example.Run();
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception in Physics Debug Example: " + std::string(e.what()));
        return -1;
    }
    
    return 0;
}