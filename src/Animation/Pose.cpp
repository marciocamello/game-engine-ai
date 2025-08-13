#include "Animation/Pose.h"
#include "Animation/Animation.h"
#include "Core/Logger.h"
#include <algorithm>

namespace GameEngine {
namespace Animation {

    // BoneTransform implementation
    Math::Mat4 BoneTransform::ToMatrix() const {
        Math::Mat4 translationMatrix = glm::translate(Math::Mat4(1.0f), position);
        Math::Mat4 rotationMatrix = glm::mat4_cast(rotation);
        Math::Mat4 scaleMatrix = glm::scale(Math::Mat4(1.0f), scale);
        
        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    BoneTransform BoneTransform::Lerp(const BoneTransform& a, const BoneTransform& b, float t) {
        BoneTransform result;
        result.position = glm::mix(a.position, b.position, t);
        result.rotation = glm::slerp(a.rotation, b.rotation, t);
        result.scale = glm::mix(a.scale, b.scale, t);
        return result;
    }

    BoneTransform BoneTransform::Slerp(const BoneTransform& a, const BoneTransform& b, float t) {
        // Same as Lerp for now, but could implement different interpolation for position/scale
        return Lerp(a, b, t);
    }

    BoneTransform BoneTransform::operator+(const BoneTransform& other) const {
        BoneTransform result;
        result.position = position + other.position;
        result.rotation = rotation * other.rotation; // Quaternion multiplication for rotation
        result.scale = scale * other.scale; // Component-wise multiplication for scale
        return result;
    }

    BoneTransform BoneTransform::operator*(float weight) const {
        BoneTransform result;
        result.position = position * weight;
        result.rotation = glm::slerp(Math::Quat(1.0f, 0.0f, 0.0f, 0.0f), rotation, weight);
        result.scale = glm::mix(Math::Vec3(1.0f), scale, weight);
        return result;
    }

    BoneTransform& BoneTransform::operator+=(const BoneTransform& other) {
        position += other.position;
        rotation = rotation * other.rotation;
        scale *= other.scale;
        return *this;
    }

    BoneTransform& BoneTransform::operator*=(float weight) {
        position *= weight;
        rotation = glm::slerp(Math::Quat(1.0f, 0.0f, 0.0f, 0.0f), rotation, weight);
        scale = glm::mix(Math::Vec3(1.0f), scale, weight);
        return *this;
    }

    // Pose implementation
    Pose::Pose(std::shared_ptr<Skeleton> skeleton) : m_skeleton(skeleton) {
        if (skeleton) {
            ResetToBindPose();
        }
    }

    void Pose::SetSkeleton(std::shared_ptr<Skeleton> skeleton) {
        m_skeleton = skeleton;
        if (skeleton) {
            ResetToBindPose();
        }
    }

    void Pose::SetBoneTransform(const std::string& boneName, const BoneTransform& transform) {
        m_boneTransforms[boneName] = transform;
        
        // Update by-id map if skeleton is available
        if (auto skeleton = m_skeleton.lock()) {
            if (auto bone = skeleton->GetBone(boneName)) {
                m_boneTransformsById[bone->GetId()] = transform;
            }
        }
    }

    void Pose::SetBoneTransform(int32_t boneId, const BoneTransform& transform) {
        m_boneTransformsById[boneId] = transform;
        
        // Update by-name map if skeleton is available
        if (auto skeleton = m_skeleton.lock()) {
            if (auto bone = skeleton->GetBone(boneId)) {
                m_boneTransforms[bone->GetName()] = transform;
            }
        }
    }

    BoneTransform Pose::GetBoneTransform(const std::string& boneName) const {
        auto it = m_boneTransforms.find(boneName);
        if (it != m_boneTransforms.end()) {
            return it->second;
        }
        
        // Return bind pose transform if available
        return GetBindPoseTransform(boneName);
    }

    BoneTransform Pose::GetBoneTransform(int32_t boneId) const {
        auto it = m_boneTransformsById.find(boneId);
        if (it != m_boneTransformsById.end()) {
            return it->second;
        }
        
        // Try to get by name if skeleton is available
        if (auto skeleton = m_skeleton.lock()) {
            if (auto bone = skeleton->GetBone(boneId)) {
                return GetBoneTransform(bone->GetName());
            }
        }
        
        return BoneTransform(); // Default transform
    }

    bool Pose::HasBoneTransform(const std::string& boneName) const {
        return m_boneTransforms.find(boneName) != m_boneTransforms.end();
    }

    void Pose::SetLocalTransform(const std::string& boneName, const BoneTransform& transform) {
        SetBoneTransform(boneName, transform);
    }

    BoneTransform Pose::GetLocalTransform(const std::string& boneName) const {
        return GetBoneTransform(boneName);
    }

    void Pose::SetWorldTransform(const std::string& boneName, const BoneTransform& transform) {
        // Convert world transform to local transform
        if (auto skeleton = m_skeleton.lock()) {
            if (auto bone = skeleton->GetBone(boneName)) {
                BoneTransform localTransform = transform;
                
                // If bone has parent, convert world to local
                if (auto parent = bone->GetParent()) {
                    BoneTransform parentWorld = GetWorldTransform(parent->GetName());
                    // This is a simplified conversion - in practice, you'd need proper matrix math
                    // localTransform = inverse(parentWorld) * transform;
                }
                
                SetBoneTransform(boneName, localTransform);
            }
        }
    }

    BoneTransform Pose::GetWorldTransform(const std::string& boneName) const {
        if (auto skeleton = m_skeleton.lock()) {
            if (auto bone = skeleton->GetBone(boneName)) {
                BoneTransform worldTransform = GetBoneTransform(boneName);
                
                // If bone has parent, accumulate parent transforms
                if (auto parent = bone->GetParent()) {
                    BoneTransform parentWorld = GetWorldTransform(parent->GetName());
                    // This is a simplified combination - in practice, you'd need proper matrix math
                    worldTransform.position += parentWorld.position;
                    worldTransform.rotation = parentWorld.rotation * worldTransform.rotation;
                    worldTransform.scale *= parentWorld.scale;
                }
                
                return worldTransform;
            }
        }
        
        return GetBoneTransform(boneName);
    }

    void Pose::Reset() {
        m_boneTransforms.clear();
        m_boneTransformsById.clear();
    }

    void Pose::ResetToBindPose() {
        Reset();
        
        if (auto skeleton = m_skeleton.lock()) {
            for (const auto& bone : skeleton->GetAllBones()) {
                BoneTransform bindTransform;
                
                // Extract bind pose transform from bone
                Math::Vec3 position, scale;
                Math::Quat rotation;
                Bone::DecomposeTransform(bone->GetBindPose(), position, rotation, scale);
                
                bindTransform.position = position;
                bindTransform.rotation = rotation;
                bindTransform.scale = scale;
                
                SetBoneTransform(bone->GetName(), bindTransform);
            }
        }
    }

    void Pose::Clear() {
        Reset();
    }

    Pose Pose::Blend(const Pose& poseA, const Pose& poseB, float weight) {
        Pose result;
        
        // Set skeleton from poseA (assuming they're compatible)
        result.SetSkeleton(poseA.GetSkeleton());
        
        // Blend all bone transforms
        for (const auto& [boneName, transformA] : poseA.m_boneTransforms) {
            BoneTransform transformB = poseB.GetBoneTransform(boneName);
            BoneTransform blended = BoneTransform::Lerp(transformA, transformB, weight);
            result.SetBoneTransform(boneName, blended);
        }
        
        // Handle bones that exist only in poseB
        for (const auto& [boneName, transformB] : poseB.m_boneTransforms) {
            if (!poseA.HasBoneTransform(boneName)) {
                BoneTransform bindPose = result.GetBindPoseTransform(boneName);
                BoneTransform blended = BoneTransform::Lerp(bindPose, transformB, weight);
                result.SetBoneTransform(boneName, blended);
            }
        }
        
        return result;
    }

    Pose Pose::BlendAdditive(const Pose& basePose, const Pose& additivePose, float weight) {
        Pose result = basePose;
        
        // Apply additive transforms
        for (const auto& [boneName, additiveTransform] : additivePose.m_boneTransforms) {
            BoneTransform baseTransform = result.GetBoneTransform(boneName);
            BoneTransform weightedAdditive = additiveTransform * weight;
            BoneTransform blended = baseTransform + weightedAdditive;
            result.SetBoneTransform(boneName, blended);
        }
        
        return result;
    }

    void Pose::BlendWith(const Pose& other, float weight) {
        *this = Blend(*this, other, weight);
    }

    void Pose::BlendAdditiveWith(const Pose& additive, float weight) {
        *this = BlendAdditive(*this, additive, weight);
    }

    void Pose::EvaluateLocalToWorld() {
        if (auto skeleton = m_skeleton.lock()) {
            // Process bones in hierarchy order (parents before children)
            std::function<void(std::shared_ptr<Bone>)> processHierarchy = [&](std::shared_ptr<Bone> bone) {
                if (!bone) return;
                
                BoneTransform localTransform = GetBoneTransform(bone->GetName());
                BoneTransform worldTransform = localTransform;
                
                // Apply parent transform if exists
                if (auto parent = bone->GetParent()) {
                    BoneTransform parentWorld = GetWorldTransform(parent->GetName());
                    // Combine transforms (simplified - should use proper matrix math)
                    worldTransform.position = parentWorld.position + localTransform.position;
                    worldTransform.rotation = parentWorld.rotation * localTransform.rotation;
                    worldTransform.scale = parentWorld.scale * localTransform.scale;
                }
                
                SetWorldTransform(bone->GetName(), worldTransform);
                
                // Process children
                for (auto& child : bone->GetChildren()) {
                    processHierarchy(child);
                }
            };
            
            if (auto rootBone = skeleton->GetRootBone()) {
                processHierarchy(rootBone);
            }
        }
    }

    void Pose::EvaluateWorldToLocal() {
        // This would convert world transforms back to local transforms
        // Implementation depends on specific requirements
    }

    void Pose::ApplyToSkeleton() {
        if (auto skeleton = m_skeleton.lock()) {
            for (const auto& [boneName, transform] : m_boneTransforms) {
                if (auto bone = skeleton->GetBone(boneName)) {
                    bone->SetLocalTransform(transform.ToMatrix());
                }
            }
            skeleton->UpdateBoneTransforms();
        }
    }

    void Pose::ExtractFromSkeleton() {
        if (auto skeleton = m_skeleton.lock()) {
            Reset();
            
            for (const auto& bone : skeleton->GetAllBones()) {
                BoneTransform transform;
                
                // Extract transform from bone
                Math::Vec3 position, scale;
                Math::Quat rotation;
                Bone::DecomposeTransform(bone->GetLocalTransform(), position, rotation, scale);
                
                transform.position = position;
                transform.rotation = rotation;
                transform.scale = scale;
                
                SetBoneTransform(bone->GetName(), transform);
            }
        }
    }

    std::vector<Math::Mat4> Pose::GetSkinningMatrices() const {
        std::vector<Math::Mat4> matrices;
        GetSkinningMatrices(matrices);
        return matrices;
    }

    void Pose::GetSkinningMatrices(std::vector<Math::Mat4>& outMatrices) const {
        outMatrices.clear();
        
        if (auto skeleton = m_skeleton.lock()) {
            outMatrices.reserve(skeleton->GetBoneCount());
            
            for (const auto& bone : skeleton->GetAllBones()) {
                BoneTransform worldTransform = GetWorldTransform(bone->GetName());
                Math::Mat4 worldMatrix = worldTransform.ToMatrix();
                Math::Mat4 skinningMatrix = worldMatrix * bone->GetInverseBindPose();
                outMatrices.push_back(skinningMatrix);
            }
        }
    }

    std::vector<std::string> Pose::GetBoneNames() const {
        std::vector<std::string> names;
        names.reserve(m_boneTransforms.size());
        
        for (const auto& [boneName, transform] : m_boneTransforms) {
            names.push_back(boneName);
        }
        
        return names;
    }

    bool Pose::ValidatePose() const {
        if (auto skeleton = m_skeleton.lock()) {
            // Check that all skeleton bones have transforms
            for (const auto& bone : skeleton->GetAllBones()) {
                if (!HasBoneTransform(bone->GetName())) {
                    LOG_WARNING("Pose missing transform for bone: " + bone->GetName());
                    return false;
                }
            }
        }
        
        return true;
    }

    bool Pose::IsCompatibleWith(const Pose& other) const {
        auto thisSkeleton = m_skeleton.lock();
        auto otherSkeleton = other.m_skeleton.lock();
        
        // Both should have the same skeleton or both should be null
        return thisSkeleton == otherSkeleton;
    }

    void Pose::PrintPoseInfo() const {
        LOG_INFO("Pose Information:");
        LOG_INFO("  Bone Count: " + std::to_string(GetBoneCount()));
        
        if (auto skeleton = m_skeleton.lock()) {
            LOG_INFO("  Skeleton: " + skeleton->GetName());
        } else {
            LOG_INFO("  Skeleton: None");
        }
        
        for (const auto& [boneName, transform] : m_boneTransforms) {
            LOG_INFO("  Bone '" + boneName + "':");
            LOG_INFO("    Position: (" + std::to_string(transform.position.x) + ", " + 
                     std::to_string(transform.position.y) + ", " + std::to_string(transform.position.z) + ")");
        }
    }

    void Pose::UpdateBoneTransformMaps() {
        if (auto skeleton = m_skeleton.lock()) {
            m_boneTransformsById.clear();
            
            for (const auto& [boneName, transform] : m_boneTransforms) {
                if (auto bone = skeleton->GetBone(boneName)) {
                    m_boneTransformsById[bone->GetId()] = transform;
                }
            }
        }
    }

    BoneTransform Pose::GetBindPoseTransform(const std::string& boneName) const {
        if (auto skeleton = m_skeleton.lock()) {
            if (auto bone = skeleton->GetBone(boneName)) {
                BoneTransform bindTransform;
                
                Math::Vec3 position, scale;
                Math::Quat rotation;
                Bone::DecomposeTransform(bone->GetBindPose(), position, rotation, scale);
                
                bindTransform.position = position;
                bindTransform.rotation = rotation;
                bindTransform.scale = scale;
                
                return bindTransform;
            }
        }
        
        return BoneTransform(); // Default identity transform
    }

    // PoseEvaluator implementation
    Pose PoseEvaluator::EvaluateAnimation(const Animation& animation, float time) {
        return EvaluateAnimation(animation, time, nullptr);
    }

    Pose PoseEvaluator::EvaluateAnimation(const Animation& animation, float time, std::shared_ptr<Skeleton> skeleton) {
        Pose pose(skeleton);
        
        // Sample all animated bones
        auto animatedBones = animation.GetAnimatedBoneNames();
        for (const auto& boneName : animatedBones) {
            BoneTransform transform = EvaluateBoneAnimation(animation, boneName, time);
            pose.SetBoneTransform(boneName, transform);
        }
        
        return pose;
    }

    Pose PoseEvaluator::EvaluateAnimationLayers(const std::vector<AnimationLayer>& layers, std::shared_ptr<Skeleton> skeleton) {
        if (layers.empty()) {
            return Pose(skeleton);
        }
        
        // Start with the first layer
        Pose result = EvaluateAnimation(*layers[0].animation, layers[0].time, skeleton);
        if (layers[0].weight < 1.0f) {
            Pose bindPose(skeleton);
            bindPose.ResetToBindPose();
            result = Pose::Blend(bindPose, result, layers[0].weight);
        }
        
        // Blend additional layers
        for (size_t i = 1; i < layers.size(); ++i) {
            const auto& layer = layers[i];
            Pose layerPose = EvaluateAnimation(*layer.animation, layer.time, skeleton);
            
            if (layer.additive) {
                result.BlendAdditiveWith(layerPose, layer.weight);
            } else {
                result.BlendWith(layerPose, layer.weight);
            }
        }
        
        return result;
    }

    void PoseEvaluator::ApplyPoseToSkeleton(const Pose& pose, std::shared_ptr<Skeleton> skeleton) {
        if (!skeleton) return;
        
        for (const auto& bone : skeleton->GetAllBones()) {
            BoneTransform transform = pose.GetBoneTransform(bone->GetName());
            bone->SetLocalTransform(transform.ToMatrix());
        }
        
        skeleton->UpdateBoneTransforms();
    }

    Pose PoseEvaluator::ExtractPoseFromSkeleton(std::shared_ptr<Skeleton> skeleton) {
        Pose pose(skeleton);
        pose.ExtractFromSkeleton();
        return pose;
    }

    void PoseEvaluator::ConvertLocalToWorld(Pose& pose) {
        pose.EvaluateLocalToWorld();
    }

    void PoseEvaluator::ConvertWorldToLocal(Pose& pose) {
        pose.EvaluateWorldToLocal();
    }

    BoneTransform PoseEvaluator::EvaluateBoneAnimation(const Animation& animation, const std::string& boneName, float time) {
        BoneTransform transform;
        
        auto bonePose = animation.SampleBone(boneName, time);
        
        if (bonePose.hasPosition) {
            transform.position = bonePose.position;
        }
        if (bonePose.hasRotation) {
            transform.rotation = bonePose.rotation;
        }
        if (bonePose.hasScale) {
            transform.scale = bonePose.scale;
        }
        
        return transform;
    }

} // namespace Animation
} // namespace GameEngine