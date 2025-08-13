#include "Animation/BlendTree.h"
#include "Animation/AnimationController.h"
#include "Core/Logger.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {
namespace Animation {

    BlendTree::BlendTree(Type type) 
        : m_type(type) {
        LOG_INFO("BlendTree: Created blend tree of type " + std::to_string(static_cast<int>(type)));
    }

    BlendTree::~BlendTree() {
        LOG_INFO("BlendTree: Destroyed blend tree");
    }

    void BlendTree::SetType(Type type) {
        if (m_type != type) {
            m_type = type;
            LOG_INFO("BlendTree: Changed type to " + std::to_string(static_cast<int>(type)));
            
            // Clear nodes when changing type as they may not be compatible
            if (!m_nodes.empty()) {
                LOG_WARNING("BlendTree: Clearing nodes due to type change");
                m_nodes.clear();
            }
        }
    }

    void BlendTree::SetParameter(const std::string& parameter) {
        m_parameterX = parameter;
        m_parameterY.clear();
        LOG_INFO("BlendTree: Set parameter to '" + parameter + "'");
    }

    void BlendTree::SetParameters(const std::string& paramX, const std::string& paramY) {
        m_parameterX = paramX;
        m_parameterY = paramY;
        LOG_INFO("BlendTree: Set parameters to '" + paramX + "' and '" + paramY + "'");
    }

    void BlendTree::AddMotion(std::shared_ptr<SkeletalAnimation> animation, float threshold) {
        AddMotion(animation, threshold, animation ? animation->GetName() : "Unknown");
    }

    void BlendTree::AddMotion(std::shared_ptr<SkeletalAnimation> animation, float threshold, const std::string& name) {
        if (!animation) {
            LOG_ERROR("BlendTree: Cannot add null animation");
            return;
        }

        if (m_type != Type::Simple1D) {
            LOG_ERROR("BlendTree: Cannot add 1D motion to non-1D blend tree");
            return;
        }

        BlendTreeNode node;
        node.animation = animation;
        node.threshold = threshold;
        node.name = name;

        m_nodes.push_back(node);
        SortNodesByThreshold();

        LOG_INFO("BlendTree: Added 1D motion '" + name + "' at threshold " + std::to_string(threshold));
    }

    void BlendTree::AddMotion(std::shared_ptr<SkeletalAnimation> animation, const Math::Vec2& position) {
        AddMotion(animation, position, animation ? animation->GetName() : "Unknown");
    }

    void BlendTree::AddMotion(std::shared_ptr<SkeletalAnimation> animation, const Math::Vec2& position, const std::string& name) {
        if (!animation) {
            LOG_ERROR("BlendTree: Cannot add null animation");
            return;
        }

        if (m_type == Type::Simple1D) {
            LOG_ERROR("BlendTree: Cannot add 2D motion to 1D blend tree");
            return;
        }

        BlendTreeNode node;
        node.animation = animation;
        node.position = position;
        node.name = name;

        m_nodes.push_back(node);

        LOG_INFO("BlendTree: Added 2D motion '" + name + "' at position (" + 
                std::to_string(position.x) + ", " + std::to_string(position.y) + ")");
    }

    void BlendTree::AddChildBlendTree(std::shared_ptr<BlendTree> childTree, float threshold) {
        AddChildBlendTree(childTree, threshold, "ChildTree_" + std::to_string(m_nodes.size()));
    }

    void BlendTree::AddChildBlendTree(std::shared_ptr<BlendTree> childTree, const Math::Vec2& position) {
        AddChildBlendTree(childTree, position, "ChildTree_" + std::to_string(m_nodes.size()));
    }

    void BlendTree::AddChildBlendTree(std::shared_ptr<BlendTree> childTree, float threshold, const std::string& name) {
        if (!childTree) {
            LOG_ERROR("BlendTree: Cannot add null child blend tree");
            return;
        }

        if (m_type != Type::Simple1D) {
            LOG_ERROR("BlendTree: Cannot add 1D child blend tree to non-1D blend tree");
            return;
        }

        BlendTreeNode node;
        node.childTree = childTree;
        node.threshold = threshold;
        node.name = name;

        m_nodes.push_back(node);
        SortNodesByThreshold();

        LOG_INFO("BlendTree: Added 1D child blend tree '" + name + "' at threshold " + std::to_string(threshold));
    }

    void BlendTree::AddChildBlendTree(std::shared_ptr<BlendTree> childTree, const Math::Vec2& position, const std::string& name) {
        if (!childTree) {
            LOG_ERROR("BlendTree: Cannot add null child blend tree");
            return;
        }

        if (m_type == Type::Simple1D) {
            LOG_ERROR("BlendTree: Cannot add 2D child blend tree to 1D blend tree");
            return;
        }

        BlendTreeNode node;
        node.childTree = childTree;
        node.position = position;
        node.name = name;

        m_nodes.push_back(node);

        LOG_INFO("BlendTree: Added 2D child blend tree '" + name + "' at position (" + 
                std::to_string(position.x) + ", " + std::to_string(position.y) + ")");
    }

    void BlendTree::RemoveMotion(std::shared_ptr<SkeletalAnimation> animation) {
        if (!animation) {
            return;
        }

        auto it = std::remove_if(m_nodes.begin(), m_nodes.end(),
            [animation](const BlendTreeNode& node) {
                return node.animation == animation;
            });

        if (it != m_nodes.end()) {
            size_t removedCount = std::distance(it, m_nodes.end());
            m_nodes.erase(it, m_nodes.end());
            LOG_INFO("BlendTree: Removed " + std::to_string(removedCount) + " motion(s)");
        }
    }

    void BlendTree::RemoveMotion(const std::string& name) {
        auto it = std::remove_if(m_nodes.begin(), m_nodes.end(),
            [&name](const BlendTreeNode& node) {
                return node.name == name;
            });

        if (it != m_nodes.end()) {
            m_nodes.erase(it, m_nodes.end());
            LOG_INFO("BlendTree: Removed motion '" + name + "'");
        }
    }

    void BlendTree::RemoveChildBlendTree(std::shared_ptr<BlendTree> childTree) {
        if (!childTree) {
            return;
        }

        auto it = std::remove_if(m_nodes.begin(), m_nodes.end(),
            [childTree](const BlendTreeNode& node) {
                return node.childTree == childTree;
            });

        if (it != m_nodes.end()) {
            size_t removedCount = std::distance(it, m_nodes.end());
            m_nodes.erase(it, m_nodes.end());
            LOG_INFO("BlendTree: Removed " + std::to_string(removedCount) + " child blend tree(s)");
        }
    }

    void BlendTree::RemoveChildBlendTree(const std::string& name) {
        auto it = std::remove_if(m_nodes.begin(), m_nodes.end(),
            [&name](const BlendTreeNode& node) {
                return node.name == name && node.IsChildTree();
            });

        if (it != m_nodes.end()) {
            m_nodes.erase(it, m_nodes.end());
            LOG_INFO("BlendTree: Removed child blend tree '" + name + "'");
        }
    }

    void BlendTree::ClearMotions() {
        if (!m_nodes.empty()) {
            m_nodes.clear();
            LOG_INFO("BlendTree: Cleared all motions");
        }
    }

    void BlendTree::Evaluate(AnimationController* controller, Pose& pose, float time) const {
        if (!controller) {
            LOG_ERROR("BlendTree: Cannot evaluate with null controller");
            return;
        }

        if (m_nodes.empty()) {
            LOG_WARNING("BlendTree: Cannot evaluate empty blend tree");
            return;
        }

        // Calculate weights for all nodes
        std::vector<float> weights(m_nodes.size(), 0.0f);
        
        if (m_type == Type::Simple1D) {
            float parameter = controller->GetFloat(m_parameterX);
            CalculateWeights1D(parameter, weights);
        } else {
            Math::Vec2 parameters = GetParameterValues(controller);
            CalculateWeights2D(parameters, weights);
        }

        // Normalize weights
        NormalizeWeights(weights);

        // Evaluate nodes and blend poses
        Pose tempPose;
        if (controller->HasValidSkeleton()) {
            tempPose.SetSkeleton(controller->GetSkeleton());
        }

        bool firstValidNode = true;
        for (size_t i = 0; i < m_nodes.size(); ++i) {
            if (weights[i] > 0.001f) { // Skip nodes with negligible weight
                if (firstValidNode) {
                    EvaluateNode(m_nodes[i], weights[i], time, controller, pose);
                    firstValidNode = false;
                } else {
                    tempPose.Reset();
                    EvaluateNode(m_nodes[i], weights[i], time, controller, tempPose);
                    pose.BlendWith(tempPose, weights[i]);
                }
            }
        }
    }

    std::vector<AnimationSample> BlendTree::GetAnimationSamples(AnimationController* controller, float time) const {
        std::vector<AnimationSample> samples;

        if (!controller || m_nodes.empty()) {
            return samples;
        }

        // Calculate weights for all nodes
        std::vector<float> weights(m_nodes.size(), 0.0f);
        
        if (m_type == Type::Simple1D) {
            float parameter = controller->GetFloat(m_parameterX);
            CalculateWeights1D(parameter, weights);
        } else {
            Math::Vec2 parameters = GetParameterValues(controller);
            CalculateWeights2D(parameters, weights);
        }

        // Normalize weights
        NormalizeWeights(weights);

        // Collect animation samples
        for (size_t i = 0; i < m_nodes.size(); ++i) {
            if (weights[i] > 0.001f) {
                const BlendTreeNode& node = m_nodes[i];
                
                if (node.IsAnimation()) {
                    AnimationSample sample;
                    sample.animation = node.animation;
                    sample.weight = weights[i];
                    sample.time = time;
                    samples.push_back(sample);
                } else if (node.IsChildTree()) {
                    // Recursively get samples from child blend tree
                    auto childSamples = node.childTree->GetAnimationSamples(controller, time);
                    for (auto& childSample : childSamples) {
                        childSample.weight *= weights[i]; // Scale by parent weight
                        samples.push_back(childSample);
                    }
                }
            }
        }

        return samples;
    }

    float BlendTree::GetDuration(AnimationController* controller) const {
        if (!controller || m_nodes.empty()) {
            return 0.0f;
        }

        float maxDuration = 0.0f;
        
        for (const auto& node : m_nodes) {
            float nodeDuration = 0.0f;
            
            if (node.IsAnimation()) {
                nodeDuration = node.animation->GetDuration();
            } else if (node.IsChildTree()) {
                nodeDuration = node.childTree->GetDuration(controller);
            }
            
            maxDuration = std::max(maxDuration, nodeDuration);
        }

        return maxDuration;
    }

    bool BlendTree::Validate() const {
        auto errors = GetValidationErrors();
        return errors.empty();
    }

    std::vector<std::string> BlendTree::GetValidationErrors() const {
        std::vector<std::string> errors;

        // Check if we have any nodes
        if (m_nodes.empty()) {
            errors.push_back("Blend tree has no motions or child trees");
        }

        // Check parameter configuration
        if (m_type == Type::Simple1D) {
            if (m_parameterX.empty()) {
                errors.push_back("1D blend tree requires a parameter");
            }
        } else {
            if (m_parameterX.empty() || m_parameterY.empty()) {
                errors.push_back("2D blend tree requires two parameters");
            }
        }

        // Check node validity
        for (size_t i = 0; i < m_nodes.size(); ++i) {
            const auto& node = m_nodes[i];
            
            if (!node.IsValid()) {
                errors.push_back("Node " + std::to_string(i) + " has no animation or child tree");
            }

            if (node.IsAnimation() && !node.animation) {
                errors.push_back("Node " + std::to_string(i) + " has null animation");
            }

            if (node.IsChildTree() && !node.childTree) {
                errors.push_back("Node " + std::to_string(i) + " has null child tree");
            }

            // Validate child trees recursively
            if (node.IsChildTree() && node.childTree) {
                auto childErrors = node.childTree->GetValidationErrors();
                for (const auto& childError : childErrors) {
                    errors.push_back("Child tree '" + node.name + "': " + childError);
                }
            }
        }

        // Check for duplicate thresholds in 1D blend trees
        if (m_type == Type::Simple1D && m_nodes.size() > 1) {
            for (size_t i = 0; i < m_nodes.size() - 1; ++i) {
                for (size_t j = i + 1; j < m_nodes.size(); ++j) {
                    if (std::abs(m_nodes[i].threshold - m_nodes[j].threshold) < 0.001f) {
                        errors.push_back("Duplicate thresholds found: " + std::to_string(m_nodes[i].threshold));
                    }
                }
            }
        }

        return errors;
    }

    std::vector<std::string> BlendTree::GetMotionNames() const {
        std::vector<std::string> names;
        names.reserve(m_nodes.size());
        
        for (const auto& node : m_nodes) {
            names.push_back(node.name);
        }
        
        return names;
    }

    void BlendTree::PrintBlendTreeInfo() const {
        LOG_INFO("=== Blend Tree Information ===");
        LOG_INFO("Type: " + std::to_string(static_cast<int>(m_type)));
        LOG_INFO("Parameter X: " + m_parameterX);
        LOG_INFO("Parameter Y: " + m_parameterY);
        LOG_INFO("Node Count: " + std::to_string(m_nodes.size()));
        
        for (size_t i = 0; i < m_nodes.size(); ++i) {
            const auto& node = m_nodes[i];
            std::string nodeInfo = "Node " + std::to_string(i) + ": " + node.name;
            
            if (node.IsAnimation()) {
                nodeInfo += " (Animation)";
            } else if (node.IsChildTree()) {
                nodeInfo += " (Child Tree)";
            }
            
            if (m_type == Type::Simple1D) {
                nodeInfo += " - Threshold: " + std::to_string(node.threshold);
            } else {
                nodeInfo += " - Position: (" + std::to_string(node.position.x) + 
                           ", " + std::to_string(node.position.y) + ")";
            }
            
            LOG_INFO(nodeInfo);
        }
        
        auto errors = GetValidationErrors();
        if (!errors.empty()) {
            LOG_WARNING("Validation Errors:");
            for (const auto& error : errors) {
                LOG_WARNING("  - " + error);
            }
        }
        
        LOG_INFO("==============================");
    }

    // Private helper methods

    void BlendTree::CalculateWeights1D(float parameter, std::vector<float>& weights) const {
        if (m_nodes.empty()) {
            return;
        }

        weights.assign(m_nodes.size(), 0.0f);

        if (m_nodes.size() == 1) {
            weights[0] = 1.0f;
            return;
        }

        // Find the two closest nodes for interpolation
        int lowerIndex = -1;
        int upperIndex = -1;

        for (int i = 0; i < static_cast<int>(m_nodes.size()); ++i) {
            if (m_nodes[i].threshold <= parameter) {
                lowerIndex = i;
            }
            if (m_nodes[i].threshold >= parameter && upperIndex == -1) {
                upperIndex = i;
                break;
            }
        }

        // Handle edge cases
        if (lowerIndex == -1) {
            // Parameter is below all thresholds
            weights[0] = 1.0f;
        } else if (upperIndex == -1) {
            // Parameter is above all thresholds
            weights[m_nodes.size() - 1] = 1.0f;
        } else if (lowerIndex == upperIndex) {
            // Exact match
            weights[lowerIndex] = 1.0f;
        } else {
            // Interpolate between two nodes
            float lowerThreshold = m_nodes[lowerIndex].threshold;
            float upperThreshold = m_nodes[upperIndex].threshold;
            float range = upperThreshold - lowerThreshold;
            
            if (range > 0.001f) {
                float t = (parameter - lowerThreshold) / range;
                weights[lowerIndex] = 1.0f - t;
                weights[upperIndex] = t;
            } else {
                weights[lowerIndex] = 1.0f;
            }
        }
    }

    void BlendTree::CalculateWeights2D(const Math::Vec2& parameter, std::vector<float>& weights) const {
        weights.assign(m_nodes.size(), 0.0f);

        switch (m_type) {
            case Type::SimpleDirectional2D:
            case Type::FreeformDirectional2D:
                CalculateDirectionalWeights(parameter, weights);
                break;
                
            case Type::FreeformCartesian2D:
                CalculateCartesianWeights(parameter, weights);
                break;
                
            default:
                LOG_ERROR("BlendTree: Invalid 2D blend tree type");
                break;
        }
    }

    void BlendTree::CalculateDirectionalWeights(const Math::Vec2& direction, std::vector<float>& weights) const {
        if (m_nodes.empty()) {
            return;
        }

        // Normalize the input direction
        Math::Vec2 normalizedDirection = glm::normalize(direction);
        float inputMagnitude = glm::length(direction);

        // Calculate weights based on angular distance
        float totalWeight = 0.0f;
        
        for (size_t i = 0; i < m_nodes.size(); ++i) {
            Math::Vec2 nodeDirection = glm::normalize(m_nodes[i].position);
            
            // Calculate angular similarity (dot product)
            float similarity = glm::dot(normalizedDirection, nodeDirection);
            similarity = std::max(0.0f, similarity); // Clamp to positive values
            
            // Apply magnitude influence
            float nodeMagnitude = glm::length(m_nodes[i].position);
            float magnitudeWeight = 1.0f - std::abs(inputMagnitude - nodeMagnitude) / std::max(inputMagnitude, nodeMagnitude);
            magnitudeWeight = std::max(0.0f, magnitudeWeight);
            
            weights[i] = similarity * magnitudeWeight;
            totalWeight += weights[i];
        }

        // Normalize weights
        if (totalWeight > 0.001f) {
            for (float& weight : weights) {
                weight /= totalWeight;
            }
        }
    }

    void BlendTree::CalculateCartesianWeights(const Math::Vec2& position, std::vector<float>& weights) const {
        if (m_nodes.empty()) {
            return;
        }

        if (m_nodes.size() == 1) {
            weights[0] = 1.0f;
            return;
        }

        // Calculate inverse distance weights
        float totalWeight = 0.0f;
        
        for (size_t i = 0; i < m_nodes.size(); ++i) {
            float distance = glm::length(position - m_nodes[i].position);
            
            if (distance < 0.001f) {
                // Very close to a node, give it full weight
                weights.assign(m_nodes.size(), 0.0f);
                weights[i] = 1.0f;
                return;
            }
            
            weights[i] = 1.0f / (distance * distance); // Inverse square distance
            totalWeight += weights[i];
        }

        // Normalize weights
        if (totalWeight > 0.001f) {
            for (float& weight : weights) {
                weight /= totalWeight;
            }
        }
    }

    Math::Vec2 BlendTree::GetParameterValues(AnimationController* controller) const {
        Math::Vec2 parameters(0.0f);
        
        if (controller) {
            parameters.x = controller->GetFloat(m_parameterX);
            parameters.y = controller->GetFloat(m_parameterY);
        }
        
        return parameters;
    }

    void BlendTree::NormalizeWeights(std::vector<float>& weights) const {
        float totalWeight = 0.0f;
        
        for (float weight : weights) {
            totalWeight += weight;
        }
        
        if (totalWeight > 0.001f) {
            for (float& weight : weights) {
                weight /= totalWeight;
            }
        }
    }

    void BlendTree::SortNodesByThreshold() {
        if (m_type == Type::Simple1D) {
            std::sort(m_nodes.begin(), m_nodes.end(),
                [](const BlendTreeNode& a, const BlendTreeNode& b) {
                    return a.threshold < b.threshold;
                });
        }
    }

    void BlendTree::ValidateNodeConfiguration() const {
        // This method can be extended for more complex validation
        for (const auto& node : m_nodes) {
            if (!node.IsValid()) {
                LOG_WARNING("BlendTree: Invalid node found - " + node.name);
            }
        }
    }

    void BlendTree::EvaluateNode(const BlendTreeNode& node, float weight, float time, 
                                AnimationController* controller, Pose& pose) const {
        if (node.IsAnimation()) {
            // Evaluate animation and apply to pose
            Pose animPose = PoseEvaluator::EvaluateAnimation(*node.animation, time, controller->GetSkeleton());
            
            if (weight >= 0.999f) {
                pose = animPose;
            } else {
                pose.BlendWith(animPose, weight);
            }
        } else if (node.IsChildTree()) {
            // Recursively evaluate child blend tree
            node.childTree->Evaluate(controller, pose, time);
            
            if (weight < 0.999f) {
                // Scale the pose by the weight
                // This is a simplified approach - in practice, you might want more sophisticated blending
                pose.BlendWith(pose, weight);
            }
        }
    }

    void BlendTree::BlendPoses(const Pose& poseA, const Pose& poseB, float weight, Pose& result) const {
        result = Pose::Blend(poseA, poseB, weight);
    }

} // namespace Animation
} // namespace GameEngine