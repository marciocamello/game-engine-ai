#include "Graphics/RenderSkeleton.h"
#include "Core/Logger.h"
#include <algorithm>
#include <iostream>

namespace GameEngine {
namespace Graphics {

// RenderBone implementation
RenderBone::RenderBone(const std::string& name, int32_t index) 
    : m_name(name), m_index(index) {
}

void RenderBone::SetParent(std::shared_ptr<RenderBone> parent) {
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

void RenderBone::AddChild(std::shared_ptr<RenderBone> child) {
    if (!child) return;
    
    // Check if already a child
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it == m_children.end()) {
        m_children.push_back(child);
        child->m_parent = shared_from_this();
    }
}

void RenderBone::RemoveChild(std::shared_ptr<RenderBone> child) {
    if (!child) return;
    
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        m_children.erase(it);
        child->m_parent.reset();
    }
}

Math::Mat4 RenderBone::GetSkinningMatrix() const {
    return m_worldTransform * m_inverseBindMatrix;
}

void RenderBone::UpdateTransforms(const Math::Mat4& parentTransform) {
    m_worldTransform = parentTransform * m_localTransform;
    
    // Update children recursively
    for (auto& child : m_children) {
        if (child) {
            child->UpdateTransforms(m_worldTransform);
        }
    }
}

size_t RenderBone::GetDepth() const {
    if (auto parent = m_parent.lock()) {
        return parent->GetDepth() + 1;
    }
    return 0;
}

// RenderSkeleton implementation
void RenderSkeleton::AddBone(std::shared_ptr<RenderBone> bone) {
    if (!bone) return;
    
    m_bones.push_back(bone);
    m_matricesDirty = true;
    BuildBoneMap();
}

void RenderSkeleton::SetBones(const std::vector<std::shared_ptr<RenderBone>>& bones) {
    m_bones = bones;
    m_matricesDirty = true;
    BuildBoneMap();
}

std::shared_ptr<RenderBone> RenderSkeleton::GetBone(size_t index) const {
    if (index >= m_bones.size()) {
        return nullptr;
    }
    return m_bones[index];
}

std::shared_ptr<RenderBone> RenderSkeleton::FindBone(const std::string& name) const {
    auto it = m_boneMap.find(name);
    if (it != m_boneMap.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<Math::Mat4> RenderSkeleton::GetBoneMatrices() const {
    if (m_matricesDirty) {
        const_cast<RenderSkeleton*>(this)->UpdateBoneMatrices();
    }
    return m_boneMatrices;
}

void RenderSkeleton::UpdateBoneMatrices() {
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

void RenderSkeleton::BuildHierarchy() {
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

void RenderSkeleton::ValidateHierarchy() const {
    if (!m_rootBone) {
        LOG_WARNING("RenderSkeleton has no root bone");
        return;
    }
    
    // Check for cycles and orphaned bones
    std::unordered_set<std::shared_ptr<RenderBone>> visited;
    std::function<void(std::shared_ptr<RenderBone>)> validateBone = [&](std::shared_ptr<RenderBone> bone) {
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

void RenderSkeleton::PrintHierarchy() const {
    if (m_rootBone) {
        std::cout << "RenderSkeleton Hierarchy:" << std::endl;
        PrintBoneHierarchy(m_rootBone, 0);
    } else {
        std::cout << "RenderSkeleton has no root bone" << std::endl;
    }
}

size_t RenderSkeleton::GetMaxDepth() const {
    size_t maxDepth = 0;
    for (const auto& bone : m_bones) {
        if (bone) {
            maxDepth = std::max(maxDepth, bone->GetDepth());
        }
    }
    return maxDepth;
}

void RenderSkeleton::SetBindPose() {
    // Store current transforms as bind pose
    for (auto& bone : m_bones) {
        if (bone) {
            bone->SetInverseBindMatrix(glm::inverse(bone->GetWorldTransform()));
        }
    }
}

void RenderSkeleton::RestoreBindPose() {
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

void RenderSkeleton::BuildBoneMap() {
    m_boneMap.clear();
    for (const auto& bone : m_bones) {
        if (bone && !bone->GetName().empty()) {
            m_boneMap[bone->GetName()] = bone;
        }
    }
}

void RenderSkeleton::PrintBoneHierarchy(std::shared_ptr<RenderBone> bone, size_t depth) const {
    if (!bone) return;
    
    std::string indent(depth * 2, ' ');
    std::cout << indent << "- " << bone->GetName() 
              << " (index: " << bone->GetIndex() << ")" << std::endl;
    
    for (const auto& child : bone->GetChildren()) {
        PrintBoneHierarchy(child, depth + 1);
    }
}

// RenderSkin implementation
std::vector<Math::Mat4> RenderSkin::GetSkinningMatrices() const {
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

bool RenderSkin::IsValid() const {
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

} // namespace Graphics
} // namespace GameEngine