#include "Animation/IKSolver.h"
#include "Core/Logger.h"
#include <cmath>

namespace GameEngine {
namespace Animation {

TwoBoneIK::TwoBoneIK() : IKSolver(Type::TwoBone) {
}

void TwoBoneIK::SetUpperBone(int boneIndex) {
    m_upperBone = boneIndex;
    if (m_upperBone >= 0 && m_lowerBone >= 0 && m_endEffector >= 0) {
        m_boneChain = { m_upperBone, m_lowerBone, m_endEffector };
    }
}

void TwoBoneIK::SetLowerBone(int boneIndex) {
    m_lowerBone = boneIndex;
    if (m_upperBone >= 0 && m_lowerBone >= 0 && m_endEffector >= 0) {
        m_boneChain = { m_upperBone, m_lowerBone, m_endEffector };
    }
}

void TwoBoneIK::SetEndEffector(int boneIndex) {
    m_endEffector = boneIndex;
    if (m_upperBone >= 0 && m_lowerBone >= 0 && m_endEffector >= 0) {
        m_boneChain = { m_upperBone, m_lowerBone, m_endEffector };
    }
}

bool TwoBoneIK::IsValidConfiguration() const {
    return m_upperBone >= 0 && m_lowerBone >= 0 && m_endEffector >= 0;
}

bool TwoBoneIK::Solve(Skeleton& skeleton) {
    if (!IsValidConfiguration()) {
        LOG_ERROR("TwoBoneIK: Invalid configuration - all bones must be set");
        return false;
    }

    if (!ValidateChain(skeleton)) {
        LOG_ERROR("TwoBoneIK: Invalid bone chain");
        return false;
    }

    if (!IsTargetReachable(skeleton)) {
        LOG_WARNING("TwoBoneIK: Target is not reachable");
        // Still attempt to solve for best effort
    }

    // Store original FK pose for blending
    StoreOriginalPose(skeleton);

    SolveTwoBoneIK(skeleton);

    // Apply IK/FK blending
    ApplyIKFKBlending(skeleton);

    return true;
}

void TwoBoneIK::SolveTwoBoneIK(Skeleton& skeleton) {
    // Get current bone positions
    Math::Vec3 shoulderPos = GetBonePosition(skeleton, m_upperBone);
    Math::Vec3 elbowPos = GetBonePosition(skeleton, m_lowerBone);
    Math::Vec3 wristPos = GetBonePosition(skeleton, m_endEffector);

    // Calculate bone lengths
    float upperLength = glm::length(elbowPos - shoulderPos);
    float lowerLength = glm::length(wristPos - elbowPos);

    if (upperLength < 0.001f || lowerLength < 0.001f) {
        LOG_ERROR("TwoBoneIK: Invalid bone lengths");
        return;
    }

    // Calculate new elbow position
    Math::Vec3 newElbowPos = CalculateElbowPosition(shoulderPos, m_targetPosition, 
                                                   m_poleTarget, upperLength, lowerLength);

    // Calculate rotations for upper bone (shoulder to elbow)
    Math::Vec3 upperDirection = glm::normalize(newElbowPos - shoulderPos);
    Math::Vec3 originalUpperDirection = glm::normalize(elbowPos - shoulderPos);
    Math::Quat upperRotation = CalculateBoneRotation(originalUpperDirection, upperDirection);

    // Apply upper bone rotation with constraints
    Math::Quat currentUpperRotation = GetBoneRotation(skeleton, m_upperBone);
    Math::Quat newUpperRotation = upperRotation * currentUpperRotation;
    ApplyBoneConstraints(skeleton, m_upperBone, newUpperRotation);

    // Update skeleton to get new elbow position after upper bone rotation
    // skeleton.UpdateBoneTransforms(); // Commented out for testing
    Math::Vec3 actualElbowPos = GetBonePosition(skeleton, m_lowerBone);

    // Calculate rotation for lower bone (elbow to target)
    Math::Vec3 lowerDirection = glm::normalize(m_targetPosition - actualElbowPos);
    Math::Vec3 originalLowerDirection = glm::normalize(wristPos - elbowPos);
    Math::Quat lowerRotation = CalculateBoneRotation(originalLowerDirection, lowerDirection);

    // Apply lower bone rotation with constraints
    Math::Quat currentLowerRotation = GetBoneRotation(skeleton, m_lowerBone);
    Math::Quat newLowerRotation = lowerRotation * currentLowerRotation;
    ApplyBoneConstraints(skeleton, m_lowerBone, newLowerRotation);

    // Apply target rotation to end effector if specified
    Math::Quat identityQuat(1.0f, 0.0f, 0.0f, 0.0f);
    if (m_targetRotation != identityQuat) {
        ApplyBoneConstraints(skeleton, m_endEffector, m_targetRotation);
    }

    // Update final bone transforms
    // skeleton.UpdateBoneTransforms(); // Commented out for testing
}

Math::Vec3 TwoBoneIK::CalculateElbowPosition(const Math::Vec3& shoulder, const Math::Vec3& target, 
                                           const Math::Vec3& poleTarget, float upperLength, float lowerLength) const {
    Math::Vec3 shoulderToTarget = target - shoulder;
    float targetDistance = glm::length(shoulderToTarget);
    
    if (targetDistance < 0.001f) {
        return shoulder + Math::Vec3(0, upperLength, 0);
    }

    // Clamp target distance to reachable range
    float maxReach = upperLength + lowerLength;
    float minReach = std::abs(upperLength - lowerLength);
    targetDistance = glm::clamp(targetDistance, minReach + 0.001f, maxReach - 0.001f);

    // Calculate elbow position using law of cosines
    float cosAngle = (upperLength * upperLength + targetDistance * targetDistance - lowerLength * lowerLength) /
                     (2.0f * upperLength * targetDistance);
    cosAngle = glm::clamp(cosAngle, -1.0f, 1.0f);
    
    float angle = std::acos(cosAngle);

    // Create coordinate system
    Math::Vec3 targetDirection = glm::normalize(shoulderToTarget);
    Math::Vec3 poleDirection = glm::normalize(poleTarget - shoulder);
    
    // Remove component of pole direction that's parallel to target direction
    poleDirection = poleDirection - glm::dot(poleDirection, targetDirection) * targetDirection;
    poleDirection = glm::normalize(poleDirection);

    // Calculate elbow position
    Math::Vec3 elbowDirection = std::cos(angle) * targetDirection + std::sin(angle) * poleDirection;
    return shoulder + upperLength * elbowDirection;
}

Math::Quat TwoBoneIK::CalculateBoneRotation(const Math::Vec3& from, const Math::Vec3& to, const Math::Vec3& up) const {
    Math::Vec3 fromNorm = glm::normalize(from);
    Math::Vec3 toNorm = glm::normalize(to);

    // Check if vectors are parallel
    float dot = glm::dot(fromNorm, toNorm);
    if (dot > 0.9999f) {
        return Math::Quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
    }
    if (dot < -0.9999f) {
        // 180 degree rotation - find perpendicular axis
        Math::Vec3 axis = glm::cross(fromNorm, up);
        if (glm::length(axis) < 0.001f) {
            axis = glm::cross(fromNorm, Math::Vec3(1, 0, 0));
        }
        axis = glm::normalize(axis);
        return glm::angleAxis(Math::PI, axis);
    }

    // Calculate rotation quaternion
    Math::Vec3 axis = glm::cross(fromNorm, toNorm);
    axis = glm::normalize(axis);
    float angle = std::acos(glm::clamp(dot, -1.0f, 1.0f));
    
    return glm::angleAxis(angle, axis);
}

} // namespace Animation
} // namespace GameEngine