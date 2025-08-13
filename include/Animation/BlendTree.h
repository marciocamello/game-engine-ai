#pragma once

#include "Animation/SkeletalAnimation.h"
#include "Animation/Pose.h"
#include "Core/Math.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace GameEngine {
namespace Animation {

    // Forward declarations
    class AnimationController;

    /**
     * Animation sample for blend tree evaluation
     */
    struct AnimationSample {
        std::shared_ptr<SkeletalAnimation> animation;
        float weight;
        float time;

        bool IsValid() const { return animation != nullptr && weight > 0.0f; }
    };

    /**
     * Blend tree system for parameter-driven animation blending
     */
    class BlendTree {
    public:
        enum class Type { 
            Simple1D,               // 1D blend space with single parameter
            SimpleDirectional2D,    // 2D directional blend space
            FreeformDirectional2D,  // 2D freeform directional blend space
            FreeformCartesian2D     // 2D freeform cartesian blend space
        };

        // Lifecycle
        BlendTree(Type type = Type::Simple1D);
        ~BlendTree();

        // Properties
        void SetType(Type type);
        void SetParameter(const std::string& parameter);
        void SetParameters(const std::string& paramX, const std::string& paramY);

        Type GetType() const { return m_type; }
        const std::string& GetParameterX() const { return m_parameterX; }
        const std::string& GetParameterY() const { return m_parameterY; }

        // Motion management for 1D blend trees
        void AddMotion(std::shared_ptr<SkeletalAnimation> animation, float threshold);
        void AddMotion(std::shared_ptr<SkeletalAnimation> animation, float threshold, const std::string& name);
        
        // Motion management for 2D blend trees
        void AddMotion(std::shared_ptr<SkeletalAnimation> animation, const Math::Vec2& position);
        void AddMotion(std::shared_ptr<SkeletalAnimation> animation, const Math::Vec2& position, const std::string& name);
        
        // Child blend tree support
        void AddChildBlendTree(std::shared_ptr<BlendTree> childTree, float threshold);
        void AddChildBlendTree(std::shared_ptr<BlendTree> childTree, const Math::Vec2& position);
        void AddChildBlendTree(std::shared_ptr<BlendTree> childTree, float threshold, const std::string& name);
        void AddChildBlendTree(std::shared_ptr<BlendTree> childTree, const Math::Vec2& position, const std::string& name);
        
        // Motion removal
        void RemoveMotion(std::shared_ptr<SkeletalAnimation> animation);
        void RemoveMotion(const std::string& name);
        void RemoveChildBlendTree(std::shared_ptr<BlendTree> childTree);
        void RemoveChildBlendTree(const std::string& name);
        void ClearMotions();

        // Evaluation
        void Evaluate(AnimationController* controller, Pose& pose, float time) const;
        std::vector<AnimationSample> GetAnimationSamples(AnimationController* controller, float time) const;
        float GetDuration(AnimationController* controller) const;

        // Validation
        bool Validate() const;
        std::vector<std::string> GetValidationErrors() const;

        // Information
        size_t GetMotionCount() const { return m_nodes.size(); }
        bool IsEmpty() const { return m_nodes.empty(); }
        std::vector<std::string> GetMotionNames() const;

        // Debugging
        void PrintBlendTreeInfo() const;

    private:
        struct BlendTreeNode {
            std::shared_ptr<SkeletalAnimation> animation;
            std::shared_ptr<BlendTree> childTree;
            float threshold = 0.0f;
            Math::Vec2 position = Math::Vec2(0.0f);
            float weight = 0.0f;
            std::string name;

            bool IsAnimation() const { return animation != nullptr; }
            bool IsChildTree() const { return childTree != nullptr; }
            bool IsValid() const { return IsAnimation() || IsChildTree(); }
        };

        Type m_type;
        std::string m_parameterX;
        std::string m_parameterY;
        std::vector<BlendTreeNode> m_nodes;

        // Weight calculation methods
        void CalculateWeights1D(float parameter, std::vector<float>& weights) const;
        void CalculateWeights2D(const Math::Vec2& parameter, std::vector<float>& weights) const;
        void CalculateDirectionalWeights(const Math::Vec2& direction, std::vector<float>& weights) const;
        void CalculateCartesianWeights(const Math::Vec2& position, std::vector<float>& weights) const;

        // Helper methods
        Math::Vec2 GetParameterValues(AnimationController* controller) const;
        void NormalizeWeights(std::vector<float>& weights) const;
        void SortNodesByThreshold();
        void ValidateNodeConfiguration() const;
        
        // Evaluation helpers
        void EvaluateNode(const BlendTreeNode& node, float weight, float time, 
                         AnimationController* controller, Pose& pose) const;
        void BlendPoses(const Pose& poseA, const Pose& poseB, float weight, Pose& result) const;
    };

} // namespace Animation
} // namespace GameEngine