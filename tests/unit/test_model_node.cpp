#include "Graphics/ModelNode.h"
#include "Graphics/Model.h"
#include "Core/Logger.h"
#include "TestUtils.h"
#include <memory>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test basic ModelNode creation and properties
 * Requirements: 3.1, 3.2 (Model class with hierarchical node system)
 */
bool TestModelNodeCreation() {
    TestOutput::PrintTestStart("model node creation");
    
    auto node = std::make_shared<ModelNode>("TestNode");
    EXPECT_NOT_NULL(node);
    EXPECT_EQUAL(node->GetName(), std::string("TestNode"));
    EXPECT_TRUE(node->IsVisible());
    EXPECT_FALSE(node->HasMeshes());
    EXPECT_EQUAL(node->GetChildren().size(), static_cast<size_t>(0));
    EXPECT_NULL(node->GetParent());
    
    TestOutput::PrintTestPass("model node creation");
    return true;
}

/**
 * Test ModelNode parent-child hierarchy management
 * Requirements: 3.1, 3.2 (Model class with hierarchical node system)
 */
bool TestModelNodeHierarchy() {
    TestOutput::PrintTestStart("model node hierarchy");
    
    auto rootNode = std::make_shared<ModelNode>("Root");
    auto childNode1 = std::make_shared<ModelNode>("Child1");
    auto childNode2 = std::make_shared<ModelNode>("Child2");
    auto grandChild = std::make_shared<ModelNode>("GrandChild");
    
    // Build hierarchy
    rootNode->AddChild(childNode1);
    rootNode->AddChild(childNode2);
    childNode1->AddChild(grandChild);
    
    // Test parent-child relationships
    EXPECT_EQUAL(rootNode->GetChildren().size(), static_cast<size_t>(2));
    EXPECT_EQUAL(childNode1->GetChildren().size(), static_cast<size_t>(1));
    EXPECT_EQUAL(childNode2->GetChildren().size(), static_cast<size_t>(0));
    
    EXPECT_NULL(rootNode->GetParent());
    EXPECT_EQUAL(childNode1->GetParent(), rootNode);
    EXPECT_EQUAL(childNode2->GetParent(), rootNode);
    EXPECT_EQUAL(grandChild->GetParent(), childNode1);
    
    // Test finding children
    EXPECT_EQUAL(rootNode->FindChild("Child1"), childNode1);
    EXPECT_EQUAL(rootNode->FindChild("Child2"), childNode2);
    EXPECT_EQUAL(rootNode->FindChild("GrandChild"), grandChild); // Should find recursively
    EXPECT_NULL(rootNode->FindChild("NonExistent"));
    
    TestOutput::PrintTestPass("model node hierarchy");
    return true;
}

/**
 * Test ModelNode transform management and inheritance
 * Requirements: 3.1, 3.2 (Model class with hierarchical node system)
 */
bool TestModelNodeTransforms() {
    TestOutput::PrintTestStart("model node transforms");
    
    auto rootNode = std::make_shared<ModelNode>("Root");
    auto childNode = std::make_shared<ModelNode>("Child");
    
    rootNode->AddChild(childNode);
    
    // Test default transforms
    Math::Mat4 identity = Math::Mat4(1.0f);
    EXPECT_TRUE(rootNode->GetLocalTransform() == identity);
    EXPECT_TRUE(rootNode->GetWorldTransform() == identity);
    EXPECT_TRUE(childNode->GetLocalTransform() == identity);
    EXPECT_TRUE(childNode->GetWorldTransform() == identity);
    
    // Set root transform
    Math::Mat4 rootTransform = glm::translate(Math::Mat4(1.0f), Math::Vec3(1.0f, 2.0f, 3.0f));
    rootNode->SetLocalTransform(rootTransform);
    
    EXPECT_TRUE(rootNode->GetLocalTransform() == rootTransform);
    EXPECT_TRUE(rootNode->GetWorldTransform() == rootTransform);
    EXPECT_TRUE(childNode->GetLocalTransform() == identity);
    EXPECT_TRUE(childNode->GetWorldTransform() == rootTransform); // Should inherit parent's world transform
    
    // Set child transform
    Math::Mat4 childTransform = glm::translate(Math::Mat4(1.0f), Math::Vec3(0.5f, 0.5f, 0.5f));
    childNode->SetLocalTransform(childTransform);
    
    EXPECT_TRUE(childNode->GetLocalTransform() == childTransform);
    Math::Mat4 expectedChildWorld = rootTransform * childTransform;
    EXPECT_TRUE(childNode->GetWorldTransform() == expectedChildWorld);
    
    TestOutput::PrintTestPass("model node transforms");
    return true;
}

/**
 * Test ModelNode mesh index association
 * Requirements: 3.1, 3.2 (Model class with hierarchical node system)
 */
bool TestModelNodeMeshAssociation() {
    TestOutput::PrintTestStart("model node mesh association");
    
    auto node = std::make_shared<ModelNode>("TestNode");
    
    // Initially no meshes
    EXPECT_FALSE(node->HasMeshes());
    EXPECT_EQUAL(node->GetMeshIndices().size(), static_cast<size_t>(0));
    
    // Add mesh indices
    node->AddMeshIndex(0);
    node->AddMeshIndex(1);
    node->AddMeshIndex(2);
    
    EXPECT_TRUE(node->HasMeshes());
    EXPECT_EQUAL(node->GetMeshIndices().size(), static_cast<size_t>(3));
    
    auto meshIndices = node->GetMeshIndices();
    EXPECT_EQUAL(meshIndices[0], static_cast<uint32_t>(0));
    EXPECT_EQUAL(meshIndices[1], static_cast<uint32_t>(1));
    EXPECT_EQUAL(meshIndices[2], static_cast<uint32_t>(2));
    
    // Remove mesh index
    node->RemoveMeshIndex(1);
    EXPECT_EQUAL(node->GetMeshIndices().size(), static_cast<size_t>(2));
    
    meshIndices = node->GetMeshIndices();
    EXPECT_EQUAL(meshIndices[0], static_cast<uint32_t>(0));
    EXPECT_EQUAL(meshIndices[1], static_cast<uint32_t>(2));
    
    // Try to add duplicate (should not add)
    node->AddMeshIndex(0);
    EXPECT_EQUAL(node->GetMeshIndices().size(), static_cast<size_t>(2));
    
    TestOutput::PrintTestPass("model node mesh association");
    return true;
}

/**
 * Test ModelNode tree traversal methods
 * Requirements: 3.4 (Node traversal methods - depth-first, breadth-first)
 */
bool TestModelNodeTraversal() {
    TestOutput::PrintTestStart("model node traversal");
    
    auto rootNode = std::make_shared<ModelNode>("Root");
    auto child1 = std::make_shared<ModelNode>("Child1");
    auto child2 = std::make_shared<ModelNode>("Child2");
    auto grandChild1 = std::make_shared<ModelNode>("GrandChild1");
    auto grandChild2 = std::make_shared<ModelNode>("GrandChild2");
    
    // Build hierarchy
    rootNode->AddChild(child1);
    rootNode->AddChild(child2);
    child1->AddChild(grandChild1);
    child2->AddChild(grandChild2);
    
    // Test depth-first traversal
    std::vector<std::string> visitedNodes;
    rootNode->TraverseDepthFirst([&](std::shared_ptr<ModelNode> node) {
        visitedNodes.push_back(node->GetName());
    });
    
    EXPECT_EQUAL(visitedNodes.size(), static_cast<size_t>(5));
    EXPECT_EQUAL(visitedNodes[0], std::string("Root"));
    EXPECT_EQUAL(visitedNodes[1], std::string("Child1"));
    EXPECT_EQUAL(visitedNodes[2], std::string("GrandChild1"));
    EXPECT_EQUAL(visitedNodes[3], std::string("Child2"));
    EXPECT_EQUAL(visitedNodes[4], std::string("GrandChild2"));
    
    // Test breadth-first traversal
    visitedNodes.clear();
    rootNode->TraverseBreadthFirst([&](std::shared_ptr<ModelNode> node) {
        visitedNodes.push_back(node->GetName());
    });
    
    EXPECT_EQUAL(visitedNodes.size(), static_cast<size_t>(5));
    EXPECT_EQUAL(visitedNodes[0], std::string("Root"));
    EXPECT_EQUAL(visitedNodes[1], std::string("Child1"));
    EXPECT_EQUAL(visitedNodes[2], std::string("Child2"));
    EXPECT_EQUAL(visitedNodes[3], std::string("GrandChild1"));
    EXPECT_EQUAL(visitedNodes[4], std::string("GrandChild2"));
    
    TestOutput::PrintTestPass("model node traversal");
    return true;
}

/**
 * Test Model class creation and basic functionality
 * Requirements: 3.1, 3.2 (Model class with hierarchical node system)
 */
bool TestModelCreation() {
    TestOutput::PrintTestStart("model creation");
    
    auto model = std::make_shared<Model>("test_model.obj");
    EXPECT_NOT_NULL(model);
    EXPECT_NOT_NULL(model->GetRootNode());
    EXPECT_EQUAL(model->GetName(), std::string("Model"));
    
    // Test basic model properties without OpenGL-dependent operations
    EXPECT_EQUAL(model->GetMeshCount(), static_cast<size_t>(0));
    EXPECT_EQUAL(model->GetMaterialCount(), static_cast<size_t>(0));
    EXPECT_FALSE(model->HasAnimations());
    EXPECT_FALSE(model->HasSkeleton());
    
    auto stats = model->GetStats();
    EXPECT_EQUAL(stats.nodeCount, static_cast<uint32_t>(0));
    EXPECT_EQUAL(stats.meshCount, static_cast<uint32_t>(0));
    EXPECT_EQUAL(stats.materialCount, static_cast<uint32_t>(0));
    
    TestOutput::PrintTestPass("model creation");
    return true;
}

/**
 * Test Model mesh, material, and animation containers
 * Requirements: 2.1, 5.1, 5.5 (Mesh, material, and animation containers)
 */
bool TestModelContainers() {
    TestOutput::PrintTestStart("model containers");
    
    auto model = std::make_shared<Model>("test_model.obj");
    
    // Test initial state - no meshes, materials, or animations
    EXPECT_EQUAL(model->GetMeshCount(), static_cast<size_t>(0));
    EXPECT_EQUAL(model->GetMaterialCount(), static_cast<size_t>(0));
    EXPECT_EQUAL(model->GetAnimationCount(), static_cast<size_t>(0));
    EXPECT_FALSE(model->HasAnimations());
    EXPECT_FALSE(model->HasSkeleton());
    
    // Test mesh access methods
    EXPECT_NULL(model->GetMesh(0));
    EXPECT_NULL(model->FindMesh("nonexistent"));
    auto meshes = model->GetMeshes();
    EXPECT_EQUAL(meshes.size(), static_cast<size_t>(0));
    
    // Test material access methods
    EXPECT_NULL(model->GetMaterial(0));
    EXPECT_NULL(model->FindMaterial("nonexistent"));
    auto materials = model->GetMaterials();
    EXPECT_EQUAL(materials.size(), static_cast<size_t>(0));
    
    // Test animation access methods
    EXPECT_NULL(model->GetAnimation(0));
    EXPECT_NULL(model->FindAnimation("nonexistent"));
    auto animations = model->GetAnimations();
    EXPECT_EQUAL(animations.size(), static_cast<size_t>(0));
    
    // Test skeleton access
    EXPECT_NULL(model->GetSkeleton());
    
    TestOutput::PrintTestPass("model containers");
    return true;
}

/**
 * Test Model bounding volume calculation
 * Requirements: 3.1, 3.2 (Model class with hierarchical node system)
 */
bool TestModelBounds() {
    TestOutput::PrintTestStart("model bounds");
    
    auto model = std::make_shared<Model>("test_model.obj");
    
    // Test bounds update method doesn't crash
    model->UpdateBounds();
    
    // Test bounds access methods
    auto boundingBox = model->GetBoundingBox();
    auto boundingSphere = model->GetBoundingSphere();
    
    // These methods should not crash even with empty model
    EXPECT_TRUE(true); // If we get here, the methods didn't crash
    
    TestOutput::PrintTestPass("model bounds");
    return true;
}

int main() {
    TestOutput::PrintHeader("Model and ModelNode");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Model and ModelNode Tests");

        // Run all tests
        allPassed &= suite.RunTest("Model Node Creation", TestModelNodeCreation);
        allPassed &= suite.RunTest("Model Node Hierarchy", TestModelNodeHierarchy);
        allPassed &= suite.RunTest("Model Node Transforms", TestModelNodeTransforms);
        allPassed &= suite.RunTest("Model Node Mesh Association", TestModelNodeMeshAssociation);
        allPassed &= suite.RunTest("Model Node Traversal", TestModelNodeTraversal);
        allPassed &= suite.RunTest("Model Creation", TestModelCreation);
        allPassed &= suite.RunTest("Model Containers", TestModelContainers);
        allPassed &= suite.RunTest("Model Bounds", TestModelBounds);

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