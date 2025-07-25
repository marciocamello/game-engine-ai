#include "Graphics/MorphTarget.h"
#include "Core/Logger.h"
#include <algorithm>

namespace GameEngine {

// MorphTarget implementation
MorphTarget::MorphTarget(const std::string& name) : m_name(name) {
}

bool MorphTarget::IsValid() const {
    if (m_positionDeltas.empty()) {
        return false;
    }
    
    size_t vertexCount = m_positionDeltas.size();
    
    // Check that all attribute arrays have the same size (if they exist)
    if (!m_normalDeltas.empty() && m_normalDeltas.size() != vertexCount) {
        return false;
    }
    
    if (!m_tangentDeltas.empty() && m_tangentDeltas.size() != vertexCount) {
        return false;
    }
    
    // If sparse, check that indices are valid
    if (IsSparse()) {
        for (uint32_t index : m_sparseIndices) {
            if (index >= vertexCount) {
                return false;
            }
        }
    }
    
    return true;
}

size_t MorphTarget::GetVertexCount() const {
    if (IsSparse()) {
        return m_sparseIndices.size();
    }
    return m_positionDeltas.size();
}

// MorphTargetSet implementation
void MorphTargetSet::AddMorphTarget(std::shared_ptr<MorphTarget> target) {
    if (!target) return;
    
    m_morphTargets.push_back(target);
    BuildMorphTargetMap();
}

void MorphTargetSet::SetMorphTargets(const std::vector<std::shared_ptr<MorphTarget>>& targets) {
    m_morphTargets = targets;
    BuildMorphTargetMap();
}

std::shared_ptr<MorphTarget> MorphTargetSet::GetMorphTarget(size_t index) const {
    if (index >= m_morphTargets.size()) {
        return nullptr;
    }
    return m_morphTargets[index];
}

std::shared_ptr<MorphTarget> MorphTargetSet::FindMorphTarget(const std::string& name) const {
    auto it = m_morphTargetMap.find(name);
    if (it != m_morphTargetMap.end()) {
        return it->second;
    }
    return nullptr;
}

void MorphTargetSet::SetWeights(const std::vector<float>& weights) {
    for (size_t i = 0; i < std::min(weights.size(), m_morphTargets.size()); ++i) {
        if (m_morphTargets[i]) {
            m_morphTargets[i]->SetWeight(weights[i]);
        }
    }
}

std::vector<float> MorphTargetSet::GetWeights() const {
    std::vector<float> weights;
    weights.reserve(m_morphTargets.size());
    
    for (const auto& target : m_morphTargets) {
        if (target) {
            weights.push_back(target->GetWeight());
        } else {
            weights.push_back(0.0f);
        }
    }
    
    return weights;
}

void MorphTargetSet::SetWeight(size_t index, float weight) {
    if (index < m_morphTargets.size() && m_morphTargets[index]) {
        m_morphTargets[index]->SetWeight(weight);
    }
}

float MorphTargetSet::GetWeight(size_t index) const {
    if (index < m_morphTargets.size() && m_morphTargets[index]) {
        return m_morphTargets[index]->GetWeight();
    }
    return 0.0f;
}

void MorphTargetSet::ApplyMorphTargets(std::vector<Math::Vec3>& positions,
                                      std::vector<Math::Vec3>& normals,
                                      std::vector<Math::Vec3>& tangents) const {
    for (const auto& target : m_morphTargets) {
        if (!target || target->GetWeight() == 0.0f) continue;
        
        float weight = target->GetWeight();
        const auto& positionDeltas = target->GetPositionDeltas();
        const auto& normalDeltas = target->GetNormalDeltas();
        const auto& tangentDeltas = target->GetTangentDeltas();
        
        if (target->IsSparse()) {
            // Apply sparse morph target
            const auto& indices = target->GetSparseIndices();
            
            for (size_t i = 0; i < indices.size(); ++i) {
                uint32_t vertexIndex = indices[i];
                
                if (vertexIndex < positions.size() && i < positionDeltas.size()) {
                    positions[vertexIndex] += positionDeltas[i] * weight;
                }
                
                if (vertexIndex < normals.size() && i < normalDeltas.size()) {
                    normals[vertexIndex] += normalDeltas[i] * weight;
                }
                
                if (vertexIndex < tangents.size() && i < tangentDeltas.size()) {
                    tangents[vertexIndex] += tangentDeltas[i] * weight;
                }
            }
        } else {
            // Apply dense morph target
            size_t vertexCount = std::min(positions.size(), positionDeltas.size());
            
            for (size_t i = 0; i < vertexCount; ++i) {
                positions[i] += positionDeltas[i] * weight;
            }
            
            if (!normalDeltas.empty()) {
                size_t normalCount = std::min(normals.size(), normalDeltas.size());
                for (size_t i = 0; i < normalCount; ++i) {
                    normals[i] += normalDeltas[i] * weight;
                }
            }
            
            if (!tangentDeltas.empty()) {
                size_t tangentCount = std::min(tangents.size(), tangentDeltas.size());
                for (size_t i = 0; i < tangentCount; ++i) {
                    tangents[i] += tangentDeltas[i] * weight;
                }
            }
        }
    }
}

std::vector<Math::Vec3> MorphTargetSet::GetCombinedPositionDeltas() const {
    if (m_morphTargets.empty()) {
        return {};
    }
    
    // Determine the size needed
    size_t maxVertexCount = 0;
    for (const auto& target : m_morphTargets) {
        if (target && !target->GetPositionDeltas().empty()) {
            if (target->IsSparse()) {
                for (uint32_t index : target->GetSparseIndices()) {
                    maxVertexCount = std::max(maxVertexCount, static_cast<size_t>(index + 1));
                }
            } else {
                maxVertexCount = std::max(maxVertexCount, target->GetPositionDeltas().size());
            }
        }
    }
    
    std::vector<Math::Vec3> combinedDeltas(maxVertexCount, Math::Vec3(0.0f));
    
    for (const auto& target : m_morphTargets) {
        if (!target || target->GetWeight() == 0.0f) continue;
        
        float weight = target->GetWeight();
        const auto& deltas = target->GetPositionDeltas();
        
        if (target->IsSparse()) {
            const auto& indices = target->GetSparseIndices();
            for (size_t i = 0; i < indices.size() && i < deltas.size(); ++i) {
                uint32_t vertexIndex = indices[i];
                if (vertexIndex < combinedDeltas.size()) {
                    combinedDeltas[vertexIndex] += deltas[i] * weight;
                }
            }
        } else {
            for (size_t i = 0; i < deltas.size() && i < combinedDeltas.size(); ++i) {
                combinedDeltas[i] += deltas[i] * weight;
            }
        }
    }
    
    return combinedDeltas;
}

std::vector<Math::Vec3> MorphTargetSet::GetCombinedNormalDeltas() const {
    if (m_morphTargets.empty()) {
        return {};
    }
    
    // Similar implementation to position deltas
    size_t maxVertexCount = 0;
    for (const auto& target : m_morphTargets) {
        if (target && !target->GetNormalDeltas().empty()) {
            if (target->IsSparse()) {
                for (uint32_t index : target->GetSparseIndices()) {
                    maxVertexCount = std::max(maxVertexCount, static_cast<size_t>(index + 1));
                }
            } else {
                maxVertexCount = std::max(maxVertexCount, target->GetNormalDeltas().size());
            }
        }
    }
    
    if (maxVertexCount == 0) {
        return {};
    }
    
    std::vector<Math::Vec3> combinedDeltas(maxVertexCount, Math::Vec3(0.0f));
    
    for (const auto& target : m_morphTargets) {
        if (!target || target->GetWeight() == 0.0f) continue;
        
        float weight = target->GetWeight();
        const auto& deltas = target->GetNormalDeltas();
        
        if (deltas.empty()) continue;
        
        if (target->IsSparse()) {
            const auto& indices = target->GetSparseIndices();
            for (size_t i = 0; i < indices.size() && i < deltas.size(); ++i) {
                uint32_t vertexIndex = indices[i];
                if (vertexIndex < combinedDeltas.size()) {
                    combinedDeltas[vertexIndex] += deltas[i] * weight;
                }
            }
        } else {
            for (size_t i = 0; i < deltas.size() && i < combinedDeltas.size(); ++i) {
                combinedDeltas[i] += deltas[i] * weight;
            }
        }
    }
    
    return combinedDeltas;
}

std::vector<Math::Vec3> MorphTargetSet::GetCombinedTangentDeltas() const {
    // Similar implementation to normal deltas
    if (m_morphTargets.empty()) {
        return {};
    }
    
    size_t maxVertexCount = 0;
    for (const auto& target : m_morphTargets) {
        if (target && !target->GetTangentDeltas().empty()) {
            if (target->IsSparse()) {
                for (uint32_t index : target->GetSparseIndices()) {
                    maxVertexCount = std::max(maxVertexCount, static_cast<size_t>(index + 1));
                }
            } else {
                maxVertexCount = std::max(maxVertexCount, target->GetTangentDeltas().size());
            }
        }
    }
    
    if (maxVertexCount == 0) {
        return {};
    }
    
    std::vector<Math::Vec3> combinedDeltas(maxVertexCount, Math::Vec3(0.0f));
    
    for (const auto& target : m_morphTargets) {
        if (!target || target->GetWeight() == 0.0f) continue;
        
        float weight = target->GetWeight();
        const auto& deltas = target->GetTangentDeltas();
        
        if (deltas.empty()) continue;
        
        if (target->IsSparse()) {
            const auto& indices = target->GetSparseIndices();
            for (size_t i = 0; i < indices.size() && i < deltas.size(); ++i) {
                uint32_t vertexIndex = indices[i];
                if (vertexIndex < combinedDeltas.size()) {
                    combinedDeltas[vertexIndex] += deltas[i] * weight;
                }
            }
        } else {
            for (size_t i = 0; i < deltas.size() && i < combinedDeltas.size(); ++i) {
                combinedDeltas[i] += deltas[i] * weight;
            }
        }
    }
    
    return combinedDeltas;
}

bool MorphTargetSet::IsValid() const {
    for (const auto& target : m_morphTargets) {
        if (target && !target->IsValid()) {
            return false;
        }
    }
    return true;
}

void MorphTargetSet::ValidateConsistency() const {
    if (m_morphTargets.empty()) {
        return;
    }
    
    // Check that all morph targets have consistent vertex counts
    size_t expectedVertexCount = 0;
    bool hasSparse = false;
    
    for (const auto& target : m_morphTargets) {
        if (!target) continue;
        
        if (target->IsSparse()) {
            hasSparse = true;
        } else if (expectedVertexCount == 0) {
            expectedVertexCount = target->GetPositionDeltas().size();
        } else if (target->GetPositionDeltas().size() != expectedVertexCount) {
            LOG_WARNING("Morph target vertex count mismatch: " + target->GetName());
        }
    }
    
    if (hasSparse && expectedVertexCount > 0) {
        LOG_WARNING("Mixing sparse and dense morph targets may cause issues");
    }
}

void MorphTargetSet::BuildMorphTargetMap() {
    m_morphTargetMap.clear();
    for (const auto& target : m_morphTargets) {
        if (target && !target->GetName().empty()) {
            m_morphTargetMap[target->GetName()] = target;
        }
    }
}

// MorphTargetAnimator implementation
void MorphTargetAnimator::SetTargetWeights(const std::vector<float>& weights, float duration) {
    if (!m_morphTargets) return;
    
    InitializeWeights();
    
    m_targetWeights = weights;
    m_targetWeights.resize(m_morphTargets->GetMorphTargetCount(), 0.0f);
    
    if (duration <= 0.0f) {
        SetWeightsImmediate(weights);
        return;
    }
    
    m_startWeights = m_currentWeights;
    m_animationDuration = duration;
    m_animationTime = 0.0f;
    m_animating = true;
}

void MorphTargetAnimator::SetTargetWeight(size_t index, float weight, float duration) {
    if (!m_morphTargets || index >= m_morphTargets->GetMorphTargetCount()) return;
    
    InitializeWeights();
    
    if (index >= m_targetWeights.size()) {
        m_targetWeights.resize(m_morphTargets->GetMorphTargetCount(), 0.0f);
    }
    
    m_targetWeights[index] = weight;
    
    if (duration <= 0.0f) {
        SetWeightImmediate(index, weight);
        return;
    }
    
    m_startWeights = m_currentWeights;
    m_animationDuration = duration;
    m_animationTime = 0.0f;
    m_animating = true;
}

void MorphTargetAnimator::SetTargetWeight(const std::string& name, float weight, float duration) {
    if (!m_morphTargets) return;
    
    auto target = m_morphTargets->FindMorphTarget(name);
    if (!target) return;
    
    // Find index of the target
    for (size_t i = 0; i < m_morphTargets->GetMorphTargetCount(); ++i) {
        if (m_morphTargets->GetMorphTarget(i) == target) {
            SetTargetWeight(i, weight, duration);
            break;
        }
    }
}

void MorphTargetAnimator::Update(float deltaTime) {
    if (!m_animating || !m_morphTargets) return;
    
    m_animationTime += deltaTime;
    
    if (m_animationTime >= m_animationDuration) {
        // Animation complete
        m_currentWeights = m_targetWeights;
        m_animating = false;
    } else {
        // Interpolate weights
        float t = m_animationTime / m_animationDuration;
        t = EaseInOut(t); // Apply easing curve
        
        for (size_t i = 0; i < m_currentWeights.size(); ++i) {
            if (i < m_startWeights.size() && i < m_targetWeights.size()) {
                m_currentWeights[i] = glm::mix(m_startWeights[i], m_targetWeights[i], t);
            }
        }
    }
    
    // Apply weights to morph targets
    m_morphTargets->SetWeights(m_currentWeights);
}

void MorphTargetAnimator::SetWeightsImmediate(const std::vector<float>& weights) {
    if (!m_morphTargets) return;
    
    m_currentWeights = weights;
    m_currentWeights.resize(m_morphTargets->GetMorphTargetCount(), 0.0f);
    m_targetWeights = m_currentWeights;
    m_animating = false;
    
    m_morphTargets->SetWeights(m_currentWeights);
}

void MorphTargetAnimator::SetWeightImmediate(size_t index, float weight) {
    if (!m_morphTargets || index >= m_morphTargets->GetMorphTargetCount()) return;
    
    InitializeWeights();
    
    if (index < m_currentWeights.size()) {
        m_currentWeights[index] = weight;
        m_targetWeights[index] = weight;
        m_morphTargets->SetWeight(index, weight);
    }
}

void MorphTargetAnimator::SetWeightImmediate(const std::string& name, float weight) {
    if (!m_morphTargets) return;
    
    auto target = m_morphTargets->FindMorphTarget(name);
    if (!target) return;
    
    // Find index and set weight
    for (size_t i = 0; i < m_morphTargets->GetMorphTargetCount(); ++i) {
        if (m_morphTargets->GetMorphTarget(i) == target) {
            SetWeightImmediate(i, weight);
            break;
        }
    }
}

float MorphTargetAnimator::GetAnimationProgress() const {
    if (!m_animating || m_animationDuration <= 0.0f) {
        return 1.0f;
    }
    return std::min(m_animationTime / m_animationDuration, 1.0f);
}

void MorphTargetAnimator::InitializeWeights() {
    if (!m_morphTargets) return;
    
    size_t targetCount = m_morphTargets->GetMorphTargetCount();
    
    if (m_currentWeights.size() != targetCount) {
        m_currentWeights.resize(targetCount, 0.0f);
    }
    
    if (m_targetWeights.size() != targetCount) {
        m_targetWeights.resize(targetCount, 0.0f);
    }
}

float MorphTargetAnimator::EaseInOut(float t) const {
    // Smooth cubic easing curve
    return t * t * (3.0f - 2.0f * t);
}

} // namespace GameEngine