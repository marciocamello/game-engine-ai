#pragma once

#include "Core/Math.h"
#include "Animation/Skeleton.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace GameEngine {
namespace Animation {

/**
 * Base class for Inverse Kinematics solvers
 * Provides common functionality for IK chain management and solving
 */
class IKSolver {
public:
    enum class Type { TwoBone, FABRIK, CCD };

    // Lifecycle
    IKSolver(Type type = Type::TwoBone);
    virtual ~IKSolver() = default;

    // Chain setup
    void SetChain(const std::vector<int>& boneIndices);
    void SetTarget(const Math::Vec3& position, const Math::Quat& rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f));
    void SetPoleTarget(const Math::Vec3& position);

    // Constraints
    void SetBoneConstraints(int boneIndex, float minAngle, float maxAngle);
    void SetChainLength(float length);
    void SetIterations(int iterations);
    void SetTolerance(float tolerance);

    // Solving
    virtual bool Solve(Skeleton& skeleton) = 0;
    bool IsTargetReachable(const Skeleton& skeleton) const;

    // Properties
    Type GetType() const { return m_type; }
    const std::vector<int>& GetChain() const { return m_boneChain; }
    const Math::Vec3& GetTarget() const { return m_targetPosition; }
    const Math::Quat& GetTargetRotation() const { return m_targetRotation; }
    const Math::Vec3& GetPoleTarget() const { return m_poleTarget; }
    int GetIterations() const { return m_iterations; }
    float GetTolerance() const { return m_tolerance; }

    // IK/FK Blending
    void SetIKWeight(float weight);
    float GetIKWeight() const { return m_ikWeight; }
    void SetBlendMode(bool smoothBlending) { m_smoothBlending = smoothBlending; }
    bool GetBlendMode() const { return m_smoothBlending; }

    // Validation
    bool ValidateChain(const Skeleton& skeleton) const;

protected:
    Type m_type;
    std::vector<int> m_boneChain;
    Math::Vec3 m_targetPosition;
    Math::Quat m_targetRotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);
    Math::Vec3 m_poleTarget;

    int m_iterations = 10;
    float m_tolerance = 0.01f;
    float m_chainLength = 0.0f;
    float m_ikWeight = 1.0f;
    bool m_smoothBlending = true;

    std::unordered_map<int, std::pair<float, float>> m_boneConstraints;
    
    // Store original FK poses for blending
    mutable std::vector<Math::Quat> m_originalRotations;
    mutable std::vector<Math::Vec3> m_originalPositions;

    // Helper methods
    float CalculateChainLength(const Skeleton& skeleton) const;
    void ApplyBoneConstraints(Skeleton& skeleton, int boneIndex, const Math::Quat& rotation) const;
    Math::Vec3 GetBonePosition(const Skeleton& skeleton, int boneIndex) const;
    Math::Quat GetBoneRotation(const Skeleton& skeleton, int boneIndex) const;
    void SetBoneRotation(Skeleton& skeleton, int boneIndex, const Math::Quat& rotation) const;
    
    // IK/FK Blending helpers
    void StoreOriginalPose(const Skeleton& skeleton) const;
    void ApplyIKFKBlending(Skeleton& skeleton) const;
    Math::Quat BlendRotations(const Math::Quat& fkRotation, const Math::Quat& ikRotation, float weight) const;
    
    // Temporary helper methods for testing (will be replaced with proper Skeleton methods)
    int GetParent(const Skeleton& skeleton, int boneIndex) const;
    Math::Mat4 GetBoneWorldTransform(const Skeleton& skeleton, int boneIndex) const;
    void SetBoneLocalTransform(Skeleton& skeleton, int boneIndex, const Math::Mat4& transform) const;
};

/**
 * Two-bone IK solver for arms and legs
 * Solves IK for chains with exactly 3 bones (upper, lower, end effector)
 */
class TwoBoneIK : public IKSolver {
public:
    TwoBoneIK();

    // Bone setup
    void SetUpperBone(int boneIndex);
    void SetLowerBone(int boneIndex);
    void SetEndEffector(int boneIndex);

    // Properties
    int GetUpperBone() const { return m_upperBone; }
    int GetLowerBone() const { return m_lowerBone; }
    int GetEndEffector() const { return m_endEffector; }

    // Solving
    bool Solve(Skeleton& skeleton) override;

    // Validation
    bool IsValidConfiguration() const;

private:
    int m_upperBone = -1;
    int m_lowerBone = -1;
    int m_endEffector = -1;

    // Two-bone IK specific solving
    void SolveTwoBoneIK(Skeleton& skeleton);
    Math::Vec3 CalculateElbowPosition(const Math::Vec3& shoulder, const Math::Vec3& target, 
                                     const Math::Vec3& poleTarget, float upperLength, float lowerLength) const;
    Math::Quat CalculateBoneRotation(const Math::Vec3& from, const Math::Vec3& to, 
                                    const Math::Vec3& up = Math::Vec3(0, 1, 0)) const;
};

/**
 * FABRIK (Forward and Backward Reaching Inverse Kinematics) solver
 * Handles complex IK chains with multiple bones
 */
class FABRIKIK : public IKSolver {
public:
    FABRIKIK();

    // Solving
    bool Solve(Skeleton& skeleton) override;

    // FABRIK specific settings
    void SetSubBasePosition(const Math::Vec3& position);
    const Math::Vec3& GetSubBasePosition() const { return m_subBasePosition; }
    
    // Bone length validation
    void SetBoneLengthValidation(bool enabled) { m_validateBoneLengths = enabled; }
    bool GetBoneLengthValidation() const { return m_validateBoneLengths; }
    
    // Joint angle constraints for FABRIK
    void SetJointAngleConstraints(bool enabled) { m_useJointConstraints = enabled; }
    bool GetJointAngleConstraints() const { return m_useJointConstraints; }

private:
    Math::Vec3 m_subBasePosition;
    std::vector<Math::Vec3> m_positions;
    std::vector<float> m_boneLengths;
    bool m_validateBoneLengths = true;
    bool m_useJointConstraints = true;

    // FABRIK algorithm steps
    void ForwardReach(std::vector<Math::Vec3>& positions);
    void BackwardReach(std::vector<Math::Vec3>& positions);
    void ApplyPositionsToSkeleton(Skeleton& skeleton, const std::vector<Math::Vec3>& positions);
    void InitializePositions(const Skeleton& skeleton);
    void CalculateBoneLengths(const Skeleton& skeleton);
    Math::Quat CalculateRotationBetweenVectors(const Math::Vec3& from, const Math::Vec3& to) const;
    
    // Enhanced constraint handling
    void ApplyJointConstraints(std::vector<Math::Vec3>& positions);
    bool ValidateBoneLengths(const std::vector<Math::Vec3>& positions) const;
    void CorrectBoneLengths(std::vector<Math::Vec3>& positions);
};

} // namespace Animation
} // namespace GameEngine