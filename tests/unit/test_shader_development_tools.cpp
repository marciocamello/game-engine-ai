#include "TestUtils.h"
#include "Graphics/ShaderIntrospection.h"
#include "Graphics/ShaderDiagnostics.h"
#include <iostream>
#include <glad/glad.h>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test shader introspection basic functionality
 * Requirements: 10.1, 10.3, 10.4
 */
bool TestShaderIntrospectionBasic() {
    TestOutput::PrintTestStart("shader introspection basic");

    // Test with invalid shader (should handle gracefully)
    auto invalidData = ShaderIntrospection::IntrospectShaderProgram(0, "InvalidShader");
    EXPECT_FALSE(invalidData.isValid);
    EXPECT_TRUE(invalidData.errors.size() > 0);
    EXPECT_EQUAL(invalidData.shaderName, "InvalidShader");

    // Test uniform type name conversion
    std::string floatType = ShaderIntrospection::GetUniformTypeName(GL_FLOAT);
    EXPECT_EQUAL(floatType, "float");
    
    std::string vec3Type = ShaderIntrospection::GetUniformTypeName(GL_FLOAT_VEC3);
    EXPECT_EQUAL(vec3Type, "vec3");
    
    std::string mat4Type = ShaderIntrospection::GetUniformTypeName(GL_FLOAT_MAT4);
    EXPECT_EQUAL(mat4Type, "mat4");
    
    std::string samplerType = ShaderIntrospection::GetUniformTypeName(GL_SAMPLER_2D);
    EXPECT_EQUAL(samplerType, "sampler2D");

    TestOutput::PrintTestPass("shader introspection basic");
    return true;
}

/**
 * Test shader diagnostics system
 * Requirements: 10.2, 10.7, 8.5
 */
bool TestShaderDiagnostics() {
    TestOutput::PrintTestStart("shader diagnostics");

    const std::string testShaderName = "DiagnosticTestShader";

    // Clear any existing diagnostics
    ShaderDiagnostics::ClearDiagnostics();

    // Test basic logging
    ShaderDiagnostics::LogInfo(testShaderName, "Test info message");
    ShaderDiagnostics::LogWarning(testShaderName, "Test warning message", "Test suggestion");
    ShaderDiagnostics::LogError(testShaderName, "Test error message");

    // Get diagnostics
    auto allDiagnostics = ShaderDiagnostics::GetDiagnostics();
    EXPECT_TRUE(allDiagnostics.size() >= 3);

    auto shaderDiagnostics = ShaderDiagnostics::GetShaderDiagnostics(testShaderName);
    EXPECT_TRUE(shaderDiagnostics.size() >= 3);

    // Test shader registration
    ShaderDiagnostics::RegisterShader(testShaderName, 12345);
    auto trackedShaders = ShaderDiagnostics::GetTrackedShaders();
    EXPECT_TRUE(std::find(trackedShaders.begin(), trackedShaders.end(), testShaderName) != trackedShaders.end());

    // Test compilation logging
    ShaderDiagnostics::LogCompilation(testShaderName, true, 15.5, "");
    ShaderDiagnostics::LogLinking(testShaderName, true, 8.2, "");

    // Test performance logging
    ShaderDiagnostics::LogPerformance(testShaderName, "frame_time", 16.67, "ms");

    // Test diagnostic report generation
    std::string report = ShaderDiagnostics::GenerateDiagnosticReport();
    EXPECT_TRUE(report.length() > 0);
    EXPECT_TRUE(report.find(testShaderName) != std::string::npos);

    // Test shader report generation
    std::string shaderReport = ShaderDiagnostics::GenerateShaderReport(testShaderName);
    EXPECT_TRUE(shaderReport.length() > 0);
    EXPECT_TRUE(shaderReport.find(testShaderName) != std::string::npos);

    // Clean up
    ShaderDiagnostics::UnregisterShader(testShaderName);

    TestOutput::PrintTestPass("shader diagnostics");
    return true;
}

/**
 * Test error suggestion system
 * Requirements: 10.2, 10.7, 8.5
 */
bool TestErrorSuggestions() {
    TestOutput::PrintTestStart("error suggestions");

    // Test various error types
    std::string undeclaredError = "error: 'myVariable' : undeclared identifier";
    std::string suggestion = ShaderDiagnostics::GetErrorSuggestion(undeclaredError);
    EXPECT_TRUE(suggestion.find("declared") != std::string::npos || 
                suggestion.find("typos") != std::string::npos);

    std::string syntaxError = "error: syntax error, unexpected token";
    suggestion = ShaderDiagnostics::GetErrorSuggestion(syntaxError);
    EXPECT_TRUE(suggestion.find("semicolon") != std::string::npos || 
                suggestion.find("syntax") != std::string::npos);

    std::string versionError = "error: version directive must occur first";
    suggestion = ShaderDiagnostics::GetErrorSuggestion(versionError);
    EXPECT_TRUE(suggestion.find("version") != std::string::npos);

    std::string linkingError = "error: linking failed";
    suggestion = ShaderDiagnostics::GetErrorSuggestion(linkingError);
    EXPECT_TRUE(suggestion.find("interface") != std::string::npos || 
                suggestion.find("match") != std::string::npos);

    TestOutput::PrintTestPass("error suggestions");
    return true;
}

/**
 * Test performance suggestions
 * Requirements: 10.2, 10.7, 8.5
 */
bool TestPerformanceSuggestions() {
    TestOutput::PrintTestStart("performance suggestions");

    const std::string testShaderName = "PerfTestShader";

    // Test frame time suggestion
    std::string suggestion = ShaderDiagnostics::GetPerformanceSuggestion(testShaderName, "frame_time", 25.0);
    EXPECT_TRUE(suggestion.find("60 FPS") != std::string::npos || 
                suggestion.find("optimization") != std::string::npos);

    // Test compile time suggestion
    suggestion = ShaderDiagnostics::GetPerformanceSuggestion(testShaderName, "compile_time", 1500.0);
    EXPECT_TRUE(suggestion.find("compilation") != std::string::npos || 
                suggestion.find("complexity") != std::string::npos);

    // Test uniform updates suggestion
    suggestion = ShaderDiagnostics::GetPerformanceSuggestion(testShaderName, "uniform_updates", 150.0);
    EXPECT_TRUE(suggestion.find("Uniform Buffer") != std::string::npos || 
                suggestion.find("UBO") != std::string::npos);

    // Test texture bindings suggestion
    suggestion = ShaderDiagnostics::GetPerformanceSuggestion(testShaderName, "texture_bindings", 20.0);
    EXPECT_TRUE(suggestion.find("atlas") != std::string::npos || 
                suggestion.find("array") != std::string::npos);

    // Test memory usage suggestion
    suggestion = ShaderDiagnostics::GetPerformanceSuggestion(testShaderName, "memory_usage", 2048 * 1024.0);
    EXPECT_TRUE(suggestion.find("memory") != std::string::npos || 
                suggestion.find("optimize") != std::string::npos);

    TestOutput::PrintTestPass("performance suggestions");
    return true;
}

/**
 * Test diagnostic configuration
 * Requirements: 10.2, 10.7, 8.5
 */
bool TestDiagnosticConfiguration() {
    TestOutput::PrintTestStart("diagnostic configuration");

    // Test verbose logging
    ShaderDiagnostics::EnableVerboseLogging(true);
    ShaderDiagnostics::EnableVerboseLogging(false);

    // Test callback setting
    bool callbackCalled = false;
    ShaderDiagnostics::SetDiagnosticCallback([&callbackCalled](const DiagnosticInfo& info) {
        callbackCalled = true;
    });

    // Reset minimum severity to ensure callback is triggered
    ShaderDiagnostics::SetMinimumSeverity(DiagnosticSeverity::Info);

    // Log something to trigger callback
    ShaderDiagnostics::LogInfo("TestShader", "Test message for callback");
    
    // The callback should have been called
    EXPECT_TRUE(callbackCalled);

    // Test minimum severity setting (after callback test)
    ShaderDiagnostics::SetMinimumSeverity(DiagnosticSeverity::Warning);

    TestOutput::PrintTestPass("diagnostic configuration");
    return true;
}

int main() {
    TestOutput::PrintHeader("Shader Development Tools");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Shader Development Tools Tests");

        // Run all tests
        allPassed &= suite.RunTest("Shader Introspection Basic", TestShaderIntrospectionBasic);
        allPassed &= suite.RunTest("Shader Diagnostics", TestShaderDiagnostics);
        allPassed &= suite.RunTest("Error Suggestions", TestErrorSuggestions);
        allPassed &= suite.RunTest("Performance Suggestions", TestPerformanceSuggestions);
        allPassed &= suite.RunTest("Diagnostic Configuration", TestDiagnosticConfiguration);

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