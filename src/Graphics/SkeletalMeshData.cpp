#include "Graphics/SkeletalMeshData.h"
#include "Core/Logger.h"
#include "Core/OpenGLContext.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <glm/glm.hpp>

namespace GameEngine {
namespace Graphics {

    SkeletalMeshData::SkeletalMeshData(size_t vertexCount) {
        Resize(vertexCount);
    }

    SkeletalMeshData::~SkeletalMeshData() {
        CleanupGPUResources();
    }

    SkeletalMeshData::SkeletalMeshData(const SkeletalMeshData& other) {
        CopyFrom(other);
    }

    SkeletalMeshData& SkeletalMeshData::operator=(const SkeletalMeshData& other) {
        if (this != &other) {
            CleanupGPUResources();
            CopyFrom(other);
        }
        return *this;
    }

    SkeletalMeshData::SkeletalMeshData(SkeletalMeshData&& other) noexcept {
        MoveFrom(std::move(other));
    }

    SkeletalMeshData& SkeletalMeshData::operator=(SkeletalMeshData&& other) noexcept {
        if (this != &other) {
            CleanupGPUResources();
            MoveFrom(std::move(other));
        }
        return *this;
    }

    bool SkeletalMeshData::IsValid() const {
        // Check if arrays have same size
        if (boneIndices.size() != boneWeights.size()) {
            return false;
        }

        // Check if we have any data
        if (boneIndices.empty()) {
            return false;
        }

        // Validate bone indices (should be non-negative)
        for (const auto& indices : boneIndices) {
            if (indices.x < 0 || indices.y < 0 || indices.z < 0 || indices.w < 0) {
                return false;
            }
        }

        // Validate weight normalization
        return ValidateWeightNormalization();
    }

    void SkeletalMeshData::NormalizeWeights() {
        for (auto& weights : boneWeights) {
            float sum = weights.x + weights.y + weights.z + weights.w;
            
            if (sum > 0.001f) {
                // Normalize weights to sum to 1.0
                weights.x /= sum;
                weights.y /= sum;
                weights.z /= sum;
                weights.w /= sum;
            } else {
                // If no weights, set first weight to 1.0 (bind to first bone)
                weights = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
            }
        }
    }

    uint32_t SkeletalMeshData::GetMaxBoneIndex() const {
        if (boneIndices.empty()) {
            return 0;
        }

        uint32_t maxIndex = 0;
        for (const auto& indices : boneIndices) {
            maxIndex = std::max(maxIndex, static_cast<uint32_t>(indices.x));
            maxIndex = std::max(maxIndex, static_cast<uint32_t>(indices.y));
            maxIndex = std::max(maxIndex, static_cast<uint32_t>(indices.z));
            maxIndex = std::max(maxIndex, static_cast<uint32_t>(indices.w));
        }

        return maxIndex;
    }

    void SkeletalMeshData::Resize(size_t vertexCount) {
        boneIndices.resize(vertexCount, glm::ivec4(0));
        boneWeights.resize(vertexCount, glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
    }

    void SkeletalMeshData::Clear() {
        boneIndices.clear();
        boneWeights.clear();
        CleanupGPUResources();
    }

    void SkeletalMeshData::SetVertexBoneData(size_t vertexIndex, 
                                            const std::vector<uint32_t>& indices,
                                            const std::vector<float>& weights) {
        if (vertexIndex >= boneIndices.size()) {
            LOG_ERROR("Vertex index out of bounds: " + std::to_string(vertexIndex));
            return;
        }

        if (indices.size() != weights.size()) {
            LOG_ERROR("Bone indices and weights arrays must have same size");
            return;
        }

        // Initialize with zeros
        glm::ivec4 boneIdx(0);
        glm::vec4 boneWeight(0.0f);

        // Copy up to 4 influences
        size_t count = std::min(indices.size(), size_t(4));
        for (size_t i = 0; i < count; ++i) {
            switch (i) {
                case 0: 
                    boneIdx.x = static_cast<int32_t>(indices[i]);
                    boneWeight.x = weights[i];
                    break;
                case 1:
                    boneIdx.y = static_cast<int32_t>(indices[i]);
                    boneWeight.y = weights[i];
                    break;
                case 2:
                    boneIdx.z = static_cast<int32_t>(indices[i]);
                    boneWeight.z = weights[i];
                    break;
                case 3:
                    boneIdx.w = static_cast<int32_t>(indices[i]);
                    boneWeight.w = weights[i];
                    break;
            }
        }

        // Normalize weights
        float sum = boneWeight.x + boneWeight.y + boneWeight.z + boneWeight.w;
        if (sum > 0.001f) {
            boneWeight.x /= sum;
            boneWeight.y /= sum;
            boneWeight.z /= sum;
            boneWeight.w /= sum;
        } else {
            // Default to first bone with full weight
            boneWeight = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
        }

        boneIndices[vertexIndex] = boneIdx;
        boneWeights[vertexIndex] = boneWeight;
    }

    void SkeletalMeshData::GetVertexBoneData(size_t vertexIndex,
                                            std::vector<uint32_t>& outIndices,
                                            std::vector<float>& outWeights) const {
        outIndices.clear();
        outWeights.clear();

        if (vertexIndex >= boneIndices.size()) {
            LOG_ERROR("Vertex index out of bounds: " + std::to_string(vertexIndex));
            return;
        }

        const glm::ivec4& indices = boneIndices[vertexIndex];
        const glm::vec4& weights = boneWeights[vertexIndex];

        // Add non-zero influences
        if (weights.x > 0.001f) {
            outIndices.push_back(static_cast<uint32_t>(indices.x));
            outWeights.push_back(weights.x);
        }
        if (weights.y > 0.001f) {
            outIndices.push_back(static_cast<uint32_t>(indices.y));
            outWeights.push_back(weights.y);
        }
        if (weights.z > 0.001f) {
            outIndices.push_back(static_cast<uint32_t>(indices.z));
            outWeights.push_back(weights.z);
        }
        if (weights.w > 0.001f) {
            outIndices.push_back(static_cast<uint32_t>(indices.w));
            outWeights.push_back(weights.w);
        }
    }

    bool SkeletalMeshData::UploadToGPU() {
        if (!OpenGLContext::HasActiveContext()) {
            LOG_ERROR("Cannot upload skeletal data to GPU: No OpenGL context available");
            return false;
        }

        if (boneIndices.empty() || boneWeights.empty()) {
            LOG_WARNING("Cannot upload empty skeletal data to GPU");
            return false;
        }

        EnsureGPUBuffersCreated();

        // Upload bone indices
        glBindBuffer(GL_ARRAY_BUFFER, boneIndicesVBO);
        glBufferData(GL_ARRAY_BUFFER, 
                    boneIndices.size() * sizeof(glm::ivec4), 
                    boneIndices.data(), 
                    GL_STATIC_DRAW);

        // Upload bone weights
        glBindBuffer(GL_ARRAY_BUFFER, boneWeightsVBO);
        glBufferData(GL_ARRAY_BUFFER, 
                    boneWeights.size() * sizeof(glm::vec4), 
                    boneWeights.data(), 
                    GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            LOG_ERROR("OpenGL error uploading skeletal data: " + std::to_string(error));
            return false;
        }

        LOG_DEBUG("Successfully uploaded skeletal data to GPU (" + 
                 std::to_string(boneIndices.size()) + " vertices)");
        return true;
    }

    void SkeletalMeshData::BindForRendering(GLuint boneIndicesLocation, GLuint boneWeightsLocation) const {
        if (boneIndicesVBO == 0 || boneWeightsVBO == 0) {
            LOG_WARNING("Attempting to bind skeletal data with invalid VBOs");
            return;
        }

        // Bind bone indices
        glBindBuffer(GL_ARRAY_BUFFER, boneIndicesVBO);
        glEnableVertexAttribArray(boneIndicesLocation);
        glVertexAttribIPointer(boneIndicesLocation, 4, GL_INT, 0, nullptr);

        // Bind bone weights
        glBindBuffer(GL_ARRAY_BUFFER, boneWeightsVBO);
        glEnableVertexAttribArray(boneWeightsLocation);
        glVertexAttribPointer(boneWeightsLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void SkeletalMeshData::UnbindFromRendering() const {
        // Note: Vertex attribute arrays are disabled when VAO is unbound
        // This method is provided for completeness but typically not needed
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void SkeletalMeshData::CleanupGPUResources() {
        if (!OpenGLContext::HasActiveContext()) {
            return;
        }

        if (boneIndicesVBO != 0) {
            glDeleteBuffers(1, &boneIndicesVBO);
            boneIndicesVBO = 0;
        }

        if (boneWeightsVBO != 0) {
            glDeleteBuffers(1, &boneWeightsVBO);
            boneWeightsVBO = 0;
        }
    }

    size_t SkeletalMeshData::GetMemoryUsage() const {
        size_t cpuMemory = boneIndices.size() * sizeof(glm::ivec4) + 
                          boneWeights.size() * sizeof(glm::vec4);
        
        // Estimate GPU memory usage (same as CPU data)
        size_t gpuMemory = (boneIndicesVBO != 0 || boneWeightsVBO != 0) ? cpuMemory : 0;
        
        return cpuMemory + gpuMemory;
    }

    bool SkeletalMeshData::ValidateWeightNormalization(float epsilon) const {
        for (const auto& weights : boneWeights) {
            float sum = weights.x + weights.y + weights.z + weights.w;
            if (std::abs(sum - 1.0f) > epsilon) {
                return false;
            }
        }
        return true;
    }

    void SkeletalMeshData::GetInfluenceStatistics(uint32_t& outMinInfluences, 
                                                 uint32_t& outMaxInfluences, 
                                                 float& outAvgInfluences) const {
        if (boneWeights.empty()) {
            outMinInfluences = outMaxInfluences = 0;
            outAvgInfluences = 0.0f;
            return;
        }

        outMinInfluences = 4;
        outMaxInfluences = 0;
        uint32_t totalInfluences = 0;

        for (const auto& weights : boneWeights) {
            uint32_t influences = 0;
            if (weights.x > 0.001f) influences++;
            if (weights.y > 0.001f) influences++;
            if (weights.z > 0.001f) influences++;
            if (weights.w > 0.001f) influences++;

            outMinInfluences = std::min(outMinInfluences, influences);
            outMaxInfluences = std::max(outMaxInfluences, influences);
            totalInfluences += influences;
        }

        outAvgInfluences = static_cast<float>(totalInfluences) / static_cast<float>(boneWeights.size());
    }

    void SkeletalMeshData::EnsureGPUBuffersCreated() {
        if (!OpenGLContext::HasActiveContext()) {
            return;
        }

        if (boneIndicesVBO == 0) {
            glGenBuffers(1, &boneIndicesVBO);
        }

        if (boneWeightsVBO == 0) {
            glGenBuffers(1, &boneWeightsVBO);
        }
    }

    void SkeletalMeshData::CopyFrom(const SkeletalMeshData& other) {
        boneIndices = other.boneIndices;
        boneWeights = other.boneWeights;
        
        // Don't copy GPU resources - they will be created when needed
        boneIndicesVBO = 0;
        boneWeightsVBO = 0;
    }

    void SkeletalMeshData::MoveFrom(SkeletalMeshData&& other) noexcept {
        boneIndices = std::move(other.boneIndices);
        boneWeights = std::move(other.boneWeights);
        
        // Move GPU resources
        boneIndicesVBO = other.boneIndicesVBO;
        boneWeightsVBO = other.boneWeightsVBO;
        
        // Clear other's resources
        other.boneIndicesVBO = 0;
        other.boneWeightsVBO = 0;
    }

} // namespace Graphics
} // namespace GameEngine