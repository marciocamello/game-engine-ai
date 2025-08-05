#include "TestUtils.h"
#include "Graphics/ShaderCompiler.h"
#include "Graphics/Shader.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test ShaderCompiler initialization and basic functionality
 * Requirements: 6.1, 6.4, 6.6 (GLSL compilation with error handling and reporting)
 */
bool TestShaderCompilerInitialization() {
    TestOutput::PrintTestStart("shader compiler initialization");

    ShaderCompiler compiler;
    EXPECT_TRUE(compiler.Initialize());
    
    // Test configuration
    ShaderOptimizationSettings optSettings = compiler.GetOptimizationSettings();
    EXPECT_TRUE(optSettings.enableOptimization);
    EXPECT_TRUE(optSettings.removeUnusedVariables);
    EXPECT_TRUE(optSettings.stripComments);
    
    ShaderValidationSettings valSettings = compiler.GetValidationSettings();
    EXPECT_TRUE(valSettings.enableValidation);
    EXPECT_TRUE(valSettings.checkSyntax);
    EXPECT_TRUE(valSettings.checkSemantics);
    
    compiler.Shutdown();

    TestOutput::PrintTestPass("shader compiler initialization");
    return true;
}

/**
 * Test shader source optimization functionality
 * Requirements: 6.1, 6.4 (shader optimization and validation features)
 */
bool TestShaderSourceOptimization() {
    TestOutput::PrintTestStart("shader source optimization");

    ShaderCompiler compiler;
    EXPECT_TRUE(compiler.Initialize());
    
    // Test comment removal
    std::string sourceWithComments = R"(
        #version 330 core
        // This is a comment
        layout(location = 0) in vec3 position;
        /* Multi-line
           comment */
        void main() {
            gl_Position = vec4(position, 1.0);
        }
    )";
    
    std::string optimized = compiler.OptimizeShaderSource(sourceWithComments, Shader::Type::Vertex);
    
    // Should not contain comments after optimization
    EXPECT_TRUE(optimized.find("// This is a comment") == std::string::npos);
    EXPECT_TRUE(optimized.find("/* Multi-line") == std::string::npos);
    
    // Should still contain essential code
    EXPECT_TRUE(optimized.find("#version 330 core") != std::string::npos);
    EXPECT_TRUE(optimized.find("gl_Position") != std::string::npos);
    
    compiler.Shutdown();

    TestOutput::PrintTestPass("shader source optimization");
    return true;
}

/**
 * Test shader validation functionality
 * Requirements: 6.4, 6.6 (shader validation and analysis)
 */
bool TestShaderValidation() {
    TestOutput::PrintTestStart("shader validation");

    ShaderCompiler compiler;
    EXPECT_TRUE(compiler.Initialize());
    
    // Test valid vertex shader
    std::string validVertexShader = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        void main() {
            gl_Position = vec4(position, 1.0);
        }
    )";
    
    std::vector<std::string> warnings;
    EXPECT_TRUE(compiler.ValidateShaderSource(validVertexShader, Shader::Type::Vertex, warnings));
    
    // Test invalid shader (missing main function)
    std::string invalidShader = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        // Missing main function
    )";
    
    warnings.clear();
    EXPECT_FALSE(compiler.ValidateShaderSource(invalidShader, Shader::Type::Vertex, warnings));
    EXPECT_TRUE(warnings.size() > 0);
    
    compiler.Shutdown();

    TestOutput::PrintTestPass("shader validation");
    return true;
}

/**
 * Test shader compilation statistics
 * Requirements: 6.6 (compilation performance monitoring and statistics)
 */
bool TestCompilationStatistics() {
    TestOutput::PrintTestStart("compilation statistics");

    ShaderCompiler compiler;
    EXPECT_TRUE(compiler.Initialize());
    
    // Get initial stats
    ShaderCompilationStats initialStats = compiler.GetCompilationStats();
    EXPECT_EQUAL(initialStats.totalCompilations, 0);
    EXPECT_EQUAL(initialStats.successfulCompilations, 0);
    EXPECT_EQUAL(initialStats.failedCompilations, 0);
    
    // Reset stats should work
    compiler.ResetStats();
    ShaderCompilationStats resetStats = compiler.GetCompilationStats();
    EXPECT_EQUAL(resetStats.totalCompilations, 0);
    
    compiler.Shutdown();

    TestOutput::PrintTestPass("compilation statistics");
    return true;
}

/**
 * Test global defines functionality
 * Requirements: 6.1 (GLSL compilation with preprocessor support)
 */
bool TestGlobalDefines() {
    TestOutput::PrintTestStart("global defines");

    ShaderCompiler compiler;
    EXPECT_TRUE(compiler.Initialize());
    
    // Add global defines
    compiler.AddGlobalDefine("TEST_DEFINE", "1");
    compiler.AddGlobalDefine("MAX_LIGHTS", "8");
    
    // Test preprocessing with defines
    std::string source = R"(
        #version 330 core
        #ifdef TEST_DEFINE
        uniform int testValue;
        #endif
        void main() {}
    )";
    
    std::string processed = compiler.PreprocessShader(source);
    
    // Should contain the define
    EXPECT_TRUE(processed.find("#define TEST_DEFINE 1") != std::string::npos);
    EXPECT_TRUE(processed.find("#define MAX_LIGHTS 8") != std::string::npos);
    
    // Remove defines
    compiler.RemoveGlobalDefine("TEST_DEFINE");
    compiler.ClearGlobalDefines();
    
    compiler.Shutdown();

    TestOutput::PrintTestPass("global defines");
    return true;
}

int main() {
    TestOutput::PrintHeader("ShaderCompiler");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ShaderCompiler Tests");

        // Run all tests
        allPassed &= suite.RunTest("Shader Compiler Initialization", TestShaderCompilerInitialization);
        allPassed &= suite.RunTest("Shader Source Optimization", TestShaderSourceOptimization);
        allPassed &= suite.RunTest("Shader Validation", TestShaderValidation);
        allPassed &= suite.RunTest("Compilation Statistics", TestCompilationStatistics);
        allPassed &= suite.RunTest("Global Defines", TestGlobalDefines);

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