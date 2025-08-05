#include "TestUtils.h"
#include "Graphics/ShaderError.h"
#include "Graphics/ShaderProfiler.h"
#include <iostream>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test ShaderCompilationError exception creation and formatting
 * Requirements: 8.1, 8.4, 10.2
 */
bool TestShaderCompilationError() {
    TestOutput::PrintTestStart("shader compilation error");

    try {
        // Test basic error creation
        ShaderCompilationError error("TestShader", "Syntax error at line 10", 10);
        
        std::string shaderName = error.GetShaderName();
        if (shaderName != "TestShader") {
            TestOutput::PrintTestFail("shader compilation error", "TestShader", shaderName);
            return false;
        }
        
        int lineNumber = error.GetLineNumber();
        if (lineNumber != 10) {
            TestOutput::PrintTestFail("shader compilation error", "10", std::to_string(lineNumber));
            return false;
        }
        
        std::string formattedError = error.GetFormattedError();
        if (formattedError.find("TestShader") == std::string::npos) {
            TestOutput::PrintTestFail("shader compilation error", "contains TestShader", "missing TestShader");
            return false;
        }
        
        if (formattedError.find("Line 10") == std::string::npos) {
            TestOutput::PrintTestFail("shader compilation error", "contains Line 10", "missing Line 10");
            return false;
        }
        
        if (formattedError.find("Syntax error") == std::string::npos) {
            TestOutput::PrintTestFail("shader compilation error", "contains Syntax error", "missing Syntax error");
            return false;
        }

        TestOutput::PrintTestPass("shader compilation error");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("shader compilation error", "no exception", e.what());
        return false;
    }
}

/**
 * Test ShaderErrorHandler error parsing functionality
 * Requirements: 8.1, 8.4, 10.2
 */
bool TestShaderErrorParsing() {
    TestOutput::PrintTestStart("shader error parsing");

    try {
        std::string errorLog = "0:10: error: 'undeclared_variable' : undeclared identifier\n"
                              "0:15: error: syntax error";
        
        auto errors = ShaderErrorHandler::ParseErrorLog(errorLog, "vertex");
        
        if (errors.size() != 2) {
            TestOutput::PrintTestFail("shader error parsing", "2 errors", std::to_string(errors.size()));
            return false;
        }
        
        if (errors.size() >= 2) {
            if (errors[0].lineNumber != 10) {
                TestOutput::PrintTestFail("shader error parsing", "line 10", std::to_string(errors[0].lineNumber));
                return false;
            }
            
            if (errors[0].message.find("undeclared identifier") == std::string::npos) {
                TestOutput::PrintTestFail("shader error parsing", "contains undeclared identifier", errors[0].message);
                return false;
            }
            
            if (errors[0].shaderType != "vertex") {
                TestOutput::PrintTestFail("shader error parsing", "vertex", errors[0].shaderType);
                return false;
            }
            
            if (errors[1].lineNumber != 15) {
                TestOutput::PrintTestFail("shader error parsing", "line 15", std::to_string(errors[1].lineNumber));
                return false;
            }
            
            if (errors[1].message.find("syntax error") == std::string::npos) {
                TestOutput::PrintTestFail("shader error parsing", "contains syntax error", errors[1].message);
                return false;
            }
        }

        TestOutput::PrintTestPass("shader error parsing");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("shader error parsing", "no exception", e.what());
        return false;
    }
}

/**
 * Test ShaderValidator source validation functionality
 * Requirements: 6.2, 6.3, 6.5
 */
bool TestShaderValidation() {
    TestOutput::PrintTestStart("shader validation");

    // Test valid shader source
    std::string validShader = R"(#version 330 core
in vec3 position;
uniform mat4 mvpMatrix;
void main() {
    gl_Position = mvpMatrix * vec4(position, 1.0);
})";
    
    auto result = ShaderValidator::ValidateShaderSource(validShader, "vertex");
    EXPECT_TRUE(result.isValid);
    
    // Test invalid shader source (missing version)
    std::string invalidShader = R"(in vec3 position;
void main() {
    gl_Position = vec4(position, 1.0);
})";
    
    auto invalidResult = ShaderValidator::ValidateShaderSource(invalidShader, "vertex");
    EXPECT_FALSE(invalidResult.isValid);
    EXPECT_TRUE(invalidResult.errors.size() > 0);

    TestOutput::PrintTestPass("shader validation");
    return true;
}

/**
 * Test ShaderProfiler performance tracking
 * Requirements: 6.2, 6.3, 6.5
 */
bool TestShaderProfiling() {
    TestOutput::PrintTestStart("shader profiling");

    auto& profiler = ShaderProfiler::GetInstance();
    profiler.StartProfiling();
    profiler.ResetStats();
    
    // Test compilation time recording
    profiler.RecordCompilationTime("TestShader", 15.5);
    profiler.RecordLinkingTime("TestShader", 5.2);
    profiler.RecordDrawCall("TestShader");
    profiler.RecordFrameTime("TestShader", 2.1);
    
    auto stats = profiler.GetShaderStats("TestShader");
    EXPECT_NEARLY_EQUAL(stats.compilationTimeMs, 15.5);
    EXPECT_NEARLY_EQUAL(stats.linkingTimeMs, 5.2);
    EXPECT_EQUAL(stats.totalDrawCalls, uint64_t(1));
    EXPECT_NEARLY_EQUAL(stats.averageFrameTimeMs, 2.1);
    
    profiler.StopProfiling();

    TestOutput::PrintTestPass("shader profiling");
    return true;
}

/**
 * Test GPUMemoryTracker memory tracking functionality
 * Requirements: 6.2, 6.3, 6.5
 */
bool TestGPUMemoryTracking() {
    TestOutput::PrintTestStart("gpu memory tracking");

    auto& tracker = GPUMemoryTracker::GetInstance();
    
    // Track some memory usage
    tracker.TrackShaderMemory("TestShader", 1024 * 1024); // 1MB
    tracker.TrackTextureMemory(1, 2 * 1024 * 1024); // 2MB
    tracker.TrackBufferMemory(1, 512 * 1024); // 512KB
    
    size_t totalMemory = tracker.GetTotalMemoryUsage();
    size_t expectedTotal = 1024 * 1024 + 2 * 1024 * 1024 + 512 * 1024;
    EXPECT_EQUAL(totalMemory, expectedTotal);
    
    size_t shaderMemory = tracker.GetShaderMemoryUsage();
    EXPECT_EQUAL(shaderMemory, size_t(1024 * 1024));
    
    // Test memory release
    tracker.ReleaseShaderMemory("TestShader");
    size_t newShaderMemory = tracker.GetShaderMemoryUsage();
    EXPECT_EQUAL(newShaderMemory, size_t(0));

    TestOutput::PrintTestPass("gpu memory tracking");
    return true;
}

/**
 * Test ShaderAnalyzer source analysis functionality
 * Requirements: 6.2, 6.3, 6.5
 */
bool TestShaderAnalysis() {
    TestOutput::PrintTestStart("shader analysis");

    std::string shaderSource = R"(#version 330 core
uniform mat4 mvpMatrix;
uniform sampler2D diffuseTexture;
in vec3 position;
in vec2 texCoord;
out vec4 fragColor;

void main() {
    vec4 texColor = texture(diffuseTexture, texCoord);
    fragColor = texColor * 2.0;
    if (fragColor.a < 0.5) {
        discard;
    }
})";
    
    auto analysis = ShaderAnalyzer::AnalyzeShaderSource(shaderSource, "fragment");
    
    EXPECT_TRUE(analysis.estimatedInstructions > 0);
    EXPECT_TRUE(analysis.textureReads > 0);
    EXPECT_TRUE(analysis.uniformsUsed > 0);
    EXPECT_TRUE(analysis.qualityScore >= 0 && analysis.qualityScore <= 100);

    TestOutput::PrintTestPass("shader analysis");
    return true;
}

/**
 * Test error suggestion system
 * Requirements: 8.1, 8.4, 10.2
 */
bool TestErrorSuggestions() {
    TestOutput::PrintTestStart("error suggestions");

    // Test various error types and their suggestions
    std::string undeclaredError = "undeclared identifier 'myVariable'";
    std::string suggestion = ShaderErrorHandler::GetErrorSuggestion(undeclaredError);
    EXPECT_TRUE(suggestion.find("typos") != std::string::npos || 
                suggestion.find("declared") != std::string::npos);
    
    std::string syntaxError = "syntax error, unexpected token";
    suggestion = ShaderErrorHandler::GetErrorSuggestion(syntaxError);
    EXPECT_TRUE(suggestion.find("semicolon") != std::string::npos || 
                suggestion.find("syntax") != std::string::npos);
    
    std::string versionError = "version directive must occur first";
    suggestion = ShaderErrorHandler::GetErrorSuggestion(versionError);
    EXPECT_TRUE(suggestion.find("version") != std::string::npos);

    TestOutput::PrintTestPass("error suggestions");
    return true;
}

int main() {
    TestOutput::PrintHeader("ShaderErrorHandling");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ShaderErrorHandling Tests");

        // Run all tests
        allPassed &= suite.RunTest("Shader Compilation Error", TestShaderCompilationError);
        allPassed &= suite.RunTest("Shader Error Parsing", TestShaderErrorParsing);
        allPassed &= suite.RunTest("Shader Validation", TestShaderValidation);
        allPassed &= suite.RunTest("Shader Profiling", TestShaderProfiling);
        allPassed &= suite.RunTest("GPU Memory Tracking", TestGPUMemoryTracking);
        allPassed &= suite.RunTest("Shader Analysis", TestShaderAnalysis);
        allPassed &= suite.RunTest("Error Suggestions", TestErrorSuggestions);

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