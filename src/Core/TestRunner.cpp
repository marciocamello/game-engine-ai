#include "Core/TestRunner.h"
#include "Core/Logger.h"
#include <algorithm>
#include <iostream>

namespace GameEngine {
namespace Testing {

TestRunner& TestRunner::GetInstance() {
    static TestRunner instance;
    return instance;
}

void TestRunner::RegisterVisualTest(const VisualTest& test) {
    m_tests.push_back(test);
    Logger::GetInstance().Info("Registered visual test: " + test.name);
}

void TestRunner::RegisterRenderTest(const std::string& name, std::function<void()> renderFunc) {
    VisualTest test;
    test.name = name;
    test.description = "Render test: " + name;
    test.setup = []() {};
    test.update = [](float) {};
    test.render = renderFunc;
    test.cleanup = []() {};
    
    RegisterVisualTest(test);
}

void TestRunner::RegisterPhysicsTest(const std::string& name, std::function<void(float)> updateFunc) {
    VisualTest test;
    test.name = name;
    test.description = "Physics test: " + name;
    test.setup = []() {};
    test.update = updateFunc;
    test.render = []() {};
    test.cleanup = []() {};
    
    RegisterVisualTest(test);
}

void TestRunner::RunTest(const std::string& testName) {
    auto it = std::find_if(m_tests.begin(), m_tests.end(),
        [&testName](const VisualTest& test) {
            return test.name == testName;
        });
    
    if (it != m_tests.end()) {
        StopCurrentTest(); // Stop any running test first
        
        m_currentTest = &(*it);
        m_currentTest->isActive = true;
        
        Logger::GetInstance().Info("Starting visual test: " + testName);
        m_currentTest->setup();
    } else {
        Logger::GetInstance().Error("Visual test not found: " + testName);
    }
}

void TestRunner::RunAllTests() {
    Logger::GetInstance().Info("Running all visual tests sequentially...");
    
    for (auto& test : m_tests) {
        Logger::GetInstance().Info("Running test: " + test.name);
        RunTest(test.name);
        
        // Run for a short duration (this is just for demo)
        // In a real implementation, you'd want user control
        for (int i = 0; i < 60; ++i) { // ~1 second at 60fps
            Update(1.0f / 60.0f);
            Render();
        }
        
        StopCurrentTest();
    }
}

void TestRunner::StopCurrentTest() {
    if (m_currentTest) {
        Logger::GetInstance().Info("Stopping visual test: " + m_currentTest->name);
        m_currentTest->cleanup();
        m_currentTest->isActive = false;
        m_currentTest = nullptr;
    }
    
    ClearDebugDrawing();
}

std::vector<std::string> TestRunner::GetAvailableTests() const {
    std::vector<std::string> testNames;
    testNames.reserve(m_tests.size());
    
    for (const auto& test : m_tests) {
        testNames.push_back(test.name);
    }
    
    return testNames;
}

std::string TestRunner::GetCurrentTestName() const {
    return m_currentTest ? m_currentTest->name : "";
}

void TestRunner::Update(float deltaTime) {
    if (m_currentTest && m_currentTest->isActive) {
        m_currentTest->update(deltaTime);
    }
}

void TestRunner::Render() {
    if (m_currentTest && m_currentTest->isActive) {
        m_currentTest->render();
    }
    
    if (m_debugOverlayEnabled) {
        RenderDebugOverlay();
    }
}

void TestRunner::DrawDebugLine(const Math::Vec3& start, const Math::Vec3& end, const Math::Vec3& color) {
    if (m_debugOverlayEnabled) {
        m_debugLines.push_back({start, end, color});
    }
}

void TestRunner::DrawDebugSphere(const Math::Vec3& center, float radius, const Math::Vec3& color) {
    if (m_debugOverlayEnabled) {
        m_debugSpheres.push_back({center, color, radius});
    }
}

void TestRunner::DrawDebugBox(const Math::Vec3& center, const Math::Vec3& size, const Math::Vec3& color) {
    if (m_debugOverlayEnabled) {
        m_debugBoxes.push_back({center, size, color});
    }
}

void TestRunner::DrawDebugText(const std::string& text, const Math::Vec3& position, const Math::Vec3& color) {
    if (m_debugOverlayEnabled) {
        m_debugTexts.push_back({text, position, color});
    }
}

void TestRunner::ClearDebugDrawing() {
    m_debugLines.clear();
    m_debugSpheres.clear();
    m_debugBoxes.clear();
    m_debugTexts.clear();
}

void TestRunner::RenderDebugOverlay() {
    // This is a placeholder implementation
    // In a real engine, this would use the graphics renderer to draw debug shapes
    
    // For now, just log debug info periodically
    static int frameCount = 0;
    frameCount++;
    
    if (frameCount % 60 == 0) { // Every second at 60fps
        if (!m_debugLines.empty() || !m_debugSpheres.empty() || 
            !m_debugBoxes.empty() || !m_debugTexts.empty()) {
            
            Logger::GetInstance().Debug("Debug overlay - Lines: " + std::to_string(m_debugLines.size()) +
                         ", Spheres: " + std::to_string(m_debugSpheres.size()) +
                         ", Boxes: " + std::to_string(m_debugBoxes.size()) +
                         ", Texts: " + std::to_string(m_debugTexts.size()));
        }
    }
    
    // Clear debug drawing after rendering (single frame)
    ClearDebugDrawing();
}

} // namespace Testing
} // namespace GameEngine