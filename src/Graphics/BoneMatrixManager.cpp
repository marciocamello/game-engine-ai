#include "Graphics/BoneMatrixManager.h"
#include "Graphics/RenderSkeleton.h"
#include "Core/Logger.h"
#include "Core/Math.h"
#include <algorithm>
#include <stdexcept>

namespace GameEngine {
namespace Graphics {

BoneMatrixManager::BoneMatrixManager()
    : m_maxBones(DEFAULT_MAX_BONES)
    , m_boneMatrixUBO(0)
    , m_initialized(false)
    , m_isDirty(true)
    , m_isBatching(false)
    , m_matrixUpdates(0)
    , m_uboUpdates(0)
{
    m_cachedMatrices.resize(DEFAULT_MAX_BONES, glm::mat4(1.0f));
}

BoneMatrixManager::~BoneMatrixManager() {
    Shutdown();
}

bool BoneMatrixManager::Initialize() {
    if (m_initialized) {
        LOG_WARNING("BoneMatrixManager already initialized");
        return true;
    }

    if (!CreateUBO()) {
        LOG_ERROR("Failed to create bone matrix UBO");
        return false;
    }

    m_initialized = true;
    LOG_INFO("BoneMatrixManager initialized successfully");
    return true;
}

void BoneMatrixManager::Shutdown() {
    if (!m_initialized) {
        return;
    }

    CleanupUBO();
    
    LogPerformanceStats();
    
    m_initialized = false;
    LOG_INFO("BoneMatrixManager shutdown complete");
}

void BoneMatrixManager::CleanupUBO() {
    if (m_boneMatrixUBO != 0) {
        glDeleteBuffers(1, &m_boneMatrixUBO);
        m_boneMatrixUBO = 0;
    }
}

void BoneMatrixManager::CalculateBoneMatrices(const Graphics::RenderSkeleton& skeleton,
                                            std::vector<glm::mat4>& outMatrices) {
    if (!m_initialized) {
        throw std::runtime_error("BoneMatrixManager not initialized");
    }

    const auto& bones = skeleton.GetBones();
    uint32_t boneCount = static_cast<uint32_t>(bones.size());
    
    ValidateBoneCount(boneCount);
    
    // Resize output matrix array
    outMatrices.resize(m_maxBones, glm::mat4(1.0f));
    
    // Calculate final bone matrices using RenderSkeleton's method
    auto boneMatrices = skeleton.GetBoneMatrices();
    
    // Convert from Math::Mat4 to glm::mat4 and copy to output
    for (uint32_t i = 0; i < boneCount && i < static_cast<uint32_t>(boneMatrices.size()); ++i) {
        // Convert Math::Mat4 to glm::mat4
        const Math::Mat4& mathMatrix = boneMatrices[i];
        outMatrices[i] = glm::mat4(
            mathMatrix[0][0], mathMatrix[0][1], mathMatrix[0][2], mathMatrix[0][3],
            mathMatrix[1][0], mathMatrix[1][1], mathMatrix[1][2], mathMatrix[1][3],
            mathMatrix[2][0], mathMatrix[2][1], mathMatrix[2][2], mathMatrix[2][3],
            mathMatrix[3][0], mathMatrix[3][1], mathMatrix[3][2], mathMatrix[3][3]
        );
    }
    
    // Fill remaining slots with identity matrices
    for (uint32_t i = boneCount; i < m_maxBones; ++i) {
        outMatrices[i] = glm::mat4(1.0f);
    }
    
    m_matrixUpdates++;
    m_isDirty = true; // Mark as dirty for next GPU update
}

void BoneMatrixManager::UpdateBoneMatricesUBO(const std::vector<glm::mat4>& matrices) {
    if (!m_initialized) {
        throw std::runtime_error("BoneMatrixManager not initialized");
    }

    if (m_boneMatrixUBO == 0) {
        LOG_ERROR("Bone matrix UBO not created");
        return;
    }

    if (matrices.size() != m_maxBones) {
        LOG_ERROR("Matrix array size doesn't match max bones");
        return;
    }

    // Performance optimization: only update if dirty or not batching
    if (!m_isDirty && !m_isBatching) {
        return;
    }
    
    // Cache matrices for batching
    if (m_isBatching) {
        m_cachedMatrices = matrices;
        return;
    }

    // Bind UBO and upload data
    glBindBuffer(GL_UNIFORM_BUFFER, m_boneMatrixUBO);
    
    // Upload matrix data to GPU
    size_t dataSize = matrices.size() * sizeof(glm::mat4);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, dataSize, matrices.data());
    
    // Bind to uniform buffer binding point
    glBindBufferBase(GL_UNIFORM_BUFFER, UBO_BINDING_POINT, m_boneMatrixUBO);
    
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    m_uboUpdates++;
    m_isDirty = false;
    
    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LOG_ERROR("OpenGL error during UBO update");
    }
}

void BoneMatrixManager::SetMaxBones(uint32_t maxBones) {
    if (maxBones == 0 || maxBones > MAX_SUPPORTED_BONES) {
        LOG_ERROR("Invalid max bones count");
        return;
    }

    if (m_initialized && maxBones != m_maxBones) {
        LOG_WARNING("Changing max bones requires reinitialization");
        
        // Cleanup and reinitialize with new size
        CleanupUBO();
        m_maxBones = maxBones;
        m_cachedMatrices.resize(maxBones, glm::mat4(1.0f));
        CreateUBO();
    } else {
        m_maxBones = maxBones;
        m_cachedMatrices.resize(maxBones, glm::mat4(1.0f));
    }
    
    LOG_INFO("Max bones set successfully");
}

bool BoneMatrixManager::InitializeUBO() {
    return CreateUBO();
}

uint32_t BoneMatrixManager::GetMatrixUpdates() const {
    return m_matrixUpdates;
}

uint32_t BoneMatrixManager::GetUBOUpdates() const {
    return m_uboUpdates;
}

void BoneMatrixManager::ResetPerformanceCounters() {
    m_matrixUpdates = 0;
    m_uboUpdates = 0;
    LOG_INFO("Performance counters reset");
}

void BoneMatrixManager::BeginBatch() {
    m_isBatching = true;
    LOG_INFO("Bone matrix batching started");
}

void BoneMatrixManager::EndBatch() {
    if (!m_isBatching) {
        return;
    }
    
    m_isBatching = false;
    
    // Upload cached matrices if we have any
    if (m_isDirty && !m_cachedMatrices.empty()) {
        // Perform the actual GPU upload
        if (m_boneMatrixUBO != 0) {
            glBindBuffer(GL_UNIFORM_BUFFER, m_boneMatrixUBO);
            
            size_t dataSize = m_cachedMatrices.size() * sizeof(glm::mat4);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, dataSize, m_cachedMatrices.data());
            
            glBindBufferBase(GL_UNIFORM_BUFFER, UBO_BINDING_POINT, m_boneMatrixUBO);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            
            m_uboUpdates++;
            m_isDirty = false;
            
            // Check for OpenGL errors
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                LOG_ERROR("OpenGL error during batch UBO update");
            }
        }
    }
    
    LOG_INFO("Bone matrix batching ended");
}

void BoneMatrixManager::ValidateBoneCount(uint32_t boneCount) {
    if (boneCount > m_maxBones) {
        LOG_WARNING("Skeleton has more bones than supported - extra bones will be ignored");
    }
}

bool BoneMatrixManager::CreateUBO() {
    // Generate UBO
    glGenBuffers(1, &m_boneMatrixUBO);
    if (m_boneMatrixUBO == 0) {
        LOG_ERROR("Failed to generate bone matrix UBO");
        return false;
    }

    // Bind and allocate storage
    glBindBuffer(GL_UNIFORM_BUFFER, m_boneMatrixUBO);
    
    size_t bufferSize = m_maxBones * sizeof(glm::mat4);
    glBufferData(GL_UNIFORM_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
    
    // Initialize with identity matrices
    std::vector<glm::mat4> identityMatrices(m_maxBones, glm::mat4(1.0f));
    glBufferSubData(GL_UNIFORM_BUFFER, 0, bufferSize, identityMatrices.data());
    
    // Bind to uniform buffer binding point
    glBindBufferBase(GL_UNIFORM_BUFFER, UBO_BINDING_POINT, m_boneMatrixUBO);
    
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LOG_ERROR("OpenGL error during UBO creation");
        CleanupUBO();
        return false;
    }
    
    LOG_INFO("Bone matrix UBO created successfully");
    return true;
}

void BoneMatrixManager::LogPerformanceStats() const {
    if (m_matrixUpdates > 0 || m_uboUpdates > 0) {
        LOG_INFO("BoneMatrixManager Performance Stats logged");
    }
}

} // namespace Graphics
} // namespace GameEngine