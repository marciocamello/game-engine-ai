#include "Animation/MorphTarget.h"
#include "../../engine/core/Logger.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {

    // ========================================
    // MorphTarget Implementation
    // ========================================

    MorphTarget::MorphTarget(const std::string& name)
        : m_name(name), m_weight(0.0f), m_isCompressed(false) {
    }

    MorphTarget::~MorphTarget() {
    }

    void MorphTarget::SetVertexDeltas(const std::vector<Math::Vec3>& positionDeltas) {
        m_positionDeltas = positionDeltas;
        m_isCompressed = false;
        UpdateAffectedVertices();
    }

    void MorphTarget::SetNormalDeltas(const std::vector<Math::Vec3>& normalDeltas) {
        m_normalDeltas = normalDeltas;
        m_isCompressed = false;
        UpdateAffectedVertices();
    }

    void MorphTarget::SetTangentDeltas(const std::vector<Math::Vec3>& tangentDeltas) {
        m_tangentDeltas = tangentDeltas;
        m_isCompressed = false;
        UpdateAffectedVertices();
    }

    void MorphTarget::SetWeight(float weight) {
        m_weight = Math::Clamp(weight, 0.0f, 1.0f);
    }

    void MorphTarget::ApplyToMesh(Mesh& mesh, float weight) const {
        if (weight <= 0.0f || !IsValid()) {
            return;
        }

        auto vertices = mesh.GetVertices();
        ApplyToVertices(const_cast<std::vector<Vertex>&>(vertices), weight);
        mesh.SetVertices(vertices);
    }

    void MorphTarget::ApplyToVertices(std::vector<Vertex>& vertices, float weight) const {
        if (weight <= 0.0f || !IsValid()) {
            return;
        }

        const float clampedWeight = Math::Clamp(weight, 0.0f, 1.0f);

        // Apply position deltas
        if (HasPositionDeltas()) {
            const size_t maxVertices = std::min(vertices.size(), m_positionDeltas.size());
            for (size_t i = 0; i < maxVertices; ++i) {
                vertices[i].position += m_positionDeltas[i] * clampedWeight;
            }
        }

        // Apply normal deltas
        if (HasNormalDeltas()) {
            const size_t maxVertices = std::min(vertices.size(), m_normalDeltas.size());
            for (size_t i = 0; i < maxVertices; ++i) {
                vertices[i].normal += m_normalDeltas[i] * clampedWeight;
                // Normalize the normal after modification
                vertices[i].normal = glm::normalize(vertices[i].normal);
            }
        }

        // Apply tangent deltas
        if (HasTangentDeltas()) {
            const size_t maxVertices = std::min(vertices.size(), m_tangentDeltas.size());
            for (size_t i = 0; i < maxVertices; ++i) {
                vertices[i].tangent += m_tangentDeltas[i] * clampedWeight;
                // Normalize the tangent after modification
                vertices[i].tangent = glm::normalize(vertices[i].tangent);
            }
        }
    }

    void MorphTarget::Compress(float tolerance) {
        if (m_isCompressed) {
            return;
        }
        
        // Use tolerance parameter in UpdateAffectedVertices
        (void)tolerance; // Suppress warning for now

        UpdateAffectedVertices();

        // Create compressed versions using only affected vertices
        if (!m_affectedVertices.empty()) {
            std::vector<Math::Vec3> compressedPositions;
            std::vector<Math::Vec3> compressedNormals;
            std::vector<Math::Vec3> compressedTangents;

            compressedPositions.reserve(m_affectedVertices.size());
            compressedNormals.reserve(m_affectedVertices.size());
            compressedTangents.reserve(m_affectedVertices.size());

            for (uint32_t vertexIndex : m_affectedVertices) {
                if (HasPositionDeltas() && vertexIndex < m_positionDeltas.size()) {
                    compressedPositions.push_back(m_positionDeltas[vertexIndex]);
                }
                if (HasNormalDeltas() && vertexIndex < m_normalDeltas.size()) {
                    compressedNormals.push_back(m_normalDeltas[vertexIndex]);
                }
                if (HasTangentDeltas() && vertexIndex < m_tangentDeltas.size()) {
                    compressedTangents.push_back(m_tangentDeltas[vertexIndex]);
                }
            }

            // Replace original data with compressed data
            m_positionDeltas = std::move(compressedPositions);
            m_normalDeltas = std::move(compressedNormals);
            m_tangentDeltas = std::move(compressedTangents);
        }

        m_isCompressed = true;
    }

    size_t MorphTarget::GetMemoryUsage() const {
        size_t usage = sizeof(MorphTarget);
        usage += m_name.capacity();
        usage += m_positionDeltas.capacity() * sizeof(Math::Vec3);
        usage += m_normalDeltas.capacity() * sizeof(Math::Vec3);
        usage += m_tangentDeltas.capacity() * sizeof(Math::Vec3);
        usage += m_affectedVertices.capacity() * sizeof(uint32_t);
        return usage;
    }

    bool MorphTarget::IsValid() const {
        return !m_name.empty() && (HasPositionDeltas() || HasNormalDeltas() || HasTangentDeltas());
    }

    void MorphTarget::UpdateAffectedVertices() {
        m_affectedVertices.clear();

        const size_t maxVertices = std::max({
            m_positionDeltas.size(),
            m_normalDeltas.size(),
            m_tangentDeltas.size()
        });

        for (size_t i = 0; i < maxVertices; ++i) {
            if (IsVertexAffected(i)) {
                m_affectedVertices.push_back(static_cast<uint32_t>(i));
            }
        }
    }

    bool MorphTarget::IsVertexAffected(size_t vertexIndex, float tolerance) const {
        // Check if any delta for this vertex is above tolerance
        if (vertexIndex < m_positionDeltas.size()) {
            const Math::Vec3& delta = m_positionDeltas[vertexIndex];
            if (glm::length(delta) > tolerance) {
                return true;
            }
        }

        if (vertexIndex < m_normalDeltas.size()) {
            const Math::Vec3& delta = m_normalDeltas[vertexIndex];
            if (glm::length(delta) > tolerance) {
                return true;
            }
        }

        if (vertexIndex < m_tangentDeltas.size()) {
            const Math::Vec3& delta = m_tangentDeltas[vertexIndex];
            if (glm::length(delta) > tolerance) {
                return true;
            }
        }

        return false;
    }

    // ========================================
    // MorphTargetController Implementation
    // ========================================

    MorphTargetController::MorphTargetController()
        : m_blendMode(BlendMode::Additive) {
    }

    MorphTargetController::~MorphTargetController() {
    }

    void MorphTargetController::AddMorphTarget(std::shared_ptr<MorphTarget> morphTarget) {
        if (!morphTarget || !morphTarget->IsValid()) {
            LOG_ERROR("Cannot add invalid morph target");
            return;
        }

        const std::string& name = morphTarget->GetName();
        m_morphTargets[name] = morphTarget;

        // Initialize weight animation data
        m_weightAnimations[name] = WeightAnimation();
    }

    void MorphTargetController::RemoveMorphTarget(const std::string& name) {
        m_morphTargets.erase(name);
        m_weightAnimations.erase(name);
    }

    std::shared_ptr<MorphTarget> MorphTargetController::GetMorphTarget(const std::string& name) const {
        auto it = m_morphTargets.find(name);
        return (it != m_morphTargets.end()) ? it->second : nullptr;
    }

    std::vector<std::shared_ptr<MorphTarget>> MorphTargetController::GetAllMorphTargets() const {
        std::vector<std::shared_ptr<MorphTarget>> targets;
        targets.reserve(m_morphTargets.size());

        for (const auto& pair : m_morphTargets) {
            targets.push_back(pair.second);
        }

        return targets;
    }

    void MorphTargetController::SetWeight(const std::string& name, float weight) {
        auto morphTarget = GetMorphTarget(name);
        if (!morphTarget) {
            LOG_WARNING("Morph target '" + name + "' not found");
            return;
        }

        const float clampedWeight = Math::Clamp(weight, 0.0f, 1.0f);
        morphTarget->SetWeight(clampedWeight);

        // Update animation data
        auto& animation = m_weightAnimations[name];
        animation.currentWeight = clampedWeight;
        animation.targetWeight = clampedWeight;
        animation.isActive = false;
    }

    float MorphTargetController::GetWeight(const std::string& name) const {
        auto morphTarget = GetMorphTarget(name);
        return morphTarget ? morphTarget->GetWeight() : 0.0f;
    }

    void MorphTargetController::SetAllWeights(const std::unordered_map<std::string, float>& weights) {
        for (const auto& pair : weights) {
            SetWeight(pair.first, pair.second);
        }
    }

    void MorphTargetController::AnimateWeight(const std::string& name, float targetWeight, float duration) {
        auto morphTarget = GetMorphTarget(name);
        if (!morphTarget) {
            LOG_WARNING("Morph target '" + name + "' not found for animation");
            return;
        }

        if (duration <= 0.0f) {
            SetWeight(name, targetWeight);
            return;
        }

        auto& animation = m_weightAnimations[name];
        animation.currentWeight = morphTarget->GetWeight();
        animation.targetWeight = Math::Clamp(targetWeight, 0.0f, 1.0f);
        animation.duration = duration;
        animation.elapsedTime = 0.0f;
        animation.isActive = true;
    }

    void MorphTargetController::Update(float deltaTime) {
        for (auto& pair : m_weightAnimations) {
            const std::string& name = pair.first;
            WeightAnimation& animation = pair.second;

            if (animation.isActive) {
                UpdateWeightAnimation(name, deltaTime);
            }
        }
    }

    void MorphTargetController::ApplyToMesh(Mesh& mesh) const {
        auto vertices = mesh.GetVertices();
        ApplyToVertices(const_cast<std::vector<Vertex>&>(vertices));
        mesh.SetVertices(vertices);
    }

    void MorphTargetController::ApplyToVertices(std::vector<Vertex>& vertices) const {
        if (m_morphTargets.empty()) {
            return;
        }

        switch (m_blendMode) {
            case BlendMode::Additive: {
                // Apply all morph targets additively
                for (const auto& pair : m_morphTargets) {
                    const auto& morphTarget = pair.second;
                    const float weight = morphTarget->GetWeight();
                    if (weight > 0.0f) {
                        morphTarget->ApplyToVertices(vertices, weight);
                    }
                }
                break;
            }

            case BlendMode::Override: {
                // Find the morph target with the highest weight
                std::shared_ptr<MorphTarget> dominantTarget = nullptr;
                float maxWeight = 0.0f;

                for (const auto& pair : m_morphTargets) {
                    const auto& morphTarget = pair.second;
                    const float weight = morphTarget->GetWeight();
                    if (weight > maxWeight) {
                        maxWeight = weight;
                        dominantTarget = morphTarget;
                    }
                }

                // Apply only the dominant morph target
                if (dominantTarget && maxWeight > 0.0f) {
                    dominantTarget->ApplyToVertices(vertices, maxWeight);
                }
                break;
            }
        }
    }

    size_t MorphTargetController::GetMemoryUsage() const {
        size_t usage = sizeof(MorphTargetController);
        
        for (const auto& pair : m_morphTargets) {
            usage += pair.first.capacity();
            if (pair.second) {
                usage += pair.second->GetMemoryUsage();
            }
        }

        usage += m_weightAnimations.size() * sizeof(WeightAnimation);
        
        return usage;
    }

    bool MorphTargetController::IsValid() const {
        for (const auto& pair : m_morphTargets) {
            if (!pair.second || !pair.second->IsValid()) {
                return false;
            }
        }
        return true;
    }

    std::vector<std::string> MorphTargetController::GetValidationErrors() const {
        std::vector<std::string> errors;

        for (const auto& pair : m_morphTargets) {
            if (!pair.second) {
                errors.push_back("Null morph target: " + pair.first);
            } else if (!pair.second->IsValid()) {
                errors.push_back("Invalid morph target: " + pair.first);
            }
        }

        return errors;
    }

    void MorphTargetController::UpdateWeightAnimation(const std::string& name, float deltaTime) {
        auto& animation = m_weightAnimations[name];
        auto morphTarget = GetMorphTarget(name);

        if (!morphTarget || !animation.isActive) {
            return;
        }

        animation.elapsedTime += deltaTime;
        const float t = Math::Clamp(animation.elapsedTime / animation.duration, 0.0f, 1.0f);
        
        const float newWeight = InterpolateWeight(animation.currentWeight, animation.targetWeight, t);
        morphTarget->SetWeight(newWeight);

        // Check if animation is complete
        if (t >= 1.0f) {
            animation.isActive = false;
            animation.elapsedTime = 0.0f;
        }
    }

    float MorphTargetController::InterpolateWeight(float current, float target, float t) const {
        // Use smooth interpolation (ease-in-out)
        const float smoothT = t * t * (3.0f - 2.0f * t);
        return Math::Lerp(current, target, smoothT);
    }

    // ========================================
    // MorphTargetSet Implementation
    // ========================================

    MorphTargetSet::MorphTargetSet()
        : m_controller(std::make_shared<MorphTargetController>()) {
    }

    MorphTargetSet::~MorphTargetSet() {
    }

    void MorphTargetSet::AddMorphTarget(std::shared_ptr<MorphTarget> morphTarget) {
        if (m_controller) {
            m_controller->AddMorphTarget(morphTarget);
        }
    }

    void MorphTargetSet::RemoveMorphTarget(const std::string& name) {
        if (m_controller) {
            m_controller->RemoveMorphTarget(name);
        }
    }

    std::shared_ptr<MorphTarget> MorphTargetSet::GetMorphTarget(const std::string& name) const {
        return m_controller ? m_controller->GetMorphTarget(name) : nullptr;
    }

    std::vector<std::shared_ptr<MorphTarget>> MorphTargetSet::GetAllMorphTargets() const {
        return m_controller ? m_controller->GetAllMorphTargets() : std::vector<std::shared_ptr<MorphTarget>>();
    }

    size_t MorphTargetSet::GetMorphTargetCount() const {
        return m_controller ? m_controller->GetMorphTargetCount() : 0;
    }

    size_t MorphTargetSet::GetMemoryUsage() const {
        size_t usage = sizeof(MorphTargetSet);
        if (m_controller) {
            usage += m_controller->GetMemoryUsage();
        }
        return usage;
    }

} // namespace GameEngine