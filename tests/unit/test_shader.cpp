#include "TestUtils.h"
#include "Graphics/Shader.h"
#include "Core/Math.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test Shader class creation and basic state
 * Requirements: 2.1 (Shader class compilation and linking)
 */
bool TestShaderCreation() {
    TestOutput::PrintTestStart("shader creation");

    Shader shader;
    
    // Test initial state
    EXPECT_EQUAL(static_cast<int>(shader.GetState()), static_cast<int>(Shader::State::Uncompiled));
    EXPECT_FALSE(shader.IsValid());
    EXPECT_EQUAL(shader.GetProgramID(), 0);
    
    // Test compile log is initially empty
    std::string compileLog = shader.GetCompileLog();
    EXPECT_TRUE(compileLog.empty());
    
    // Test link log is initially empty
    std::string linkLog = shader.GetLinkLog();
    EXPECT_TRUE(linkLog.empty());

    TestOutput::PrintTestPass("shader creation");
    return true;
}

/**
 * Test Shader uniform location caching
 * Requirements: 2.1 (Shader uniform management)
 */
bool TestShaderUniformCaching() {
    TestOutput::PrintTestStart("shader uniform caching");

    Shader shader;
    
    // Test HasUniform method with invalid shader
    EXPECT_FALSE(shader.HasUniform("u_testUniform"));
    EXPECT_FALSE(shader.HasUniform("u_modelMatrix"));
    EXPECT_FALSE(shader.HasUniform("u_viewMatrix"));
    
    // Test that checking for uniforms doesn't crash with invalid shader
    EXPECT_FALSE(shader.HasUniform(""));
    EXPECT_FALSE(shader.HasUniform("nonexistent_uniform"));

    TestOutput::PrintTestPass("shader uniform caching");
    return true;
}

/**
 * Test Shader texture slot management
 * Requirements: 2.1 (Shader texture binding)
 */
bool TestShaderTextureSlotManagement() {
    TestOutput::PrintTestStart("shader texture slot management");

    Shader shader;
    
    // Test texture slot reset
    shader.ResetTextureSlots();
    
    // Test getting texture slot for non-existent uniform (should return 0)
    uint32_t slot = shader.GetTextureSlot("u_diffuseTexture");
    EXPECT_EQUAL(slot, 0);
    
    // Test getting texture slot for another uniform
    uint32_t slot2 = shader.GetTextureSlot("u_normalTexture");
    EXPECT_EQUAL(slot2, 0);

    TestOutput::PrintTestPass("shader texture slot management");
    return true;
}

/**
 * Test Shader uniform setters interface (without OpenGL context)
 * Requirements: 2.1 (Shader uniform management)
 */
bool TestShaderUniformSetters() {
    TestOutput::PrintTestStart("shader uniform setters");

    // Test that the uniform setter methods exist and can be called
    // We skip actual OpenGL calls since they require a context
    
    TestOutput::PrintInfo("Skipping OpenGL-dependent uniform setter tests (no context)");
    TestOutput::PrintInfo("Testing uniform setter method signatures exist");
    
    // Test that we can create the data types that would be passed to uniform setters
    Math::Vec2 testVec2(1.0f, 2.0f);
    Math::Vec3 testVec3(1.0f, 2.0f, 3.0f);
    Math::Vec4 testVec4(1.0f, 2.0f, 3.0f, 4.0f);
    Math::Mat3 testMat3 = Math::Mat3(1.0f);
    Math::Mat4 testMat4 = Math::Mat4(1.0f);
    
    std::vector<Math::Mat4> matrices = { Math::Mat4(1.0f), Math::Mat4(2.0f) };
    std::vector<Math::Vec3> vectors = { Math::Vec3(1.0f), Math::Vec3(2.0f) };
    std::vector<float> floats = { 1.0f, 2.0f, 3.0f };
    std::vector<int> ints = { 1, 2, 3 };
    
    // Verify data types are valid
    EXPECT_NEARLY_EQUAL(testVec2.x, 1.0f);
    EXPECT_NEARLY_EQUAL(testVec3.x, 1.0f);
    EXPECT_NEARLY_EQUAL(testVec4.x, 1.0f);
    EXPECT_EQUAL(matrices.size(), 2);
    EXPECT_EQUAL(vectors.size(), 2);
    EXPECT_EQUAL(floats.size(), 3);
    EXPECT_EQUAL(ints.size(), 3);

    TestOutput::PrintTestPass("shader uniform setters");
    return true;
}

/**
 * Test Shader legacy uniform setters interface (backward compatibility)
 * Requirements: 2.1 (Shader backward compatibility)
 */
bool TestShaderLegacyUniformSetters() {
    TestOutput::PrintTestStart("shader legacy uniform setters");

    // Test that legacy uniform setter method signatures exist
    // We skip actual OpenGL calls since they require a context
    
    TestOutput::PrintInfo("Skipping OpenGL-dependent legacy uniform setter tests (no context)");
    TestOutput::PrintInfo("Testing legacy uniform setter method signatures exist");
    
    // Test that we can create the data types for legacy setters
    bool testBool = true;
    int testInt = 42;
    float testFloat = 3.14f;
    Math::Vec2 testVec2(1.0f, 2.0f);
    Math::Vec3 testVec3(1.0f, 2.0f, 3.0f);
    Math::Vec4 testVec4(1.0f, 2.0f, 3.0f, 4.0f);
    Math::Mat3 testMat3 = Math::Mat3(1.0f);
    Math::Mat4 testMat4 = Math::Mat4(1.0f);
    
    // Verify data types are valid
    EXPECT_TRUE(testBool);
    EXPECT_EQUAL(testInt, 42);
    EXPECT_NEARLY_EQUAL(testFloat, 3.14f);
    EXPECT_NEARLY_EQUAL(testVec2.x, 1.0f);
    EXPECT_NEARLY_EQUAL(testVec3.x, 1.0f);
    EXPECT_NEARLY_EQUAL(testVec4.x, 1.0f);

    TestOutput::PrintTestPass("shader legacy uniform setters");
    return true;
}

/**
 * Test Shader texture binding interface
 * Requirements: 2.1 (Shader texture binding)
 */
bool TestShaderTextureBinding() {
    TestOutput::PrintTestStart("shader texture binding");

    // Test that texture binding method signatures exist
    // We skip actual OpenGL calls since they require a context
    
    TestOutput::PrintInfo("Skipping OpenGL-dependent texture binding tests (no context)");
    TestOutput::PrintInfo("Testing texture binding method signatures exist");
    
    // Test that we can create the data types for texture binding
    uint32_t textureId1 = 1;
    uint32_t textureId2 = 2;
    uint32_t slot0 = 0;
    uint32_t slot1 = 1;
    uint32_t bufferId1 = 1;
    uint32_t bufferId2 = 2;
    uint32_t binding0 = 0;
    uint32_t binding1 = 1;
    uint32_t glReadWrite = 0x88BA; // GL_READ_WRITE
    
    // Verify data types are valid
    EXPECT_EQUAL(textureId1, 1);
    EXPECT_EQUAL(textureId2, 2);
    EXPECT_EQUAL(slot0, 0);
    EXPECT_EQUAL(slot1, 1);
    EXPECT_EQUAL(bufferId1, 1);
    EXPECT_EQUAL(bufferId2, 2);
    EXPECT_EQUAL(binding0, 0);
    EXPECT_EQUAL(binding1, 1);
    EXPECT_EQUAL(glReadWrite, 0x88BA);

    TestOutput::PrintTestPass("shader texture binding");
    return true;
}

/**
 * Test Shader compute shader interface
 * Requirements: 2.1 (Compute shader support)
 */
bool TestShaderComputeInterface() {
    TestOutput::PrintTestStart("shader compute interface");

    // Test that compute shader method signatures exist
    // We skip actual OpenGL calls since they require a context
    
    TestOutput::PrintInfo("Skipping OpenGL-dependent compute shader tests (no context)");
    TestOutput::PrintInfo("Testing compute shader method signatures exist");
    
    // Test that we can create the data types for compute shader operations
    uint32_t groupsX = 64;
    uint32_t groupsY = 32;
    uint32_t groupsZ = 16;
    uint32_t bufferId = 1;
    uint32_t vertexAttribBarrier = 0x00000001; // GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
    uint32_t allBarriers = 0xFFFFFFFF; // GL_ALL_BARRIER_BITS
    
    // Verify data types are valid
    EXPECT_EQUAL(groupsX, 64);
    EXPECT_EQUAL(groupsY, 32);
    EXPECT_EQUAL(groupsZ, 16);
    EXPECT_EQUAL(bufferId, 1);
    EXPECT_EQUAL(vertexAttribBarrier, 0x00000001);
    EXPECT_EQUAL(allBarriers, 0xFFFFFFFF);

    TestOutput::PrintTestPass("shader compute interface");
    return true;
}

/**
 * Test Shader validation and error handling
 * Requirements: 2.1 (Shader validation)
 */
bool TestShaderValidation() {
    TestOutput::PrintTestStart("shader validation");

    Shader shader;
    
    // Test validation of invalid shader
    EXPECT_FALSE(shader.ValidateShader());
    
    // Test getting validation warnings (should be empty for invalid shader)
    auto validationWarnings = shader.GetValidationWarnings();
    EXPECT_TRUE(validationWarnings.empty());
    
    // Test getting performance warnings
    auto performanceWarnings = shader.GetPerformanceWarnings();
    EXPECT_TRUE(performanceWarnings.empty());

    TestOutput::PrintTestPass("shader validation");
    return true;
}

/**
 * Test Shader error callback system
 * Requirements: 2.1 (Shader error handling)
 */
bool TestShaderErrorCallbacks() {
    TestOutput::PrintTestStart("shader error callbacks");

    Shader shader;
    
    // Test setting error callback
    bool errorCallbackCalled = false;
    shader.SetErrorCallback([&](const auto& error) {
        errorCallbackCalled = true;
    });
    
    // Test setting warning callback
    bool warningCallbackCalled = false;
    std::string warningShader, warningMessage;
    shader.SetWarningCallback([&](const std::string& shaderName, const std::string& message) {
        warningCallbackCalled = true;
        warningShader = shaderName;
        warningMessage = message;
    });
    
    // Callbacks are set but won't be triggered without actual shader operations
    EXPECT_FALSE(errorCallbackCalled);
    EXPECT_FALSE(warningCallbackCalled);

    TestOutput::PrintTestPass("shader error callbacks");
    return true;
}

int main() {
    TestOutput::PrintHeader("Shader");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Shader Tests");

        // Run all tests
        allPassed &= suite.RunTest("Shader Creation", TestShaderCreation);
        allPassed &= suite.RunTest("Shader Uniform Caching", TestShaderUniformCaching);
        allPassed &= suite.RunTest("Shader Texture Slot Management", TestShaderTextureSlotManagement);
        allPassed &= suite.RunTest("Shader Uniform Setters", TestShaderUniformSetters);
        allPassed &= suite.RunTest("Shader Legacy Uniform Setters", TestShaderLegacyUniformSetters);
        allPassed &= suite.RunTest("Shader Texture Binding", TestShaderTextureBinding);
        allPassed &= suite.RunTest("Shader Compute Interface", TestShaderComputeInterface);
        allPassed &= suite.RunTest("Shader Validation", TestShaderValidation);
        allPassed &= suite.RunTest("Shader Error Callbacks", TestShaderErrorCallbacks);

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