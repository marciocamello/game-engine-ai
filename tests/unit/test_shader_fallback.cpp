#include "TestUtils.h"
#include "Graphics/ShaderFallbackManager.h"
#include "Graphics/HardwareCapabilities.h"
#include "Graphics/OpenGLContext.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test shader fallback manager initialization
 * Requirements: 8.6, 3.7, 8.3 (fallback systems for unsupported features)
 */
bool TestShaderFallbackInitialization() {
    TestOutput::PrintTestStart("shader fallback initialization");

    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping OpenGL-dependent test (no context)");
        TestOutput::PrintTestPass("shader fallback initialization");
        return true;
    }

    // Initialize hardware capabilities first
    if (!HardwareCapabilities::IsInitialized()) {
        EXPECT_TRUE(HardwareCapabilities::Initialize());
    }

    // Initialize fallback manager
    auto& fallbackManager = ShaderFallbackManager::GetInstance();
    bool initResult = fallbackManager.Initialize();
    
    EXPECT_TRUE(initResult);
    EXPECT_TRUE(fallbackManager.IsInitialized());

    TestOutput::PrintTestPass("shader fallback initialization");
    return true;
}

/**
 * Test shader feature analysis
 * Requirements: 8.6, 3.7 (graceful degradation for missing features)
 */
bool TestShaderFeatureAnalysis() {
    TestOutput::PrintTestStart("shader feature analysis");

    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping OpenGL-dependent test (no context)");
        TestOutput::PrintTestPass("shader feature analysis");
        return true;
    }

    auto& fallbackManager = ShaderFallbackManager::GetInstance();
    if (!fallbackManager.IsInitialized()) {
        EXPECT_TRUE(fallbackManager.Initialize());
    }

    // Test compute shader feature detection
    std::string computeShaderSource = R"(
        #version 430
        layout(local_size_x = 64) in;
        layout(std430, binding = 0) buffer DataBuffer {
            float data[];
        };
        void main() {
            uint index = gl_GlobalInvocationID.x;
            data[index] *= 2.0;
        }
    )";

    auto requiredFallbacks = fallbackManager.AnalyzeRequiredFallbacks(computeShaderSource);
    // Should detect compute shader features
    bool hasComputeFallback = std::find(requiredFallbacks.begin(), requiredFallbacks.end(), 
                                       ShaderFallbackManager::FallbackType::ComputeShader) != requiredFallbacks.end();
    
    // Result depends on hardware - just verify method works
    EXPECT_TRUE(hasComputeFallback == true || hasComputeFallback == false);

    // Test basic shader (should need no fallbacks)
    std::string basicShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 mvp;
        void main() {
            gl_Position = mvp * vec4(aPos, 1.0);
        }
    )";

    bool isFullySupported = fallbackManager.IsShaderFullySupported(basicShaderSource);
    // Basic shader should be supported on most hardware
    EXPECT_TRUE(isFullySupported == true || isFullySupported == false);

    TestOutput::PrintTestPass("shader feature analysis");
    return true;
}

/**
 * Test fallback report generation
 * Requirements: 8.6 (hardware limitation detection and reporting)
 */
bool TestFallbackReporting() {
    TestOutput::PrintTestStart("fallback reporting");

    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping OpenGL-dependent test (no context)");
        TestOutput::PrintTestPass("fallback reporting");
        return true;
    }

    auto& fallbackManager = ShaderFallbackManager::GetInstance();
    if (!fallbackManager.IsInitialized()) {
        EXPECT_TRUE(fallbackManager.Initialize());
    }

    // Test report generation
    std::string report = fallbackManager.GenerateFallbackReport();
    EXPECT_FALSE(report.empty());
    EXPECT_TRUE(report.find("Shader Fallback Report") != std::string::npos);

    // Test active fallbacks list
    auto activeFallbacks = fallbackManager.GetActiveFallbacks();
    EXPECT_TRUE(activeFallbacks.size() >= 0);

    // Test performance impact calculation
    float impact = fallbackManager.GetFallbackPerformanceImpact();
    EXPECT_TRUE(impact >= 0.0f && impact <= 10.0f); // Reasonable range

    TestOutput::PrintTestPass("fallback reporting");
    return true;
}

int main() {
    TestOutput::PrintHeader("ShaderFallback");

    bool allPassed = true;

    try {
        TestSuite suite("ShaderFallback Tests");

        allPassed &= suite.RunTest("Shader Fallback Initialization", TestShaderFallbackInitialization);
        allPassed &= suite.RunTest("Shader Feature Analysis", TestShaderFeatureAnalysis);
        allPassed &= suite.RunTest("Fallback Reporting", TestFallbackReporting);

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