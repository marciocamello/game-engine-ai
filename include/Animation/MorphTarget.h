#pragma once

#include "../../engine/core/Math.h"
#include "Graphics/Mesh.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace GameEngine {

    /**
     * MorphTarget represents a single morph target with vertex deltas
     * for position, normal, and tangent modifications.
     * Requirements: 5.1, 5.2, 5.3
     */
    class MorphTarget {
    public:
        // Lifecycle
        MorphTarget(const std::string& name);
        ~MorphTarget();

        // Vertex data management
        void SetVertexDeltas(const std::vector<Math::Vec3>& positionDeltas);
        void SetNormalDeltas(const std::vector<Math::Vec3>& normalDeltas);
        void SetTangentDeltas(const std::vector<Math::Vec3>& tangentDeltas);

        const std::vector<Math::Vec3>& GetVertexDeltas() const { return m_positionDeltas; }
        const std::vector<Math::Vec3>& GetNormalDeltas() const { return m_normalDeltas; }
        const std::vector<Math::Vec3>& GetTangentDeltas() const { return m_tangentDeltas; }

        // Properties
        void SetName(const std::string& name) { m_name = name; }
        void SetWeight(float weight);
        float GetWeight() const { return m_weight; }
        const std::string& GetName() const { return m_name; }

        // Application methods
        void ApplyToMesh(Mesh& mesh, float weight) const;
        void ApplyToVertices(std::vector<Vertex>& vertices, float weight) const;

        // Optimization
        void Compress(float tolerance = 0.001f);
        size_t GetMemoryUsage() const;

        // Validation
        bool IsValid() const;
        bool HasPositionDeltas() const { return !m_positionDeltas.empty(); }
        bool HasNormalDeltas() const { return !m_normalDeltas.empty(); }
        bool HasTangentDeltas() const { return !m_tangentDeltas.empty(); }

    private:
        std::string m_name;
        float m_weight = 0.0f;

        // Vertex delta data
        std::vector<Math::Vec3> m_positionDeltas;
        std::vector<Math::Vec3> m_normalDeltas;
        std::vector<Math::Vec3> m_tangentDeltas;

        // Sparse representation for optimization
        std::vector<uint32_t> m_affectedVertices;
        bool m_isCompressed = false;

        // Helper methods
        void UpdateAffectedVertices();
        bool IsVertexAffected(size_t vertexIndex, float tolerance = 0.001f) const;
    };

    /**
     * MorphTargetController manages multiple morph targets and their weights
     * with animation support and blending modes.
     * Requirements: 5.4, 5.5, 5.6
     */
    class MorphTargetController {
    public:
        enum class BlendMode {
            Additive,   // Add all morph target effects
            Override    // Use highest weight morph target
        };

        // Lifecycle
        MorphTargetController();
        ~MorphTargetController();

        // Morph target management
        void AddMorphTarget(std::shared_ptr<MorphTarget> morphTarget);
        void RemoveMorphTarget(const std::string& name);
        std::shared_ptr<MorphTarget> GetMorphTarget(const std::string& name) const;
        std::vector<std::shared_ptr<MorphTarget>> GetAllMorphTargets() const;

        // Weight control
        void SetWeight(const std::string& name, float weight);
        float GetWeight(const std::string& name) const;
        void SetAllWeights(const std::unordered_map<std::string, float>& weights);

        // Animation support
        void AnimateWeight(const std::string& name, float targetWeight, float duration);
        void Update(float deltaTime);

        // Blending mode
        void SetBlendMode(BlendMode mode) { m_blendMode = mode; }
        BlendMode GetBlendMode() const { return m_blendMode; }

        // Application
        void ApplyToMesh(Mesh& mesh) const;
        void ApplyToVertices(std::vector<Vertex>& vertices) const;

        // Statistics
        size_t GetMorphTargetCount() const { return m_morphTargets.size(); }
        size_t GetMemoryUsage() const;

        // Validation
        bool IsValid() const;
        std::vector<std::string> GetValidationErrors() const;

    private:
        struct WeightAnimation {
            float currentWeight = 0.0f;
            float targetWeight = 0.0f;
            float duration = 0.0f;
            float elapsedTime = 0.0f;
            bool isActive = false;
        };

        std::unordered_map<std::string, std::shared_ptr<MorphTarget>> m_morphTargets;
        std::unordered_map<std::string, WeightAnimation> m_weightAnimations;
        BlendMode m_blendMode = BlendMode::Additive;

        // Helper methods
        void UpdateWeightAnimation(const std::string& name, float deltaTime);
        float InterpolateWeight(float current, float target, float t) const;
    };

    /**
     * MorphTargetSet represents a collection of morph targets for a specific mesh
     * This is used by the Mesh class to manage morph targets
     */
    class MorphTargetSet {
    public:
        MorphTargetSet();
        ~MorphTargetSet();

        // Morph target management
        void AddMorphTarget(std::shared_ptr<MorphTarget> morphTarget);
        void RemoveMorphTarget(const std::string& name);
        std::shared_ptr<MorphTarget> GetMorphTarget(const std::string& name) const;
        std::vector<std::shared_ptr<MorphTarget>> GetAllMorphTargets() const;

        // Controller access
        std::shared_ptr<MorphTargetController> GetController() { return m_controller; }
        std::shared_ptr<const MorphTargetController> GetController() const { return m_controller; }

        // Statistics
        size_t GetMorphTargetCount() const;
        size_t GetMemoryUsage() const;

    private:
        std::shared_ptr<MorphTargetController> m_controller;
    };

} // namespace GameEngine