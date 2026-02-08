#include "TestUtils.h"
#include "Graphics/SkeletalMeshData.h"
#include "Graphics/Mesh.h"
#include "Core/Logger.h"
#include <iostream>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Graphics;

/**
 * Test basic skeletal mesh data creation and validation
 * Requirements: 1.1, 4.1
 */
bool TestSkeletalMeshDataCreation() {
    TestOutput::PrintTestStart("skeletal mesh data creation");

    // Create skeletal mesh data with 3 vertices
    SkeletalMeshData skeletalData(3);

    // Verify basic properties
    EXPECT_EQUAL(skeletalData.GetVertexCount(), static_cast<size_t>(3));
    EXPECT_TRUE(skeletalData.IsValid());

    // Check default bone data
    std::vector<uint32_t> indices;
    std::vector<float> weights;
    skeletalData.GetVertexBoneData(0, indices, weights);
    
    EXPECT_EQUAL(indices.size(), static_cast<size_t>(1));
    EXPECT_EQUAL(weights.size(), static_cast<size_t>(1));
    EXPECT_EQUAL(indices[0], static_cast<uint32_t>(0));
    EXPECT_NEARLY_EQUAL(weights[0], 1.0f);

    TestOutput::PrintTestPass("skeletal mesh data creation");
    return true;
}

/**
 * Test bone weight normalization functionality
 * Requirements: 4.2
 */
bool TestBoneWeightNormalization() {
    TestOutput::PrintTestStart("bone weight normalization");

    SkeletalMeshData skeletalData(1);

    // Set unnormalized weights
    std::vector<uint32_t> indices = {0, 1, 2, 3};
    std::vector<float> weights = {0.5f, 0.3f, 0.2f, 0.1f}; // Sum = 1.1

    skeletalData.SetVertexBoneData(0, indices, weights);

    // Verify weights are normalized
    std::vector<uint32_t> outIndices;
    std::vector<float> outWeights;
    skeletalData.GetVertexBoneData(0, outIndices, outWeights);

    float sum = 0.0f;
    for (float weight : outWeights) {
        sum += weight;
    }

    EXPECT_NEARLY_EQUAL(sum, 1.0f);
    EXPECT_TRUE(skeletalData.ValidateWeightNormalization());

    TestOutput::PrintTestPass("bone weight normalization");
    return true;
}

/**
 * Test maximum bone index calculation
 * Requirements: 4.1
 */
bool TestMaxBoneIndex() {
    TestOutput::PrintTestStart("maximum bone index calculation");

    SkeletalMeshData skeletalData(2);

    // Set bone data for first vertex
    std::vector<uint32_t> indices1 = {5, 10, 15};
    std::vector<float> weights1 = {0.5f, 0.3f, 0.2f};
    skeletalData.SetVertexBoneData(0, indices1, weights1);

    // Set bone data for second vertex
    std::vector<uint32_t> indices2 = {20, 25};
    std::vector<float> weights2 = {0.7f, 0.3f};
    skeletalData.SetVertexBoneData(1, indices2, weights2);

    // Verify maximum bone index
    uint32_t maxIndex = skeletalData.GetMaxBoneIndex();
    EXPECT_EQUAL(maxIndex, static_cast<uint32_t>(25));

    TestOutput::PrintTestPass("maximum bone index calculation");
    return true;
}

/**
 * Test bone influence statistics
 * Requirements: 4.2
 */
bool TestBoneInfluenceStatistics() {
    TestOutput::PrintTestStart("bone influence statistics");

    SkeletalMeshData skeletalData(3);

    // Vertex 0: 1 bone influence
    skeletalData.SetVertexBoneData(0, {0}, {1.0f});

    // Vertex 1: 2 bone influences
    skeletalData.SetVertexBoneData(1, {0, 1}, {0.6f, 0.4f});

    // Vertex 2: 4 bone influences
    skeletalData.SetVertexBoneData(2, {0, 1, 2, 3}, {0.4f, 0.3f, 0.2f, 0.1f});

    uint32_t minInfluences, maxInfluences;
    float avgInfluences;
    skeletalData.GetInfluenceStatistics(minInfluences, maxInfluences, avgInfluences);

    EXPECT_EQUAL(minInfluences, static_cast<uint32_t>(1));
    EXPECT_EQUAL(maxInfluences, static_cast<uint32_t>(4));
    EXPECT_NEARLY_EQUAL(avgInfluences, 2.33f, 0.1f);

    TestOutput::PrintTestPass("bone influence statistics");
    return true;
}

/**
 * Test skeletal data validation
 * Requirements: 4.2
 */
bool TestSkeletalDataValidation() {
    TestOutput::PrintTestStart("skeletal data validation");

    // Test valid data
    SkeletalMeshData validData(2);
    validData.SetVertexBoneData(0, {0, 1}, {0.6f, 0.4f});
    validData.SetVertexBoneData(1, {2}, {1.0f});

    EXPECT_TRUE(validData.IsValid());
    EXPECT_TRUE(validData.ValidateWeightNormalization());

    // Test empty data
    SkeletalMeshData emptyData;
    EXPECT_FALSE(emptyData.IsValid());

    TestOutput::PrintTestPass("skeletal data validation");
    return true;
}

/**
 * Test mesh integration with skeletal data
 * Requirements: 1.1, 4.1
 */
bool TestMeshSkeletalIntegration() {
    TestOutput::PrintTestStart("mesh skeletal integration");

    // Create a mesh
    auto mesh = std::make_shared<Mesh>("test_skeletal_mesh");

    // Create vertices
    std::vector<Vertex> vertices = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}}
    };
    std::vector<uint32_t> indices = {0, 1, 2};

    mesh->SetVertices(vertices);
    mesh->SetIndices(indices);

    // Create skeletal data
    auto skeletalData = std::make_unique<SkeletalMeshData>(3);
    skeletalData->SetVertexBoneData(0, {0, 1}, {0.7f, 0.3f});
    skeletalData->SetVertexBoneData(1, {1, 2}, {0.5f, 0.5f});
    skeletalData->SetVertexBoneData(2, {0}, {1.0f});

    // Verify skeletal data is valid
    EXPECT_TRUE(skeletalData->IsValid());

    // Set skeletal data on mesh
    mesh->SetSkeletalData(std::move(skeletalData));

    // Verify mesh has skeletal data
    EXPECT_TRUE(mesh->HasSkeletalData());
    EXPECT_NOT_NULL(mesh->GetSkeletalData());

    // Verify skeletal data properties
    const auto* meshSkeletalData = mesh->GetSkeletalData();
    EXPECT_EQUAL(meshSkeletalData->GetVertexCount(), static_cast<size_t>(3));
    EXPECT_TRUE(meshSkeletalData->IsValid());

    TestOutput::PrintTestPass("mesh skeletal integration");
    return true;
}

/**
 * Test skeletal data memory management
 * Requirements: 4.2
 */
bool TestSkeletalDataMemoryManagement() {
    TestOutput::PrintTestStart("skeletal data memory management");

    // Create skeletal data
    auto skeletalData = std::make_unique<SkeletalMeshData>(100);

    // Set some bone data
    for (size_t i = 0; i < 100; ++i) {
        std::vector<uint32_t> indices = {
            static_cast<uint32_t>(i % 10), 
            static_cast<uint32_t>((i + 1) % 10)
        };
        std::vector<float> weights = {0.6f, 0.4f};
        skeletalData->SetVertexBoneData(i, indices, weights);
    }

    // Verify memory usage calculation
    size_t memoryUsage = skeletalData->GetMemoryUsage();
    EXPECT_TRUE(memoryUsage > 0);

    // Test copy constructor
    SkeletalMeshData copiedData(*skeletalData);
    EXPECT_EQUAL(copiedData.GetVertexCount(), skeletalData->GetVertexCount());
    EXPECT_TRUE(copiedData.IsValid());

    // Test move constructor
    SkeletalMeshData movedData(std::move(*skeletalData));
    EXPECT_EQUAL(movedData.GetVertexCount(), static_cast<size_t>(100));
    EXPECT_TRUE(movedData.IsValid());

    TestOutput::PrintTestPass("skeletal data memory management");
    return true;
}

/**
 * Test four bone influence constraint
 * Requirements: 2.4
 */
bool TestFourBoneInfluenceConstraint() {
    TestOutput::PrintTestStart("four bone influence constraint");

    SkeletalMeshData skeletalData(1);

    // Try to set more than 4 bone influences
    std::vector<uint32_t> indices = {0, 1, 2, 3, 4, 5}; // 6 bones
    std::vector<float> weights = {0.3f, 0.2f, 0.2f, 0.1f, 0.1f, 0.1f};

    skeletalData.SetVertexBoneData(0, indices, weights);

    // Verify only 4 influences are stored
    std::vector<uint32_t> outIndices;
    std::vector<float> outWeights;
    skeletalData.GetVertexBoneData(0, outIndices, outWeights);

    EXPECT_TRUE(outIndices.size() <= 4);
    EXPECT_TRUE(outWeights.size() <= 4);

    // Verify weights are still normalized
    float sum = 0.0f;
    for (float weight : outWeights) {
        sum += weight;
    }
    EXPECT_NEARLY_EQUAL(sum, 1.0f);

    TestOutput::PrintTestPass("four bone influence constraint");
    return true;
}

int main() {
    TestOutput::PrintHeader("Skeletal Mesh Data");

    bool allPassed = true;

    try {
        // Initialize logger for testing
        Logger::GetInstance().SetLogLevel(LogLevel::Warning);

        // Create test suite for result tracking
        TestSuite suite("Skeletal Mesh Data Tests");

        // Run all tests
        allPassed &= suite.RunTest("Skeletal Mesh Data Creation", TestSkeletalMeshDataCreation);
        allPassed &= suite.RunTest("Bone Weight Normalization", TestBoneWeightNormalization);
        allPassed &= suite.RunTest("Maximum Bone Index", TestMaxBoneIndex);
        allPassed &= suite.RunTest("Bone Influence Statistics", TestBoneInfluenceStatistics);
        allPassed &= suite.RunTest("Skeletal Data Validation", TestSkeletalDataValidation);
        allPassed &= suite.RunTest("Mesh Skeletal Integration", TestMeshSkeletalIntegration);
        allPassed &= suite.RunTest("Skeletal Data Memory Management", TestSkeletalDataMemoryManagement);
        allPassed &= suite.RunTest("Four Bone Influence Constraint", TestFourBoneInfluenceConstraint);

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