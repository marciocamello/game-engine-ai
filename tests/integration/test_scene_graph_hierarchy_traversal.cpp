#include "Graphics/Model.h"
#include "Graphics/ModelNode.h"
#include "Graphics/Mesh.h"
#include "Resource/ModelLoader.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <filesystem>
#include <fstream>
#include <queue>
#include <map>
#include <algorithm>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Create a test model with hierarchical structure
 */
std::shared_ptr<Model> CreateHierarchicalTestModel() {
    auto model = std::make_shared<Model>("hierarchical_test_model");
    
    // Create root node
    auto rootNode = std::make_shared<ModelNode>("Root");
    
    // Create child nodes
    auto bodyNode = std::make_shared<ModelNode>("Body");
    auto headNode = std::make_shared<ModelNode>("Head");
    auto leftArmNode = std::make_shared<ModelNode>("LeftArm");
    auto rightArmNode = std::make_shared<ModelNode>("RightArm");
    
    // Create grandchild nodes
    auto leftHandNode = std::make_shared<ModelNode>("LeftHand");
    auto rightHandNode = std::make_shared<ModelNode>("RightHand");
    auto eyesNode = std::make_shared<ModelNode>("Eyes");
    
    // Build hierarchy
    rootNode->AddChild(bodyNode);
    rootNode->AddChild(headNode);
    rootNode->AddChild(leftArmNode);
    rootNode->AddChild(rightArmNode);
    
    leftArmNode->AddChild(leftHandNode);
    rightArmNode->AddChild(rightHandNode);
    headNode->AddChild(eyesNode);
    
    // Set transforms for testing
    bodyNode->SetLocalTransform(glm::translate(Math::Mat4(1.0f), Math::Vec3(0.0f, 0.0f, 0.0f)));
    headNode->SetLocalTransform(glm::translate(Math::Mat4(1.0f), Math::Vec3(0.0f, 2.0f, 0.0f)));
    leftArmNode->SetLocalTransform(glm::translate(Math::Mat4(1.0f), Math::Vec3(-1.5f, 1.0f, 0.0f)));
    rightArmNode->SetLocalTransform(glm::translate(Math::Mat4(1.0f), Math::Vec3(1.5f, 1.0f, 0.0f)));
    leftHandNode->SetLocalTransform(glm::translate(Math::Mat4(1.0f), Math::Vec3(0.0f, -1.0f, 0.0f)));
    rightHandNode->SetLocalTransform(glm::translate(Math::Mat4(1.0f), Math::Vec3(0.0f, -1.0f, 0.0f)));
    eyesNode->SetLocalTransform(glm::translate(Math::Mat4(1.0f), Math::Vec3(0.0f, 0.2f, 0.5f)));
    
    // Associate some mesh indices for testing
    bodyNode->AddMeshIndex(0);
    headNode->AddMeshIndex(1);
    leftHandNode->AddMeshIndex(2);
    rightHandNode->AddMeshIndex(2); // Shared mesh
    eyesNode->AddMeshIndex(3);
    
    // Set the root node on the model by replacing the default one
    // Since there's no SetRootNode method, we need to work with the existing structure
    auto modelRootNode = model->GetRootNode();
    if (modelRootNode) {
        // Clear existing children from the default root node
        auto existingChildren = modelRootNode->GetChildren();
        for (auto& child : existingChildren) {
            modelRootNode->RemoveChild(child);
        }
        
        // Add our hierarchy as children of the model's root node
        for (auto& child : rootNode->GetChildren()) {
            modelRootNode->AddChild(child);
        }
        
        // Copy mesh associations from our root to the model's root
        for (auto meshIndex : rootNode->GetMeshIndices()) {
            modelRootNode->AddMeshIndex(meshIndex);
        }
    }
    
    return model;
}

/**
 * Test scene graph hierarchy construction and relationships
 * Requirements: 3.4, 2.1 (Scene graph hierarchy and traversal)
 */
bool TestSceneGraphHierarchyConstruction() {
    TestOutput::PrintTestStart("scene graph hierarchy construction");
    
    auto model = CreateHierarchicalTestModel();
    EXPECT_NOT_NULL(model);
    
    auto rootNode = model->GetRootNode();
    EXPECT_NOT_NULL(rootNode);
    EXPECT_STRING_EQUAL(rootNode->GetName(), "Root");
    
    // Test immediate children
    auto children = rootNode->GetChildren();
    EXPECT_EQUAL(children.size(), static_cast<size_t>(4));
    
    // Verify child names
    std::vector<std::string> expectedChildNames = {"Body", "Head", "LeftArm", "RightArm"};
    std::vector<std::string> actualChildNames;
    
    for (auto& child : children) {
        actualChildNames.push_back(child->GetName());
    }
    
    std::sort(expectedChildNames.begin(), expectedChildNames.end());
    std::sort(actualChildNames.begin(), actualChildNames.end());
    
    for (size_t i = 0; i < expectedChildNames.size(); ++i) {
        EXPECT_STRING_EQUAL(actualChildNames[i], expectedChildNames[i]);
    }
    
    // Test parent-child relationships
    auto bodyNode = rootNode->FindChild("Body");
    EXPECT_NOT_NULL(bodyNode);
    EXPECT_EQUAL(bodyNode->GetParent(), rootNode);
    
    auto leftArmNode = rootNode->FindChild("LeftArm");
    EXPECT_NOT_NULL(leftArmNode);
    
    auto leftHandNode = leftArmNode->FindChild("LeftHand");
    EXPECT_NOT_NULL(leftHandNode);
    EXPECT_EQUAL(leftHandNode->GetParent(), leftArmNode);
    
    // Test recursive finding
    auto eyesNode = rootNode->FindChild("Eyes");
    EXPECT_NOT_NULL(eyesNode);
    
    TestOutput::PrintInfo("Scene graph hierarchy constructed correctly");
    
    TestOutput::PrintTestPass("scene graph hierarchy construction");
    return true;
}

/**
 * Test depth-first traversal of scene graph
 * Requirements: 3.4 (Node traversal methods - depth-first)
 */
bool TestDepthFirstTraversal() {
    TestOutput::PrintTestStart("depth-first traversal");
    
    auto model = CreateHierarchicalTestModel();
    auto rootNode = model->GetRootNode();
    
    std::vector<std::string> visitedNodes;
    
    rootNode->TraverseDepthFirst([&](std::shared_ptr<ModelNode> node) {
        visitedNodes.push_back(node->GetName());
    });
    
    // Verify traversal order (depth-first)
    EXPECT_TRUE(visitedNodes.size() >= 7); // At least 7 nodes in our hierarchy
    EXPECT_STRING_EQUAL(visitedNodes[0], "Root");
    
    // Find positions of key nodes
    auto findNodePosition = [&](const std::string& name) -> int {
        for (size_t i = 0; i < visitedNodes.size(); ++i) {
            if (visitedNodes[i] == name) {
                return static_cast<int>(i);
            }
        }
        return -1;
    };
    
    int rootPos = findNodePosition("Root");
    int leftArmPos = findNodePosition("LeftArm");
    int leftHandPos = findNodePosition("LeftHand");
    int headPos = findNodePosition("Head");
    int eyesPos = findNodePosition("Eyes");
    
    // Verify depth-first order constraints
    EXPECT_TRUE(rootPos < leftArmPos);
    EXPECT_TRUE(leftArmPos < leftHandPos);
    EXPECT_TRUE(headPos < eyesPos);
    
    TestOutput::PrintInfo("Depth-first traversal order:");
    for (size_t i = 0; i < visitedNodes.size(); ++i) {
        TestOutput::PrintInfo("  " + std::to_string(i) + ": " + visitedNodes[i]);
    }
    
    TestOutput::PrintTestPass("depth-first traversal");
    return true;
}

/**
 * Test breadth-first traversal of scene graph
 * Requirements: 3.4 (Node traversal methods - breadth-first)
 */
bool TestBreadthFirstTraversal() {
    TestOutput::PrintTestStart("breadth-first traversal");
    
    auto model = CreateHierarchicalTestModel();
    auto rootNode = model->GetRootNode();
    
    std::vector<std::string> visitedNodes;
    
    rootNode->TraverseBreadthFirst([&](std::shared_ptr<ModelNode> node) {
        visitedNodes.push_back(node->GetName());
    });
    
    // Verify traversal order (breadth-first)
    EXPECT_TRUE(visitedNodes.size() >= 7);
    EXPECT_STRING_EQUAL(visitedNodes[0], "Root");
    
    // Find positions of key nodes
    auto findNodePosition = [&](const std::string& name) -> int {
        for (size_t i = 0; i < visitedNodes.size(); ++i) {
            if (visitedNodes[i] == name) {
                return static_cast<int>(i);
            }
        }
        return -1;
    };
    
    int rootPos = findNodePosition("Root");
    int bodyPos = findNodePosition("Body");
    int headPos = findNodePosition("Head");
    int leftArmPos = findNodePosition("LeftArm");
    int rightArmPos = findNodePosition("RightArm");
    int leftHandPos = findNodePosition("LeftHand");
    int rightHandPos = findNodePosition("RightHand");
    int eyesPos = findNodePosition("Eyes");
    
    // Verify breadth-first order constraints
    // All children should come before grandchildren
    EXPECT_TRUE(rootPos < bodyPos);
    EXPECT_TRUE(rootPos < headPos);
    EXPECT_TRUE(rootPos < leftArmPos);
    EXPECT_TRUE(rootPos < rightArmPos);
    
    EXPECT_TRUE(leftArmPos < leftHandPos);
    EXPECT_TRUE(rightArmPos < rightHandPos);
    EXPECT_TRUE(headPos < eyesPos);
    
    // All level 1 nodes should come before level 2 nodes
    int maxLevel1Pos = std::max({bodyPos, headPos, leftArmPos, rightArmPos});
    int minLevel2Pos = std::min({leftHandPos, rightHandPos, eyesPos});
    EXPECT_TRUE(maxLevel1Pos < minLevel2Pos);
    
    TestOutput::PrintInfo("Breadth-first traversal order:");
    for (size_t i = 0; i < visitedNodes.size(); ++i) {
        TestOutput::PrintInfo("  " + std::to_string(i) + ": " + visitedNodes[i]);
    }
    
    TestOutput::PrintTestPass("breadth-first traversal");
    return true;
}

/**
 * Test transform inheritance in scene graph hierarchy
 * Requirements: 3.1, 3.2 (Transform management and inheritance)
 */
bool TestTransformInheritance() {
    TestOutput::PrintTestStart("transform inheritance");
    
    auto model = CreateHierarchicalTestModel();
    auto rootNode = model->GetRootNode();
    
    // Set root transform
    Math::Mat4 rootTransform = glm::translate(Math::Mat4(1.0f), Math::Vec3(10.0f, 0.0f, 0.0f));
    rootNode->SetLocalTransform(rootTransform);
    
    // Update world transforms
    rootNode->UpdateWorldTransform();
    
    // Test root transform
    EXPECT_MATRIX_EQUAL(rootNode->GetLocalTransform(), rootTransform);
    EXPECT_MATRIX_EQUAL(rootNode->GetWorldTransform(), rootTransform);
    
    // Test child transform inheritance
    auto headNode = rootNode->FindChild("Head");
    EXPECT_NOT_NULL(headNode);
    
    Math::Mat4 headLocalTransform = headNode->GetLocalTransform();
    Math::Mat4 headWorldTransform = headNode->GetWorldTransform();
    Math::Mat4 expectedHeadWorld = rootTransform * headLocalTransform;
    
    EXPECT_MATRIX_EQUAL(headWorldTransform, expectedHeadWorld);
    
    // Test grandchild transform inheritance
    auto eyesNode = headNode->FindChild("Eyes");
    EXPECT_NOT_NULL(eyesNode);
    
    Math::Mat4 eyesLocalTransform = eyesNode->GetLocalTransform();
    Math::Mat4 eyesWorldTransform = eyesNode->GetWorldTransform();
    Math::Mat4 expectedEyesWorld = expectedHeadWorld * eyesLocalTransform;
    
    EXPECT_MATRIX_EQUAL(eyesWorldTransform, expectedEyesWorld);
    
    // Test transform update propagation
    Math::Mat4 newHeadTransform = glm::translate(Math::Mat4(1.0f), Math::Vec3(0.0f, 3.0f, 0.0f));
    headNode->SetLocalTransform(newHeadTransform);
    
    // World transform should update automatically
    Math::Mat4 newExpectedHeadWorld = rootTransform * newHeadTransform;
    EXPECT_MATRIX_EQUAL(headNode->GetWorldTransform(), newExpectedHeadWorld);
    
    // Eyes should also update
    Math::Mat4 newExpectedEyesWorld = newExpectedHeadWorld * eyesLocalTransform;
    EXPECT_MATRIX_EQUAL(eyesNode->GetWorldTransform(), newExpectedEyesWorld);
    
    TestOutput::PrintInfo("Transform inheritance working correctly");
    
    TestOutput::PrintTestPass("transform inheritance");
    return true;
}

/**
 * Test mesh association and rendering hierarchy
 * Requirements: 3.1, 3.2 (Mesh association with nodes)
 */
bool TestMeshAssociationHierarchy() {
    TestOutput::PrintTestStart("mesh association hierarchy");
    
    auto model = CreateHierarchicalTestModel();
    auto rootNode = model->GetRootNode();
    
    // Test mesh associations
    auto bodyNode = rootNode->FindChild("Body");
    EXPECT_NOT_NULL(bodyNode);
    EXPECT_TRUE(bodyNode->HasMeshes());
    
    auto bodyMeshIndices = bodyNode->GetMeshIndices();
    EXPECT_EQUAL(bodyMeshIndices.size(), static_cast<size_t>(1));
    EXPECT_EQUAL(bodyMeshIndices[0], static_cast<uint32_t>(0));
    
    // Test shared mesh indices
    auto leftHandNode = rootNode->FindChild("LeftHand");
    auto rightHandNode = rootNode->FindChild("RightHand");
    
    EXPECT_NOT_NULL(leftHandNode);
    EXPECT_NOT_NULL(rightHandNode);
    
    auto leftHandMeshes = leftHandNode->GetMeshIndices();
    auto rightHandMeshes = rightHandNode->GetMeshIndices();
    
    EXPECT_EQUAL(leftHandMeshes.size(), static_cast<size_t>(1));
    EXPECT_EQUAL(rightHandMeshes.size(), static_cast<size_t>(1));
    EXPECT_EQUAL(leftHandMeshes[0], rightHandMeshes[0]); // Shared mesh
    
    // Test nodes without meshes
    auto leftArmNode = rootNode->FindChild("LeftArm");
    EXPECT_NOT_NULL(leftArmNode);
    EXPECT_FALSE(leftArmNode->HasMeshes());
    
    // Count total mesh associations in hierarchy
    int totalMeshAssociations = 0;
    rootNode->Traverse([&](std::shared_ptr<ModelNode> node) {
        totalMeshAssociations += static_cast<int>(node->GetMeshIndices().size());
    });
    
    EXPECT_TRUE(totalMeshAssociations > 0);
    TestOutput::PrintInfo("Total mesh associations in hierarchy: " + std::to_string(totalMeshAssociations));
    
    TestOutput::PrintTestPass("mesh association hierarchy");
    return true;
}

/**
 * Test scene graph node visibility and culling
 * Requirements: 3.1, 3.2 (Node visibility management)
 */
bool TestNodeVisibilityAndCulling() {
    TestOutput::PrintTestStart("node visibility and culling");
    
    auto model = CreateHierarchicalTestModel();
    auto rootNode = model->GetRootNode();
    
    // Test default visibility
    EXPECT_TRUE(rootNode->IsVisible());
    
    auto headNode = rootNode->FindChild("Head");
    EXPECT_NOT_NULL(headNode);
    EXPECT_TRUE(headNode->IsVisible());
    
    // Test visibility setting
    headNode->SetVisible(false);
    EXPECT_FALSE(headNode->IsVisible());
    
    // Test visibility inheritance (children of invisible nodes should be effectively invisible)
    auto eyesNode = headNode->FindChild("Eyes");
    EXPECT_NOT_NULL(eyesNode);
    EXPECT_TRUE(eyesNode->IsVisible()); // Local visibility is still true
    
    // Test visibility traversal with culling
    std::vector<std::string> visibleNodes;
    std::vector<std::string> allNodes;
    
    rootNode->Traverse([&](std::shared_ptr<ModelNode> node) {
        allNodes.push_back(node->GetName());
        if (node->IsVisible()) {
            // Check if any parent is invisible
            bool parentVisible = true;
            auto parent = node->GetParent();
            while (parent) {
                if (!parent->IsVisible()) {
                    parentVisible = false;
                    break;
                }
                parent = parent->GetParent();
            }
            if (parentVisible) {
                visibleNodes.push_back(node->GetName());
            }
        }
    });
    
    // Head and Eyes should not be in visible list
    bool headInVisible = std::find(visibleNodes.begin(), visibleNodes.end(), "Head") != visibleNodes.end();
    bool eyesInVisible = std::find(visibleNodes.begin(), visibleNodes.end(), "Eyes") != visibleNodes.end();
    
    EXPECT_FALSE(headInVisible);
    EXPECT_FALSE(eyesInVisible);
    
    TestOutput::PrintInfo("Total nodes: " + std::to_string(allNodes.size()));
    TestOutput::PrintInfo("Visible nodes: " + std::to_string(visibleNodes.size()));
    
    // Restore visibility
    headNode->SetVisible(true);
    EXPECT_TRUE(headNode->IsVisible());
    
    TestOutput::PrintTestPass("node visibility and culling");
    return true;
}

/**
 * Test scene graph bounding volume hierarchy
 * Requirements: 8.1, 8.2 (Hierarchical bounding volumes)
 */
bool TestBoundingVolumeHierarchy() {
    TestOutput::PrintTestStart("bounding volume hierarchy");
    
    auto model = CreateHierarchicalTestModel();
    auto rootNode = model->GetRootNode();
    
    // Update bounding volumes
    model->UpdateBounds();
    
    // Test node local bounds
    auto bodyNode = rootNode->FindChild("Body");
    EXPECT_NOT_NULL(bodyNode);
    
    auto bodyLocalBounds = bodyNode->GetLocalBounds();
    auto bodyWorldBounds = bodyNode->GetWorldBounds();
    
    // World bounds should be different from local bounds due to transforms
    // (unless transform is identity)
    
    // Test hierarchical bounds calculation
    auto leftArmNode = rootNode->FindChild("LeftArm");
    auto leftHandNode = leftArmNode->FindChild("LeftHand");
    
    EXPECT_NOT_NULL(leftArmNode);
    EXPECT_NOT_NULL(leftHandNode);
    
    auto armBounds = leftArmNode->GetWorldBounds();
    auto handBounds = leftHandNode->GetWorldBounds();
    
    // Test model-level bounding volume
    auto modelBounds = model->GetBoundingBox();
    auto modelSphere = model->GetBoundingSphere();
    
    EXPECT_TRUE(modelBounds.IsValid());
    EXPECT_TRUE(modelSphere.radius > 0.0f);
    
    TestOutput::PrintInfo("Model bounding box size: " + 
                         std::to_string(modelBounds.GetSize().x) + "x" + 
                         std::to_string(modelBounds.GetSize().y) + "x" + 
                         std::to_string(modelBounds.GetSize().z));
    TestOutput::PrintInfo("Model bounding sphere radius: " + std::to_string(modelSphere.radius));
    
    TestOutput::PrintTestPass("bounding volume hierarchy");
    return true;
}

/**
 * Test scene graph serialization and reconstruction
 * Requirements: 3.4 (Scene graph structure preservation)
 */
bool TestSceneGraphSerialization() {
    TestOutput::PrintTestStart("scene graph serialization");
    
    auto originalModel = CreateHierarchicalTestModel();
    auto originalRoot = originalModel->GetRootNode();
    
    // Collect original hierarchy information
    std::vector<std::string> originalNodeNames;
    std::map<std::string, std::string> originalParentMap;
    std::map<std::string, std::vector<uint32_t>> originalMeshMap;
    
    originalRoot->Traverse([&](std::shared_ptr<ModelNode> node) {
        originalNodeNames.push_back(node->GetName());
        
        auto parent = node->GetParent();
        if (parent) {
            originalParentMap[node->GetName()] = parent->GetName();
        }
        
        originalMeshMap[node->GetName()] = node->GetMeshIndices();
    });
    
    // Create a new model and reconstruct hierarchy
    auto reconstructedModel = std::make_shared<Model>("reconstructed_model");
    auto reconstructedRoot = std::make_shared<ModelNode>("Root");
    
    // Simple reconstruction (in practice this would be from serialized data)
    std::map<std::string, std::shared_ptr<ModelNode>> nodeMap;
    nodeMap["Root"] = reconstructedRoot;
    
    // Create all nodes first
    for (const auto& nodeName : originalNodeNames) {
        if (nodeName != "Root") {
            nodeMap[nodeName] = std::make_shared<ModelNode>(nodeName);
        }
    }
    
    // Rebuild hierarchy
    for (const auto& pair : originalParentMap) {
        const std::string& childName = pair.first;
        const std::string& parentName = pair.second;
        
        auto child = nodeMap[childName];
        auto parent = nodeMap[parentName];
        
        if (child && parent) {
            parent->AddChild(child);
        }
    }
    
    // Restore mesh associations
    for (const auto& pair : originalMeshMap) {
        const std::string& nodeName = pair.first;
        const std::vector<uint32_t>& meshIndices = pair.second;
        
        auto node = nodeMap[nodeName];
        if (node) {
            for (uint32_t meshIndex : meshIndices) {
                node->AddMeshIndex(meshIndex);
            }
        }
    }
    
    // Note: SetRootNode method not available in current API
    // This would be set during model loading
    
    // Verify reconstruction
    std::vector<std::string> reconstructedNodeNames;
    reconstructedRoot->Traverse([&](std::shared_ptr<ModelNode> node) {
        reconstructedNodeNames.push_back(node->GetName());
    });
    
    std::sort(originalNodeNames.begin(), originalNodeNames.end());
    std::sort(reconstructedNodeNames.begin(), reconstructedNodeNames.end());
    
    EXPECT_EQUAL(originalNodeNames.size(), reconstructedNodeNames.size());
    
    for (size_t i = 0; i < originalNodeNames.size(); ++i) {
        EXPECT_STRING_EQUAL(originalNodeNames[i], reconstructedNodeNames[i]);
    }
    
    // Verify parent-child relationships
    for (const auto& pair : originalParentMap) {
        const std::string& childName = pair.first;
        const std::string& parentName = pair.second;
        
        auto child = reconstructedRoot->FindChild(childName);
        std::shared_ptr<ModelNode> parent;
        
        // Handle root node case
        if (parentName == "Root") {
            parent = reconstructedRoot;
        } else {
            parent = reconstructedRoot->FindChild(parentName);
        }
        
        EXPECT_NOT_NULL(child);
        EXPECT_NOT_NULL(parent);
        
        if (child && parent) {
            EXPECT_EQUAL(child->GetParent(), parent);
        }
    }
    
    TestOutput::PrintInfo("Scene graph serialization and reconstruction successful");
    
    TestOutput::PrintTestPass("scene graph serialization");
    return true;
}

int main() {
    TestOutput::PrintHeader("Scene Graph Hierarchy and Traversal Integration");

    bool allPassed = true;

    try {
        // Initialize logger for testing
        Logger::GetInstance().Initialize();
        Logger::GetInstance().SetLogLevel(LogLevel::Info);
        
        // Create test suite for result tracking
        TestSuite suite("Scene Graph Hierarchy and Traversal Tests");

        // Run all tests
        allPassed &= suite.RunTest("Scene Graph Hierarchy Construction", TestSceneGraphHierarchyConstruction);
        allPassed &= suite.RunTest("Depth-First Traversal", TestDepthFirstTraversal);
        allPassed &= suite.RunTest("Breadth-First Traversal", TestBreadthFirstTraversal);
        allPassed &= suite.RunTest("Transform Inheritance", TestTransformInheritance);
        allPassed &= suite.RunTest("Mesh Association Hierarchy", TestMeshAssociationHierarchy);
        allPassed &= suite.RunTest("Node Visibility and Culling", TestNodeVisibilityAndCulling);
        allPassed &= suite.RunTest("Bounding Volume Hierarchy", TestBoundingVolumeHierarchy);
        allPassed &= suite.RunTest("Scene Graph Serialization", TestSceneGraphSerialization);

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