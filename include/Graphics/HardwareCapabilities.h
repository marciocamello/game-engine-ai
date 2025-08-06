#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace GameEngine {
    
    /**
     * Hardware capability detection and reporting system
     * Detects OpenGL features, extensions, and hardware limitations
     */
    class HardwareCapabilities {
    public:
        struct ComputeShaderLimits {
            int maxWorkGroupSizeX = 0;
            int maxWorkGroupSizeY = 0;
            int maxWorkGroupSizeZ = 0;
            int maxWorkGroupInvocations = 0;
            int maxWorkGroupCount[3] = {0, 0, 0};
            int maxSharedMemorySize = 0;
            int maxStorageBufferBindings = 0;
            int maxImageUnits = 0;
        };
        
        struct ShaderLimits {
            int maxVertexUniforms = 0;
            int maxFragmentUniforms = 0;
            int maxGeometryUniforms = 0;
            int maxTessControlUniforms = 0;
            int maxTessEvaluationUniforms = 0;
            int maxVertexTextureUnits = 0;
            int maxFragmentTextureUnits = 0;
            int maxCombinedTextureUnits = 0;
            int maxUniformBufferBindings = 0;
            int maxUniformBlockSize = 0;
        };
        
        struct TextureLimits {
            int maxTextureSize = 0;
            int max3DTextureSize = 0;
            int maxCubeMapSize = 0;
            int maxArrayTextureLayers = 0;
            int maxTextureBufferSize = 0;
            int maxRenderbufferSize = 0;
        };
        
        struct GeneralLimits {
            int maxViewportWidth = 0;
            int maxViewportHeight = 0;
            int maxDrawBuffers = 0;
            int maxColorAttachments = 0;
            int maxSamples = 0;
            int maxVertexAttributes = 0;
        };
        
        /**
         * Initialize hardware capability detection
         * Must be called with active OpenGL context
         * @return true if initialization successful, false otherwise
         */
        static bool Initialize();
        
        /**
         * Check if hardware capabilities have been detected
         * @return true if initialized, false otherwise
         */
        static bool IsInitialized();
        
        /**
         * Get singleton instance
         * @return reference to singleton instance
         */
        static HardwareCapabilities& GetInstance();
        
        // OpenGL version and extension detection
        
        /**
         * Get OpenGL version as major.minor
         * @return OpenGL version (e.g., 4.6f for OpenGL 4.6)
         */
        float GetOpenGLVersion() const { return m_openGLVersion; }
        
        /**
         * Get OpenGL version string
         * @return OpenGL version string
         */
        const std::string& GetOpenGLVersionString() const { return m_versionString; }
        
        /**
         * Get GPU vendor string
         * @return GPU vendor (e.g., "NVIDIA Corporation")
         */
        const std::string& GetVendor() const { return m_vendor; }
        
        /**
         * Get GPU renderer string
         * @return GPU renderer (e.g., "GeForce RTX 3080")
         */
        const std::string& GetRenderer() const { return m_renderer; }
        
        /**
         * Check if specific OpenGL extension is supported
         * @param extension Extension name (e.g., "GL_ARB_compute_shader")
         * @return true if extension is supported, false otherwise
         */
        bool HasExtension(const std::string& extension) const;
        
        /**
         * Get list of all supported extensions
         * @return vector of extension names
         */
        const std::vector<std::string>& GetSupportedExtensions() const { return m_extensions; }
        
        // Feature detection
        
        /**
         * Check if compute shaders are supported
         * @return true if compute shaders are available, false otherwise
         */
        bool SupportsComputeShaders() const;
        
        /**
         * Check if tessellation shaders are supported
         * @return true if tessellation is available, false otherwise
         */
        bool SupportsTessellation() const;
        
        /**
         * Check if geometry shaders are supported
         * @return true if geometry shaders are available, false otherwise
         */
        bool SupportsGeometryShaders() const;
        
        /**
         * Check if shader storage buffer objects are supported
         * @return true if SSBOs are available, false otherwise
         */
        bool SupportsStorageBuffers() const;
        
        /**
         * Check if image load/store operations are supported
         * @return true if image operations are available, false otherwise
         */
        bool SupportsImageLoadStore() const;
        
        /**
         * Check if atomic operations are supported
         * @return true if atomic operations are available, false otherwise
         */
        bool SupportsAtomicOperations() const;
        
        /**
         * Check if uniform buffer objects are supported
         * @return true if UBOs are available, false otherwise
         */
        bool SupportsUniformBuffers() const;
        
        /**
         * Check if texture arrays are supported
         * @return true if texture arrays are available, false otherwise
         */
        bool SupportsTextureArrays() const;
        
        /**
         * Check if multisampling is supported
         * @return true if multisampling is available, false otherwise
         */
        bool SupportsMultisampling() const;
        
        // Hardware limits
        
        /**
         * Get compute shader hardware limits
         * @return compute shader limits structure
         */
        const ComputeShaderLimits& GetComputeShaderLimits() const { return m_computeLimits; }
        
        /**
         * Get general shader hardware limits
         * @return shader limits structure
         */
        const ShaderLimits& GetShaderLimits() const { return m_shaderLimits; }
        
        /**
         * Get texture hardware limits
         * @return texture limits structure
         */
        const TextureLimits& GetTextureLimits() const { return m_textureLimits; }
        
        /**
         * Get general OpenGL limits
         * @return general limits structure
         */
        const GeneralLimits& GetGeneralLimits() const { return m_generalLimits; }
        
        // Capability-based recommendations
        
        /**
         * Get recommended maximum work group size for compute shaders
         * @return recommended work group size (x, y, z)
         */
        std::tuple<int, int, int> GetRecommendedWorkGroupSize() const;
        
        /**
         * Check if hardware can handle specific compute workload
         * @param workGroupSizeX Work group size in X dimension
         * @param workGroupSizeY Work group size in Y dimension
         * @param workGroupSizeZ Work group size in Z dimension
         * @param numGroups Total number of work groups
         * @return true if workload is feasible, false otherwise
         */
        bool CanHandleComputeWorkload(int workGroupSizeX, int workGroupSizeY, int workGroupSizeZ, int numGroups) const;
        
        /**
         * Get maximum safe texture size for current hardware
         * @return maximum texture size that should work reliably
         */
        int GetSafeMaxTextureSize() const;
        
        /**
         * Check if hardware supports specific shader complexity
         * @param uniformCount Number of uniforms needed
         * @param textureCount Number of textures needed
         * @return true if shader complexity is supported, false otherwise
         */
        bool CanHandleShaderComplexity(int uniformCount, int textureCount) const;
        
        // Reporting and diagnostics
        
        /**
         * Generate comprehensive hardware capability report
         * @return detailed capability report string
         */
        std::string GenerateCapabilityReport() const;
        
        /**
         * Get list of hardware limitations and warnings
         * @return vector of limitation descriptions
         */
        std::vector<std::string> GetHardwareLimitations() const;
        
        /**
         * Get list of missing features that might affect functionality
         * @return vector of missing feature descriptions
         */
        std::vector<std::string> GetMissingFeatures() const;
        
        /**
         * Check if current hardware meets minimum requirements
         * @return true if minimum requirements are met, false otherwise
         */
        bool MeetsMinimumRequirements() const;
        
        /**
         * Get performance tier of current hardware
         * @return performance tier (0=low, 1=medium, 2=high, 3=ultra)
         */
        int GetPerformanceTier() const;
        
    private:
        HardwareCapabilities() = default;
        ~HardwareCapabilities() = default;
        
        // Prevent copying
        HardwareCapabilities(const HardwareCapabilities&) = delete;
        HardwareCapabilities& operator=(const HardwareCapabilities&) = delete;
        
        // Internal initialization methods
        void DetectOpenGLVersion();
        void DetectExtensions();
        void DetectComputeShaderLimits();
        void DetectShaderLimits();
        void DetectTextureLimits();
        void DetectGeneralLimits();
        void AnalyzePerformanceCharacteristics();
        
        // Helper methods
        bool CheckMinimumOpenGLVersion(float minVersion) const;
        void LogCapabilityInfo() const;
        
        // State
        static bool s_initialized;
        static HardwareCapabilities s_instance;
        
        // OpenGL information
        float m_openGLVersion = 0.0f;
        std::string m_versionString;
        std::string m_vendor;
        std::string m_renderer;
        std::vector<std::string> m_extensions;
        std::unordered_set<std::string> m_extensionSet; // For fast lookup
        
        // Hardware limits
        ComputeShaderLimits m_computeLimits;
        ShaderLimits m_shaderLimits;
        TextureLimits m_textureLimits;
        GeneralLimits m_generalLimits;
        
        // Performance characteristics
        int m_performanceTier = 0;
        bool m_meetsMinimumRequirements = false;
    };
}