#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace GameEngine {

// Forward declarations
namespace Graphics {
    class RenderSkeleton;
}

namespace Graphics {

/**
 * BoneMatrixManager - Efficiently manages bone transformation matrices and GPU uploads
 * 
 * This class handles the calculation and management of bone matrices for skeletal animation,
 * providing efficient GPU buffer management and optimization for real-time rendering.
 * 
 * Features:
 * - Bone matrix calculation from RenderSkeleton data
 * - Efficient GPU buffer management using UBOs
 * - Support for up to 128 bones per skeleton
 * - Performance optimization with dirty flagging
 * - Automatic GPU memory management
 */
class BoneMatrixManager {
public:
    // Constructor and destructor
    BoneMatrixManager();
    ~BoneMatrixManager();

    // Initialization and cleanup
    bool Initialize();
    void Shutdown();
    void CleanupUBO();

    // Matrix calculation and management
    void CalculateBoneMatrices(const Graphics::RenderSkeleton& skeleton,
                              std::vector<glm::mat4>& outMatrices);

    void UpdateBoneMatricesUBO(const std::vector<glm::mat4>& matrices);

    // Performance optimization
    void SetMaxBones(uint32_t maxBones);
    uint32_t GetMaxBones() const { return m_maxBones; }
    
    // Dirty flagging for optimization
    void MarkDirty() { m_isDirty = true; }
    void ClearDirty() { m_isDirty = false; }
    bool IsDirty() const { return m_isDirty; }
    
    // Batching support
    void BeginBatch();
    void EndBatch();
    bool IsBatching() const { return m_isBatching; }

    // GPU resource management
    bool InitializeUBO();
    GLuint GetBoneMatrixUBO() const { return m_boneMatrixUBO; }

    // Performance tracking
    uint32_t GetMatrixUpdates() const;
    uint32_t GetUBOUpdates() const;
    void ResetPerformanceCounters();

    // Validation
    bool IsInitialized() const { return m_initialized; }

private:
    // Constants
    static constexpr uint32_t DEFAULT_MAX_BONES = 128;
    static constexpr uint32_t MAX_SUPPORTED_BONES = 256;
    static constexpr uint32_t UBO_BINDING_POINT = 0;

    // Core data
    uint32_t m_maxBones;
    GLuint m_boneMatrixUBO;
    bool m_initialized;
    
    // Performance optimization
    bool m_isDirty;
    bool m_isBatching;
    std::vector<glm::mat4> m_cachedMatrices;

    // Performance tracking
    uint32_t m_matrixUpdates;
    uint32_t m_uboUpdates;

    // Internal methods
    void ValidateBoneCount(uint32_t boneCount);
    bool CreateUBO();
    void LogPerformanceStats() const;

    // Non-copyable
    BoneMatrixManager(const BoneMatrixManager&) = delete;
    BoneMatrixManager& operator=(const BoneMatrixManager&) = delete;
};

} // namespace Graphics
} // namespace GameEngine