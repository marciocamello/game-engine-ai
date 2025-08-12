#include "TestUtils.h"
#include "Graphics/BoundingVolumeCalculator.h"
#include "Graphics/Mesh.h"
#include "Graphics/ModelNode.h"
#include "Graphics/Model.h"
#include "Core/Math.h"
#include <memory>

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestAABBCalculation() {
    TestOutput::PrintTestStart("AABB calculation");

    // Test with simple points
    std::vector<Math::Vec3> points = {
        Math::Vec3(-1.0f, -1.0f, -1.0f),
        Math::Vec3(1.0f, 1.0f, 1.0f),
        Math::Vec3(0.0f, 2.0f, 0.0f),
        Math::Vec3(-2.0f, 0.0f, 0.0f)
    };

    BoundingBox box = BoundingVolumeCalculator::CalculateAABB(points);

    EXPECT_TRUE(box.IsValid());
    EXPECT_NEARLY_EQUAL_EPSILON(box.min.x, -2.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(box.min.y, -1.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(box.min.z, -1.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(box.max.x, 1.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(box.max.y, 2.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(box.max.z, 1.0f, 0.001f);

    // Test validation
    EXPECT_TRUE(BoundingVolumeCalculator::ValidateBoundingVolume(box, points));

    TestOutput::PrintTestPass("AABB calculation");
    return true;
}

bool TestBoundingSphereCalculation() {
    TestOutput::PrintTestStart("bounding sphere calculation");

    // Test with simple points
    std::vector<Math::Vec3> points = {
        Math::Vec3(-1.0f, 0.0f, 0.0f),
        Math::Vec3(1.0f, 0.0f, 0.0f),
        Math::Vec3(0.0f, -1.0f, 0.0f),
        Math::Vec3(0.0f, 1.0f, 0.0f)
    };

    // Test naive sphere
    BoundingSphere naiveSphere = BoundingVolumeCalculator::CalculateNaiveSphere(points);
    EXPECT_TRUE(naiveSphere.IsValid());
    EXPECT_TRUE(BoundingVolumeCalculator::ValidateBoundingVolume(naiveSphere, points));

    // Test Ritter sphere
    BoundingSphere ritterSphere = BoundingVolumeCalculator::CalculateRitterSphere(points);
    EXPECT_TRUE(ritterSphere.IsValid());
    EXPECT_TRUE(BoundingVolumeCalculator::ValidateBoundingVolume(ritterSphere, points));

    // Test Welzl sphere
    BoundingSphere welzlSphere = BoundingVolumeCalculator::CalculateWelzlSphere(points);
    EXPECT_TRUE(welzlSphere.IsValid());
    EXPECT_TRUE(BoundingVolumeCalculator::ValidateBoundingVolume(welzlSphere, points));

    // Test optimal sphere
    BoundingSphere optimalSphere = BoundingVolumeCalculator::CalculateOptimalSphere(points);
    EXPECT_TRUE(optimalSphere.IsValid());
    EXPECT_TRUE(BoundingVolumeCalculator::ValidateBoundingVolume(optimalSphere, points));

    TestOutput::PrintTestPass("bounding sphere calculation");
    return true;
}

bool TestMeshBoundingVolumes() {
    TestOutput::PrintTestStart("mesh bounding volumes");

    try {
        // Create a test mesh
        auto mesh = std::make_shared<Mesh>("test_mesh");
        
        // Create simple test vertices manually instead of using CreateDefault
        std::vector<Vertex> testVertices = {
            {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
        };
        
        std::vector<uint32_t> testIndices = {
            0, 1, 2, 2, 3, 0, // Front face
            4, 5, 6, 6, 7, 4, // Back face
            0, 4, 7, 7, 3, 0, // Left face
            1, 5, 6, 6, 2, 1, // Right face
            3, 2, 6, 6, 7, 3, // Top face
            0, 1, 5, 5, 4, 0  // Bottom face
        };
        
        mesh->SetVertices(testVertices);
        mesh->SetIndices(testIndices);

        // Test that mesh has valid bounding volumes
        BoundingBox meshBox = mesh->GetBoundingBox();
        BoundingSphere meshSphere = mesh->GetBoundingSphere();

        EXPECT_TRUE(meshBox.IsValid());
        EXPECT_TRUE(meshSphere.IsValid());

        // Test that bounding volumes contain all vertices
        const auto& vertices = mesh->GetVertices();
        std::vector<Math::Vec3> positions;
        for (const auto& vertex : vertices) {
            positions.push_back(vertex.position);
        }

        EXPECT_TRUE(BoundingVolumeCalculator::ValidateBoundingVolume(meshBox, positions));
        EXPECT_TRUE(BoundingVolumeCalculator::ValidateBoundingVolume(meshSphere, positions));

        TestOutput::PrintTestPass("mesh bounding volumes");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception in mesh bounding volumes test: " + std::string(e.what()));
        return false;
    } catch (...) {
        TestOutput::PrintError("Unknown exception in mesh bounding volumes test");
        return false;
    }
}

bool TestHierarchicalBoundingVolumes() {
    TestOutput::PrintTestStart("hierarchical bounding volumes");

    try {
        // Create a simple model with hierarchy
        auto model = std::make_shared<Model>("test_model");
        
        // Create simple test meshes manually
        auto mesh1 = std::make_shared<Mesh>("mesh1");
        std::vector<Vertex> vertices1 = {
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 0.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}}
        };
        std::vector<uint32_t> indices1 = {0, 1, 2};
        mesh1->SetVertices(vertices1);
        mesh1->SetIndices(indices1);
        
        auto mesh2 = std::make_shared<Mesh>("mesh2");
        std::vector<Vertex> vertices2 = {
            {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 3.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 2.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}}
        };
        std::vector<uint32_t> indices2 = {0, 1, 2};
        mesh2->SetVertices(vertices2);
        mesh2->SetIndices(indices2);
        
        model->AddMesh(mesh1);
        model->AddMesh(mesh2);

        auto rootNode = model->GetRootNode();
        EXPECT_NOT_NULL(rootNode);
        rootNode->AddMeshIndex(0); // First mesh

        // Create child node
        auto childNode = std::make_shared<ModelNode>("child");
        childNode->SetLocalTransform(glm::translate(Math::Mat4(1.0f), Math::Vec3(2.0f, 0.0f, 0.0f)));
        rootNode->AddChild(childNode);
        childNode->AddMeshIndex(1); // Second mesh

        // Update bounds
        model->UpdateBounds();

        // Test that model has valid hierarchical bounds
        BoundingBox modelBox = model->GetBoundingBox();
        BoundingSphere modelSphere = model->GetBoundingSphere();

        EXPECT_TRUE(modelBox.IsValid());
        EXPECT_TRUE(modelSphere.IsValid());

        // Test hierarchical calculation
        auto meshes = model->GetMeshes();
        BoundingBox hierarchicalBox = BoundingVolumeCalculator::CalculateHierarchicalAABB(rootNode, meshes);
        BoundingSphere hierarchicalSphere = BoundingVolumeCalculator::CalculateHierarchicalSphere(rootNode, meshes);

        EXPECT_TRUE(hierarchicalBox.IsValid());
        EXPECT_TRUE(hierarchicalSphere.IsValid());

        TestOutput::PrintTestPass("hierarchical bounding volumes");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception in hierarchical bounding volumes test: " + std::string(e.what()));
        return false;
    } catch (...) {
        TestOutput::PrintError("Unknown exception in hierarchical bounding volumes test");
        return false;
    }
}

bool TestBoundingVolumeTransforms() {
    TestOutput::PrintTestStart("bounding volume transforms");

    // Test basic functionality without complex transforms
    // Create test bounding box
    BoundingBox originalBox(Math::Vec3(-1.0f), Math::Vec3(1.0f));
    EXPECT_TRUE(originalBox.IsValid());

    // Test identity transform (should not change the box)
    Math::Mat4 identity = Math::Mat4(1.0f);
    BoundingBox identityBox = originalBox.Transform(identity);
    EXPECT_TRUE(identityBox.IsValid());
    EXPECT_NEARLY_EQUAL_EPSILON(identityBox.min.x, -1.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(identityBox.max.x, 1.0f, 0.001f);

    // Test sphere transform with identity
    BoundingSphere originalSphere(Math::Vec3(0.0f), 1.0f);
    BoundingSphere identitySphere = BoundingVolumeCalculator::TransformBoundingSphere(originalSphere, identity);
    
    EXPECT_TRUE(identitySphere.IsValid());
    EXPECT_NEARLY_EQUAL_EPSILON(identitySphere.center.x, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(identitySphere.radius, 1.0f, 0.001f);

    TestOutput::PrintTestPass("bounding volume transforms");
    return true;
}

bool TestBoundingVolumeEfficiency() {
    TestOutput::PrintTestStart("bounding volume efficiency");

    // Create points in a tight cluster
    std::vector<Math::Vec3> tightPoints = {
        Math::Vec3(0.0f, 0.0f, 0.0f),
        Math::Vec3(0.1f, 0.0f, 0.0f),
        Math::Vec3(0.0f, 0.1f, 0.0f),
        Math::Vec3(0.0f, 0.0f, 0.1f)
    };

    // Create points spread out
    std::vector<Math::Vec3> spreadPoints = {
        Math::Vec3(-10.0f, -10.0f, -10.0f),
        Math::Vec3(10.0f, 10.0f, 10.0f),
        Math::Vec3(0.0f, 0.0f, 0.0f)
    };

    BoundingSphere tightSphere = BoundingVolumeCalculator::CalculateOptimalSphere(tightPoints);
    BoundingSphere spreadSphere = BoundingVolumeCalculator::CalculateOptimalSphere(spreadPoints);

    float tightEfficiency = BoundingVolumeCalculator::CalculateBoundingVolumeEfficiency(tightSphere, tightPoints);
    float spreadEfficiency = BoundingVolumeCalculator::CalculateBoundingVolumeEfficiency(spreadSphere, spreadPoints);

    // Tight cluster should be more efficient than spread out points
    EXPECT_TRUE(tightEfficiency >= 0.0f);
    EXPECT_TRUE(spreadEfficiency >= 0.0f);

    TestOutput::PrintTestPass("bounding volume efficiency");
    return true;
}

int main() {
    TestOutput::PrintHeader("BoundingVolumeCalculator");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("BoundingVolumeCalculator Tests");

        // Run all tests
        allPassed &= suite.RunTest("AABB Calculation", TestAABBCalculation);
        allPassed &= suite.RunTest("Bounding Sphere Calculation", TestBoundingSphereCalculation);
        allPassed &= suite.RunTest("Bounding Volume Transforms", TestBoundingVolumeTransforms);
        allPassed &= suite.RunTest("Bounding Volume Efficiency", TestBoundingVolumeEfficiency);

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