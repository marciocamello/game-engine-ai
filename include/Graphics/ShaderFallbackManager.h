#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

namespace GameEngine {
    class Shader;
    class HardwareCapabilities;
    
    /**
     * Manages fallback shaders and alternative implementations for unsupported hardware features
     */
    class ShaderFallbackManager {
    public:
        enum class FallbackType {
            ComputeShader,      // Fallback for compute shader functionality
            GeometryShader,     // Fallback for geometry shader functionality
            TessellationShader, // Fallback for tessellation functionality
            StorageBuffer,      // Fallback for shader storage buffers
            ImageLoadStore,     // Fallback for image load/store operations
            AtomicOperations,   // Fallback for atomic operations
            AdvancedTexturing,  // Fallback for advanced texture features
            HighPrecision       // Fallback for high precision operations
        };
        
        struct FallbackInfo {
            FallbackType type;
            std::string originalShaderName;
            std::string fallbackShaderName;
            std::string description;
            std::function<bool()> isNeeded; // Function to check if fallback is needed
            std::function<std::shared_ptr<Shader>()> createFallback; // Function to create fallback shader
        };
        
        /**
         * Get singleton instance
         * @return reference to singleton instance
         */
        static ShaderFallbackManager& GetInstance();
        
        /**
         * Initialize the fallback manager
         * Must be called after HardwareCapabilities is initialized
         * @return true if initialization successful, false otherwise
         */
        bool Initialize();
        
        /**
         * Shutdown the fallback manager
         */
        void Shutdown();
        
        /**
         * Check if fallback manager is initialized
         * @return true if initialized, false otherwise
         */
        bool IsInitialized() const { return m_initialized; }
        
        // Fallback registration and management
        
        /**
         * Register a fallback shader for a specific feature
         * @param type Type of fallback
         * @param originalName Name of the original shader
         * @param fallbackName Name of the fallback shader
         * @param description Description of the fallback
         * @param isNeeded Function to check if fallback is needed
         * @param createFallback Function to create the fallback shader
         */
        void RegisterFallback(FallbackType type, const std::string& originalName, 
                            const std::string& fallbackName, const std::string& description,
                            std::function<bool()> isNeeded,
                            std::function<std::shared_ptr<Shader>()> createFallback);
        
        /**
         * Get fallback shader for a specific shader if needed
         * @param originalShaderName Name of the original shader
         * @return fallback shader if needed, nullptr if original should be used
         */
        std::shared_ptr<Shader> GetFallbackShader(const std::string& originalShaderName);
        
        /**
         * Check if a fallback is needed for a specific shader
         * @param originalShaderName Name of the original shader
         * @return true if fallback is needed, false otherwise
         */
        bool IsFallbackNeeded(const std::string& originalShaderName) const;
        
        /**
         * Get fallback type for a specific shader
         * @param originalShaderName Name of the original shader
         * @return fallback type, or empty if no fallback registered
         */
        std::vector<FallbackType> GetFallbackTypes(const std::string& originalShaderName) const;
        
        // Built-in fallback implementations
        
        /**
         * Create compute shader fallback using vertex/fragment shaders
         * @param computeShaderName Name of the compute shader to fallback
         * @return fallback shader implementation
         */
        std::shared_ptr<Shader> CreateComputeShaderFallback(const std::string& computeShaderName);
        
        /**
         * Create geometry shader fallback using vertex shader instancing
         * @param geometryShaderName Name of the geometry shader to fallback
         * @return fallback shader implementation
         */
        std::shared_ptr<Shader> CreateGeometryShaderFallback(const std::string& geometryShaderName);
        
        /**
         * Create tessellation fallback using higher vertex density
         * @param tessellationShaderName Name of the tessellation shader to fallback
         * @return fallback shader implementation
         */
        std::shared_ptr<Shader> CreateTessellationFallback(const std::string& tessellationShaderName);
        
        /**
         * Create storage buffer fallback using texture buffers
         * @param originalShaderName Name of the shader using storage buffers
         * @return fallback shader implementation
         */
        std::shared_ptr<Shader> CreateStorageBufferFallback(const std::string& originalShaderName);
        
        /**
         * Create image load/store fallback using render targets
         * @param originalShaderName Name of the shader using image operations
         * @return fallback shader implementation
         */
        std::shared_ptr<Shader> CreateImageLoadStoreFallback(const std::string& originalShaderName);
        
        /**
         * Create atomic operations fallback using CPU synchronization
         * @param originalShaderName Name of the shader using atomic operations
         * @return fallback shader implementation
         */
        std::shared_ptr<Shader> CreateAtomicOperationsFallback(const std::string& originalShaderName);
        
        // Fallback shader source generation
        
        /**
         * Generate fallback shader source with feature replacements
         * @param originalSource Original shader source code
         * @param fallbackType Type of fallback to apply
         * @return modified shader source with fallback implementations
         */
        std::string GenerateFallbackShaderSource(const std::string& originalSource, FallbackType fallbackType);
        
        /**
         * Replace compute shader functionality with vertex/fragment equivalent
         * @param computeSource Original compute shader source
         * @return vertex and fragment shader sources for equivalent functionality
         */
        std::pair<std::string, std::string> ConvertComputeToVertexFragment(const std::string& computeSource);
        
        /**
         * Replace geometry shader functionality with vertex shader instancing
         * @param geometrySource Original geometry shader source
         * @param vertexSource Original vertex shader source
         * @return modified vertex shader with geometry functionality
         */
        std::string ConvertGeometryToVertex(const std::string& geometrySource, const std::string& vertexSource);
        
        /**
         * Replace storage buffer operations with texture buffer operations
         * @param shaderSource Original shader source with storage buffers
         * @return modified shader source using texture buffers
         */
        std::string ConvertStorageBuffersToTextureBuffers(const std::string& shaderSource);
        
        /**
         * Replace image load/store operations with render target operations
         * @param shaderSource Original shader source with image operations
         * @return modified shader source using render targets
         */
        std::string ConvertImageOpsToRenderTargets(const std::string& shaderSource);
        
        // Hardware compatibility checking
        
        /**
         * Check if current hardware supports all features used by a shader
         * @param shaderSource Shader source code to analyze
         * @return true if all features are supported, false if fallbacks needed
         */
        bool IsShaderFullySupported(const std::string& shaderSource) const;
        
        /**
         * Analyze shader source and determine required fallbacks
         * @param shaderSource Shader source code to analyze
         * @return vector of fallback types needed for this shader
         */
        std::vector<FallbackType> AnalyzeRequiredFallbacks(const std::string& shaderSource) const;
        
        /**
         * Get list of unsupported features in a shader
         * @param shaderSource Shader source code to analyze
         * @return vector of feature descriptions that are not supported
         */
        std::vector<std::string> GetUnsupportedFeatures(const std::string& shaderSource) const;
        
        // Reporting and diagnostics
        
        /**
         * Generate fallback usage report
         * @return detailed report of active fallbacks and their impact
         */
        std::string GenerateFallbackReport() const;
        
        /**
         * Get list of active fallbacks
         * @return vector of currently active fallback descriptions
         */
        std::vector<std::string> GetActiveFallbacks() const;
        
        /**
         * Get performance impact of current fallbacks
         * @return estimated performance impact (0.0 = no impact, 1.0 = severe impact)
         */
        float GetFallbackPerformanceImpact() const;
        
        /**
         * Log fallback information for debugging
         */
        void LogFallbackInfo() const;
        
    private:
        ShaderFallbackManager() = default;
        ~ShaderFallbackManager() = default;
        
        // Prevent copying
        ShaderFallbackManager(const ShaderFallbackManager&) = delete;
        ShaderFallbackManager& operator=(const ShaderFallbackManager&) = delete;
        
        // Internal methods
        void RegisterBuiltinFallbacks();
        void AnalyzeHardwareCapabilities();
        std::string LoadFallbackShaderTemplate(const std::string& templateName) const;
        
        // Helper methods for shader analysis
        bool ContainsComputeShaderFeatures(const std::string& shaderSource) const;
        bool ContainsGeometryShaderFeatures(const std::string& shaderSource) const;
        bool ContainsTessellationFeatures(const std::string& shaderSource) const;
        bool ContainsStorageBufferFeatures(const std::string& shaderSource) const;
        bool ContainsImageLoadStoreFeatures(const std::string& shaderSource) const;
        bool ContainsAtomicOperations(const std::string& shaderSource) const;
        
        // String replacement helpers
        std::string ReplaceShaderKeywords(const std::string& source, 
                                        const std::unordered_map<std::string, std::string>& replacements) const;
        std::vector<std::string> ExtractShaderFunctions(const std::string& source, const std::string& functionPrefix) const;
        
        // State
        bool m_initialized = false;
        
        // Fallback registry
        std::unordered_map<std::string, std::vector<FallbackInfo>> m_fallbacks; // shader name -> fallback info
        std::unordered_map<std::string, std::shared_ptr<Shader>> m_fallbackShaders; // cached fallback shaders
        
        // Hardware capability flags (cached for performance)
        bool m_supportsComputeShaders = true;
        bool m_supportsGeometryShaders = true;
        bool m_supportsTessellation = true;
        bool m_supportsStorageBuffers = true;
        bool m_supportsImageLoadStore = true;
        bool m_supportsAtomicOperations = true;
        
        // Statistics
        mutable size_t m_fallbacksCreated = 0;
        mutable size_t m_fallbacksUsed = 0;
        mutable float m_totalPerformanceImpact = 0.0f;
    };
}