#include "Animation/AnimationSkeleton.h"
#include "Core/Logger.h"
#include <algorithm>
#include <unordered_set>

namespace GameEngine {
namespace Animation {

    AnimationSkeleton::AnimationSkeleton(const std::string& name)
        : m_name(name), m_nextBoneId(0) {
    }

    std::shared_ptr<Bone> AnimationSkeleton::CreateBone(const std::string& name, const Math::Mat4& bindPose) {
        // Check if bone with this name already exists
        if (GetBone(name)) {
            LOG_WARNING("Bone with name '" + name + "' already exists in animation skeleton '" + m_name + "'");
            return nullptr;
        }

        // Create new bone
        auto bone = std::make_shared<Bone>(name, m_nextBoneId++);
        bone->SetBindPose(bindPose);
        bone->SetInverseBindPose(glm::inverse(bindPose));

        // Add to skeleton
        m_bones.push_back(bone);
        m_bonesByName[name] = bone;
        m_bonesById[bone->GetId()] = bone;

        // Set as root if this is the first bone
        if (!m_rootBone) {
            m_rootBone = bone;
        }

        LOG_INFO("Created bone '" + name + "' with ID " + std::to_string(bone->GetId()) + " in animation skeleton");
        return bone;
    }

    std::shared_ptr<Bone> AnimationSkeleton::GetBone(const std::string& name) const {
        auto it = m_bonesByName.find(name);
        return (it != m_bonesByName.end()) ? it->second : nullptr;
    }

    std::shared_ptr<Bone> AnimationSkeleton::GetBone(int32_t id) const {
        auto it = m_bonesById.find(id);
        return (it != m_bonesById.end()) ? it->second : nullptr;
    }

    bool AnimationSkeleton::AddBone(std::shared_ptr<Bone> bone, const std::string& parentName) {
        if (!bone) {
            LOG_ERROR("Cannot add null bone to animation skeleton");
            return false;
        }

        // Check if bone already exists
        if (GetBone(bone->GetName())) {
            LOG_WARNING("Bone '" + bone->GetName() + "' already exists in animation skeleton");
            return false;
        }

        // Add to skeleton
        m_bones.push_back(bone);
        m_bonesByName[bone->GetName()] = bone;
        m_bonesById[bone->GetId()] = bone;

        // Set parent if specified
        if (!parentName.empty()) {
            auto parent = GetBone(parentName);
            if (parent) {
                bone->SetParent(parent);
            } else {
                LOG_WARNING("Parent bone '" + parentName + "' not found for bone '" + bone->GetName() + "'");
            }
        }

        // Set as root if this is the first bone or no parent specified
        if (!m_rootBone || parentName.empty()) {
            m_rootBone = bone;
        }

        return true;
    }

    bool AnimationSkeleton::RemoveBone(const std::string& name) {
        auto bone = GetBone(name);
        if (!bone) {
            return false;
        }

        // Remove from parent
        if (auto parent = bone->GetParent()) {
            parent->RemoveChild(bone);
        }

        // Reparent children to this bone's parent
        auto parent = bone->GetParent();
        for (auto& child : bone->GetChildren()) {
            child->SetParent(parent);
        }

        // Remove from collections
        m_bones.erase(std::remove(m_bones.begin(), m_bones.end(), bone), m_bones.end());
        m_bonesByName.erase(name);
        m_bonesById.erase(bone->GetId());

        // Update root bone if necessary
        if (m_rootBone == bone) {
            m_rootBone = m_bones.empty() ? nullptr : m_bones[0];
        }

        return true;
    }

    void AnimationSkeleton::SetBoneParent(const std::string& boneName, const std::string& parentName) {
        auto bone = GetBone(boneName);
        auto parent = GetBone(parentName);

        if (!bone) {
            LOG_ERROR("Bone '" + boneName + "' not found in animation skeleton");
            return;
        }

        if (!parentName.empty() && !parent) {
            LOG_ERROR("Parent bone '" + parentName + "' not found in animation skeleton");
            return;
        }

        bone->SetParent(parent);
    }

    std::vector<std::shared_ptr<Bone>> AnimationSkeleton::GetRootBones() const {
        std::vector<std::shared_ptr<Bone>> rootBones;
        for (const auto& bone : m_bones) {
            if (bone->IsRoot()) {
                rootBones.push_back(bone);
            }
        }
        return rootBones;
    }

    void AnimationSkeleton::UpdateBoneTransforms() {
        if (!m_rootBone) {
            return;
        }

        // Start recursive update from root with identity transform
        UpdateBoneTransformsRecursive(m_rootBone, Math::Mat4(1.0f));
    }

    void AnimationSkeleton::UpdateBoneTransforms(std::shared_ptr<Bone> bone) {
        if (!bone) {
            return;
        }

        // Calculate parent world transform
        Math::Mat4 parentTransform(1.0f);
        if (auto parent = bone->GetParent()) {
            parentTransform = parent->GetWorldTransform();
        }

        // Update this bone and its children
        UpdateBoneTransformsRecursive(bone, parentTransform);
    }

    void AnimationSkeleton::UpdateBoneTransformsRecursive(std::shared_ptr<Bone> bone, const Math::Mat4& parentTransform) {
        if (!bone) {
            return;
        }

        // Calculate world transform: parent * local
        Math::Mat4 worldTransform = parentTransform * bone->GetLocalTransform();
        bone->SetWorldTransform(worldTransform);

        // Update children
        for (auto& child : bone->GetChildren()) {
            UpdateBoneTransformsRecursive(child, worldTransform);
        }
    }

    void AnimationSkeleton::UpdateBoneTransformsOptimized() {
        // Optimized version that updates bones in order without recursion
        for (auto& bone : m_bones) {
            if (bone->IsRoot()) {
                bone->CalculateWorldTransform(Math::Mat4(1.0f));
            } else {
                auto parent = bone->GetParent();
                if (parent) {
                    bone->CalculateWorldTransform(parent->GetWorldTransform());
                }
            }
        }
    }

    std::vector<Math::Mat4> AnimationSkeleton::GetSkinningMatrices() const {
        std::vector<Math::Mat4> matrices;
        matrices.reserve(m_bones.size());

        for (const auto& bone : m_bones) {
            matrices.push_back(bone->GetSkinningMatrix());
        }

        return matrices;
    }

    void AnimationSkeleton::GetSkinningMatrices(std::vector<Math::Mat4>& outMatrices) const {
        outMatrices.clear();
        outMatrices.reserve(m_bones.size());

        for (const auto& bone : m_bones) {
            outMatrices.push_back(bone->GetSkinningMatrix());
        }
    }

    void AnimationSkeleton::SetBoneLocalTransform(int32_t boneId, const Math::Mat4& transform) {
        auto bone = GetBone(boneId);
        if (bone) {
            bone->SetLocalTransform(transform);
        }
    }

    void AnimationSkeleton::SetBoneLocalTransform(const std::string& boneName, const Math::Mat4& transform) {
        auto bone = GetBone(boneName);
        if (bone) {
            bone->SetLocalTransform(transform);
        }
    }

    void AnimationSkeleton::SetBoneLocalTransforms(const std::vector<Math::Mat4>& transforms) {
        size_t count = std::min(transforms.size(), m_bones.size());
        for (size_t i = 0; i < count; ++i) {
            m_bones[i]->SetLocalTransform(transforms[i]);
        }
    }

    void AnimationSkeleton::SetBindPose() {
        for (auto& bone : m_bones) {
            bone->SetBindPose(bone->GetWorldTransform());
            bone->SetInverseBindPose(glm::inverse(bone->GetWorldTransform()));
        }
        m_hasValidBindPose = true;
        LOG_INFO("Set bind pose for animation skeleton '" + m_name + "' with " + std::to_string(m_bones.size()) + " bones");
    }

    void AnimationSkeleton::RestoreBindPose() {
        if (!m_hasValidBindPose) {
            LOG_WARNING("No valid bind pose to restore for animation skeleton '" + m_name + "'");
            return;
        }

        for (auto& bone : m_bones) {
            bone->SetLocalTransform(bone->GetBindPose());
        }
        UpdateBoneTransforms();
    }

    void AnimationSkeleton::RebuildBoneMaps() {
        m_bonesByName.clear();
        m_bonesById.clear();

        for (const auto& bone : m_bones) {
            m_bonesByName[bone->GetName()] = bone;
            m_bonesById[bone->GetId()] = bone;
        }
    }

    std::vector<std::string> AnimationSkeleton::GetBoneNames() const {
        std::vector<std::string> names;
        names.reserve(m_bones.size());

        for (const auto& bone : m_bones) {
            names.push_back(bone->GetName());
        }

        return names;
    }

    bool AnimationSkeleton::ValidateHierarchy() const {
        std::unordered_set<int32_t> visitedIds;
        
        for (const auto& bone : m_bones) {
            if (bone->IsRoot()) {
                if (!ValidateHierarchyRecursive(bone, visitedIds)) {
                    return false;
                }
            }
        }

        // Check that all bones were visited (no orphaned bones)
        return visitedIds.size() == m_bones.size();
    }

    bool AnimationSkeleton::ValidateHierarchyRecursive(std::shared_ptr<Bone> bone, std::unordered_set<int32_t>& visitedIds) const {
        if (!bone) {
            return false;
        }

        // Check for cycles
        if (visitedIds.find(bone->GetId()) != visitedIds.end()) {
            LOG_ERROR("Cycle detected in animation skeleton hierarchy at bone '" + bone->GetName() + "'");
            return false;
        }

        visitedIds.insert(bone->GetId());

        // Validate children
        for (const auto& child : bone->GetChildren()) {
            if (!ValidateHierarchyRecursive(child, visitedIds)) {
                return false;
            }
        }

        return true;
    }

    void AnimationSkeleton::PrintHierarchy() const {
        LOG_INFO("Animation Skeleton '" + m_name + "' hierarchy:");
        if (m_rootBone) {
            PrintHierarchyRecursive(m_rootBone, 0);
        } else {
            LOG_INFO("  (No root bone)");
        }
    }

    void AnimationSkeleton::PrintHierarchyRecursive(std::shared_ptr<Bone> bone, int32_t depth) const {
        if (!bone) {
            return;
        }

        std::string indent(depth * 2, ' ');
        LOG_INFO(indent + "- " + bone->GetName() + " (ID: " + std::to_string(bone->GetId()) + ")");

        for (const auto& child : bone->GetChildren()) {
            PrintHierarchyRecursive(child, depth + 1);
        }
    }

    int32_t AnimationSkeleton::GetMaxDepth() const {
        int32_t maxDepth = 0;
        for (const auto& bone : m_bones) {
            maxDepth = std::max(maxDepth, bone->GetDepth());
        }
        return maxDepth;
    }

    AnimationSkeleton::SkeletonData AnimationSkeleton::Serialize() const {
        SkeletonData data;
        data.name = m_name;
        data.boneNames.reserve(m_bones.size());
        data.boneParents.reserve(m_bones.size());
        data.bindPoses.reserve(m_bones.size());

        for (const auto& bone : m_bones) {
            data.boneNames.push_back(bone->GetName());
            
            // Store parent index (-1 for root bones)
            auto parent = bone->GetParent();
            int32_t parentIndex = -1;
            if (parent) {
                auto it = std::find_if(m_bones.begin(), m_bones.end(),
                    [&parent](const std::shared_ptr<Bone>& b) { return b == parent; });
                if (it != m_bones.end()) {
                    parentIndex = static_cast<int32_t>(std::distance(m_bones.begin(), it));
                }
            }
            data.boneParents.push_back(parentIndex);
            data.bindPoses.push_back(bone->GetBindPose());
        }

        return data;
    }

    bool AnimationSkeleton::Deserialize(const SkeletonData& data) {
        // Clear existing data
        m_bones.clear();
        m_bonesByName.clear();
        m_bonesById.clear();
        m_rootBone = nullptr;
        m_nextBoneId = 0;

        m_name = data.name;

        // Create all bones first
        for (size_t i = 0; i < data.boneNames.size(); ++i) {
            auto bone = std::make_shared<Bone>(data.boneNames[i], static_cast<int32_t>(i));
            bone->SetBindPose(data.bindPoses[i]);
            bone->SetInverseBindPose(glm::inverse(data.bindPoses[i]));
            
            m_bones.push_back(bone);
            m_bonesByName[bone->GetName()] = bone;
            m_bonesById[bone->GetId()] = bone;
        }

        // Set up hierarchy
        for (size_t i = 0; i < data.boneParents.size(); ++i) {
            int32_t parentIndex = data.boneParents[i];
            if (parentIndex >= 0 && parentIndex < static_cast<int32_t>(m_bones.size())) {
                m_bones[i]->SetParent(m_bones[parentIndex]);
            } else {
                // This is a root bone
                if (!m_rootBone) {
                    m_rootBone = m_bones[i];
                }
            }
        }

        m_nextBoneId = static_cast<int32_t>(m_bones.size());
        m_hasValidBindPose = true;

        return ValidateHierarchy();
    }

} // namespace Animation
} // namespace GameEngine