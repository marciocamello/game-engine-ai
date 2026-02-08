#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace GameEngine {

// Forward declarations
class Material;

namespace Graphics {

/**
 * SkinningShaderManager - Manages specialized shaders for vertex skinning operations
 * 
 * This class handles the loading, compilation, and management of GLSL shaders specifically
 * designed for skeletal animation rendering. It provides efficient shader resource management
 * with caching and hot-reloading support for development workflows.
 * 
 * Features:
 * - Automatic shader loading from assets/shaders directory
 * - Shader compilation with detailed error reporting
 * - Uniform location caching for performance
 * - Compiled shader program caching
 * - Hot-reloading support for development
 * - Proper GPU resource cleanup
 */
class SkinningShaderManager {
public:
    // Constructor and destructor
    SkinningShaderManager();
    ~SkinningShaderManager();

    // Initialization and cleanup
    bool Initialize();
    void Shutdown();

    // Shader management
    bool LoadSkinningShaders();
    bool ReloadShaders(); // Hot-reloading support
    bool ValidateShaderProgram();

    // Shader binding and uniform management
    void BindSkinningShader();
    void UnbindShader();
    
    void SetBoneMatrices(const std::vector<glm::mat4>& matrices);
    void SetMaterialUniforms(const Material& material);
    void SetTransformUniforms(const glm::mat4& model,
                             const glm::mat4& view,
                             const glm::mat4& projection);

    // Validation and debugging
    void LogShaderInfo();
    bool IsInitialized() const { return m_initialized; }
    GLuint GetShaderProgram() const { return m_skinningProgram; }

    // Performance tracking
    uint32_t GetShaderBinds() const;
    uint32_t GetUniformUpdates() const;
    void ResetPerformanceCounters();

private:
    // Constants
    static constexpr const char* VERTEX_SHADER_PATH = "assets/shaders/skinned.vert";
    static constexpr const char* FRAGMENT_SHADER_PATH = "assets/shaders/skinned.frag";
    static constexpr uint32_t MAX_BONES = 128;
    static constexpr uint32_t BONE_MATRICES_UBO_BINDING = 0;

    // Core data
    GLuint m_skinningProgram;
    bool m_initialized;

    // Uniform locations (cached for performance)
    GLint m_boneMatricesLocation;
    GLint m_modelMatrixLocation;
    GLint m_viewMatrixLocation;
    GLint m_projectionMatrixLocation;
    
    // Material uniform locations
    struct MaterialUniforms {
        GLint diffuseColor;
        GLint specularColor;
        GLint shininess;
        GLint diffuseTexture;
        GLint normalTexture;
        GLint specularTexture;
        GLint emissiveTexture;
        GLint opacity;
    } m_materialUniforms;

    // Performance tracking
    uint32_t m_shaderBinds;
    uint32_t m_uniformUpdates;

    // Shader compilation and linking
    bool CompileShader(const std::string& source, GLenum type, GLuint& shader);
    bool LinkProgram(GLuint vertexShader, GLuint fragmentShader);
    bool LoadShaderFromFile(const std::string& filepath, std::string& outSource);
    
    // Uniform location caching
    void CacheUniformLocations();
    void CacheMaterialUniformLocations();
    
    // Error handling and logging
    void LogShaderCompileError(GLuint shader, const std::string& shaderType);
    void LogProgramLinkError(GLuint program);
    bool CheckShaderCompileStatus(GLuint shader);
    bool CheckProgramLinkStatus(GLuint program);

    // Resource management
    void CleanupShaderProgram();
    void CleanupShader(GLuint& shader);

    // Non-copyable
    SkinningShaderManager(const SkinningShaderManager&) = delete;
    SkinningShaderManager& operator=(const SkinningShaderManager&) = delete;
};

} // namespace Graphics
} // namespace GameEngine