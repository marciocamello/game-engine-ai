#include "Animation/IKSolver.h"
#include "Core/Logger.h"
#include <algorithm>

namespace GameEngine {
namespace Animation {

FABRIKIK::FABRIKIK() : IKSolver(Type::FABRIK) {
    m_subBasePosition = Math::Vec3(0.0f);
    m_validateBoneLengths = true;
    m_useJointConstraints = true;
}

void FABRIKIK::SetSubBasePosition(const Math::Vec3& position) {
    m_subBasePosition = position;
}

bool FABRIKIK::Solve(AnimationSkeleton& skeleton) {
    if (m_boneChain.empty()) {
        LOG_ERROR("FABRIK: No bones in chain");
        return false;
    }

    if (!ValidateChain(skeleton)) {
        LOG_ERROR("FABRIK: Invalid bone chain");
        return false;
    }

    // Store original FK pose for blending
    StoreOriginalPose(skeleton);

    // Initialize positions and bone lengths
    InitializePositions(skeleton);
    CalculateBoneLengths(skeleton);

    if (m_positions.empty() || m_boneLengths.empty()) {
        LOG_ERROR("FABRIK: Failed to initialize positions or bone lengths");
        return false;
    }

    // Check if target is reachable
    if (!IsTargetReachable(skeleton)) {
        LOG_WARNING("FABRIK: Target may not be fully reachable");
    }

    // Store original root position
    Math::Vec3 rootPosition = m_positions[0];

    // FABRIK algorithm iterations
    for (int iteration = 0; iteration < m_iterations; ++iteration) {
        // Check if we've reached the target within tolerance
        float distanceToTarget = glm::length(m_positions.back() - m_targetPosition);
        if (distanceToTarget <= m_tolerance) {
            break;
        }

        // Forward reaching phase
        ForwardReach(m_positions);

        // Backward reaching phase
        BackwardReach(m_positions);

        // Restore root position
        m_positions[0] = rootPosition;
        
        // Apply joint constraints if enabled
        if (m_useJointConstraints) {
            ApplyJointConstraints(m_positions);
        }
        
        // Validate and correct bone lengths if enabled
        if (m_validateBoneLengths) {
            if (!ValidateBoneLengths(m_positions)) {
                CorrectBoneLengths(m_positions);
            }
        }
    }

    // Apply the calculated positions back to the skeleton
    ApplyPositionsToSkeleton(skeleton, m_positions);

    // Apply IK/FK blending
    ApplyIKFKBlending(skeleton);

    return true;
}

void FABRIKIK::ForwardReach(std::vector<Math::Vec3>& positions) {
    // Set the end effector to the target position
    positions.back() = m_targetPosition;

    // Work backwards from end effector to root
    for (int i = static_cast<int>(positions.size()) - 2; i >= 0; --i) {
        Math::Vec3 direction = glm::normalize(positions[i] - positions[i + 1]);
        positions[i] = positions[i + 1] + direction * m_boneLengths[i];
    }
}

void FABRIKIK::BackwardReach(std::vector<Math::Vec3>& positions) {
    // Set the root to its original position (or sub-base if specified)
    if (m_subBasePosition != Math::Vec3(0.0f)) {
        positions[0] = m_subBasePosition;
    }

    // Work forwards from root to end effector
    for (size_t i = 1; i < positions.size(); ++i) {
        Math::Vec3 direction = glm::normalize(positions[i] - positions[i - 1]);
        positions[i] = positions[i - 1] + direction * m_boneLengths[i - 1];
    }
}

void FABRIKIK::ApplyPositionsToSkeleton(AnimationSkeleton& skeleton, const std::vector<Math::Vec3>& positions) {
    // Apply rotations to bones based on the new positions
    for (size_t i = 0; i < m_boneChain.size() - 1; ++i) {
        int boneIndex = m_boneChain[i];
        
        // Calculate the direction from current bone to next bone
        Math::Vec3 newDirection = glm::normalize(positions[i + 1] - positions[i]);
        
        // Get the original direction
        Math::Vec3 originalPos = GetBonePosition(skeleton, boneIndex);
        Math::Vec3 originalNextPos = GetBonePosition(skeleton, m_boneChain[i + 1]);
        Math::Vec3 originalDirection = glm::normalize(originalNextPos - originalPos);

        // Calculate rotation needed
        Math::Quat rotation = CalculateRotationBetweenVectors(originalDirection, newDirection);
        
        // Apply rotation with constraints
        Math::Quat currentRotation = GetBoneRotation(skeleton, boneIndex);
        Math::Quat newRotation = rotation * currentRotation;
        ApplyBoneConstraints(skeleton, boneIndex, newRotation);
    }

    // Update bone transforms
    // skeleton.UpdateBoneTransforms(); // Commented out for testing
}

void FABRIKIK::InitializePositions(const AnimationSkeleton& skeleton) {
    m_positions.clear();
    m_positions.reserve(m_boneChain.size());

    for (int boneIndex : m_boneChain) {
        Math::Vec3 position = GetBonePosition(skeleton, boneIndex);
        m_positions.push_back(position);
    }
}

void FABRIKIK::CalculateBoneLengths(const AnimationSkeleton& skeleton) {
    m_boneLengths.clear();
    
    if (m_positions.size() < 2) {
        return;
    }

    m_boneLengths.reserve(m_positions.size() - 1);

    for (size_t i = 0; i < m_positions.size() - 1; ++i) {
        float length = glm::length(m_positions[i + 1] - m_positions[i]);
        m_boneLengths.push_back(std::max(length, 0.001f)); // Ensure minimum length
    }
}

Math::Quat FABRIKIK::CalculateRotationBetweenVectors(const Math::Vec3& from, const Math::Vec3& to) const {
    Math::Vec3 fromNorm = glm::normalize(from);
    Math::Vec3 toNorm = glm::normalize(to);

    float dot = glm::dot(fromNorm, toNorm);
    
    // Check if vectors are parallel
    if (dot > 0.9999f) {
        return Math::Quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
    }
    
    // Check if vectors are opposite
    if (dot < -0.9999f) {
        // Find perpendicular axis
        Math::Vec3 axis = glm::cross(fromNorm, Math::Vec3(1, 0, 0));
        if (glm::length(axis) < 0.001f) {
            axis = glm::cross(fromNorm, Math::Vec3(0, 1, 0));
        }
        axis = glm::normalize(axis);
        return glm::angleAxis(Math::PI, axis);
    }

    // Calculate rotation
    Math::Vec3 axis = glm::cross(fromNorm, toNorm);
    axis = glm::normalize(axis);
    float angle = std::acos(glm::clamp(dot, -1.0f, 1.0f));
    
    return glm::angleAxis(angle, axis);
}

void FABRIKIK::ApplyJointConstraints(std::vector<Math::Vec3>& positions) {
    // Apply joint angle constraints to prevent unrealistic poses
    for (size_t i = 1; i < positions.size() - 1; ++i) {
        int boneIndex = m_boneChain[i];
        
        // Check if this bone has constraints
        auto constraintIt = m_boneConstraints.find(boneIndex);
        if (constraintIt == m_boneConstraints.end()) {
            continue;
        }
        
        float minAngle = constraintIt->second.first;
        float maxAngle = constraintIt->second.second;
        
        // Calculate current joint angle
        Math::Vec3 prevDir = glm::normalize(positions[i] - positions[i - 1]);
        Math::Vec3 nextDir = glm::normalize(positions[i + 1] - positions[i]);
        
        float currentAngle = std::acos(glm::clamp(glm::dot(prevDir, nextDir), -1.0f, 1.0f));
        
        // Clamp angle if necessary
        if (currentAngle < minAngle || currentAngle > maxAngle) {
            float targetAngle = glm::clamp(currentAngle, minAngle, maxAngle);
            
            // Adjust position to maintain target angle
            Math::Vec3 axis = glm::cross(prevDir, nextDir);
            if (glm::length(axis) > 0.001f) {
                axis = glm::normalize(axis);
                Math::Quat rotation = glm::angleAxis(targetAngle - currentAngle, axis);
                Math::Vec3 adjustedDir = rotation * nextDir;
                
                float boneLength = glm::length(positions[i + 1] - positions[i]);
                positions[i + 1] = positions[i] + adjustedDir * boneLength;
            }
        }
    }
}

bool FABRIKIK::ValidateBoneLengths(const std::vector<Math::Vec3>& positions) const {
    if (positions.size() != m_boneLengths.size() + 1) {
        return false;
    }
    
    const float tolerance = 0.01f; // 1cm tolerance
    
    for (size_t i = 0; i < m_boneLengths.size(); ++i) {
        float currentLength = glm::length(positions[i + 1] - positions[i]);
        float expectedLength = m_boneLengths[i];
        
        if (std::abs(currentLength - expectedLength) > tolerance) {
            return false;
        }
    }
    
    return true;
}

void FABRIKIK::CorrectBoneLengths(std::vector<Math::Vec3>& positions) {
    // Correct bone lengths to maintain original proportions
    for (size_t i = 0; i < m_boneLengths.size() && i + 1 < positions.size(); ++i) {
        Math::Vec3 direction = positions[i + 1] - positions[i];
        float currentLength = glm::length(direction);
        
        if (currentLength > 0.001f) {
            direction = glm::normalize(direction);
            positions[i + 1] = positions[i] + direction * m_boneLengths[i];
        }
    }
}

} // namespace Animation
} // namespace GameEngine