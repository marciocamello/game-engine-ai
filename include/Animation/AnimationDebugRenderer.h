#pragma once

#include "Core/Math.h"
#include "Physics/PhysicsDebugDrawer.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace GameEngine {
    namespace Animation {
        class AnimationSkeleton;
        class AnimationController;
        class AnimationStateMachine;
        class IKSolver;
        class BlendTree;
        class Bone;
        struct StateMachineDebugInfo; // Forward declaration
    }

    namespace Animation {

        /**
         * @brief Debug visualization modes for animation system
         */
        enum class AnimationDebugMode {
            None = 0,
            Skeleton = 1,
            StateMachine = 2,
            IKChains = 4,
            BlendTrees = 8,
            All = Skeleton | StateMachine | IKChains | BlendTrees
        };



        /**
         * @brief Debug information for IK chains
         */
        struct IKChainDebugInfo {
            std::string solverName;
            std::vector<int> boneIndices;
            Math::Vec3 targetPosition;
            Math::Quat targetRotation;
            Math::Vec3 poleTarget;
            bool isTargetReachable = false;
            float chainLength = 0.0f;
            int iterations = 0;
            float tolerance = 0.0f;
        };

        /**
         * @brief Debug information for blend trees
         */
        struct BlendTreeDebugInfo {
            std::string name;
            std::string type;
            std::vector<std::string> parameterNames;
            std::vector<float> parameterValues;
            std::vector<std::string> animationNames;
            std::vector<float> animationWeights;
        };

        /**
         * @brief Animation system debug renderer
         * 
         * Provides visual debugging for skeletal animation, state machines,
         * IK chains, and blend trees using the existing debug drawing system.
         */
        class AnimationDebugRenderer {
        public:
            AnimationDebugRenderer();
            ~AnimationDebugRenderer();

            // Initialization
            bool Initialize(std::shared_ptr<Physics::IPhysicsDebugDrawer> debugDrawer);
            void Shutdown();

            // Debug mode control
            void SetDebugMode(AnimationDebugMode mode);
            AnimationDebugMode GetDebugMode() const;
            void EnableDebugMode(AnimationDebugMode mode, bool enabled);
            bool IsDebugModeEnabled(AnimationDebugMode mode) const;

            // Skeleton visualization
            void DrawSkeleton(const AnimationSkeleton& skeleton, const Math::Mat4& worldTransform = Math::Mat4(1.0f));
            void DrawBone(const Math::Vec3& startPos, const Math::Vec3& endPos, float thickness = 0.02f, const Math::Vec3& color = Math::Vec3(1.0f, 1.0f, 0.0f));
            void DrawBoneHierarchyFromBone(std::shared_ptr<Bone> bone, const Math::Mat4& parentTransform, const Math::Vec3& color = Math::Vec3(1.0f, 1.0f, 0.0f));
            void DrawJoint(const Math::Vec3& position, float radius = 0.05f, const Math::Vec3& color = Math::Vec3(0.0f, 1.0f, 0.0f));

            // State machine visualization
            void DrawStateMachineInfo(const StateMachineDebugInfo& info, const Math::Vec3& position);
            void DrawStateTransition(const Math::Vec3& fromPos, const Math::Vec3& toPos, float progress, const Math::Vec3& color = Math::Vec3(0.0f, 0.0f, 1.0f));
            void DrawParameterValues(const std::unordered_map<std::string, float>& parameters, const Math::Vec3& position);

            // IK chain visualization
            void DrawIKChain(const IKChainDebugInfo& ikInfo, const AnimationSkeleton& skeleton);
            void DrawIKTarget(const Math::Vec3& position, const Math::Quat& rotation, float size = 0.1f, const Math::Vec3& color = Math::Vec3(1.0f, 0.0f, 0.0f));
            void DrawIKPoleTarget(const Math::Vec3& position, float size = 0.05f, const Math::Vec3& color = Math::Vec3(0.0f, 1.0f, 1.0f));
            void DrawIKConstraints(const AnimationSkeleton& skeleton, int boneIndex, float minAngle, float maxAngle);

            // Blend tree visualization
            void DrawBlendTreeInfo(const BlendTreeDebugInfo& info, const Math::Vec3& position);
            void DrawBlendWeights(const std::vector<std::string>& animationNames, const std::vector<float>& weights, const Math::Vec3& position);

            // Text rendering for debug info
            void DrawDebugText(const Math::Vec3& position, const std::string& text, const Math::Vec3& color = Math::Vec3(1.0f));
            void DrawDebugValue(const Math::Vec3& position, const std::string& label, float value, const Math::Vec3& color = Math::Vec3(1.0f));

            // Utility methods
            void Clear();
            void SetBoneThickness(float thickness);
            void SetJointRadius(float radius);
            void SetTextScale(float scale);

            // Color schemes
            void SetSkeletonColor(const Math::Vec3& color);
            void SetJointColor(const Math::Vec3& color);
            void SetIKChainColor(const Math::Vec3& color);
            void SetIKTargetColor(const Math::Vec3& color);

        private:
            std::shared_ptr<Physics::IPhysicsDebugDrawer> m_debugDrawer;
            AnimationDebugMode m_debugMode = AnimationDebugMode::None;

            // Rendering settings
            float m_boneThickness = 0.02f;
            float m_jointRadius = 0.05f;
            float m_textScale = 1.0f;

            // Color scheme
            Math::Vec3 m_skeletonColor = Math::Vec3(1.0f, 1.0f, 0.0f);  // Yellow
            Math::Vec3 m_jointColor = Math::Vec3(0.0f, 1.0f, 0.0f);     // Green
            Math::Vec3 m_ikChainColor = Math::Vec3(1.0f, 0.5f, 0.0f);   // Orange
            Math::Vec3 m_ikTargetColor = Math::Vec3(1.0f, 0.0f, 0.0f);  // Red
            Math::Vec3 m_textColor = Math::Vec3(1.0f, 1.0f, 1.0f);      // White

            // Helper methods
            void DrawBoneCapsule(const Math::Vec3& startPos, const Math::Vec3& endPos, float thickness, const Math::Vec3& color);
            void DrawArrow(const Math::Vec3& start, const Math::Vec3& end, float arrowSize, const Math::Vec3& color);
            void DrawCoordinateSystem(const Math::Vec3& position, const Math::Quat& rotation, float size);
            std::string FormatFloat(float value, int precision = 2) const;
        };

        /**
         * @brief Helper class to collect debug information from animation controller
         */
        class AnimationDebugInfoCollector {
        public:
            static StateMachineDebugInfo CollectStateMachineInfo(const AnimationController& controller);
            static std::vector<IKChainDebugInfo> CollectIKChainInfo(const AnimationController& controller);
            static std::vector<BlendTreeDebugInfo> CollectBlendTreeInfo(const AnimationController& controller);
        };

    } // namespace Animation
} // namespace GameEngine