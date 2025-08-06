#include "Graphics/ShaderFallbackManager.h"
#include "Graphics/HardwareCapabilities.h"
#include "Graphics/Shader.h"
#include "Core/Logger.h"
#include <regex>
#include <sstream>
#include <algorithm>

namespace GameEngine {
    
    ShaderFallbackManager& ShaderFallbackManager::GetInstance() {
        static ShaderFallbackManager instance;
        return instance;
    }
    
    bool ShaderFallbackManager::Initialize() {
        if (m_initialized) {
            LOG_WARNING("ShaderFallbackManager already initialized");
            return true;
        }
        
        if (!HardwareCapabilities::IsInitialized()) {
            LOG_ERROR("Cannot initialize ShaderFallbackManager: HardwareCapabilities not initialized");
            return false;
        }
        
        LOG_INFO("Initializing ShaderFallbackManager");
        
        // Analyze hardware capabilities
        AnalyzeHardwareCapabilities();
        
        // Register built-in fallbacks
        RegisterBuiltinFallbacks();
        
        // Clear caches
        m_fallbackShaders.clear();
        m_fallbacksCreated = 0;
        m_fallbacksUsed = 0;
        m_totalPerformanceImpact = 0.0f;
        
        m_initialized = true;
        
        // Log fallback information
        LogFallbackInfo();
        
        LOG_INFO("ShaderFallbackManager initialized successfully");
        return true;
    }
    
    void ShaderFallbackManager::Shutdown() {
        if (!m_initialized) {
            return;
        }
        
        LOG_INFO("Shutting down ShaderFallbackManager");
        
        m_fallbacks.clear();
        m_fallbackShaders.clear();
        
        m_initialized = false;
        LOG_INFO("ShaderFallbackManager shutdown complete");
    }
    
    void ShaderFallbackManager::RegisterFallback(FallbackType type, const std::string& originalName,
                                                const std::string& fallbackName, const std::string& description,
                                                std::function<bool()> isNeeded,
                                                std::function<std::shared_ptr<Shader>()> createFallback) {
        FallbackInfo info;
        info.type = type;
        info.originalShaderName = originalName;
        info.fallbackShaderName = fallbackName;
        info.description = description;
        info.isNeeded = isNeeded;
        info.createFallback = createFallback;
        
        m_fallbacks[originalName].push_back(info);
        
        if (m_initialized) {
            LOG_INFO("Registered fallback for '" + originalName + "': " + description);
        }
    }
    
    std::shared_ptr<Shader> ShaderFallbackManager::GetFallbackShader(const std::string& originalShaderName) {
        // Check if we have any fallbacks for this shader
        auto it = m_fallbacks.find(originalShaderName);
        if (it == m_fallbacks.end()) {
            return nullptr; // No fallbacks registered
        }
        
        // Check if any fallbacks are needed
        bool needsFallback = false;
        FallbackInfo* selectedFallback = nullptr;
        
        for (auto& fallback : it->second) {
            if (fallback.isNeeded && fallback.isNeeded()) {
                needsFallback = true;
                selectedFallback = &fallback;
                break; // Use first needed fallback
            }
        }
        
        if (!needsFallback || !selectedFallback) {
            return nullptr; // No fallback needed
        }
        
        // Check if we already have a cached fallback shader
        std::string cacheKey = originalShaderName + "_" + selectedFallback->fallbackShaderName;
        auto cacheIt = m_fallbackShaders.find(cacheKey);
        if (cacheIt != m_fallbackShaders.end()) {
            m_fallbacksUsed++;
            return cacheIt->second;
        }
        
        // Create new fallback shader
        std::shared_ptr<Shader> fallbackShader = nullptr;
        if (selectedFallback->createFallback) {
            fallbackShader = selectedFallback->createFallback();
        }
        
        if (fallbackShader) {
            m_fallbackShaders[cacheKey] = fallbackShader;
            m_fallbacksCreated++;
            m_fallbacksUsed++;
            
            LOG_INFO("Created fallback shader for '" + originalShaderName + "': " + selectedFallback->description);
        } else {
            LOG_ERROR("Failed to create fallback shader for '" + originalShaderName + "'");
        }
        
        return fallbackShader;
    }
    
    bool ShaderFallbackManager::IsFallbackNeeded(const std::string& originalShaderName) const {
        auto it = m_fallbacks.find(originalShaderName);
        if (it == m_fallbacks.end()) {
            return false;
        }
        
        for (const auto& fallback : it->second) {
            if (fallback.isNeeded && fallback.isNeeded()) {
                return true;
            }
        }
        
        return false;
    }
    
    std::vector<ShaderFallbackManager::FallbackType> ShaderFallbackManager::GetFallbackTypes(const std::string& originalShaderName) const {
        std::vector<FallbackType> types;
        
        auto it = m_fallbacks.find(originalShaderName);
        if (it != m_fallbacks.end()) {
            for (const auto& fallback : it->second) {
                if (fallback.isNeeded && fallback.isNeeded()) {
                    types.push_back(fallback.type);
                }
            }
        }
        
        return types;
    }
    
    std::shared_ptr<Shader> ShaderFallbackManager::CreateComputeShaderFallback(const std::string& computeShaderName) {
        if (m_supportsComputeShaders) {
            return nullptr; // No fallback needed
        }
        
        LOG_INFO("Creating compute shader fallback for: " + computeShaderName);
        
        // Create a basic vertex/fragment shader pair that simulates compute functionality
        // This is a simplified implementation - in practice, you'd need to analyze the compute shader
        // and convert its operations to vertex/fragment operations using textures as data storage
        
        auto fallbackShader = std::make_shared<Shader>();
        
        std::string vertexSource = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec2 aTexCoord;
            
            out vec2 TexCoord;
            
            void main() {
                gl_Position = vec4(aPos, 0.0, 1.0);
                TexCoord = aTexCoord;
            }
        )";
        
        std::string fragmentSource = R"(
            #version 330 core
            in vec2 TexCoord;
            out vec4 FragColor;
            
            uniform sampler2D inputTexture;
            
            // Fallback compute functionality using fragment shader
            void main() {
                // Simple pass-through - real implementation would contain
                // the converted compute shader logic
                vec4 inputData = texture(inputTexture, TexCoord);
                
                // Placeholder for compute operations
                FragColor = inputData;
            }
        )";
        
        if (fallbackShader->LoadFromSource(vertexSource, fragmentSource)) {
            m_totalPerformanceImpact += 0.3f; // Estimate 30% performance impact
            return fallbackShader;
        }
        
        LOG_ERROR("Failed to create compute shader fallback");
        return nullptr;
    }
    
    std::shared_ptr<Shader> ShaderFallbackManager::CreateGeometryShaderFallback(const std::string& geometryShaderName) {
        if (m_supportsGeometryShaders) {
            return nullptr; // No fallback needed
        }
        
        LOG_INFO("Creating geometry shader fallback for: " + geometryShaderName);
        
        // Create vertex/fragment shader that uses instancing to simulate geometry shader functionality
        auto fallbackShader = std::make_shared<Shader>();
        
        std::string vertexSource = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;
            layout (location = 2) in vec2 aTexCoord;
            
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            
            out vec3 FragPos;
            out vec3 Normal;
            out vec2 TexCoord;
            
            // Fallback geometry functionality using vertex shader instancing
            void main() {
                // Basic vertex transformation - real implementation would contain
                // the converted geometry shader logic using instancing
                FragPos = vec3(model * vec4(aPos, 1.0));
                Normal = mat3(transpose(inverse(model))) * aNormal;
                TexCoord = aTexCoord;
                
                gl_Position = projection * view * vec4(FragPos, 1.0);
            }
        )";
        
        std::string fragmentSource = R"(
            #version 330 core
            in vec3 FragPos;
            in vec3 Normal;
            in vec2 TexCoord;
            
            out vec4 FragColor;
            
            uniform vec3 lightPos;
            uniform vec3 viewPos;
            uniform vec3 lightColor;
            uniform vec3 objectColor;
            
            void main() {
                // Basic lighting calculation
                vec3 ambient = 0.15 * lightColor;
                
                vec3 norm = normalize(Normal);
                vec3 lightDir = normalize(lightPos - FragPos);
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = diff * lightColor;
                
                vec3 viewDir = normalize(viewPos - FragPos);
                vec3 reflectDir = reflect(-lightDir, norm);
                float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
                vec3 specular = spec * lightColor;
                
                vec3 result = (ambient + diffuse + specular) * objectColor;
                FragColor = vec4(result, 1.0);
            }
        )";
        
        if (fallbackShader->LoadFromSource(vertexSource, fragmentSource)) {
            m_totalPerformanceImpact += 0.2f; // Estimate 20% performance impact
            return fallbackShader;
        }
        
        LOG_ERROR("Failed to create geometry shader fallback");
        return nullptr;
    }
    
    std::shared_ptr<Shader> ShaderFallbackManager::CreateTessellationFallback(const std::string& tessellationShaderName) {
        if (m_supportsTessellation) {
            return nullptr; // No fallback needed
        }
        
        LOG_INFO("Creating tessellation fallback for: " + tessellationShaderName);
        
        // Create vertex/fragment shader with higher vertex density to simulate tessellation
        auto fallbackShader = std::make_shared<Shader>();
        
        std::string vertexSource = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;
            layout (location = 2) in vec2 aTexCoord;
            
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            uniform float tessellationLevel;
            
            out vec3 FragPos;
            out vec3 Normal;
            out vec2 TexCoord;
            
            // Fallback tessellation using pre-tessellated geometry
            void main() {
                // Apply displacement based on tessellation level
                vec3 displacedPos = aPos;
                
                // Simple displacement along normal (placeholder for real tessellation)
                float displacement = sin(aPos.x * tessellationLevel) * sin(aPos.z * tessellationLevel) * 0.1;
                displacedPos += aNormal * displacement;
                
                FragPos = vec3(model * vec4(displacedPos, 1.0));
                Normal = mat3(transpose(inverse(model))) * aNormal;
                TexCoord = aTexCoord;
                
                gl_Position = projection * view * vec4(FragPos, 1.0);
            }
        )";
        
        std::string fragmentSource = R"(
            #version 330 core
            in vec3 FragPos;
            in vec3 Normal;
            in vec2 TexCoord;
            
            out vec4 FragColor;
            
            uniform vec3 lightPos;
            uniform vec3 viewPos;
            uniform vec3 lightColor;
            uniform vec3 objectColor;
            
            void main() {
                // Standard lighting calculation
                vec3 ambient = 0.15 * lightColor;
                
                vec3 norm = normalize(Normal);
                vec3 lightDir = normalize(lightPos - FragPos);
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = diff * lightColor;
                
                vec3 viewDir = normalize(viewPos - FragPos);
                vec3 reflectDir = reflect(-lightDir, norm);
                float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
                vec3 specular = spec * lightColor;
                
                vec3 result = (ambient + diffuse + specular) * objectColor;
                FragColor = vec4(result, 1.0);
            }
        )";
        
        if (fallbackShader->LoadFromSource(vertexSource, fragmentSource)) {
            m_totalPerformanceImpact += 0.4f; // Estimate 40% performance impact (pre-tessellation is expensive)
            return fallbackShader;
        }
        
        LOG_ERROR("Failed to create tessellation fallback");
        return nullptr;
    }
    
    std::shared_ptr<Shader> ShaderFallbackManager::CreateStorageBufferFallback(const std::string& originalShaderName) {
        if (m_supportsStorageBuffers) {
            return nullptr; // No fallback needed
        }
        
        LOG_INFO("Creating storage buffer fallback for: " + originalShaderName);
        
        // This would create a shader that uses texture buffers instead of storage buffers
        // For now, return a basic shader as placeholder
        auto fallbackShader = std::make_shared<Shader>();
        
        // In a real implementation, you would:
        // 1. Parse the original shader to find storage buffer usage
        // 2. Replace storage buffer declarations with texture buffer declarations
        // 3. Replace storage buffer access with texture buffer access
        
        m_totalPerformanceImpact += 0.15f; // Estimate 15% performance impact
        return fallbackShader;
    }
    
    std::shared_ptr<Shader> ShaderFallbackManager::CreateImageLoadStoreFallback(const std::string& originalShaderName) {
        if (m_supportsImageLoadStore) {
            return nullptr; // No fallback needed
        }
        
        LOG_INFO("Creating image load/store fallback for: " + originalShaderName);
        
        // This would create a shader that uses render targets instead of image load/store
        auto fallbackShader = std::make_shared<Shader>();
        
        m_totalPerformanceImpact += 0.25f; // Estimate 25% performance impact
        return fallbackShader;
    }
    
    std::shared_ptr<Shader> ShaderFallbackManager::CreateAtomicOperationsFallback(const std::string& originalShaderName) {
        if (m_supportsAtomicOperations) {
            return nullptr; // No fallback needed
        }
        
        LOG_INFO("Creating atomic operations fallback for: " + originalShaderName);
        
        // This would create a shader that uses CPU synchronization instead of GPU atomics
        auto fallbackShader = std::make_shared<Shader>();
        
        m_totalPerformanceImpact += 0.5f; // Estimate 50% performance impact (CPU sync is expensive)
        return fallbackShader;
    }
    
    bool ShaderFallbackManager::IsShaderFullySupported(const std::string& shaderSource) const {
        auto requiredFallbacks = AnalyzeRequiredFallbacks(shaderSource);
        return requiredFallbacks.empty();
    }
    
    std::vector<ShaderFallbackManager::FallbackType> ShaderFallbackManager::AnalyzeRequiredFallbacks(const std::string& shaderSource) const {
        std::vector<FallbackType> fallbacks;
        
        if (!m_supportsComputeShaders && ContainsComputeShaderFeatures(shaderSource)) {
            fallbacks.push_back(FallbackType::ComputeShader);
        }
        
        if (!m_supportsGeometryShaders && ContainsGeometryShaderFeatures(shaderSource)) {
            fallbacks.push_back(FallbackType::GeometryShader);
        }
        
        if (!m_supportsTessellation && ContainsTessellationFeatures(shaderSource)) {
            fallbacks.push_back(FallbackType::TessellationShader);
        }
        
        if (!m_supportsStorageBuffers && ContainsStorageBufferFeatures(shaderSource)) {
            fallbacks.push_back(FallbackType::StorageBuffer);
        }
        
        if (!m_supportsImageLoadStore && ContainsImageLoadStoreFeatures(shaderSource)) {
            fallbacks.push_back(FallbackType::ImageLoadStore);
        }
        
        if (!m_supportsAtomicOperations && ContainsAtomicOperations(shaderSource)) {
            fallbacks.push_back(FallbackType::AtomicOperations);
        }
        
        return fallbacks;
    }
    
    std::vector<std::string> ShaderFallbackManager::GetUnsupportedFeatures(const std::string& shaderSource) const {
        std::vector<std::string> unsupported;
        
        if (!m_supportsComputeShaders && ContainsComputeShaderFeatures(shaderSource)) {
            unsupported.push_back("Compute Shaders");
        }
        
        if (!m_supportsGeometryShaders && ContainsGeometryShaderFeatures(shaderSource)) {
            unsupported.push_back("Geometry Shaders");
        }
        
        if (!m_supportsTessellation && ContainsTessellationFeatures(shaderSource)) {
            unsupported.push_back("Tessellation Shaders");
        }
        
        if (!m_supportsStorageBuffers && ContainsStorageBufferFeatures(shaderSource)) {
            unsupported.push_back("Shader Storage Buffers");
        }
        
        if (!m_supportsImageLoadStore && ContainsImageLoadStoreFeatures(shaderSource)) {
            unsupported.push_back("Image Load/Store Operations");
        }
        
        if (!m_supportsAtomicOperations && ContainsAtomicOperations(shaderSource)) {
            unsupported.push_back("Atomic Operations");
        }
        
        return unsupported;
    }
    
    std::string ShaderFallbackManager::GenerateFallbackReport() const {
        std::stringstream report;
        
        report << "=== Shader Fallback Report ===\n\n";
        
        report << "Fallbacks Created: " << m_fallbacksCreated << "\n";
        report << "Fallbacks Used: " << m_fallbacksUsed << "\n";
        report << "Total Performance Impact: " << (m_totalPerformanceImpact * 100.0f) << "%\n\n";
        
        report << "Hardware Support Status:\n";
        report << "  Compute Shaders: " << (m_supportsComputeShaders ? "Supported" : "NOT SUPPORTED") << "\n";
        report << "  Geometry Shaders: " << (m_supportsGeometryShaders ? "Supported" : "NOT SUPPORTED") << "\n";
        report << "  Tessellation: " << (m_supportsTessellation ? "Supported" : "NOT SUPPORTED") << "\n";
        report << "  Storage Buffers: " << (m_supportsStorageBuffers ? "Supported" : "NOT SUPPORTED") << "\n";
        report << "  Image Load/Store: " << (m_supportsImageLoadStore ? "Supported" : "NOT SUPPORTED") << "\n";
        report << "  Atomic Operations: " << (m_supportsAtomicOperations ? "Supported" : "NOT SUPPORTED") << "\n\n";
        
        if (!m_fallbacks.empty()) {
            report << "Registered Fallbacks:\n";
            for (const auto& pair : m_fallbacks) {
                report << "  Shader: " << pair.first << "\n";
                for (const auto& fallback : pair.second) {
                    report << "    - " << fallback.description << "\n";
                }
            }
        }
        
        return report.str();
    }
    
    std::vector<std::string> ShaderFallbackManager::GetActiveFallbacks() const {
        std::vector<std::string> active;
        
        for (const auto& pair : m_fallbacks) {
            for (const auto& fallback : pair.second) {
                if (fallback.isNeeded && fallback.isNeeded()) {
                    active.push_back(pair.first + ": " + fallback.description);
                }
            }
        }
        
        return active;
    }
    
    float ShaderFallbackManager::GetFallbackPerformanceImpact() const {
        return m_totalPerformanceImpact;
    }
    
    void ShaderFallbackManager::LogFallbackInfo() const {
        LOG_INFO("Shader Fallback Status:");
        LOG_INFO("  Compute Shaders: " + std::string(m_supportsComputeShaders ? "Supported" : "FALLBACK NEEDED"));
        LOG_INFO("  Geometry Shaders: " + std::string(m_supportsGeometryShaders ? "Supported" : "FALLBACK NEEDED"));
        LOG_INFO("  Tessellation: " + std::string(m_supportsTessellation ? "Supported" : "FALLBACK NEEDED"));
        LOG_INFO("  Storage Buffers: " + std::string(m_supportsStorageBuffers ? "Supported" : "FALLBACK NEEDED"));
        LOG_INFO("  Image Load/Store: " + std::string(m_supportsImageLoadStore ? "Supported" : "FALLBACK NEEDED"));
        LOG_INFO("  Atomic Operations: " + std::string(m_supportsAtomicOperations ? "Supported" : "FALLBACK NEEDED"));
        
        auto activeFallbacks = GetActiveFallbacks();
        if (!activeFallbacks.empty()) {
            LOG_WARNING("Active Fallbacks:");
            for (const auto& fallback : activeFallbacks) {
                LOG_WARNING("  - " + fallback);
            }
        }
    }
    
    void ShaderFallbackManager::RegisterBuiltinFallbacks() {
        // Register compute shader fallbacks
        RegisterFallback(FallbackType::ComputeShader, "particle_compute", "particle_vertex_fragment",
                        "Compute shader fallback using vertex/fragment shaders",
                        [this]() { return !m_supportsComputeShaders; },
                        [this]() { return CreateComputeShaderFallback("particle_compute"); });
        
        // Register geometry shader fallbacks
        RegisterFallback(FallbackType::GeometryShader, "wireframe_geometry", "wireframe_instanced",
                        "Geometry shader fallback using instanced rendering",
                        [this]() { return !m_supportsGeometryShaders; },
                        [this]() { return CreateGeometryShaderFallback("wireframe_geometry"); });
        
        // Register tessellation fallbacks
        RegisterFallback(FallbackType::TessellationShader, "terrain_tessellation", "terrain_pretessellated",
                        "Tessellation fallback using pre-tessellated geometry",
                        [this]() { return !m_supportsTessellation; },
                        [this]() { return CreateTessellationFallback("terrain_tessellation"); });
        
        // Register storage buffer fallbacks
        RegisterFallback(FallbackType::StorageBuffer, "data_processing", "data_texture_buffer",
                        "Storage buffer fallback using texture buffers",
                        [this]() { return !m_supportsStorageBuffers; },
                        [this]() { return CreateStorageBufferFallback("data_processing"); });
        
        // Register image load/store fallbacks
        RegisterFallback(FallbackType::ImageLoadStore, "image_processing", "render_target_processing",
                        "Image load/store fallback using render targets",
                        [this]() { return !m_supportsImageLoadStore; },
                        [this]() { return CreateImageLoadStoreFallback("image_processing"); });
        
        // Register atomic operations fallbacks
        RegisterFallback(FallbackType::AtomicOperations, "atomic_counter", "cpu_synchronized",
                        "Atomic operations fallback using CPU synchronization",
                        [this]() { return !m_supportsAtomicOperations; },
                        [this]() { return CreateAtomicOperationsFallback("atomic_counter"); });
    }
    
    void ShaderFallbackManager::AnalyzeHardwareCapabilities() {
        const auto& capabilities = HardwareCapabilities::GetInstance();
        
        m_supportsComputeShaders = capabilities.SupportsComputeShaders();
        m_supportsGeometryShaders = capabilities.SupportsGeometryShaders();
        m_supportsTessellation = capabilities.SupportsTessellation();
        m_supportsStorageBuffers = capabilities.SupportsStorageBuffers();
        m_supportsImageLoadStore = capabilities.SupportsImageLoadStore();
        m_supportsAtomicOperations = capabilities.SupportsAtomicOperations();
    }
    
    // Helper methods for shader feature detection
    bool ShaderFallbackManager::ContainsComputeShaderFeatures(const std::string& shaderSource) const {
        return shaderSource.find("GL_COMPUTE_SHADER") != std::string::npos ||
               shaderSource.find("local_size_") != std::string::npos ||
               shaderSource.find("gl_GlobalInvocationID") != std::string::npos;
    }
    
    bool ShaderFallbackManager::ContainsGeometryShaderFeatures(const std::string& shaderSource) const {
        return shaderSource.find("GL_GEOMETRY_SHADER") != std::string::npos ||
               shaderSource.find("EmitVertex") != std::string::npos ||
               shaderSource.find("EndPrimitive") != std::string::npos;
    }
    
    bool ShaderFallbackManager::ContainsTessellationFeatures(const std::string& shaderSource) const {
        return shaderSource.find("GL_TESS_CONTROL_SHADER") != std::string::npos ||
               shaderSource.find("GL_TESS_EVALUATION_SHADER") != std::string::npos ||
               shaderSource.find("gl_TessLevelOuter") != std::string::npos;
    }
    
    bool ShaderFallbackManager::ContainsStorageBufferFeatures(const std::string& shaderSource) const {
        return shaderSource.find("buffer") != std::string::npos ||
               shaderSource.find("GL_SHADER_STORAGE_BUFFER") != std::string::npos;
    }
    
    bool ShaderFallbackManager::ContainsImageLoadStoreFeatures(const std::string& shaderSource) const {
        return shaderSource.find("imageLoad") != std::string::npos ||
               shaderSource.find("imageStore") != std::string::npos ||
               shaderSource.find("image2D") != std::string::npos;
    }
    
    bool ShaderFallbackManager::ContainsAtomicOperations(const std::string& shaderSource) const {
        return shaderSource.find("atomicAdd") != std::string::npos ||
               shaderSource.find("atomicExchange") != std::string::npos ||
               shaderSource.find("atomicCompSwap") != std::string::npos;
    }
}