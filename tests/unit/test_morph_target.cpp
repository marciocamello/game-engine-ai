#include "TestUtils.h"
#include "Animation/MorphTarget.h"
#include "Graphics/Mesh.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test MorphTarget creation and basic properties
 * Requirements: 5.1 (morph target creation with vertex deltas)
 */
bool TestMorphTargetCreation() {
    TestOutput::PrintTestStart("morph target creation");

    MorphTarget morphTarget("TestMorph");

    EXPECT_EQUAL(morphTarget.GetName(), std::string("TestMorph"));
    EXPECT_NEARLY_EQUAL(morphTarget.GetWeight(), 0.0f);
    EXPECT_FALSE(morphTarget.HasPositionDeltas());
    EXPECT_FALSE(morphTarget.HasNormalDeltas());
    EXPECT_FALSE(morphTarget.HasTangentDeltas());

    TestOutput::PrintTestPass("morph target creation");
    return true;
}

/**
 * Test MorphTarget vertex delta storage and retrieval
 * Requirements: 5.1 (vertex position, normal, and tangent deltas)
 */
bool TestMorphTargetVertexDeltas() {
    TestOutput::PrintTestStart("morph target vertex deltas");

    MorphTarget morphTarget("TestMorph");

    // Create test vertex deltas
    std::vector<Math::Vec3> positionDeltas = {
        Math::Vec3(0.1f, 0.0f, 0.0f),
        Math::Vec3(0.0f, 0.1f, 0.0f),
        Math::Vec3(0.0f, 0.0f, 0.1f)
    };

    std::vector<Math::Vec3> normalDeltas = {
        Math::Vec3(0.05f, 0.0f, 0.0f),
        Math::Vec3(0.0f, 0.05f, 0.0f),
        Math::Vec3(0.0f, 0.0f, 0.05f)
    };

    std::vector<Math::Vec3> tangentDeltas = {
        Math::Vec3(0.02f, 0.0f, 0.0f),
        Math::Vec3(0.0f, 0.02f, 0.0f),
        Math::Vec3(0.0f, 0.0f, 0.02f)
    };

    // Set vertex deltas
    morphTarget.SetVertexDeltas(positionDeltas);
    morphTarget.SetNormalDeltas(normalDeltas);
    morphTarget.SetTangentDeltas(tangentDeltas);

    // Verify deltas were stored correctly
    EXPECT_TRUE(morphTarget.HasPositionDeltas());
    EXPECT_TRUE(morphTarget.HasNormalDeltas());
    EXPECT_TRUE(morphTarget.HasTangentDeltas());
    EXPECT_TRUE(morphTarget.IsValid());

    const auto& storedPositions = morphTarget.GetVertexDeltas();
    const auto& storedNormals = morphTarget.GetNormalDeltas();
    const auto& storedTangents = morphTarget.GetTangentDeltas();

    EXPECT_EQUAL(storedPositions.size(), static_cast<size_t>(3));
    EXPECT_EQUAL(storedNormals.size(), static_cast<size_t>(3));
    EXPECT_EQUAL(storedTangents.size(), static_cast<size_t>(3));

    // Verify first delta values
    EXPECT_VEC3_NEARLY_EQUAL(storedPositions[0], Math::Vec3(0.1f, 0.0f, 0.0f));
    EXPECT_VEC3_NEARLY_EQUAL(storedNormals[0], Math::Vec3(0.05f, 0.0f, 0.0f));
    EXPECT_VEC3_NEARLY_EQUAL(storedTangents[0], Math::Vec3(0.02f, 0.0f, 0.0f));

    TestOutput::PrintTestPass("morph target vertex deltas");
    return true;
}

/**
 * Test MorphTarget weight management
 * Requirements: 5.2 (morph target weight management)
 */
bool TestMorphTargetWeightManagement() {
    TestOutput::PrintTestStart("morph target weight management");

    MorphTarget morphTarget("TestMorph");

    // Test weight clamping
    morphTarget.SetWeight(0.5f);
    EXPECT_NEARLY_EQUAL(morphTarget.GetWeight(), 0.5f);

    morphTarget.SetWeight(-0.1f);
    EXPECT_NEARLY_EQUAL(morphTarget.GetWeight(), 0.0f);

    morphTarget.SetWeight(1.5f);
    EXPECT_NEARLY_EQUAL(morphTarget.GetWeight(), 1.0f);

    morphTarget.SetWeight(0.75f);
    EXPECT_NEARLY_EQUAL(morphTarget.GetWeight(), 0.75f);

    TestOutput::PrintTestPass("morph target weight management");
    return true;
}

/**
 * Test MorphTarget application to vertices
 * Requirements: 5.3 (morph target application to mesh vertices)
 */
bool TestMorphTargetApplicationToVertices() {
    TestOutput::PrintTestStart("morph target application to vertices");

    MorphTarget morphTarget("TestMorph");

    // Create test vertices
    std::vector<Vertex> vertices(2);
    vertices[0].position = Math::Vec3(1.0f, 0.0f, 0.0f);
    vertices[0].normal = Math::Vec3(0.0f, 1.0f, 0.0f);
    vertices[0].tangent = Math::Vec3(0.0f, 0.0f, 1.0f);

    vertices[1].position = Math::Vec3(0.0f, 1.0f, 0.0f);
    vertices[1].normal = Math::Vec3(1.0f, 0.0f, 0.0f);
    vertices[1].tangent = Math::Vec3(0.0f, 1.0f, 0.0f);

    // Create morph target deltas
    std::vector<Math::Vec3> positionDeltas = {
        Math::Vec3(0.1f, 0.0f, 0.0f),
        Math::Vec3(0.0f, 0.1f, 0.0f)
    };

    std::vector<Math::Vec3> normalDeltas = {
        Math::Vec3(0.05f, 0.0f, 0.0f),
        Math::Vec3(0.0f, 0.05f, 0.0f)
    };

    morphTarget.SetVertexDeltas(positionDeltas);
    morphTarget.SetNormalDeltas(normalDeltas);

    // Store original positions for comparison
    Math::Vec3 originalPos0 = vertices[0].position;
    Math::Vec3 originalPos1 = vertices[1].position;

    // Apply morph target with weight 0.5
    morphTarget.ApplyToVertices(vertices, 0.5f);

    // Verify positions were modified correctly
    Math::Vec3 expectedPos0 = originalPos0 + positionDeltas[0] * 0.5f;
    Math::Vec3 expectedPos1 = originalPos1 + positionDeltas[1] * 0.5f;

    EXPECT_VEC3_NEARLY_EQUAL(vertices[0].position, expectedPos0);
    EXPECT_VEC3_NEARLY_EQUAL(vertices[1].position, expectedPos1);

    TestOutput::PrintTestPass("morph target application to vertices");
    return true;
}

/**
 * Test MorphTargetController creation and basic functionality
 * Requirements: 5.4 (multiple morph target management)
 */
bool TestMorphTargetControllerCreation() {
    TestOutput::PrintTestStart("morph target controller creation");

    MorphTargetController controller;

    EXPECT_EQUAL(controller.GetMorphTargetCount(), static_cast<size_t>(0));
    EXPECT_TRUE(controller.IsValid());
    EXPECT_TRUE(controller.GetBlendMode() == MorphTargetController::BlendMode::Additive);

    TestOutput::PrintTestPass("morph target controller creation");
    return true;
}

/**
 * Test MorphTargetController morph target management
 * Requirements: 5.4 (multiple morph target management)
 */
bool TestMorphTargetControllerManagement() {
    TestOutput::PrintTestStart("morph target controller management");

    MorphTargetController controller;

    // Create test morph targets
    auto morphTarget1 = std::make_shared<MorphTarget>("Smile");
    auto morphTarget2 = std::make_shared<MorphTarget>("Frown");

    // Add vertex deltas to make them valid
    std::vector<Math::Vec3> deltas = { Math::Vec3(0.1f, 0.0f, 0.0f) };
    morphTarget1->SetVertexDeltas(deltas);
    morphTarget2->SetVertexDeltas(deltas);

    // Add morph targets to controller
    controller.AddMorphTarget(morphTarget1);
    controller.AddMorphTarget(morphTarget2);

    EXPECT_EQUAL(controller.GetMorphTargetCount(), static_cast<size_t>(2));

    // Test retrieval
    auto retrieved1 = controller.GetMorphTarget("Smile");
    auto retrieved2 = controller.GetMorphTarget("Frown");

    EXPECT_TRUE(retrieved1 != nullptr);
    EXPECT_TRUE(retrieved2 != nullptr);
    EXPECT_EQUAL(retrieved1->GetName(), std::string("Smile"));
    EXPECT_EQUAL(retrieved2->GetName(), std::string("Frown"));

    // Test removal
    controller.RemoveMorphTarget("Smile");
    EXPECT_EQUAL(controller.GetMorphTargetCount(), static_cast<size_t>(1));
    EXPECT_TRUE(controller.GetMorphTarget("Smile") == nullptr);
    EXPECT_TRUE(controller.GetMorphTarget("Frown") != nullptr);

    TestOutput::PrintTestPass("morph target controller management");
    return true;
}

/**
 * Test MorphTargetController weight animation
 * Requirements: 5.5 (morph target weight animation with keyframe interpolation)
 */
bool TestMorphTargetControllerWeightAnimation() {
    TestOutput::PrintTestStart("morph target controller weight animation");

    MorphTargetController controller;

    // Create test morph target
    auto morphTarget = std::make_shared<MorphTarget>("TestMorph");
    std::vector<Math::Vec3> deltas = { Math::Vec3(0.1f, 0.0f, 0.0f) };
    morphTarget->SetVertexDeltas(deltas);

    controller.AddMorphTarget(morphTarget);

    // Test immediate weight setting
    controller.SetWeight("TestMorph", 0.5f);
    EXPECT_NEARLY_EQUAL(controller.GetWeight("TestMorph"), 0.5f);

    // Test weight animation
    controller.AnimateWeight("TestMorph", 1.0f, 1.0f); // Animate to 1.0 over 1 second

    // Simulate time progression
    controller.Update(0.5f); // Half way through animation
    float halfwayWeight = controller.GetWeight("TestMorph");
    EXPECT_TRUE(halfwayWeight > 0.5f && halfwayWeight < 1.0f);

    controller.Update(0.5f); // Complete animation
    EXPECT_NEARLY_EQUAL(controller.GetWeight("TestMorph"), 1.0f);

    TestOutput::PrintTestPass("morph target controller weight animation");
    return true;
}

/**
 * Test MorphTargetController blending modes
 * Requirements: 5.6 (morph target blending with additive and override modes)
 */
bool TestMorphTargetControllerBlendingModes() {
    TestOutput::PrintTestStart("morph target controller blending modes");

    MorphTargetController controller;

    // Create test morph targets
    auto morphTarget1 = std::make_shared<MorphTarget>("Morph1");
    auto morphTarget2 = std::make_shared<MorphTarget>("Morph2");

    std::vector<Math::Vec3> deltas1 = { Math::Vec3(0.1f, 0.0f, 0.0f) };
    std::vector<Math::Vec3> deltas2 = { Math::Vec3(0.0f, 0.1f, 0.0f) };

    morphTarget1->SetVertexDeltas(deltas1);
    morphTarget2->SetVertexDeltas(deltas2);

    controller.AddMorphTarget(morphTarget1);
    controller.AddMorphTarget(morphTarget2);

    // Set weights
    controller.SetWeight("Morph1", 0.5f);
    controller.SetWeight("Morph2", 0.3f);

    // Test additive blending (default)
    EXPECT_TRUE(controller.GetBlendMode() == MorphTargetController::BlendMode::Additive);

    // Test override blending
    controller.SetBlendMode(MorphTargetController::BlendMode::Override);
    EXPECT_TRUE(controller.GetBlendMode() == MorphTargetController::BlendMode::Override);

    TestOutput::PrintTestPass("morph target controller blending modes");
    return true;
}

/**
 * Test MorphTargetSet functionality
 * Requirements: 5.1, 5.4 (morph target set management)
 */
bool TestMorphTargetSet() {
    TestOutput::PrintTestStart("morph target set");

    MorphTargetSet morphTargetSet;

    // Create test morph target
    auto morphTarget = std::make_shared<MorphTarget>("TestMorph");
    std::vector<Math::Vec3> deltas = { Math::Vec3(0.1f, 0.0f, 0.0f) };
    morphTarget->SetVertexDeltas(deltas);

    // Add to set
    morphTargetSet.AddMorphTarget(morphTarget);

    EXPECT_EQUAL(morphTargetSet.GetMorphTargetCount(), static_cast<size_t>(1));

    // Test controller access
    auto controller = morphTargetSet.GetController();
    EXPECT_TRUE(controller != nullptr);
    EXPECT_EQUAL(controller->GetMorphTargetCount(), static_cast<size_t>(1));

    // Test retrieval through set
    auto retrieved = morphTargetSet.GetMorphTarget("TestMorph");
    EXPECT_TRUE(retrieved != nullptr);
    EXPECT_EQUAL(retrieved->GetName(), std::string("TestMorph"));

    TestOutput::PrintTestPass("morph target set");
    return true;
}

int main() {
    TestOutput::PrintHeader("MorphTarget");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("MorphTarget Tests");

        // Run all tests
        allPassed &= suite.RunTest("MorphTarget Creation", TestMorphTargetCreation);
        allPassed &= suite.RunTest("MorphTarget Vertex Deltas", TestMorphTargetVertexDeltas);
        allPassed &= suite.RunTest("MorphTarget Weight Management", TestMorphTargetWeightManagement);
        allPassed &= suite.RunTest("MorphTarget Application to Vertices", TestMorphTargetApplicationToVertices);
        allPassed &= suite.RunTest("MorphTargetController Creation", TestMorphTargetControllerCreation);
        allPassed &= suite.RunTest("MorphTargetController Management", TestMorphTargetControllerManagement);
        allPassed &= suite.RunTest("MorphTargetController Weight Animation", TestMorphTargetControllerWeightAnimation);
        allPassed &= suite.RunTest("MorphTargetController Blending Modes", TestMorphTargetControllerBlendingModes);
        allPassed &= suite.RunTest("MorphTargetSet", TestMorphTargetSet);

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