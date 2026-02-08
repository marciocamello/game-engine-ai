#pragma once

#include "Core/Math.h"
#include <vector>
#include <memory>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace GameEngine {
namespace Graphics {

    /**
     * @brief Skeletal mesh data structure containing bone indices and weights for vertex skinning
     * 
     * This structure holds per-vertex bone influence data required for skeletal animation.
     * Each vertex can be influenced by up to 4 bones with normalized weights.
     */
    struct SkeletalMeshData {
        // Per-vertex bone data (indices into skeleton bone array)
        std::vector<glm::ivec4> boneIndices;  // Up to 4 bones per vertex
        std::vector<glm::vec4> boneWeights;   // Corresponding weights (sum = 1.0)

        // OpenGL buffer objects for GPU upload
        GLuint boneIndicesVBO = 0;
        GLuint boneWeightsVBO = 0;

        /**
         * @brief Default constructor
         */
        SkeletalMeshData() = default;

        /**
         * @brief Constructor with vertex count
         * @param vertexCount Number of vertices to allocate data for
         */
        explicit SkeletalMeshData(size_t vertexCount);

        /**
         * @brief Destructor - cleans up OpenGL resources
         */
        ~SkeletalMeshData();

        // Copy constructor and assignment operator
        SkeletalMeshData(const SkeletalMeshData& other);
        SkeletalMeshData& operator=(const SkeletalMeshData& other);

        // Move constructor and assignment operator
        SkeletalMeshData(SkeletalMeshData&& other) noexcept;
        SkeletalMeshData& operator=(SkeletalMeshData&& other) noexcept;

        /**
         * @brief Validates skeletal mesh data integrity
         * @return true if data is valid, false otherwise
         */
        bool IsValid() const;

        /**
         * @brief Normalizes bone weights to ensure they sum to 1.0 per vertex
         */
        void NormalizeWeights();

        /**
         * @brief Gets the maximum bone index used in the data
         * @return Maximum bone index, or 0 if no bones
         */
        uint32_t GetMaxBoneIndex() const;

        /**
         * @brief Gets the number of vertices with skeletal data
         * @return Vertex count
         */
        size_t GetVertexCount() const { return boneIndices.size(); }

        /**
         * @brief Resizes the skeletal data arrays
         * @param vertexCount New vertex count
         */
        void Resize(size_t vertexCount);

        /**
         * @brief Clears all skeletal data
         */
        void Clear();

        /**
         * @brief Sets bone influence data for a specific vertex
         * @param vertexIndex Index of the vertex
         * @param indices Bone indices (up to 4)
         * @param weights Bone weights (up to 4, will be normalized)
         */
        void SetVertexBoneData(size_t vertexIndex, 
                              const std::vector<uint32_t>& indices,
                              const std::vector<float>& weights);

        /**
         * @brief Gets bone influence data for a specific vertex
         * @param vertexIndex Index of the vertex
         * @param outIndices Output bone indices
         * @param outWeights Output bone weights
         */
        void GetVertexBoneData(size_t vertexIndex,
                              std::vector<uint32_t>& outIndices,
                              std::vector<float>& outWeights) const;

        /**
         * @brief Uploads skeletal data to GPU buffers
         * @return true if upload successful, false otherwise
         */
        bool UploadToGPU();

        /**
         * @brief Binds skeletal data VBOs for rendering
         * @param boneIndicesLocation Vertex attribute location for bone indices
         * @param boneWeightsLocation Vertex attribute location for bone weights
         */
        void BindForRendering(GLuint boneIndicesLocation, GLuint boneWeightsLocation) const;

        /**
         * @brief Unbinds skeletal data VBOs
         */
        void UnbindFromRendering() const;

        /**
         * @brief Cleans up OpenGL resources
         */
        void CleanupGPUResources();

        /**
         * @brief Gets memory usage in bytes
         * @return Total memory usage including CPU and GPU data
         */
        size_t GetMemoryUsage() const;

        /**
         * @brief Validates bone weight normalization
         * @param epsilon Tolerance for weight sum validation
         * @return true if all weights are properly normalized
         */
        bool ValidateWeightNormalization(float epsilon = 0.001f) const;

        /**
         * @brief Gets statistics about bone influences
         * @param outMinInfluences Minimum number of bone influences per vertex
         * @param outMaxInfluences Maximum number of bone influences per vertex
         * @param outAvgInfluences Average number of bone influences per vertex
         */
        void GetInfluenceStatistics(uint32_t& outMinInfluences, 
                                   uint32_t& outMaxInfluences, 
                                   float& outAvgInfluences) const;

    private:
        /**
         * @brief Creates OpenGL buffer objects if not already created
         */
        void EnsureGPUBuffersCreated();

        /**
         * @brief Copies data from another SkeletalMeshData instance
         * @param other Source instance to copy from
         */
        void CopyFrom(const SkeletalMeshData& other);

        /**
         * @brief Moves data from another SkeletalMeshData instance
         * @param other Source instance to move from
         */
        void MoveFrom(SkeletalMeshData&& other) noexcept;
    };

} // namespace Graphics
} // namespace GameEngine