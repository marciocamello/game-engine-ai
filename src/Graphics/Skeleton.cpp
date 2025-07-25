#include "Graphics/Skeleton.h"
#include "Core/Logger.h"
#include <algorithm>
#include <iostream>

namespace GameEngine {

// Bone implementation
Bone::Bone(const std::string& name, int32_t index) 
    : m_name(name), m_index(index) {
}

void Bone::SetParent(std::shared_ptr<Bone> parent) {
    // Remove from old parent
    if (auto oldParent = m_parent.lock()) {
        oldParent->RemoveChild(shared_from_this());
    }
    
    m_parent = parent;
    
    // Add to new parent
    if (parent) {
        parent->AddChild(shared_from_this());
    }
}

void Bone::AddChild(std::shared_ptr<Bone> child) {
    if (!child) return;
    
    // Check if already a child
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it == m_children.end()) {
        m_children.push_back(child);
        child->m_parent = shared_from_this();
    }
}

void Bone::RemoveChild(std::shared_ptr<Bone> child) {
    if (!child) return;
    
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        m_children.erase(it);
        child->m_parent.reset();
    }
}

Math::Mat4 Bone::GetSkinningMatrix() const {
    return m_worldTransform * m_inverseBindMatrix;
}

void Bone::UpdateTransforms(const Math::Mat4& parentTransform) {
    m_worldTransform = parentTransform * m_localTransform;
    
    // Update children recursively
    for (auto& child : m_children) {
        if (child) {
            child->UpdateTransforms(m_worldTransform);
        }
    }
}

size_t Bone::GetDepth() const {
    if (auto parent = m_parent.lock()) {
        return parent->GetDepth() + 1;
    }
    return 0;
}

// Skeleton implementation
void Skeleton::AddBone(std::shared_ptr<Bone> bone) {
    if (!bone) return;
    
    m_bones.push_back(bone);
    m_matricesDirty = true;
    BuildBoneMap();
}

void Skeleton::SetBones(const std::vector<std::shared_ptr<Bone>>& bones) {
    m_bones = bones;
    m_matricesDirty = true;
    BuildBoneMap();
}

std::shared_ptr<Bone> Skeleton::GetBone(size_t index) const {
    if (index >= m_bones.size()) {
        return nullptr;
    }
    return m_bones[index];
}

std::shared_ptr<Bone> Skeleton::FindBone(const std::string& name) const {
    auto it = m_boneMap.find(name);
    if (it != m_boneMap.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<Math::Mat4> Skeleton::GetBoneMatrices() const {
    if (m_matricesDirty) {
        const_cast<Skeleton*>(this)->UpdateBoneMatrices();
    }
    return m_boneMatrices;
}

void Skeleton::UpdateBoneMatrices() {
    m_boneMatrices.clear();
    m_boneMatrices.reserve(m_bones.size());
    
    // Update transforms starting from root
    if (m_rootBone) {
        m_rootBone->UpdateTransforms();
    }
    
    // Collect bone matrices
    for (const auto& bone : m_bones) {
        if (bone) {
            m_boneMatrices.push_back(bone->GetSkinningMatrix());
        } else {
            m_boneMatrices.push_back(Math::Mat4(1.0f));
        }
    }
    
    m_matricesDirty = false;
}

void Skeleton::BuildHierarchy() {
    // Find root bone (bone with no parent or index 0)
    for (const auto& bone : m_bones) {
        if (bone && bone->IsRoot()) {
            m_rootBone = bone;
            break;
        }
    }
    
    if (!m_rootBone && !m_bones.empty()) {
        // If no explicit root found, use first bone
        m_rootBone = m_bones[0];
    }
    
    BuildBoneMap();
}

void Skeleton::ValidateHierarchy() const {
    if (!m_rootBone) {
        LOG_WARNING("Skeleton has no root bone");
        return;
    }
    
    // Check for cycles and orphaned bones
    std::unordered_set<std::shared_ptr<Bone>> visited;
    std::function<void(std::shared_ptr<Bone>)> validateBone = [&](std::shared_ptr<Bone> bone) {
        if (!bone) return;
        
        if (visited.find(bone) != visited.end()) {
            LOG_ERROR("Cycle detected in bone hierarchy at bone: " + bone->GetName());
            return;
        }
        
        visited.insert(bone);
        
        for (const auto& child : bone->GetChildren()) {
            validateBone(child);
        }
    };
    
    validateBone(m_rootBone);
    
    // Check if all bones are reachable from root
    if (visited.size() != m_bones.size()) {
        LOG_WARNING("Some bones are not reachable from root bone");
    }
}

void Skeleton::PrintHierarchy() const {
    if (m_rootBone) {
        std::cout << "Skeleton Hierarchy:" << std::endl;
        PrintBoneHierarchy(m_rootBone, 0);
    } else {
        std::cout << "Skeleton has no root bone" << std::endl;
    }
}

size_t Skeleton::GetMaxDepth() const {
    size_t maxDepth = 0;
    for (const auto& bone : m_bones) {
        if (bone) {
            maxDepth = std::max(maxDepth, bone->GetDepth());
        }
    }
    return maxDepth;
}

void Skeleton::SetBindPose() {
    // Store current transforms as bind pose
    for (auto& bone : m_bones) {
        if (bone) {
            bone->SetInverseBindMatrix(glm::inverse(bone->GetWorldTransform()));
        }
    }
}

void Skeleton::RestoreBindPose() {
    // Reset all bones to identity transform
    for (auto& bone : m_bones) {
        if (bone) {
            bone->SetLocalTransform(Math::Mat4(1.0f));
        }
    }
    
    // Update transforms
    if (m_rootBone) {
        m_rootBone->UpdateTransforms();
    }
    
    m_matricesDirty = true;
}

void Skeleton::BuildBoneMap() {
    m_boneMap.clear();
    for (const auto& bone : m_bones) {
        if (bone && !bone->GetName().empty()) {
            m_boneMap[bone->GetName()] = bone;
        }
    }
}

void Skeleton::PrintBoneHierarchy(std::shared_ptr<Bone> bone, size_t depth) const {
    if (!bone) return;
    
    std::string indent(depth * 2, ' ');
    std::cout << indent << "- " << bone->GetName() 
              << " (index: " << bone->GetIndex() << ")" << std::endl;
    
    for (const auto& child : bone->GetChildren()) {
        PrintBoneHierarchy(child, depth + 1);
    }
}

// Skin implementation
std::vector<Math::Mat4> Skin::GetSkinningMatrices() const {
    std::vector<Math::Mat4> matrices;
    
    if (!m_skeleton) {
        return matrices;
    }
    
    auto boneMatrices = m_skeleton->GetBoneMatrices();
    matrices.reserve(m_joints.size());
    
    for (size_t i = 0; i < m_joints.size(); ++i) {
        uint32_t jointIndex = m_joints[i];
        
        if (jointIndex < boneMatrices.size()) {
            Math::Mat4 skinningMatrix = boneMatrices[jointIndex];
            
            // Apply inverse bind matrix if available
            if (i < m_inverseBindMatrices.size()) {
                skinningMatrix = skinningMatrix * m_inverseBindMatrices[i];
            }
            
            matrices.push_back(skinningMatrix);
        } else {
            matrices.push_back(Math::Mat4(1.0f));
        }
    }
    
    return matrices;
}

bool Skin::IsValid() const {
    if (!m_skeleton) {
        return false;
    }
    
    // Check that all joint indices are valid
    for (uint32_t jointIndex : m_joints) {
        if (jointIndex >= m_skeleton->GetBoneCount()) {
            return false;
        }
    }
    
    // Check that inverse bind matrices count matches joints count (if provided)
    if (!m_inverseBindMatrices.empty() && m_inverseBindMatrices.size() != m_joints.size()) {
        return false;
    }
    
    return true;
}

} // namespace GameEngine