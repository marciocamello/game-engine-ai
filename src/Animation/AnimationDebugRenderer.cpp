#include "Animation/AnimationDebugRenderer.h"
#include "Animation/AnimationSkeleton.h"
#include "Animation/AnimationController.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/IKSolver.h"
#include "Animation/BlendTree.h"
#include "Core/Logger.h"
#include <sstream>
#include <iomanip>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace GameEngine {
    namespace Animation {

        AnimationDebugRenderer::AnimationDebugRenderer() = default;

        AnimationDebugRenderer::~AnimationDebugRenderer() {
            Shutdown();
        }

        bool AnimationDebugRenderer::Initialize(std::shared_ptr<Physics::IPhysicsDebugDrawer> debugDrawer) {
            if (!debugDrawer) {
                LOG_ERROR("AnimationDebugRenderer: Debug drawer is null");
                return false;
            }

            m_debugDrawer = debugDrawer;
            LOG_INFO("AnimationDebugRenderer initialized successfully");
            return true;
        }

        void AnimationDebugRenderer::Shutdown() {
            m_debugDrawer.reset();
        }

        void AnimationDebugRenderer::SetDebugMode(AnimationDebugMode mode) {
            m_debugMode = mode;
        }

        AnimationDebugMode AnimationDebugRenderer::GetDebugMode() const {
            return m_debugMode;
        }

        void AnimationDebugRenderer::EnableDebugMode(AnimationDebugMode mode, bool enabled) {
            if (enabled) {
                m_debugMode = static_cast<AnimationDebugMode>(static_cast<int>(m_debugMode) | static_cast<int>(mode));
            } else {
                m_debugMode = static_cast<AnimationDebugMode>(static_cast<int>(m_debugMode) & ~static_cast<int>(mode));
            }
        }

        bool AnimationDebugRenderer::IsDebugModeEnabled(AnimationDebugMode mode) const {
            return (static_cast<int>(m_debugMode) & static_cast<int>(mode)) != 0;
        }

        void AnimationDebugRenderer::DrawSkeleton(const AnimationSkeleton& skeleton, const Math::Mat4& worldTransform) {
            if (!m_debugDrawer || !IsDebugModeEnabled(AnimationDebugMode::Skeleton)) {
                return;
            }

            // Draw all root bones and their hierarchies
            auto rootBones = skeleton.GetRootBones();
            for (auto rootBone : rootBones) {
                if (rootBone) {
                    DrawBoneHierarchyFromBone(rootBone, worldTransform, m_skeletonColor);
                }
            }
        }

        void AnimationDebugRenderer::DrawBone(const Math::Vec3& startPos, const Math::Vec3& endPos, float thickness, const Math::Vec3& color) {
            if (!m_debugDrawer) return;

            // Draw bone as a line with thickness visualization
            m_debugDrawer->DrawLine(startPos, endPos, color);
            
            // Draw bone as capsule for better visualization
            DrawBoneCapsule(startPos, endPos, thickness, color);
        }

        void AnimationDebugRenderer::DrawBoneHierarchyFromBone(std::shared_ptr<Bone> bone, const Math::Mat4& parentTransform, const Math::Vec3& color) {
            if (!m_debugDrawer || !bone) {
                return;
            }

            Math::Mat4 boneWorldTransform = parentTransform * bone->GetWorldTransform();
            
            // Extract bone position
            Math::Vec3 bonePosition = Math::Vec3(boneWorldTransform[3]);
            
            // Draw joint
            DrawJoint(bonePosition, m_jointRadius, m_jointColor);
            
            // Draw bone to parent if has parent
            auto parent = bone->GetParent();
            if (parent) {
                Math::Mat4 parentWorldTransform = parentTransform * parent->GetWorldTransform();
                Math::Vec3 parentPosition = Math::Vec3(parentWorldTransform[3]);
                
                DrawBone(parentPosition, bonePosition, m_boneThickness, color);
            }

            // Draw coordinate system for bone
            Math::Vec3 scale;
            Math::Quat boneRotation;
            Math::Vec3 translation;
            Math::Vec3 skew;
            Math::Vec4 perspective;
            glm::decompose(boneWorldTransform, scale, boneRotation, translation, skew, perspective);
            DrawCoordinateSystem(bonePosition, boneRotation, 0.1f);

            // Draw bone name
            DrawDebugText(bonePosition + Math::Vec3(0.0f, 0.1f, 0.0f), bone->GetName(), m_textColor);

            // Recursively draw child bones
            for (auto child : bone->GetChildren()) {
                DrawBoneHierarchyFromBone(child, parentTransform, color);
            }
        }

        void AnimationDebugRenderer::DrawJoint(const Math::Vec3& position, float radius, const Math::Vec3& color) {
            if (!m_debugDrawer) return;
            
            m_debugDrawer->DrawSphere(position, radius, color);
        }

        void AnimationDebugRenderer::DrawStateMachineInfo(const StateMachineDebugInfo& info, const Math::Vec3& position) {
            if (!m_debugDrawer || !IsDebugModeEnabled(AnimationDebugMode::StateMachine)) {
                return;
            }

            Math::Vec3 textPos = position;
            float lineHeight = 0.15f * m_textScale;

            // Draw current state
            DrawDebugText(textPos, "Current State: " + info.currentStateName, m_textColor);
            textPos.y -= lineHeight;

            // Draw state time
            DrawDebugValue(textPos, "State Time", info.currentStateTime, m_textColor);
            textPos.y -= lineHeight;

            // Draw transition info if transitioning
            if (info.isTransitioning) {
                DrawDebugText(textPos, "Transitioning to: " + info.transitionToState, Math::Vec3(1.0f, 1.0f, 0.0f));
                textPos.y -= lineHeight;
                
                DrawDebugValue(textPos, "Transition Progress", info.transitionProgress, Math::Vec3(1.0f, 1.0f, 0.0f));
                textPos.y -= lineHeight;
            }

            // Draw available states
            if (!info.availableStates.empty()) {
                DrawDebugText(textPos, "Available States:", m_textColor);
                textPos.y -= lineHeight;
                
                for (const auto& state : info.availableStates) {
                    DrawDebugText(textPos, "- " + state, Math::Vec3(0.8f, 1.0f, 0.8f));
                    textPos.y -= lineHeight;
                }
            }
        }

        void AnimationDebugRenderer::DrawStateTransition(const Math::Vec3& fromPos, const Math::Vec3& toPos, float progress, const Math::Vec3& color) {
            if (!m_debugDrawer) return;

            // Draw arrow showing transition direction
            DrawArrow(fromPos, toPos, 0.1f, color);
            
            // Draw progress indicator
            Math::Vec3 progressPos = Math::Lerp(fromPos, toPos, progress);
            m_debugDrawer->DrawSphere(progressPos, 0.05f, color);
        }

        void AnimationDebugRenderer::DrawParameterValues(const std::unordered_map<std::string, float>& parameters, const Math::Vec3& position) {
            if (!m_debugDrawer) return;

            Math::Vec3 textPos = position;
            float lineHeight = 0.15f * m_textScale;

            for (const auto& param : parameters) {
                DrawDebugValue(textPos, param.first, param.second, Math::Vec3(0.8f, 0.8f, 1.0f));
                textPos.y -= lineHeight;
            }
        }

        void AnimationDebugRenderer::DrawIKChain(const IKChainDebugInfo& ikInfo, const AnimationSkeleton& skeleton) {
            if (!m_debugDrawer || !IsDebugModeEnabled(AnimationDebugMode::IKChains)) {
                return;
            }

            // Draw IK chain bones
            auto allBones = skeleton.GetAllBones();
            for (size_t i = 0; i < ikInfo.boneIndices.size(); ++i) {
                int boneIndex = ikInfo.boneIndices[i];
                if (boneIndex < 0 || boneIndex >= static_cast<int>(allBones.size())) continue;

                auto bone = allBones[boneIndex];
                if (!bone) continue;

                Math::Vec3 bonePos = Math::Vec3(bone->GetWorldTransform()[3]);

                // Draw joint with IK color
                DrawJoint(bonePos, m_jointRadius * 1.2f, m_ikChainColor);

                // Draw connection to next bone in chain
                if (i < ikInfo.boneIndices.size() - 1) {
                    int nextBoneIndex = ikInfo.boneIndices[i + 1];
                    if (nextBoneIndex >= 0 && nextBoneIndex < static_cast<int>(allBones.size())) {
                        auto nextBone = allBones[nextBoneIndex];
                        if (nextBone) {
                            Math::Vec3 nextBonePos = Math::Vec3(nextBone->GetWorldTransform()[3]);
                            DrawBone(bonePos, nextBonePos, m_boneThickness * 1.5f, m_ikChainColor);
                        }
                    }
                }
            }

            // Draw IK target
            DrawIKTarget(ikInfo.targetPosition, ikInfo.targetRotation, 0.1f, m_ikTargetColor);

            // Draw pole target if valid
            if (glm::length(ikInfo.poleTarget) > 0.001f) {
                DrawIKPoleTarget(ikInfo.poleTarget, 0.05f, Math::Vec3(0.0f, 1.0f, 1.0f));
            }

            // Draw IK info text
            Math::Vec3 textPos = ikInfo.targetPosition + Math::Vec3(0.2f, 0.0f, 0.0f);
            DrawDebugText(textPos, "IK: " + ikInfo.solverName, m_textColor);
            textPos.y -= 0.1f;
            DrawDebugValue(textPos, "Chain Length", ikInfo.chainLength, m_textColor);
            textPos.y -= 0.1f;
            DrawDebugText(textPos, ikInfo.isTargetReachable ? "Target Reachable" : "Target Unreachable", 
                         ikInfo.isTargetReachable ? Math::Vec3(0.0f, 1.0f, 0.0f) : Math::Vec3(1.0f, 0.0f, 0.0f));
        }

        void AnimationDebugRenderer::DrawIKTarget(const Math::Vec3& position, const Math::Quat& rotation, float size, const Math::Vec3& color) {
            if (!m_debugDrawer) return;

            // Draw target as a cube
            m_debugDrawer->DrawBox(position, Math::Vec3(size), rotation, color);
            
            // Draw coordinate system to show target orientation
            DrawCoordinateSystem(position, rotation, size * 2.0f);
        }

        void AnimationDebugRenderer::DrawIKPoleTarget(const Math::Vec3& position, float size, const Math::Vec3& color) {
            if (!m_debugDrawer) return;

            // Draw pole target as a small sphere
            m_debugDrawer->DrawSphere(position, size, color);
        }

        void AnimationDebugRenderer::DrawIKConstraints(const AnimationSkeleton& skeleton, int boneIndex, float minAngle, float maxAngle) {
            if (!m_debugDrawer) {
                return;
            }

            // Get bone position from world transform
            auto allBones = skeleton.GetAllBones();
            if (boneIndex >= 0 && boneIndex < static_cast<int>(allBones.size())) {
                auto bone = allBones[boneIndex];
                if (bone) {
                    Math::Vec3 bonePos = Math::Vec3(bone->GetWorldTransform()[3]);
                    std::string constraintText = "Constraints: " + FormatFloat(minAngle) + " to " + FormatFloat(maxAngle);
                    DrawDebugText(bonePos + Math::Vec3(0.0f, 0.15f, 0.0f), constraintText, Math::Vec3(1.0f, 0.5f, 0.0f));
                }
            }
        }

        void AnimationDebugRenderer::DrawBlendTreeInfo(const BlendTreeDebugInfo& info, const Math::Vec3& position) {
            if (!m_debugDrawer || !IsDebugModeEnabled(AnimationDebugMode::BlendTrees)) {
                return;
            }

            Math::Vec3 textPos = position;
            float lineHeight = 0.15f * m_textScale;

            // Draw blend tree name and type
            DrawDebugText(textPos, "BlendTree: " + info.name + " (" + info.type + ")", m_textColor);
            textPos.y -= lineHeight;

            // Draw parameters
            for (size_t i = 0; i < info.parameterNames.size() && i < info.parameterValues.size(); ++i) {
                DrawDebugValue(textPos, info.parameterNames[i], info.parameterValues[i], Math::Vec3(0.8f, 1.0f, 0.8f));
                textPos.y -= lineHeight;
            }

            // Draw animation weights
            DrawBlendWeights(info.animationNames, info.animationWeights, textPos);
        }

        void AnimationDebugRenderer::DrawBlendWeights(const std::vector<std::string>& animationNames, const std::vector<float>& weights, const Math::Vec3& position) {
            if (!m_debugDrawer) return;

            Math::Vec3 textPos = position;
            float lineHeight = 0.15f * m_textScale;

            DrawDebugText(textPos, "Animation Weights:", m_textColor);
            textPos.y -= lineHeight;

            for (size_t i = 0; i < animationNames.size() && i < weights.size(); ++i) {
                Math::Vec3 weightColor = weights[i] > 0.001f ? Math::Vec3(1.0f, 1.0f, 0.0f) : Math::Vec3(0.5f, 0.5f, 0.5f);
                DrawDebugValue(textPos, animationNames[i], weights[i], weightColor);
                textPos.y -= lineHeight;
            }
        }

        void AnimationDebugRenderer::DrawDebugText(const Math::Vec3& position, const std::string& text, const Math::Vec3& color) {
            if (!m_debugDrawer) return;
            
            m_debugDrawer->DrawText(position, text, color);
        }

        void AnimationDebugRenderer::DrawDebugValue(const Math::Vec3& position, const std::string& label, float value, const Math::Vec3& color) {
            if (!m_debugDrawer) return;
            
            std::string text = label + ": " + FormatFloat(value);
            m_debugDrawer->DrawText(position, text, color);
        }

        void AnimationDebugRenderer::Clear() {
            if (m_debugDrawer) {
                m_debugDrawer->Clear();
            }
        }

        void AnimationDebugRenderer::SetBoneThickness(float thickness) {
            m_boneThickness = glm::max(thickness, 0.001f);
        }

        void AnimationDebugRenderer::SetJointRadius(float radius) {
            m_jointRadius = glm::max(radius, 0.001f);
        }

        void AnimationDebugRenderer::SetTextScale(float scale) {
            m_textScale = glm::max(scale, 0.1f);
        }

        void AnimationDebugRenderer::SetSkeletonColor(const Math::Vec3& color) {
            m_skeletonColor = color;
        }

        void AnimationDebugRenderer::SetJointColor(const Math::Vec3& color) {
            m_jointColor = color;
        }

        void AnimationDebugRenderer::SetIKChainColor(const Math::Vec3& color) {
            m_ikChainColor = color;
        }

        void AnimationDebugRenderer::SetIKTargetColor(const Math::Vec3& color) {
            m_ikTargetColor = color;
        }

        void AnimationDebugRenderer::DrawBoneCapsule(const Math::Vec3& startPos, const Math::Vec3& endPos, float thickness, const Math::Vec3& color) {
            if (!m_debugDrawer) return;

            Math::Vec3 direction = endPos - startPos;
            float length = glm::length(direction);
            
            if (length < 0.001f) return;

            Math::Vec3 normalizedDir = direction / length;
            Math::Vec3 center = (startPos + endPos) * 0.5f;
            
            // Calculate rotation to align capsule with bone direction
            Math::Vec3 up = Math::Vec3(0.0f, 1.0f, 0.0f);
            Math::Quat rotation = glm::rotation(up, normalizedDir);
            
            m_debugDrawer->DrawCapsule(center, thickness, length, rotation, color);
        }

        void AnimationDebugRenderer::DrawArrow(const Math::Vec3& start, const Math::Vec3& end, float arrowSize, const Math::Vec3& color) {
            if (!m_debugDrawer) return;

            // Draw main line
            m_debugDrawer->DrawLine(start, end, color);
            
            // Draw arrowhead
            Math::Vec3 direction = glm::normalize(end - start);
            Math::Vec3 perpendicular1 = glm::cross(direction, Math::Vec3(0.0f, 1.0f, 0.0f));
            if (glm::length(perpendicular1) < 0.1f) {
                perpendicular1 = glm::cross(direction, Math::Vec3(1.0f, 0.0f, 0.0f));
            }
            perpendicular1 = glm::normalize(perpendicular1);
            Math::Vec3 perpendicular2 = glm::cross(direction, perpendicular1);
            
            Math::Vec3 arrowBase = end - direction * arrowSize;
            Math::Vec3 arrowTip1 = arrowBase + perpendicular1 * arrowSize * 0.5f;
            Math::Vec3 arrowTip2 = arrowBase + perpendicular2 * arrowSize * 0.5f;
            Math::Vec3 arrowTip3 = arrowBase - perpendicular1 * arrowSize * 0.5f;
            Math::Vec3 arrowTip4 = arrowBase - perpendicular2 * arrowSize * 0.5f;
            
            m_debugDrawer->DrawLine(end, arrowTip1, color);
            m_debugDrawer->DrawLine(end, arrowTip2, color);
            m_debugDrawer->DrawLine(end, arrowTip3, color);
            m_debugDrawer->DrawLine(end, arrowTip4, color);
        }

        void AnimationDebugRenderer::DrawCoordinateSystem(const Math::Vec3& position, const Math::Quat& rotation, float size) {
            if (!m_debugDrawer) return;

            Math::Vec3 right = rotation * Math::Vec3(1.0f, 0.0f, 0.0f) * size;
            Math::Vec3 up = rotation * Math::Vec3(0.0f, 1.0f, 0.0f) * size;
            Math::Vec3 forward = rotation * Math::Vec3(0.0f, 0.0f, 1.0f) * size;

            // X axis - Red
            m_debugDrawer->DrawLine(position, position + right, Math::Vec3(1.0f, 0.0f, 0.0f));
            // Y axis - Green
            m_debugDrawer->DrawLine(position, position + up, Math::Vec3(0.0f, 1.0f, 0.0f));
            // Z axis - Blue
            m_debugDrawer->DrawLine(position, position + forward, Math::Vec3(0.0f, 0.0f, 1.0f));
        }

        std::string AnimationDebugRenderer::FormatFloat(float value, int precision) const {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(precision) << value;
            return oss.str();
        }

        // AnimationDebugInfoCollector implementation
        StateMachineDebugInfo AnimationDebugInfoCollector::CollectStateMachineInfo(const AnimationController& controller) {
            StateMachineDebugInfo info;
            
            auto stateMachine = controller.GetStateMachine();
            if (!stateMachine) {
                return info;
            }

            // Get current state information
            auto currentState = stateMachine->GetCurrentState();
            if (currentState) {
                info.currentStateName = currentState->GetName();
            }
            
            info.currentStateTime = stateMachine->GetCurrentStateTime();
            
            // Get debug info from state machine
            auto debugInfo = stateMachine->GetDebugInfo();
            info.isTransitioning = debugInfo.isTransitioning;
            info.transitionProgress = debugInfo.transitionProgress;
            info.transitionToState = debugInfo.transitionToState;
            info.previousStateName = debugInfo.previousStateName;
            
            // Collect parameters (simplified - would need actual parameter access)
            // This would require extending the AnimationController interface
            
            return info;
        }

        std::vector<IKChainDebugInfo> AnimationDebugInfoCollector::CollectIKChainInfo(const AnimationController& controller) {
            std::vector<IKChainDebugInfo> ikChains;
            
            // This would require access to IK solvers from the controller
            // Implementation depends on how IK is integrated with the controller
            
            return ikChains;
        }

        std::vector<BlendTreeDebugInfo> AnimationDebugInfoCollector::CollectBlendTreeInfo(const AnimationController& controller) {
            std::vector<BlendTreeDebugInfo> blendTrees;
            
            // This would require access to blend trees from the controller
            // Implementation depends on how blend trees are integrated
            
            return blendTrees;
        }

    } // namespace Animation
} // namespace GameEngine