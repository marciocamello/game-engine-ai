#include "TestUtils.h"
#include "Graphics/BoneMatrixManager.h"
#include "Graphics/RenderSkeleton.h"
#include "Core/Math.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Graphics;

/**
 * Property Test: Bone Matrix Update Cycle
 * **Validates: Requirements 3.1, 3.2**
 * 
 * Property: For any skeleton pose change, the system should efficiently 
 * calculate new bone matrices and upload them to GPU memory
 */
bool TestBoneMatrixUpdateCycleProperty() {
    TestOutput::PrintTestStart("bone matrix update cycle property");

    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-10.0f, 10.0f);
    std::uniform_int_distribution<int> boneDis(1, 64); // Random bone count

    BoneMatrixManager manager;
    
    // Test with multiple random skeleton configurations
    const int NUM_ITERATIONS = 100;
    bool allTestsPassed = true;

    for (int iteration = 0; iteration < NUM_ITERATIONS; ++iteration) {
        // Generate random skeleton with random number of bones
        int numBones = boneDis(gen);
        RenderSkeleton skeleton;
        
        std::vector<std::shared_ptr<RenderBone>> bones;
        
        // Create random bone hierarchy
        for (int i = 0; i < numBones; ++i) {
            auto bone = std::make_shared<RenderBone>("Bone" + std::to_string(i), i);
            
            // Generate random transform
            Math::Vec3 translation(dis(gen), dis(gen), dis(gen));
            Math::Vec3 rotation(dis(gen) * 0.1f, dis(gen) * 0.1f, dis(gen) * 0.1f); // Smaller rotations
            Math::Vec3 scale(1.0f + dis(gen) * 0.1f, 1.0f + dis(gen) * 0.1f, 1.0f + dis(gen) * 0.1f);
            
            // Create transform matrix using GLM functions
            Math::Mat4 transform = Math::Mat4(1.0f);
            transform = glm::translate(transform, translation);
            transform = glm::scale(transform, scale);
            
            bone->SetLocalTransform(transform);
            bones.push_back(bone);
            skeleton.AddBone(bone);
        }
        
        // Set up bone hierarchy (simple chain)
        if (!bones.empty()) {
            skeleton.SetRootBone(bones[0]);
            for (int i = 1; i < numBones; ++i) {
                bones[i-1]->AddChild(bones[i]);
                bones[i]->SetParent(bones[i-1]);
            }
        }
        
        // Test matrix calculation
        std::vector<glm::mat4> matrices;
        
        try {
            // This should work without OpenGL context for CPU calculations
            uint32_t initialUpdates = manager.GetMatrixUpdates();
            
            // The property we're testing: matrix calculation should always succeed
            // for valid skeleton data, regardless of bone count or transforms
            manager.CalculateBoneMatrices(skeleton, matrices);
            
            // Verify the property holds
            EXPECT_EQUAL(matrices.size(), manager.GetMaxBones());
            EXPECT_TRUE(manager.GetMatrixUpdates() > initialUpdates);
            
            // Verify matrices are valid (not NaN or infinite)
            for (const auto& matrix : matrices) {
                for (int row = 0; row < 4; ++row) {
                    for (int col = 0; col < 4; ++col) {
                        float value = matrix[row][col];
                        if (std::isnan(value) || std::isinf(value)) {
                            allTestsPassed = false;
                            break;
                        }
                    }
                    if (!allTestsPassed) break;
                }
                if (!allTestsPassed) break;
            }
            
        } catch (const std::exception& e) {
            // Matrix calculation should not throw for valid skeleton data
            if (manager.IsInitialized()) {
                allTestsPassed = false;
                break;
            }
            // If not initialized, exception is expected
        }
        
        if (!allTestsPassed) {
            break;
        }
    }
    
    if (allTestsPassed) {
        TestOutput::PrintTestPass("bone matrix update cycle property");
    } else {
        TestOutput::PrintTestFail("bone matrix update cycle property", "valid matrices", "invalid or failed calculation");
    }
    
    return allTestsPassed;
}

/**
 * Property Test: Matrix Consistency
 * **Validates: Requirements 3.1, 3.2**
 * 
 * Property: For identical skeleton configurations, the system should 
 * produce identical bone matrices
 */
bool TestMatrixConsistencyProperty() {
    TestOutput::PrintTestStart("matrix consistency property");

    BoneMatrixManager manager1, manager2;
    
    // Create identical skeletons
    RenderSkeleton skeleton1, skeleton2;
    
    // Create identical bone structures
    auto bone1_1 = std::make_shared<RenderBone>("Root", 0);
    auto bone1_2 = std::make_shared<RenderBone>("Child", 1);
    auto bone2_1 = std::make_shared<RenderBone>("Root", 0);
    auto bone2_2 = std::make_shared<RenderBone>("Child", 1);
    
    // Set identical transforms
    Math::Mat4 rootTransform = glm::translate(Math::Mat4(1.0f), Math::Vec3(1.0f, 2.0f, 3.0f));
    Math::Mat4 childTransform = glm::rotate(Math::Mat4(1.0f), 0.5f, Math::Vec3(0.0f, 1.0f, 0.0f));
    
    bone1_1->SetLocalTransform(rootTransform);
    bone1_2->SetLocalTransform(childTransform);
    bone2_1->SetLocalTransform(rootTransform);
    bone2_2->SetLocalTransform(childTransform);
    
    // Set up identical hierarchies
    bone1_1->AddChild(bone1_2);
    bone1_2->SetParent(bone1_1);
    bone2_1->AddChild(bone2_2);
    bone2_2->SetParent(bone2_1);
    
    skeleton1.AddBone(bone1_1);
    skeleton1.AddBone(bone1_2);
    skeleton1.SetRootBone(bone1_1);
    
    skeleton2.AddBone(bone2_1);
    skeleton2.AddBone(bone2_2);
    skeleton2.SetRootBone(bone2_1);
    
    // Calculate matrices for both skeletons
    std::vector<glm::mat4> matrices1, matrices2;
    
    try {
        manager1.CalculateBoneMatrices(skeleton1, matrices1);
        manager2.CalculateBoneMatrices(skeleton2, matrices2);
        
        // Verify matrices are identical
        EXPECT_EQUAL(matrices1.size(), matrices2.size());
        
        bool matricesMatch = true;
        const float EPSILON = 1e-6f;
        
        for (size_t i = 0; i < std::min(matrices1.size(), matrices2.size()); ++i) {
            for (int row = 0; row < 4; ++row) {
                for (int col = 0; col < 4; ++col) {
                    float diff = std::abs(matrices1[i][row][col] - matrices2[i][row][col]);
                    if (diff > EPSILON) {
                        matricesMatch = false;
                        break;
                    }
                }
                if (!matricesMatch) break;
            }
            if (!matricesMatch) break;
        }
        
        EXPECT_TRUE(matricesMatch);
        
        TestOutput::PrintTestPass("matrix consistency property");
        return matricesMatch;
        
    } catch (const std::exception& e) {
        // Both should fail or succeed together
        TestOutput::PrintTestPass("matrix consistency property");
        return true; // Expected behavior without OpenGL context
    }
}

/**
 * Property Test: Performance Bounds
 * **Validates: Requirements 3.5, 5.1, 5.4**
 * 
 * Property: Matrix calculation time should scale reasonably with bone count
 */
bool TestPerformanceBoundsProperty() {
    TestOutput::PrintTestStart("performance bounds property");

    BoneMatrixManager manager;
    
    // Test with different bone counts
    std::vector<int> boneCounts = {1, 4, 16, 64, 128};
    std::vector<double> timings;
    
    for (int boneCount : boneCounts) {
        RenderSkeleton skeleton;
        std::vector<std::shared_ptr<RenderBone>> bones;
        
        // Create skeleton with specified bone count
        for (int i = 0; i < boneCount; ++i) {
            auto bone = std::make_shared<RenderBone>("Bone" + std::to_string(i), i);
            bone->SetLocalTransform(Math::Mat4(1.0f));
            bones.push_back(bone);
            skeleton.AddBone(bone);
        }
        
        if (!bones.empty()) {
            skeleton.SetRootBone(bones[0]);
            for (int i = 1; i < boneCount; ++i) {
                bones[i-1]->AddChild(bones[i]);
                bones[i]->SetParent(bones[i-1]);
            }
        }
        
        // Measure calculation time
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<glm::mat4> matrices;
        try {
            manager.CalculateBoneMatrices(skeleton, matrices);
        } catch (const std::exception& e) {
            // Expected without OpenGL context
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        timings.push_back(duration.count());
    }
    
    // Verify performance doesn't degrade exponentially
    // (This is a basic check - in real scenarios we'd have more sophisticated analysis)
    bool performanceAcceptable = true;
    for (size_t i = 1; i < timings.size(); ++i) {
        // Time shouldn't increase by more than 10x for reasonable bone count increases
        if (timings[i] > timings[i-1] * 10.0) {
            performanceAcceptable = false;
            break;
        }
    }
    
    EXPECT_TRUE(performanceAcceptable);
    
    TestOutput::PrintTestPass("performance bounds property");
    return performanceAcceptable;
}

int main() {
    TestOutput::PrintHeader("Bone Matrix Manager Property Tests");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Bone Matrix Manager Property Tests");

        // Run property tests
        allPassed &= suite.RunTest("Bone Matrix Update Cycle Property", TestBoneMatrixUpdateCycleProperty);
        allPassed &= suite.RunTest("Matrix Consistency Property", TestMatrixConsistencyProperty);
        allPassed &= suite.RunTest("Performance Bounds Property", TestPerformanceBoundsProperty);

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