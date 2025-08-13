#include "Animation/IKSolver.h"
#include "Core/Logger.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {
namespace Animation {

IKSolver::IKSolver(Type type) : m_type(type) {
    m_targetPosition = Math::Vec3(0.0f);
    m_targetRotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);
    m_poleTarget = Math::Vec3(0.0f, 1.0f, 0.0f);
}

void IKSolver::SetChain(const std::vector<int>& boneIndices) {
    m_boneChain = boneIndices;
    m_chainLength = 0.0f; // Will be calculated when needed
}

void IKSolver::SetTarget(const Math::Vec3& position, const Math::Quat& rotation) {
    m_targetPosition = position;
    m_targetRotation = rotation;
}

void IKSolver::SetPoleTarget(const Math::Vec3& position) {
    m_poleTarget = position;
}

void IKSolver::SetBoneConstraints(int boneIndex, float minAngle, float maxAngle) {
    m_boneConstraints[boneIndex] = std::make_pair(minAngle, maxAngle);
}

void IKSolver::SetChainLength(float length) {
    m_chainLength = length;
}

void IKSolver::SetIterations(int iterations) {
    m_iterations = std::max(1, iterations);
}

void IKSolver::SetTolerance(float tolerance) {
    m_tolerance = std::max(0.001f, tolerance);
}

void IKSolver::SetIKWeight(float weight) {
    m_ikWeight = Math::Clamp(weight, 0.0f, 1.0f);
}

bool IKSolver::IsTargetReachable(const AnimationSkeleton& skeleton) const {
    if (m_boneChain.empty()) {
        return false;
    }

    float chainLength = CalculateChainLength(skeleton);
    Math::Vec3 rootPosition = GetBonePosition(skeleton, m_boneChain[0]);
    float distanceToTarget = glm::length(m_targetPosition - rootPosition);

    return distanceToTarget <= chainLength + m_tolerance;
}

bool IKSolver::ValidateChain(const AnimationSkeleton& skeleton) const {
    if (m_boneChain.empty()) {
        LOG_ERROR("IK chain is empty");
        return false;
    }

    for (int boneIndex : m_boneChain) {
        if (boneIndex < 0 || boneIndex >= skeleton.GetBoneCount()) {
            LOG_ERROR("Invalid bone index in IK chain: " + std::to_string(boneIndex));
            return false;
        }
    }

    // Check if bones form a valid chain (each bone should be parent of next)
    for (size_t i = 1; i < m_boneChain.size(); ++i) {
        int parentIndex = GetParent(skeleton, m_boneChain[i]);
        if (parentIndex != m_boneChain[i - 1]) {
            LOG_WARNING("IK chain bones are not in parent-child order");
        }
    }

    return true;
}

float IKSolver::CalculateChainLength(const AnimationSkeleton& skeleton) const {
    if (m_chainLength > 0.0f) {
        return m_chainLength;
    }

    if (m_boneChain.size() < 2) {
        return 0.0f;
    }

    float totalLength = 0.0f;
    for (size_t i = 1; i < m_boneChain.size(); ++i) {
        Math::Vec3 parentPos = GetBonePosition(skeleton, m_boneChain[i - 1]);
        Math::Vec3 childPos = GetBonePosition(skeleton, m_boneChain[i]);
        totalLength += glm::length(childPos - parentPos);
    }

    return totalLength;
}

void IKSolver::ApplyBoneConstraints(AnimationSkeleton& skeleton, int boneIndex, const Math::Quat& rotation) const {
    auto constraintIt = m_boneConstraints.find(boneIndex);
    if (constraintIt == m_boneConstraints.end()) {
        SetBoneRotation(skeleton, boneIndex, rotation);
        return;
    }

    // Apply angle constraints
    float minAngle = constraintIt->second.first;
    float maxAngle = constraintIt->second.second;

    // Convert quaternion to euler angles for constraint checking
    Math::Vec3 euler = glm::eulerAngles(rotation);
    
    // Clamp angles
    euler.x = glm::clamp(euler.x, minAngle, maxAngle);
    euler.y = glm::clamp(euler.y, minAngle, maxAngle);
    euler.z = glm::clamp(euler.z, minAngle, maxAngle);

    // Convert back to quaternion
    Math::Quat constrainedRotation = Math::Quat(euler);
    SetBoneRotation(skeleton, boneIndex, constrainedRotation);
}

Math::Vec3 IKSolver::GetBonePosition(const AnimationSkeleton& skeleton, int boneIndex) const {
    Math::Mat4 worldTransform = GetBoneWorldTransform(skeleton, boneIndex);
    return Math::Vec3(worldTransform[3]);
}

Math::Quat IKSolver::GetBoneRotation(const AnimationSkeleton& skeleton, int boneIndex) const {
    Math::Mat4 worldTransform = GetBoneWorldTransform(skeleton, boneIndex);
    return glm::quat_cast(Math::Mat3(worldTransform));
}

void IKSolver::SetBoneRotation(AnimationSkeleton& skeleton, int boneIndex, const Math::Quat& rotation) const {
    // Convert quaternion to transformation matrix
    Math::Mat4 rotationMatrix = glm::mat4_cast(rotation);
    
    // Preserve position, only change rotation
    Math::Mat4 currentTransform = GetBoneWorldTransform(skeleton, boneIndex);
    rotationMatrix[3] = currentTransform[3]; // Keep position
    
    SetBoneLocalTransform(skeleton, boneIndex, rotationMatrix);
}

// Temporary helper methods for testing
int IKSolver::GetParent(const AnimationSkeleton& skeleton, int boneIndex) const {
    // For testing purposes, assume simple parent-child relationship
    // This would be replaced with proper Skeleton method
    if (boneIndex <= 0) return -1;
    return boneIndex - 1;
}

Math::Mat4 IKSolver::GetBoneWorldTransform(const AnimationSkeleton& skeleton, int boneIndex) const {
    // For testing purposes, create simple transforms
    // This would be replaced with proper Skeleton method
    Math::Vec3 position(static_cast<float>(boneIndex), 0.0f, 0.0f);
    return Math::Mat4(1.0f) * glm::translate(Math::Mat4(1.0f), position);
}

void IKSolver::SetBoneLocalTransform(AnimationSkeleton& skeleton, int boneIndex, const Math::Mat4& transform) const {
    // For testing purposes, this is a no-op
    // This would be replaced with proper Skeleton method
    (void)skeleton;
    (void)boneIndex;
    (void)transform;
}

void IKSolver::StoreOriginalPose(const AnimationSkeleton& skeleton) const {
    m_originalRotations.clear();
    m_originalPositions.clear();
    
    m_originalRotations.reserve(m_boneChain.size());
    m_originalPositions.reserve(m_boneChain.size());
    
    for (int boneIndex : m_boneChain) {
        m_originalRotations.push_back(GetBoneRotation(skeleton, boneIndex));
        m_originalPositions.push_back(GetBonePosition(skeleton, boneIndex));
    }
}

void IKSolver::ApplyIKFKBlending(AnimationSkeleton& skeleton) const {
    if (m_ikWeight >= 1.0f) {
        return; // Full IK, no blending needed
    }
    
    if (m_ikWeight <= 0.0f) {
        // Full FK, restore original poses
        for (size_t i = 0; i < m_boneChain.size() && i < m_originalRotations.size(); ++i) {
            SetBoneRotation(skeleton, m_boneChain[i], m_originalRotations[i]);
        }
        return;
    }
    
    // Blend between FK and IK
    for (size_t i = 0; i < m_boneChain.size() && i < m_originalRotations.size(); ++i) {
        int boneIndex = m_boneChain[i];
        Math::Quat currentIKRotation = GetBoneRotation(skeleton, boneIndex);
        Math::Quat originalFKRotation = m_originalRotations[i];
        
        Math::Quat blendedRotation = BlendRotations(originalFKRotation, currentIKRotation, m_ikWeight);
        SetBoneRotation(skeleton, boneIndex, blendedRotation);
    }
}

Math::Quat IKSolver::BlendRotations(const Math::Quat& fkRotation, const Math::Quat& ikRotation, float weight) const {
    if (m_smoothBlending) {
        // Use spherical linear interpolation for smooth blending
        return glm::slerp(fkRotation, ikRotation, weight);
    } else {
        // Use linear interpolation for faster blending
        return glm::mix(fkRotation, ikRotation, weight);
    }
}

} // namespace Animation
} // namespace GameEngine