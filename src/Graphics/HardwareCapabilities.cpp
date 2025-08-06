#include "Graphics/HardwareCapabilities.h"
#include "Graphics/OpenGLContext.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <sstream>
#include <algorithm>
#include <tuple>

namespace GameEngine {
    
    // Static member definitions
    bool HardwareCapabilities::s_initialized = false;
    HardwareCapabilities HardwareCapabilities::s_instance;
    
    bool HardwareCapabilities::Initialize() {
        if (s_initialized) {
            return true;
        }
        
        // Check if OpenGL context is available
        if (!OpenGLContext::HasActiveContext()) {
            LOG_ERROR("Cannot initialize hardware capabilities: No active OpenGL context");
            return false;
        }
        
        if (!OpenGLContext::IsReady()) {
            LOG_ERROR("Cannot initialize hardware capabilities: OpenGL not ready");
            return false;
        }
        
        LOG_INFO("Initializing hardware capability detection...");
        
        // Detect all hardware capabilities
        s_instance.DetectOpenGLVersion();
        s_instance.DetectExtensions();
        s_instance.DetectComputeShaderLimits();
        s_instance.DetectShaderLimits();
        s_instance.DetectTextureLimits();
        s_instance.DetectGeneralLimits();
        s_instance.AnalyzePerformanceCharacteristics();
        
        s_initialized = true;
        
        // Log capability information
        s_instance.LogCapabilityInfo();
        
        LOG_INFO("Hardware capability detection completed");
        return true;
    }
    
    bool HardwareCapabilities::IsInitialized() {
        return s_initialized;
    }
    
    HardwareCapabilities& HardwareCapabilities::GetInstance() {
        return s_instance;
    }
    
    void HardwareCapabilities::DetectOpenGLVersion() {
        // Get version string
        const GLubyte* versionStr = glGetString(GL_VERSION);
        if (versionStr) {
            m_versionString = reinterpret_cast<const char*>(versionStr);
            
            // Parse major.minor version
            int major = 0, minor = 0;
            if (sscanf_s(m_versionString.c_str(), "%d.%d", &major, &minor) == 2) {
                m_openGLVersion = static_cast<float>(major) + static_cast<float>(minor) / 10.0f;
            }
        }
        
        // Get vendor
        const GLubyte* vendorStr = glGetString(GL_VENDOR);
        if (vendorStr) {
            m_vendor = reinterpret_cast<const char*>(vendorStr);
        }
        
        // Get renderer
        const GLubyte* rendererStr = glGetString(GL_RENDERER);
        if (rendererStr) {
            m_renderer = reinterpret_cast<const char*>(rendererStr);
        }
    }
    
    void HardwareCapabilities::DetectExtensions() {
        m_extensions.clear();
        m_extensionSet.clear();
        
        // Get number of extensions
        GLint numExtensions = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
        
        // Get each extension
        for (GLint i = 0; i < numExtensions; ++i) {
            const GLubyte* extensionStr = glGetStringi(GL_EXTENSIONS, i);
            if (extensionStr) {
                std::string extension = reinterpret_cast<const char*>(extensionStr);
                m_extensions.push_back(extension);
                m_extensionSet.insert(extension);
            }
        }
        
        LOG_INFO("Detected " + std::to_string(numExtensions) + " OpenGL extensions");
    }
    
    void HardwareCapabilities::DetectComputeShaderLimits() {
        // Check if compute shaders are supported
        if (!SupportsComputeShaders()) {
            LOG_WARNING("Compute shaders not supported on this hardware");
            return;
        }
        
        // Get compute shader limits
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &m_computeLimits.maxWorkGroupSizeX);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &m_computeLimits.maxWorkGroupSizeY);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &m_computeLimits.maxWorkGroupSizeZ);
        
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &m_computeLimits.maxWorkGroupInvocations);
        
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &m_computeLimits.maxWorkGroupCount[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &m_computeLimits.maxWorkGroupCount[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &m_computeLimits.maxWorkGroupCount[2]);
        
        glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &m_computeLimits.maxSharedMemorySize);
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &m_computeLimits.maxStorageBufferBindings);
        glGetIntegerv(GL_MAX_COMPUTE_IMAGE_UNIFORMS, &m_computeLimits.maxImageUnits);
    }
    
    void HardwareCapabilities::DetectShaderLimits() {
        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &m_shaderLimits.maxVertexUniforms);
        glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &m_shaderLimits.maxFragmentUniforms);
        
        if (SupportsGeometryShaders()) {
            glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, &m_shaderLimits.maxGeometryUniforms);
        }
        
        if (SupportsTessellation()) {
            glGetIntegerv(GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS, &m_shaderLimits.maxTessControlUniforms);
            glGetIntegerv(GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS, &m_shaderLimits.maxTessEvaluationUniforms);
        }
        
        glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &m_shaderLimits.maxVertexTextureUnits);
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_shaderLimits.maxFragmentTextureUnits);
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_shaderLimits.maxCombinedTextureUnits);
        
        if (SupportsUniformBuffers()) {
            glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &m_shaderLimits.maxUniformBufferBindings);
            glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &m_shaderLimits.maxUniformBlockSize);
        }
    }
    
    void HardwareCapabilities::DetectTextureLimits() {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_textureLimits.maxTextureSize);
        glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &m_textureLimits.max3DTextureSize);
        glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &m_textureLimits.maxCubeMapSize);
        glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &m_textureLimits.maxArrayTextureLayers);
        glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &m_textureLimits.maxTextureBufferSize);
        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &m_textureLimits.maxRenderbufferSize);
    }
    
    void HardwareCapabilities::DetectGeneralLimits() {
        GLint viewport[2];
        glGetIntegerv(GL_MAX_VIEWPORT_DIMS, viewport);
        m_generalLimits.maxViewportWidth = viewport[0];
        m_generalLimits.maxViewportHeight = viewport[1];
        
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &m_generalLimits.maxDrawBuffers);
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &m_generalLimits.maxColorAttachments);
        glGetIntegerv(GL_MAX_SAMPLES, &m_generalLimits.maxSamples);
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_generalLimits.maxVertexAttributes);
    }
    
    void HardwareCapabilities::AnalyzePerformanceCharacteristics() {
        // Determine performance tier based on various factors
        int tier = 0;
        
        // OpenGL version contributes to tier
        if (m_openGLVersion >= 4.6f) tier += 3;
        else if (m_openGLVersion >= 4.3f) tier += 2;
        else if (m_openGLVersion >= 4.0f) tier += 1;
        
        // Compute shader support
        if (SupportsComputeShaders()) {
            tier += 2;
            
            // Advanced compute capabilities
            if (m_computeLimits.maxWorkGroupInvocations >= 1024) tier += 1;
            if (m_computeLimits.maxSharedMemorySize >= 32768) tier += 1;
        }
        
        // Texture capabilities
        if (m_textureLimits.maxTextureSize >= 16384) tier += 1;
        if (m_textureLimits.maxTextureSize >= 32768) tier += 1;
        
        // Shader capabilities
        if (m_shaderLimits.maxCombinedTextureUnits >= 80) tier += 1;
        if (SupportsStorageBuffers()) tier += 1;
        
        // Clamp to valid range
        m_performanceTier = std::min(3, std::max(0, tier / 3));
        
        // Check minimum requirements
        m_meetsMinimumRequirements = CheckMinimumOpenGLVersion(3.3f) &&
                                   m_textureLimits.maxTextureSize >= 1024 &&
                                   m_shaderLimits.maxCombinedTextureUnits >= 16;
    }
    
    bool HardwareCapabilities::HasExtension(const std::string& extension) const {
        return m_extensionSet.find(extension) != m_extensionSet.end();
    }
    
    bool HardwareCapabilities::SupportsComputeShaders() const {
        return m_openGLVersion >= 4.3f || HasExtension("GL_ARB_compute_shader");
    }
    
    bool HardwareCapabilities::SupportsTessellation() const {
        return m_openGLVersion >= 4.0f || HasExtension("GL_ARB_tessellation_shader");
    }
    
    bool HardwareCapabilities::SupportsGeometryShaders() const {
        return m_openGLVersion >= 3.2f || HasExtension("GL_ARB_geometry_shader4");
    }
    
    bool HardwareCapabilities::SupportsStorageBuffers() const {
        return m_openGLVersion >= 4.3f || HasExtension("GL_ARB_shader_storage_buffer_object");
    }
    
    bool HardwareCapabilities::SupportsImageLoadStore() const {
        return m_openGLVersion >= 4.2f || HasExtension("GL_ARB_shader_image_load_store");
    }
    
    bool HardwareCapabilities::SupportsAtomicOperations() const {
        return m_openGLVersion >= 4.2f || HasExtension("GL_ARB_shader_atomic_counters");
    }
    
    bool HardwareCapabilities::SupportsUniformBuffers() const {
        return m_openGLVersion >= 3.1f || HasExtension("GL_ARB_uniform_buffer_object");
    }
    
    bool HardwareCapabilities::SupportsTextureArrays() const {
        return m_openGLVersion >= 3.0f || HasExtension("GL_EXT_texture_array");
    }
    
    bool HardwareCapabilities::SupportsMultisampling() const {
        return m_openGLVersion >= 3.0f || HasExtension("GL_ARB_multisample");
    }
    
    std::tuple<int, int, int> HardwareCapabilities::GetRecommendedWorkGroupSize() const {
        if (!SupportsComputeShaders()) {
            return std::make_tuple(1, 1, 1);
        }
        
        // Conservative recommendations based on common hardware
        int x = std::min(64, m_computeLimits.maxWorkGroupSizeX);
        int y = std::min(1, m_computeLimits.maxWorkGroupSizeY);
        int z = std::min(1, m_computeLimits.maxWorkGroupSizeZ);
        
        // Ensure we don't exceed total invocation limit
        int total = x * y * z;
        if (total > m_computeLimits.maxWorkGroupInvocations) {
            x = std::min(x, m_computeLimits.maxWorkGroupInvocations);
            y = 1;
            z = 1;
        }
        
        return std::make_tuple(x, y, z);
    }
    
    bool HardwareCapabilities::CanHandleComputeWorkload(int workGroupSizeX, int workGroupSizeY, int workGroupSizeZ, int numGroups) const {
        if (!SupportsComputeShaders()) {
            return false;
        }
        
        // Check work group size limits
        if (workGroupSizeX > m_computeLimits.maxWorkGroupSizeX ||
            workGroupSizeY > m_computeLimits.maxWorkGroupSizeY ||
            workGroupSizeZ > m_computeLimits.maxWorkGroupSizeZ) {
            return false;
        }
        
        // Check total invocations per work group
        int totalInvocations = workGroupSizeX * workGroupSizeY * workGroupSizeZ;
        if (totalInvocations > m_computeLimits.maxWorkGroupInvocations) {
            return false;
        }
        
        // Check work group count limits (simplified check)
        if (numGroups > m_computeLimits.maxWorkGroupCount[0]) {
            return false;
        }
        
        return true;
    }
    
    int HardwareCapabilities::GetSafeMaxTextureSize() const {
        // Return 75% of maximum to account for driver/hardware variations
        return static_cast<int>(m_textureLimits.maxTextureSize * 0.75f);
    }
    
    bool HardwareCapabilities::CanHandleShaderComplexity(int uniformCount, int textureCount) const {
        // Conservative estimates based on typical uniform sizes
        int estimatedUniformComponents = uniformCount * 4; // Assume Vec4 uniforms on average
        
        bool vertexOk = estimatedUniformComponents <= m_shaderLimits.maxVertexUniforms / 2;
        bool fragmentOk = estimatedUniformComponents <= m_shaderLimits.maxFragmentUniforms / 2;
        bool textureOk = textureCount <= m_shaderLimits.maxCombinedTextureUnits / 2;
        
        return vertexOk && fragmentOk && textureOk;
    }
    
    std::string HardwareCapabilities::GenerateCapabilityReport() const {
        std::stringstream report;
        
        report << "=== Hardware Capability Report ===\n\n";
        
        // Basic information
        report << "OpenGL Version: " << m_versionString << " (" << m_openGLVersion << ")\n";
        report << "Vendor: " << m_vendor << "\n";
        report << "Renderer: " << m_renderer << "\n";
        report << "Performance Tier: " << m_performanceTier << "/3\n";
        report << "Meets Minimum Requirements: " << (m_meetsMinimumRequirements ? "Yes" : "No") << "\n\n";
        
        // Feature support
        report << "=== Feature Support ===\n";
        report << "Compute Shaders: " << (SupportsComputeShaders() ? "Yes" : "No") << "\n";
        report << "Tessellation: " << (SupportsTessellation() ? "Yes" : "No") << "\n";
        report << "Geometry Shaders: " << (SupportsGeometryShaders() ? "Yes" : "No") << "\n";
        report << "Storage Buffers: " << (SupportsStorageBuffers() ? "Yes" : "No") << "\n";
        report << "Image Load/Store: " << (SupportsImageLoadStore() ? "Yes" : "No") << "\n";
        report << "Atomic Operations: " << (SupportsAtomicOperations() ? "Yes" : "No") << "\n";
        report << "Uniform Buffers: " << (SupportsUniformBuffers() ? "Yes" : "No") << "\n";
        report << "Texture Arrays: " << (SupportsTextureArrays() ? "Yes" : "No") << "\n";
        report << "Multisampling: " << (SupportsMultisampling() ? "Yes" : "No") << "\n\n";
        
        // Compute shader limits
        if (SupportsComputeShaders()) {
            report << "=== Compute Shader Limits ===\n";
            report << "Max Work Group Size: " << m_computeLimits.maxWorkGroupSizeX 
                   << " x " << m_computeLimits.maxWorkGroupSizeY 
                   << " x " << m_computeLimits.maxWorkGroupSizeZ << "\n";
            report << "Max Work Group Invocations: " << m_computeLimits.maxWorkGroupInvocations << "\n";
            report << "Max Shared Memory: " << m_computeLimits.maxSharedMemorySize << " bytes\n";
            report << "Max Storage Buffer Bindings: " << m_computeLimits.maxStorageBufferBindings << "\n";
            report << "Max Image Units: " << m_computeLimits.maxImageUnits << "\n\n";
        }
        
        // Shader limits
        report << "=== Shader Limits ===\n";
        report << "Max Vertex Uniforms: " << m_shaderLimits.maxVertexUniforms << "\n";
        report << "Max Fragment Uniforms: " << m_shaderLimits.maxFragmentUniforms << "\n";
        report << "Max Combined Texture Units: " << m_shaderLimits.maxCombinedTextureUnits << "\n";
        if (SupportsUniformBuffers()) {
            report << "Max Uniform Buffer Bindings: " << m_shaderLimits.maxUniformBufferBindings << "\n";
            report << "Max Uniform Block Size: " << m_shaderLimits.maxUniformBlockSize << " bytes\n";
        }
        report << "\n";
        
        // Texture limits
        report << "=== Texture Limits ===\n";
        report << "Max Texture Size: " << m_textureLimits.maxTextureSize << "\n";
        report << "Max 3D Texture Size: " << m_textureLimits.max3DTextureSize << "\n";
        report << "Max Cube Map Size: " << m_textureLimits.maxCubeMapSize << "\n";
        report << "Max Array Texture Layers: " << m_textureLimits.maxArrayTextureLayers << "\n";
        report << "Safe Max Texture Size: " << GetSafeMaxTextureSize() << "\n\n";
        
        // General limits
        report << "=== General Limits ===\n";
        report << "Max Viewport: " << m_generalLimits.maxViewportWidth 
               << " x " << m_generalLimits.maxViewportHeight << "\n";
        report << "Max Draw Buffers: " << m_generalLimits.maxDrawBuffers << "\n";
        report << "Max Color Attachments: " << m_generalLimits.maxColorAttachments << "\n";
        report << "Max Samples: " << m_generalLimits.maxSamples << "\n";
        report << "Max Vertex Attributes: " << m_generalLimits.maxVertexAttributes << "\n\n";
        
        // Recommendations
        if (SupportsComputeShaders()) {
            auto [x, y, z] = GetRecommendedWorkGroupSize();
            report << "=== Recommendations ===\n";
            report << "Recommended Work Group Size: " << x << " x " << y << " x " << z << "\n\n";
        }
        
        return report.str();
    }
    
    std::vector<std::string> HardwareCapabilities::GetHardwareLimitations() const {
        std::vector<std::string> limitations;
        
        if (!SupportsComputeShaders()) {
            limitations.push_back("Compute shaders not supported - advanced GPU computing features unavailable");
        }
        
        if (!SupportsTessellation()) {
            limitations.push_back("Tessellation shaders not supported - advanced geometry processing limited");
        }
        
        if (!SupportsStorageBuffers()) {
            limitations.push_back("Shader storage buffers not supported - large data processing in shaders limited");
        }
        
        if (m_textureLimits.maxTextureSize < 4096) {
            limitations.push_back("Low maximum texture size - high-resolution textures may not be supported");
        }
        
        if (m_shaderLimits.maxCombinedTextureUnits < 32) {
            limitations.push_back("Limited texture units - complex materials may be restricted");
        }
        
        if (m_performanceTier < 2) {
            limitations.push_back("Lower performance tier hardware - advanced effects may impact performance");
        }
        
        if (SupportsComputeShaders() && m_computeLimits.maxWorkGroupInvocations < 512) {
            limitations.push_back("Limited compute work group size - parallel processing efficiency reduced");
        }
        
        return limitations;
    }
    
    std::vector<std::string> HardwareCapabilities::GetMissingFeatures() const {
        std::vector<std::string> missing;
        
        if (!SupportsComputeShaders()) {
            missing.push_back("Compute Shaders (OpenGL 4.3+ or GL_ARB_compute_shader)");
        }
        
        if (!SupportsTessellation()) {
            missing.push_back("Tessellation Shaders (OpenGL 4.0+ or GL_ARB_tessellation_shader)");
        }
        
        if (!SupportsGeometryShaders()) {
            missing.push_back("Geometry Shaders (OpenGL 3.2+ or GL_ARB_geometry_shader4)");
        }
        
        if (!SupportsStorageBuffers()) {
            missing.push_back("Shader Storage Buffers (OpenGL 4.3+ or GL_ARB_shader_storage_buffer_object)");
        }
        
        if (!SupportsImageLoadStore()) {
            missing.push_back("Image Load/Store (OpenGL 4.2+ or GL_ARB_shader_image_load_store)");
        }
        
        if (!SupportsAtomicOperations()) {
            missing.push_back("Atomic Operations (OpenGL 4.2+ or GL_ARB_shader_atomic_counters)");
        }
        
        return missing;
    }
    
    bool HardwareCapabilities::MeetsMinimumRequirements() const {
        return m_meetsMinimumRequirements;
    }
    
    int HardwareCapabilities::GetPerformanceTier() const {
        return m_performanceTier;
    }
    
    bool HardwareCapabilities::CheckMinimumOpenGLVersion(float minVersion) const {
        return m_openGLVersion >= minVersion;
    }
    
    void HardwareCapabilities::LogCapabilityInfo() const {
        LOG_INFO("Hardware Capabilities Detected:");
        LOG_INFO("  OpenGL: " + m_versionString + " (" + std::to_string(m_openGLVersion) + ")");
        LOG_INFO("  Vendor: " + m_vendor);
        LOG_INFO("  Renderer: " + m_renderer);
        LOG_INFO("  Performance Tier: " + std::to_string(m_performanceTier) + "/3");
        LOG_INFO("  Compute Shaders: " + std::string(SupportsComputeShaders() ? "Supported" : "Not Supported"));
        LOG_INFO("  Max Texture Size: " + std::to_string(m_textureLimits.maxTextureSize));
        LOG_INFO("  Combined Texture Units: " + std::to_string(m_shaderLimits.maxCombinedTextureUnits));
        
        // Log any significant limitations
        auto limitations = GetHardwareLimitations();
        if (!limitations.empty()) {
            LOG_WARNING("Hardware Limitations Detected:");
            for (const auto& limitation : limitations) {
                LOG_WARNING("  - " + limitation);
            }
        }
        
        if (!m_meetsMinimumRequirements) {
            LOG_ERROR("Hardware does not meet minimum requirements!");
        }
    }
}