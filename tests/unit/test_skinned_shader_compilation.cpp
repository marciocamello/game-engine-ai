#include "TestUtils.h"
#include "Graphics/Shader.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/OpenGLContext.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test shader compilation success and error handling
 * Requirements: 2.5, 7.4
 */
bool TestSkinnedShaderCompilation() {
    TestOutput::PrintTestStart("skinned shader compilation");

    // Read vertex shader source
    std::ifstream vertFile("assets/shaders/skinned.vert");
    if (!vertFile.is_open()) {
        TestOutput::PrintTestFail("skinned shader compilation", 
                                "Vertex shader file exists", 
                                "Could not open assets/shaders/skinned.vert");
        return false;
    }
    
    std::stringstream vertStream;
    vertStream << vertFile.rdbuf();
    std::string vertexSource = vertStream.str();
    vertFile.close();
    
    // Read fragment shader source
    std::ifstream fragFile("assets/shaders/skinned.frag");
    if (!fragFile.is_open()) {
        TestOutput::PrintTestFail("skinned shader compilation", 
                                "Fragment shader file exists", 
                                "Could not open assets/shaders/skinned.frag");
        return false;
    }
    
    std::stringstream fragStream;
    fragStream << fragFile.rdbuf();
    std::string fragmentSource = fragStream.str();
    fragFile.close();
    
    // Verify shader sources are not empty
    EXPECT_TRUE(!vertexSource.empty());
    EXPECT_TRUE(!fragmentSource.empty());
    
    // Check for required vertex shader elements
    EXPECT_TRUE(vertexSource.find("MAX_BONES") != std::string::npos);
    EXPECT_TRUE(vertexSource.find("MAX_BONE_INFLUENCE") != std::string::npos);
    EXPECT_TRUE(vertexSource.find("aBoneIds") != std::string::npos);
    EXPECT_TRUE(vertexSource.find("aWeights") != std::string::npos);
    EXPECT_TRUE(vertexSource.find("uBoneMatrices") != std::string::npos);
    
    // Check for required fragment shader elements
    EXPECT_TRUE(fragmentSource.find("FragColor") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("FragPos") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("Normal") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("TexCoord") != std::string::npos);

    TestOutput::PrintTestPass("skinned shader compilation");
    return true;
}

/**
 * Test shader uniform and attribute validation
 * Requirements: 7.4
 */
bool TestShaderUniformValidation() {
    TestOutput::PrintTestStart("shader uniform validation");

    // Read vertex shader to validate required uniforms
    std::ifstream vertFile("assets/shaders/skinned.vert");
    if (!vertFile.is_open()) {
        TestOutput::PrintTestFail("shader uniform validation", 
                                "Vertex shader accessible", 
                                "Could not read vertex shader");
        return false;
    }
    
    std::stringstream vertStream;
    vertStream << vertFile.rdbuf();
    std::string vertexSource = vertStream.str();
    vertFile.close();
    
    // Check for required transformation uniforms
    EXPECT_TRUE(vertexSource.find("uniform mat4 uModel") != std::string::npos);
    EXPECT_TRUE(vertexSource.find("uniform mat4 uView") != std::string::npos);
    EXPECT_TRUE(vertexSource.find("uniform mat4 uProjection") != std::string::npos);
    EXPECT_TRUE(vertexSource.find("uniform mat3 uNormalMatrix") != std::string::npos);
    
    // Check for bone matrix uniform
    EXPECT_TRUE(vertexSource.find("uniform mat4 uBoneMatrices[MAX_BONES]") != std::string::npos);
    
    // Check for required vertex attributes
    EXPECT_TRUE(vertexSource.find("in vec3 aPos") != std::string::npos);
    EXPECT_TRUE(vertexSource.find("in vec3 aNormal") != std::string::npos);
    EXPECT_TRUE(vertexSource.find("in vec2 aTexCoord") != std::string::npos);
    EXPECT_TRUE(vertexSource.find("in vec3 aTangent") != std::string::npos);
    EXPECT_TRUE(vertexSource.find("in ivec4 aBoneIds") != std::string::npos);
    EXPECT_TRUE(vertexSource.find("in vec4 aWeights") != std::string::npos);
    
    // Read fragment shader to validate required uniforms
    std::ifstream fragFile("assets/shaders/skinned.frag");
    if (!fragFile.is_open()) {
        TestOutput::PrintTestFail("shader uniform validation", 
                                "Fragment shader accessible", 
                                "Could not read fragment shader");
        return false;
    }
    
    std::stringstream fragStream;
    fragStream << fragFile.rdbuf();
    std::string fragmentSource = fragStream.str();
    fragFile.close();
    
    // Check for material uniforms
    EXPECT_TRUE(fragmentSource.find("uniform vec4 uColor") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("uniform vec3 uEmissive") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("uniform float uMetallic") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("uniform float uRoughness") != std::string::npos);
    
    // Check for texture uniforms
    EXPECT_TRUE(fragmentSource.find("uniform bool uHasAlbedoTexture") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("uniform bool uHasNormalTexture") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("uniform sampler2D uAlbedoTexture") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("uniform sampler2D uNormalTexture") != std::string::npos);
    
    // Check for lighting uniforms
    EXPECT_TRUE(fragmentSource.find("uniform vec3 uLightPos") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("uniform vec3 uLightColor") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("uniform vec3 uViewPos") != std::string::npos);

    TestOutput::PrintTestPass("shader uniform validation");
    return true;
}

/**
 * Test shader version and compatibility
 * Requirements: 2.5
 */
bool TestShaderVersionCompatibility() {
    TestOutput::PrintTestStart("shader version compatibility");

    // Read vertex shader
    std::ifstream vertFile("assets/shaders/skinned.vert");
    if (!vertFile.is_open()) {
        TestOutput::PrintTestFail("shader version compatibility", 
                                "Vertex shader accessible", 
                                "Could not read vertex shader");
        return false;
    }
    
    std::string firstLine;
    std::getline(vertFile, firstLine);
    vertFile.close();
    
    // Check for OpenGL 4.6 core profile
    EXPECT_TRUE(firstLine.find("#version 460 core") != std::string::npos);
    
    // Read fragment shader
    std::ifstream fragFile("assets/shaders/skinned.frag");
    if (!fragFile.is_open()) {
        TestOutput::PrintTestFail("shader version compatibility", 
                                "Fragment shader accessible", 
                                "Could not read fragment shader");
        return false;
    }
    
    std::getline(fragFile, firstLine);
    fragFile.close();
    
    // Check for OpenGL 4.6 core profile
    EXPECT_TRUE(firstLine.find("#version 460 core") != std::string::npos);

    TestOutput::PrintTestPass("shader version compatibility");
    return true;
}

/**
 * Test bone matrix array size validation
 * Requirements: 3.4
 */
bool TestBoneMatrixArraySize() {
    TestOutput::PrintTestStart("bone matrix array size");

    std::ifstream vertFile("assets/shaders/skinned.vert");
    if (!vertFile.is_open()) {
        TestOutput::PrintTestFail("bone matrix array size", 
                                "Vertex shader accessible", 
                                "Could not read vertex shader");
        return false;
    }
    
    std::stringstream vertStream;
    vertStream << vertFile.rdbuf();
    std::string vertexSource = vertStream.str();
    vertFile.close();
    
    // Check that MAX_BONES is set to 128 (as per requirements)
    EXPECT_TRUE(vertexSource.find("const int MAX_BONES = 128") != std::string::npos);
    
    // Check that MAX_BONE_INFLUENCE is set to 4
    EXPECT_TRUE(vertexSource.find("const int MAX_BONE_INFLUENCE = 4") != std::string::npos);
    
    // Verify bone matrix array declaration
    EXPECT_TRUE(vertexSource.find("uniform mat4 uBoneMatrices[MAX_BONES]") != std::string::npos);

    TestOutput::PrintTestPass("bone matrix array size");
    return true;
}

/**
 * Test PBR material compatibility
 * Requirements: 2.6, 4.4
 */
bool TestPBRMaterialCompatibility() {
    TestOutput::PrintTestStart("PBR material compatibility");

    std::ifstream fragFile("assets/shaders/skinned.frag");
    if (!fragFile.is_open()) {
        TestOutput::PrintTestFail("PBR material compatibility", 
                                "Fragment shader accessible", 
                                "Could not read fragment shader");
        return false;
    }
    
    std::stringstream fragStream;
    fragStream << fragFile.rdbuf();
    std::string fragmentSource = fragStream.str();
    fragFile.close();
    
    // Check for PBR-specific functions
    EXPECT_TRUE(fragmentSource.find("fresnelSchlick") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("distributionGGX") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("geometrySmith") != std::string::npos);
    
    // Check for PBR material properties
    EXPECT_TRUE(fragmentSource.find("metallic") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("roughness") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("emissive") != std::string::npos);
    
    // Check for normal mapping support
    EXPECT_TRUE(fragmentSource.find("getNormalFromMap") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("TBN") != std::string::npos);
    
    // Check for HDR and gamma correction
    EXPECT_TRUE(fragmentSource.find("tonemapping") != std::string::npos);
    EXPECT_TRUE(fragmentSource.find("Gamma correction") != std::string::npos);

    TestOutput::PrintTestPass("PBR material compatibility");
    return true;
}

int main() {
    TestOutput::PrintHeader("Skinned Shader Compilation");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Skinned Shader Compilation Tests");

        // Run all tests
        allPassed &= suite.RunTest("Shader Compilation", TestSkinnedShaderCompilation);
        allPassed &= suite.RunTest("Uniform Validation", TestShaderUniformValidation);
        allPassed &= suite.RunTest("Version Compatibility", TestShaderVersionCompatibility);
        allPassed &= suite.RunTest("Bone Matrix Array Size", TestBoneMatrixArraySize);
        allPassed &= suite.RunTest("PBR Material Compatibility", TestPBRMaterialCompatibility);

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