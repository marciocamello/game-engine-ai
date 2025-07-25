#pragma once

#include "Core/Math.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace GameEngine {

    /**
     * @brief Morph target (blend shape) for facial animation and mesh deformation
     */
    class MorphTarget {
    public:
        MorphTarget(const std::string& name = "");
        ~MorphTarget() = default;

        // Basic properties
        void SetName(const std::string& name) { m_name = name; }
        const std::string& GetName() const { return m_name; }

        void SetWeight(float weight) { m_weight = glm::clamp(weight, 0.0f, 1.0f); }
        float GetWeight() const { return m_weight; }

        // Vertex displacement data
        void SetPositionDeltas(const std::vector<Math::Vec3>& deltas) { m_positionDeltas = deltas; }
        const std::vector<Math::Vec3>& GetPositionDeltas() const { return m_positionDeltas; }

        void SetNormalDeltas(const std::vector<Math::Vec3>& deltas) { m_normalDeltas = deltas; }
        const std::vector<Math::Vec3>& GetNormalDeltas() const { return m_normalDeltas; }

        void SetTangentDeltas(const std::vector<Math::Vec3>& deltas) { m_tangentDeltas = deltas; }
        const std::vector<Math::Vec3>& GetTangentDeltas() const { return m_tangentDeltas; }

        // Sparse data support (only store non-zero deltas)
        void SetSparseIndices(const std::vector<uint32_t>& indices) { m_sparseIndices = indices; }
        const std::vector<uint32_t>& GetSparseIndices() const { return m_sparseIndices; }

        bool IsSparse() const { return !m_sparseIndices.empty(); }

        // Validation
        bool IsValid() const;
        size_t GetVertexCount() const;

    private:
        std::string m_name;
        float m_weight = 0.0f;

        // Vertex attribute deltas
        std::vector<Math::Vec3> m_positionDeltas;
        std::vector<Math::Vec3> m_normalDeltas;
        std::vector<Math::Vec3> m_tangentDeltas;

        // Sparse data support
        std::vector<uint32_t> m_sparseIndices;
    };

    /**
     * @brief Collection of morph targets for a mesh
     */
    class MorphTargetSet {
    public:
        MorphTargetSet() = default;
        ~MorphTargetSet() = default;

        // Morph target management
        void AddMorphTarget(std::shared_ptr<MorphTarget> target);
        void SetMorphTargets(const std::vector<std::shared_ptr<MorphTarget>>& targets);
        const std::vector<std::shared_ptr<MorphTarget>>& GetMorphTargets() const { return m_morphTargets; }

        std::shared_ptr<MorphTarget> GetMorphTarget(size_t index) const;
        std::shared_ptr<MorphTarget> FindMorphTarget(const std::string& name) const;
        size_t GetMorphTargetCount() const { return m_morphTargets.size(); }

        // Weight management
        void SetWeights(const std::vector<float>& weights);
        std::vector<float> GetWeights() const;
        void SetWeight(size_t index, float weight);
        float GetWeight(size_t index) const;

        // Apply morph targets to base mesh
        void ApplyMorphTargets(std::vector<Math::Vec3>& positions,
                              std::vector<Math::Vec3>& normals,
                              std::vector<Math::Vec3>& tangents) const;

        // GPU data preparation
        std::vector<Math::Vec3> GetCombinedPositionDeltas() const;
        std::vector<Math::Vec3> GetCombinedNormalDeltas() const;
        std::vector<Math::Vec3> GetCombinedTangentDeltas() const;

        // Validation
        bool IsValid() const;
        void ValidateConsistency() const;

    private:
        std::vector<std::shared_ptr<MorphTarget>> m_morphTargets;
        std::unordered_map<std::string, std::shared_ptr<MorphTarget>> m_morphTargetMap;

        void BuildMorphTargetMap();
    };

    /**
     * @brief Morph target animation controller
     */
    class MorphTargetAnimator {
    public:
        MorphTargetAnimator() = default;
        ~MorphTargetAnimator() = default;

        void SetMorphTargetSet(std::shared_ptr<MorphTargetSet> morphTargets) { m_morphTargets = morphTargets; }
        std::shared_ptr<MorphTargetSet> GetMorphTargetSet() const { return m_morphTargets; }

        // Animation control
        void SetTargetWeights(const std::vector<float>& weights, float duration = 0.0f);
        void SetTargetWeight(size_t index, float weight, float duration = 0.0f);
        void SetTargetWeight(const std::string& name, float weight, float duration = 0.0f);

        // Update animation
        void Update(float deltaTime);

        // Immediate weight setting (no animation)
        void SetWeightsImmediate(const std::vector<float>& weights);
        void SetWeightImmediate(size_t index, float weight);
        void SetWeightImmediate(const std::string& name, float weight);

        // Animation state
        bool IsAnimating() const { return m_animating; }
        float GetAnimationProgress() const;

    private:
        std::shared_ptr<MorphTargetSet> m_morphTargets;

        // Animation state
        std::vector<float> m_currentWeights;
        std::vector<float> m_targetWeights;
        std::vector<float> m_startWeights;
        float m_animationTime = 0.0f;
        float m_animationDuration = 0.0f;
        bool m_animating = false;

        void InitializeWeights();
        float EaseInOut(float t) const; // Smooth animation curve
    };

} // namespace GameEngine