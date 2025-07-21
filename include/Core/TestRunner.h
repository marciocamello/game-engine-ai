#pragma once

#include "Core/Math.h"
#include <string>
#include <vector>
#include <functional>

namespace GameEngine {
namespace Testing {

/**
 * @brief Internal test runner for runtime visual tests
 * 
 * This system is embedded in the engine for tests that require:
 * - Visual inspection (rendering, animations, physics)
 * - Runtime behavior (AI, multiplayer, editor tools)
 * - Interactive testing (input, gameplay mechanics)
 */
class TestRunner {
public:
    struct VisualTest {
        std::string name;
        std::string description;
        std::function<void()> setup;
        std::function<void(float deltaTime)> update;
        std::function<void()> render;
        std::function<void()> cleanup;
        bool isActive = false;
    };

    static TestRunner& GetInstance();
    
    // Test registration
    void RegisterVisualTest(const VisualTest& test);
    void RegisterRenderTest(const std::string& name, std::function<void()> renderFunc);
    void RegisterPhysicsTest(const std::string& name, std::function<void(float)> updateFunc);
    
    // Test execution
    void RunTest(const std::string& testName);
    void RunAllTests();
    void StopCurrentTest();
    
    // Debug overlay system
    void EnableDebugOverlay(bool enable) { m_debugOverlayEnabled = enable; }
    void DrawDebugLine(const Math::Vec3& start, const Math::Vec3& end, const Math::Vec3& color = Math::Vec3(1.0f));
    void DrawDebugSphere(const Math::Vec3& center, float radius, const Math::Vec3& color = Math::Vec3(1.0f));
    void DrawDebugBox(const Math::Vec3& center, const Math::Vec3& size, const Math::Vec3& color = Math::Vec3(1.0f));
    void DrawDebugText(const std::string& text, const Math::Vec3& position, const Math::Vec3& color = Math::Vec3(1.0f));
    
    // Test management
    std::vector<std::string> GetAvailableTests() const;
    bool IsTestRunning() const { return m_currentTest != nullptr; }
    std::string GetCurrentTestName() const;
    
    // Update and render (called by engine)
    void Update(float deltaTime);
    void Render();

private:
    TestRunner() = default;
    
    std::vector<VisualTest> m_tests;
    VisualTest* m_currentTest = nullptr;
    bool m_debugOverlayEnabled = true;
    
    // Debug drawing data
    struct DebugLine {
        Math::Vec3 start, end, color;
    };
    struct DebugSphere {
        Math::Vec3 center, color;
        float radius;
    };
    struct DebugBox {
        Math::Vec3 center, size, color;
    };
    struct DebugText {
        std::string text;
        Math::Vec3 position, color;
    };
    
    std::vector<DebugLine> m_debugLines;
    std::vector<DebugSphere> m_debugSpheres;
    std::vector<DebugBox> m_debugBoxes;
    std::vector<DebugText> m_debugTexts;
    
    void ClearDebugDrawing();
    void RenderDebugOverlay();
};

// Convenience macros for registering tests
#define REGISTER_VISUAL_TEST(name, desc) \
    static bool registered_##name = []() { \
        GameEngine::Testing::TestRunner::GetInstance().RegisterVisualTest({ \
            #name, desc, \
            []() { /* setup */ }, \
            [](float dt) { /* update */ }, \
            []() { /* render */ }, \
            []() { /* cleanup */ } \
        }); \
        return true; \
    }();

} // namespace Testing
} // namespace GameEngine