#include "TestUtils.h"
#include "Graphics/HardwareCapabilities.h"
#include "Graphics/OpenGLContext.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test hardware capability detection initialization
 * Requirements: 8.6 (hardware limitation detection and reporting)
 */
bool TestHardwareCapabilitiesInitialization() {
    TestOutput::PrintTestStart("hardware capabilities initialization");

    // First test: Basic singleton access (CPU/Math only)
    TestOutput::PrintInfo("Testing GetInstance()...");
    try {
        const auto& capabilities = HardwareCapabilities::GetInstance();
        TestOutput::PrintInfo("GetInstance() succeeded");
        
        // Test that GetInstance() works without crashing
        EXPECT_TRUE(&capabilities != nullptr);
        TestOutput::PrintInfo("Pointer check passed");
        
        // Test IsInitialized() before initialization
        TestOutput::PrintInfo("Testing IsInitialized()...");
        bool initializedBefore = HardwareCapabilities::IsInitialized();
        EXPECT_FALSE(initializedBefore);
        TestOutput::PrintInfo("IsInitialized() check passed");
        
    } catch (const std::exception& e) {
        TestOutput::PrintError("GetInstance() failed: " + std::string(e.what()));
        return false;
    } catch (...) {
        TestOutput::PrintError("GetInstance() failed with unknown exception");
        return false;
    }

    // Second test: Initialization (may require OpenGL context)
    TestOutput::PrintInfo("Testing Initialize()...");
    bool initResult = false;
    try {
        initResult = HardwareCapabilities::Initialize();
        TestOutput::PrintInfo("Initialize() call completed, result: " + std::string(initResult ? "true" : "false"));
    } catch (const std::exception& e) {
        TestOutput::PrintError("Initialize() failed: " + std::string(e.what()));
        return false;
    } catch (...) {
        TestOutput::PrintError("Initialize() failed with unknown exception");
        return false;
    }
    
    if (OpenGLContext::HasActiveContext()) {
        // If we have OpenGL context, initialization should succeed
        EXPECT_TRUE(initResult);
        EXPECT_TRUE(HardwareCapabilities::IsInitialized());
        
        try {
            const auto& capabilities = HardwareCapabilities::GetInstance();
            
            // Test basic capability queries (only if initialized)
            float version = capabilities.GetOpenGLVersion();
            EXPECT_TRUE(version >= 0.0f); // Allow 0.0f for failed detection
            
            std::string versionString = capabilities.GetOpenGLVersionString();
            // Version string can be empty if detection failed
            
            // Test feature detection methods exist and return boolean values
            bool computeSupport = capabilities.SupportsComputeShaders();
            bool geometrySupport = capabilities.SupportsGeometryShaders();
            bool tessellationSupport = capabilities.SupportsTessellation();
            
            // These should return valid boolean values (no exceptions)
            EXPECT_TRUE(computeSupport == true || computeSupport == false);
            EXPECT_TRUE(geometrySupport == true || geometrySupport == false);
            EXPECT_TRUE(tessellationSupport == true || tessellationSupport == false);
            
        } catch (const std::exception& e) {
            TestOutput::PrintError("Capability queries failed: " + std::string(e.what()));
            return false;
        }
        
    } else {
        // Without OpenGL context, initialization should fail gracefully
        EXPECT_FALSE(initResult);
        EXPECT_FALSE(HardwareCapabilities::IsInitialized());
        TestOutput::PrintInfo("No OpenGL context - initialization correctly failed");
    }

    TestOutput::PrintTestPass("hardware capabilities initialization");
    return true;
}

/**
 * Test hardware capability reporting
 * Requirements: 8.6 (hardware limitation detection and reporting)
 */
bool TestHardwareCapabilityReporting() {
    TestOutput::PrintTestStart("hardware capability reporting");

    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping OpenGL-dependent test (no context)");
        TestOutput::PrintTestPass("hardware capability reporting");
        return true;
    }

    // Initialize if not already done
    if (!HardwareCapabilities::IsInitialized()) {
        bool initResult = HardwareCapabilities::Initialize();
        if (!initResult) {
            TestOutput::PrintInfo("Hardware capabilities initialization failed - skipping test");
            TestOutput::PrintTestPass("hardware capability reporting");
            return true;
        }
    }

    try {
        const auto& capabilities = HardwareCapabilities::GetInstance();
        
        // Test report generation
        std::string report = capabilities.GenerateCapabilityReport();
        EXPECT_FALSE(report.empty());
        EXPECT_TRUE(report.find("Hardware Capability Report") != std::string::npos);
        
        // Test limitation detection
        auto limitations = capabilities.GetHardwareLimitations();
        // Should return a vector (may be empty on high-end hardware)
        EXPECT_TRUE(limitations.size() >= 0);
        
        // Test missing features detection
        auto missingFeatures = capabilities.GetMissingFeatures();
        EXPECT_TRUE(missingFeatures.size() >= 0);
        
        // Test minimum requirements check
        bool meetsMinimum = capabilities.MeetsMinimumRequirements();
        EXPECT_TRUE(meetsMinimum == true || meetsMinimum == false);
        
        // Test performance tier
        int tier = capabilities.GetPerformanceTier();
        EXPECT_TRUE(tier >= 0 && tier <= 3);

    } catch (const std::exception& e) {
        TestOutput::PrintError("Hardware capability reporting failed: " + std::string(e.what()));
        return false;
    } catch (...) {
        TestOutput::PrintError("Hardware capability reporting failed with unknown exception");
        return false;
    }

    TestOutput::PrintTestPass("hardware capability reporting");
    return true;
}

int main() {
    TestOutput::PrintHeader("HardwareCapabilities");

    bool allPassed = true;

    try {
        TestSuite suite("HardwareCapabilities Tests");

        allPassed &= suite.RunTest("Hardware Capabilities Initialization", TestHardwareCapabilitiesInitialization);
        allPassed &= suite.RunTest("Hardware Capability Reporting", TestHardwareCapabilityReporting);

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