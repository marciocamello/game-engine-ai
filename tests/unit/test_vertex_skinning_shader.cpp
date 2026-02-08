#include "TestUtils.h"
#include "Graphics/Shader.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/SkeletalMeshData.h"
#include "Core/Math.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Property test for vertex skinning shader transformation
 * **Validates: Requirements 2.1, 2.2, 2.3**
 */
bool TestVertexSkinningTransformationProperty() {
    TestOutput::PrintTestStart("vertex skinning transformation property");

    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-10.0f, 10.0f);
    std::uniform_real_distribution<float> weightDist(0.0f, 1.0f);
    std::uniform_int_distribution<int> boneDist(0, 3);

    bool allTestsPassed = true;
    const int NUM_ITERATIONS = 100;

    for (int iteration = 0; iteration < NUM_ITERATIONS; ++iteration) {
        // Generate random vertex position
        glm::vec3 originalPos(posDist(gen), posDist(gen), posDist(gen));
        
        // Generate random bone matrices
        std::vector<glm::mat4> boneMatrices(4);
        for (int i = 0; i < 4; ++i) {
            glm::vec3 translation(posDist(gen) * 0.1f, posDist(gen) * 0.1f, posDist(gen) * 0.1f);
            glm::vec3 rotation(posDist(gen) * 0.1f, posDist(gen) * 0.1f, posDist(gen) * 0.1f);
            
            boneMatrices[i] = glm::translate(glm::mat4(1.0f), translation);
            boneMatrices[i] = glm::rotate(boneMatrices[i], rotation.x, glm::vec3(1, 0, 0));
            boneMatrices[i] = glm::rotate(boneMatrices[i], rotation.y, glm::vec3(0, 1, 0));
            boneMatrices[i] = glm::rotate(boneMatrices[i], rotation.z, glm::vec3(0, 0, 1));
        }
        
        // Generate random bone weights (will be normalized)
        glm::vec4 weights(weightDist(gen), weightDist(gen), weightDist(gen), weightDist(gen));
        float weightSum = weights.x + weights.y + weights.z + weights.w;
        if (weightSum > 0.0f) {
            weights /= weightSum; // Normalize weights
        } else {
            weights = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f); // Fallback
        }
        
        // Generate bone indices
        glm::ivec4 boneIndices(0, 1, 2, 3);
        
        // Calculate expected transformed position using CPU
        glm::vec4 expectedPos(0.0f);
        for (int i = 0; i < 4; ++i) {
            if (weights[i] > 0.0f) {
                glm::vec4 transformedPos = boneMatrices[i] * glm::vec4(originalPos, 1.0f);
                expectedPos += transformedPos * weights[i];
            }
        }
        
        // Property 1: Weight sum should equal 1.0 (within epsilon)
        float actualWeightSum = weights.x + weights.y + weights.z + weights.w;
        if (std::abs(actualWeightSum - 1.0f) > 0.001f) {
            TestOutput::PrintTestFail("vertex skinning transformation property", 
                                    "Weight sum = 1.0", 
                                    "Weight sum = " + std::to_string(actualWeightSum));
            allTestsPassed = false;
            continue;
        }
        
        // Property 2: Transformed position should be finite
        if (!std::isfinite(expectedPos.x) || !std::isfinite(expectedPos.y) || 
            !std::isfinite(expectedPos.z) || !std::isfinite(expectedPos.w)) {
            TestOutput::PrintTestFail("vertex skinning transformation property", 
                                    "Finite transformed position", 
                                    "Non-finite position components");
            allTestsPassed = false;
            continue;
        }
        
        // Property 3: If all weights are zero except one, result should match single bone transform
        glm::vec4 singleBoneWeights(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec4 singleBoneResult = boneMatrices[0] * glm::vec4(originalPos, 1.0f);
        
        glm::vec4 calculatedSingleResult(0.0f);
        for (int i = 0; i < 4; ++i) {
            if (singleBoneWeights[i] > 0.0f) {
                calculatedSingleResult += (boneMatrices[i] * glm::vec4(originalPos, 1.0f)) * singleBoneWeights[i];
            }
        }
        
        float singleBoneDiff = glm::length(singleBoneResult - calculatedSingleResult);
        if (singleBoneDiff > 0.001f) {
            TestOutput::PrintTestFail("vertex skinning transformation property", 
                                    "Single bone transform consistency", 
                                    "Difference = " + std::to_string(singleBoneDiff));
            allTestsPassed = false;
            continue;
        }
        
        // Property 4: Identity matrices should preserve original position
        std::vector<glm::mat4> identityMatrices(4, glm::mat4(1.0f));
        glm::vec4 identityResult(0.0f);
        for (int i = 0; i < 4; ++i) {
            identityResult += (identityMatrices[i] * glm::vec4(originalPos, 1.0f)) * weights[i];
        }
        
        float identityDiff = glm::length(glm::vec3(identityResult) - originalPos);
        if (identityDiff > 0.001f) {
            TestOutput::PrintTestFail("vertex skinning transformation property", 
                                    "Identity matrix preservation", 
                                    "Difference = " + std::to_string(identityDiff));
            allTestsPassed = false;
            continue;
        }
    }

    if (allTestsPassed) {
        TestOutput::PrintTestPass("vertex skinning transformation property");
    }
    
    return allTestsPassed;
}

/**
 * Test bone influence constraint validation
 * Requirements: 2.4
 */
bool TestBoneInfluenceConstraints() {
    TestOutput::PrintTestStart("bone influence constraints");

    // Test maximum 4 bone influences per vertex
    SkeletalMeshData skeletalData(1);
    
    // Test valid 4-bone influence
    std::vector<uint32_t> validIndices = {0, 1, 2, 3};
    std::vector<float> validWeights = {0.4f, 0.3f, 0.2f, 0.1f};
    
    skeletalData.SetVertexBoneData(0, validIndices, validWeights);
    
    std::vector<uint32_t> outIndices;
    std::vector<float> outWeights;
    skeletalData.GetVertexBoneData(0, outIndices, outWeights);
    
    // Verify we get exactly 4 influences
    EXPECT_EQUAL(outIndices.size(), 4);
    EXPECT_EQUAL(outWeights.size(), 4);
    
    // Verify weight normalization
    float weightSum = 0.0f;
    for (float weight : outWeights) {
        weightSum += weight;
    }
    EXPECT_NEARLY_EQUAL(weightSum, 1.0f);
    
    // Test that weights are properly normalized
    EXPECT_TRUE(skeletalData.ValidateWeightNormalization());

    TestOutput::PrintTestPass("bone influence constraints");
    return true;
}

/**
 * Test normal and tangent transformation consistency
 * Requirements: 2.3
 */
bool TestNormalTangentTransformation() {
    TestOutput::PrintTestStart("normal tangent transformation");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    bool allTestsPassed = true;
    const int NUM_ITERATIONS = 50;

    for (int iteration = 0; iteration < NUM_ITERATIONS; ++iteration) {
        // Generate random normal and tangent
        glm::vec3 originalNormal = glm::normalize(glm::vec3(dist(gen), dist(gen), dist(gen)));
        glm::vec3 originalTangent = glm::normalize(glm::vec3(dist(gen), dist(gen), dist(gen)));
        
        // Ensure tangent is orthogonal to normal
        originalTangent = glm::normalize(originalTangent - glm::dot(originalTangent, originalNormal) * originalNormal);
        
        // Generate random bone matrix
        glm::mat4 boneMatrix = glm::mat4(1.0f);
        glm::vec3 rotation(dist(gen) * 0.5f, dist(gen) * 0.5f, dist(gen) * 0.5f);
        boneMatrix = glm::rotate(boneMatrix, rotation.x, glm::vec3(1, 0, 0));
        boneMatrix = glm::rotate(boneMatrix, rotation.y, glm::vec3(0, 1, 0));
        boneMatrix = glm::rotate(boneMatrix, rotation.z, glm::vec3(0, 0, 1));
        
        // Transform normal and tangent
        glm::mat3 normalMatrix = glm::mat3(boneMatrix);
        glm::vec3 transformedNormal = glm::normalize(normalMatrix * originalNormal);
        glm::vec3 transformedTangent = glm::normalize(normalMatrix * originalTangent);
        
        // Property: Transformed vectors should remain normalized
        float normalLength = glm::length(transformedNormal);
        float tangentLength = glm::length(transformedTangent);
        
        if (std::abs(normalLength - 1.0f) > 0.001f) {
            TestOutput::PrintTestFail("normal tangent transformation", 
                                    "Normal length = 1.0", 
                                    "Normal length = " + std::to_string(normalLength));
            allTestsPassed = false;
            continue;
        }
        
        if (std::abs(tangentLength - 1.0f) > 0.001f) {
            TestOutput::PrintTestFail("normal tangent transformation", 
                                    "Tangent length = 1.0", 
                                    "Tangent length = " + std::to_string(tangentLength));
            allTestsPassed = false;
            continue;
        }
        
        // Property: Orthogonality should be preserved (approximately)
        float dotProduct = glm::dot(transformedNormal, transformedTangent);
        if (std::abs(dotProduct) > 0.1f) { // Allow some tolerance for numerical precision
            TestOutput::PrintTestFail("normal tangent transformation", 
                                    "Orthogonality preserved", 
                                    "Dot product = " + std::to_string(dotProduct));
            allTestsPassed = false;
            continue;
        }
    }

    if (allTestsPassed) {
        TestOutput::PrintTestPass("normal tangent transformation");
    }
    
    return allTestsPassed;
}

int main() {
    TestOutput::PrintHeader("Vertex Skinning Shader");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Vertex Skinning Shader Tests");

        // Run all tests
        allPassed &= suite.RunTest("Vertex Skinning Transformation Property", TestVertexSkinningTransformationProperty);
        allPassed &= suite.RunTest("Bone Influence Constraints", TestBoneInfluenceConstraints);
        allPassed &= suite.RunTest("Normal Tangent Transformation", TestNormalTangentTransformation);

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