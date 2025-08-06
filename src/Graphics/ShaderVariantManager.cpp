#include "Graphics/ShaderVariantManager.h"
#include "Graphics/Shader.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/HardwareCapabilities.h"
#include "Core/Logger.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>

namespace GameEngine {
    
    ShaderVariantManager& ShaderVariantManager::GetInstance() {
        static ShaderVariantManager instance;
        return instance;
    }

    bool ShaderVariantManager::Initialize() {
        if (m_initialized) {
            LOG_WARNING("ShaderVariantManager already initialized");
            return true;
        }

        LOG_INFO("Initializing ShaderVariantManager");
        
        // Get reference to shader manager
        m_shaderManager = &ShaderManager::GetInstance();
        if (!m_shaderManager) {
            LOG_ERROR("Failed to get ShaderManager instance");
            return false;
        }
        
        // Clear all containers
        m_variants.clear();
        m_variantUsage.clear();
        m_selectionCallback = nullptr;
        
        // Reset stats
        m_stats = VariantStats{};
        m_currentTime = 0.0f;
        
        // Set default configuration
        m_debugMode = false;
        m_asyncCompilation = false;
        m_maxCacheSize = 1000;
        m_variantLifetime = 300.0f; // 5 minutes
        
        m_initialized = true;
        LOG_INFO("ShaderVariantManager initialized successfully");
        return true;
    }

    void ShaderVariantManager::Shutdown() {
        if (!m_initialized) {
            return;
        }

        LOG_INFO("Shutting down ShaderVariantManager");
        
        ClearVariantCache();
        m_selectionCallback = nullptr;
        m_shaderManager = nullptr;
        
        m_initialized = false;
        LOG_INFO("ShaderVariantManager shutdown complete");
    }

    void ShaderVariantManager::Update(float deltaTime) {
        if (!m_initialized) {
            return;
        }

        m_currentTime += deltaTime;
        
        // Periodically clean up old variants (every 30 seconds)
        static float lastCleanupTime = 0.0f;
        if (m_currentTime - lastCleanupTime > 30.0f) {
            CleanupOldVariants();
            lastCleanupTime = m_currentTime;
        }
        
        UpdateVariantStats();
    }

    std::shared_ptr<Shader> ShaderVariantManager::CreateVariant(const std::string& baseName, const ShaderVariant& variant) {
        if (!m_initialized) {
            LOG_ERROR("ShaderVariantManager not initialized");
            return nullptr;
        }

        if (!ValidateVariant(variant)) {
            LOG_ERROR("Invalid shader variant for base shader: " + baseName);
            return nullptr;
        }

        VariantKey key = CreateVariantKey(baseName, variant);
        
        // Check if variant already exists
        auto it = m_variants.find(key);
        if (it != m_variants.end()) {
            if (m_debugMode) {
                LOG_INFO("Shader variant already exists: " + baseName + " with hash " + variant.GenerateHash());
            }
            MarkVariantUsed(baseName, variant);
            m_stats.cacheHits++;
            return it->second;
        }

        // Create new variant
        auto startTime = std::chrono::high_resolution_clock::now();
        auto shaderVariant = CreateVariantInternal(baseName, variant);
        auto endTime = std::chrono::high_resolution_clock::now();
        
        if (!shaderVariant) {
            LOG_ERROR("Failed to create shader variant: " + baseName + " with hash " + variant.GenerateHash());
            return nullptr;
        }

        // Calculate creation time
        float creationTime = std::chrono::duration<float>(endTime - startTime).count();
        
        // Store variant and usage info
        m_variants[key] = shaderVariant;
        m_variantUsage[key] = VariantUsageInfo{
            .lastUsedTime = m_currentTime,
            .useCount = 1,
            .creationTime = creationTime
        };
        
        m_stats.cacheMisses++;
        
        if (m_debugMode) {
            LOG_INFO("Created shader variant: " + baseName + " with hash " + variant.GenerateHash() + 
                    " (creation time: " + std::to_string(creationTime * 1000.0f) + "ms)");
        }
        
        // Check cache size and evict if necessary
        if (m_variants.size() > m_maxCacheSize) {
            OptimizeVariantCache();
        }
        
        return shaderVariant;
    }

    std::shared_ptr<Shader> ShaderVariantManager::GetVariant(const std::string& baseName, const ShaderVariant& variant) {
        if (!m_initialized) {
            LOG_ERROR("ShaderVariantManager not initialized");
            return nullptr;
        }

        VariantKey key = CreateVariantKey(baseName, variant);
        auto it = m_variants.find(key);
        
        if (it != m_variants.end()) {
            MarkVariantUsed(baseName, variant);
            m_stats.cacheHits++;
            return it->second;
        }
        
        m_stats.cacheMisses++;
        return nullptr;
    }

    std::shared_ptr<Shader> ShaderVariantManager::GetOrCreateVariant(const std::string& baseName, const ShaderVariant& variant) {
        auto existingVariant = GetVariant(baseName, variant);
        if (existingVariant) {
            return existingVariant;
        }
        
        return CreateVariant(baseName, variant);
    }

    void ShaderVariantManager::RemoveVariant(const std::string& baseName, const ShaderVariant& variant) {
        VariantKey key = CreateVariantKey(baseName, variant);
        
        auto it = m_variants.find(key);
        if (it != m_variants.end()) {
            if (m_debugMode) {
                LOG_INFO("Removing shader variant: " + baseName + " with hash " + variant.GenerateHash());
            }
            
            m_variants.erase(it);
            m_variantUsage.erase(key);
        }
    }

    void ShaderVariantManager::RemoveAllVariants(const std::string& baseName) {
        std::vector<VariantKey> keysToRemove;
        
        for (const auto& pair : m_variants) {
            if (pair.first.baseName == baseName) {
                keysToRemove.push_back(pair.first);
            }
        }
        
        for (const auto& key : keysToRemove) {
            m_variants.erase(key);
            m_variantUsage.erase(key);
        }
        
        if (m_debugMode && !keysToRemove.empty()) {
            LOG_INFO("Removed " + std::to_string(keysToRemove.size()) + " variants for base shader: " + baseName);
        }
    }

    std::shared_ptr<Shader> ShaderVariantManager::SelectBestVariant(const std::string& baseName, const RenderContext& context) {
        ShaderVariant variant;
        
        if (m_selectionCallback) {
            variant = m_selectionCallback(context);
        } else {
            variant = GenerateVariantFromContext(context);
        }
        
        return GetOrCreateVariant(baseName, variant);
    }

    void ShaderVariantManager::SetVariantSelectionCallback(std::function<ShaderVariant(const RenderContext&)> callback) {
        m_selectionCallback = callback;
    }

    ShaderVariant ShaderVariantManager::GenerateVariantFromContext(const RenderContext& context) {
        ShaderVariant variant("auto_generated");
        
        // Add lighting defines
        if (context.hasDirectionalLight) {
            variant.AddDefine("HAS_DIRECTIONAL_LIGHT", "1");
            variant.AddFeature("DIRECTIONAL_LIGHTING");
        }
        
        if (context.pointLightCount > 0) {
            variant.AddDefine("HAS_POINT_LIGHTS", "1");
            variant.AddDefine("MAX_POINT_LIGHTS", std::to_string(context.pointLightCount));
            variant.AddFeature("POINT_LIGHTING");
        }
        
        if (context.spotLightCount > 0) {
            variant.AddDefine("HAS_SPOT_LIGHTS", "1");
            variant.AddDefine("MAX_SPOT_LIGHTS", std::to_string(context.spotLightCount));
            variant.AddFeature("SPOT_LIGHTING");
        }
        
        if (context.hasShadows) {
            variant.AddDefine("HAS_SHADOWS", "1");
            variant.AddFeature("SHADOW_MAPPING");
        }
        
        // Add material defines
        if (context.hasAlbedoMap) {
            variant.AddDefine("HAS_ALBEDO_MAP", "1");
            variant.AddFeature("ALBEDO_TEXTURE");
        }
        
        if (context.hasNormalMap) {
            variant.AddDefine("HAS_NORMAL_MAP", "1");
            variant.AddFeature("NORMAL_MAPPING");
        }
        
        if (context.hasMetallicRoughnessMap) {
            variant.AddDefine("HAS_METALLIC_ROUGHNESS_MAP", "1");
            variant.AddFeature("METALLIC_ROUGHNESS_TEXTURE");
        }
        
        if (context.hasEmissionMap) {
            variant.AddDefine("HAS_EMISSION_MAP", "1");
            variant.AddFeature("EMISSION_TEXTURE");
        }
        
        if (context.hasAOMap) {
            variant.AddDefine("HAS_AO_MAP", "1");
            variant.AddFeature("AMBIENT_OCCLUSION_TEXTURE");
        }
        
        // Add rendering feature defines
        if (context.hasSkinning) {
            variant.AddDefine("HAS_SKINNING", "1");
            variant.AddDefine("MAX_BONES", std::to_string(context.maxBones));
            variant.AddFeature("VERTEX_SKINNING");
        }
        
        if (context.hasInstancing) {
            variant.AddDefine("HAS_INSTANCING", "1");
            variant.AddFeature("INSTANCED_RENDERING");
        }
        
        // Add debug/optimization defines
        if (context.useDebugMode) {
            variant.AddDefine("DEBUG", "1");
            variant.AddFeature("DEBUG_OUTPUT");
        }
        
        if (context.useOptimizedPath) {
            variant.AddDefine("OPTIMIZED", "1");
            variant.AddFeature("PERFORMANCE_MODE");
        }
        
        // Add hardware capability defines
        if (!context.supportsGeometryShaders) {
            variant.AddDefine("NO_GEOMETRY_SHADERS", "1");
            variant.AddFeature("FALLBACK_GEOMETRY");
        }
        
        if (!context.supportsTessellation) {
            variant.AddDefine("NO_TESSELLATION", "1");
            variant.AddFeature("FALLBACK_TESSELLATION");
        }
        
        if (!context.supportsComputeShaders) {
            variant.AddDefine("NO_COMPUTE_SHADERS", "1");
            variant.AddFeature("FALLBACK_COMPUTE");
        }
        
        if (!context.supportsStorageBuffers) {
            variant.AddDefine("NO_STORAGE_BUFFERS", "1");
            variant.AddFeature("FALLBACK_STORAGE");
        }
        
        if (!context.supportsImageLoadStore) {
            variant.AddDefine("NO_IMAGE_LOAD_STORE", "1");
            variant.AddFeature("FALLBACK_IMAGE_OPS");
        }
        
        if (!context.supportsAtomicOperations) {
            variant.AddDefine("NO_ATOMIC_OPERATIONS", "1");
            variant.AddFeature("FALLBACK_ATOMICS");
        }
        
        // Add performance tier define
        variant.AddDefine("PERFORMANCE_TIER", std::to_string(context.performanceTier));
        variant.AddFeature("PERFORMANCE_TIER_" + std::to_string(context.performanceTier));
        
        return variant;
    }

    void ShaderVariantManager::ClearVariantCache() {
        if (m_debugMode) {
            LOG_INFO("Clearing variant cache (" + std::to_string(m_variants.size()) + " variants)");
        }
        
        m_variants.clear();
        m_variantUsage.clear();
        m_stats = VariantStats{};
    }

    void ShaderVariantManager::OptimizeVariantCache() {
        if (m_variants.size() <= m_maxCacheSize) {
            return;
        }
        
        // Create list of variants sorted by usage (least recently used first)
        std::vector<std::pair<VariantKey, float>> variantsByUsage;
        
        for (const auto& pair : m_variantUsage) {
            float score = pair.second.lastUsedTime + (pair.second.useCount * 10.0f); // Favor frequently used variants
            variantsByUsage.emplace_back(pair.first, score);
        }
        
        std::sort(variantsByUsage.begin(), variantsByUsage.end(), 
                  [](const auto& a, const auto& b) { return a.second < b.second; });
        
        // Remove least used variants until we're under the cache size limit
        size_t variantsToRemove = m_variants.size() - (m_maxCacheSize * 3 / 4); // Remove to 75% of max size
        
        for (size_t i = 0; i < variantsToRemove && i < variantsByUsage.size(); ++i) {
            const VariantKey& key = variantsByUsage[i].first;
            m_variants.erase(key);
            m_variantUsage.erase(key);
        }
        
        if (m_debugMode) {
            LOG_INFO("Optimized variant cache, removed " + std::to_string(variantsToRemove) + " variants");
        }
    }

    void ShaderVariantManager::SetMaxCacheSize(size_t maxSize) {
        m_maxCacheSize = maxSize;
        
        if (m_variants.size() > m_maxCacheSize) {
            OptimizeVariantCache();
        }
    }

    size_t ShaderVariantManager::GetVariantCount() const {
        return m_variants.size();
    }

    size_t ShaderVariantManager::GetVariantCount(const std::string& baseName) const {
        size_t count = 0;
        for (const auto& pair : m_variants) {
            if (pair.first.baseName == baseName) {
                count++;
            }
        }
        return count;
    }

    std::vector<std::string> ShaderVariantManager::GetBaseShaderNames() const {
        std::vector<std::string> names;
        
        for (const auto& pair : m_variants) {
            const std::string& baseName = pair.first.baseName;
            if (std::find(names.begin(), names.end(), baseName) == names.end()) {
                names.push_back(baseName);
            }
        }
        
        return names;
    }

    std::vector<ShaderVariant> ShaderVariantManager::GetVariants(const std::string& baseName) const {
        std::vector<ShaderVariant> variants;
        
        // This is a simplified implementation - in a real system, we'd need to store the original variants
        // For now, we'll return empty variants with just the hash information
        for (const auto& pair : m_variants) {
            if (pair.first.baseName == baseName) {
                ShaderVariant variant;
                variant.name = pair.first.variantHash;
                variants.push_back(variant);
            }
        }
        
        return variants;
    }

    std::vector<VariantKey> ShaderVariantManager::GetAllVariantKeys() const {
        std::vector<VariantKey> keys;
        keys.reserve(m_variants.size());
        
        for (const auto& pair : m_variants) {
            keys.push_back(pair.first);
        }
        
        return keys;
    }

    void ShaderVariantManager::CleanupUnusedVariants() {
        std::vector<VariantKey> keysToRemove;
        
        for (const auto& pair : m_variantUsage) {
            if (m_currentTime - pair.second.lastUsedTime > m_variantLifetime) {
                keysToRemove.push_back(pair.first);
            }
        }
        
        for (const auto& key : keysToRemove) {
            m_variants.erase(key);
            m_variantUsage.erase(key);
        }
        
        if (m_debugMode && !keysToRemove.empty()) {
            LOG_INFO("Cleaned up " + std::to_string(keysToRemove.size()) + " unused variants");
        }
    }

    void ShaderVariantManager::SetVariantLifetime(float lifetimeSeconds) {
        m_variantLifetime = lifetimeSeconds;
    }

    void ShaderVariantManager::MarkVariantUsed(const std::string& baseName, const ShaderVariant& variant) {
        VariantKey key = CreateVariantKey(baseName, variant);
        
        auto it = m_variantUsage.find(key);
        if (it != m_variantUsage.end()) {
            it->second.lastUsedTime = m_currentTime;
            it->second.useCount++;
        }
    }

    VariantStats ShaderVariantManager::GetVariantStats() const {
        return m_stats;
    }

    void ShaderVariantManager::LogVariantInfo(const std::string& baseName) const {
        size_t variantCount = GetVariantCount(baseName);
        LOG_INFO("Shader '" + baseName + "' has " + std::to_string(variantCount) + " variants");
        
        if (m_debugMode) {
            for (const auto& pair : m_variants) {
                if (pair.first.baseName == baseName) {
                    auto usageIt = m_variantUsage.find(pair.first);
                    if (usageIt != m_variantUsage.end()) {
                        const auto& usage = usageIt->second;
                        LOG_INFO("  Variant " + pair.first.variantHash + 
                                ": used " + std::to_string(usage.useCount) + " times, " +
                                "last used " + std::to_string(m_currentTime - usage.lastUsedTime) + "s ago");
                    }
                }
            }
        }
    }

    void ShaderVariantManager::PrecompileCommonVariants(const std::string& baseName) {
        if (m_debugMode) {
            LOG_INFO("Precompiling common variants for: " + baseName);
        }
        
        // Create common variants
        std::vector<ShaderVariant> commonVariants = {
            ShaderVariants::CreateDefault(),
            ShaderVariants::CreateDebug(),
            ShaderVariants::CreateOptimized(),
            ShaderVariants::CreateWithDirectionalLight(),
            ShaderVariants::CreateWithPointLights(4),
            ShaderVariants::CreateWithAlbedoMap(),
            ShaderVariants::CreateWithNormalMap()
        };
        
        for (const auto& variant : commonVariants) {
            CreateVariant(baseName, variant);
        }
    }

    void ShaderVariantManager::PrecompileVariantsFromContext(const std::string& baseName, const std::vector<RenderContext>& contexts) {
        if (m_debugMode) {
            LOG_INFO("Precompiling " + std::to_string(contexts.size()) + " context-based variants for: " + baseName);
        }
        
        for (const auto& context : contexts) {
            ShaderVariant variant = GenerateVariantFromContext(context);
            CreateVariant(baseName, variant);
        }
    }

    // Private methods implementation

    std::shared_ptr<Shader> ShaderVariantManager::CreateVariantInternal(const std::string& baseName, const ShaderVariant& variant) {
        if (!m_shaderManager) {
            LOG_ERROR("ShaderManager not available");
            return nullptr;
        }
        
        // Get base shader from shader manager
        auto baseShader = m_shaderManager->GetShader(baseName);
        if (!baseShader) {
            LOG_ERROR("Base shader not found: " + baseName);
            return nullptr;
        }
        
        // For now, create a copy of the base shader
        // In a full implementation, we would recompile the shader with variant defines
        auto variantShader = std::make_shared<Shader>();
        
        // This is a simplified implementation - in reality, we would:
        // 1. Get the original shader source files
        // 2. Inject the variant defines at the top
        // 3. Recompile the shader
        // For now, we'll just return a copy of the base shader
        
        if (m_debugMode) {
            LOG_INFO("Created variant shader (simplified implementation)");
        }
        
        return baseShader; // Temporary - return base shader as placeholder
    }

    bool ShaderVariantManager::ValidateVariant(const ShaderVariant& variant) const {
        // Basic validation - check for empty or invalid defines
        for (const auto& define : variant.defines) {
            if (define.first.empty()) {
                LOG_ERROR("Shader variant contains empty define name");
                return false;
            }
        }
        
        // Check for conflicting features (basic implementation)
        if (variant.HasFeature("DEBUG_OUTPUT") && variant.HasFeature("PERFORMANCE_MODE")) {
            LOG_WARNING("Shader variant has potentially conflicting features: DEBUG_OUTPUT and PERFORMANCE_MODE");
        }
        
        return true;
    }

    VariantKey ShaderVariantManager::CreateVariantKey(const std::string& baseName, const ShaderVariant& variant) const {
        return VariantKey{
            .baseName = baseName,
            .variantHash = variant.GenerateHash()
        };
    }

    std::string ShaderVariantManager::InjectVariantDefines(const std::string& shaderSource, const ShaderVariant& variant) const {
        std::stringstream result;
        
        // Add version directive first (if present)
        std::istringstream sourceStream(shaderSource);
        std::string line;
        bool versionFound = false;
        
        while (std::getline(sourceStream, line)) {
            if (line.find("#version") == 0) {
                result << line << "\n";
                versionFound = true;
                break;
            }
        }
        
        // Inject variant defines after version
        result << variant.GeneratePreprocessorString();
        
        // Add the rest of the shader source
        if (versionFound) {
            while (std::getline(sourceStream, line)) {
                result << line << "\n";
            }
        } else {
            result << shaderSource;
        }
        
        return result.str();
    }

    bool ShaderVariantManager::CompileShaderWithVariant(std::shared_ptr<Shader> shader, const std::string& baseName, const ShaderVariant& variant) {
        // This would be implemented in a full system to recompile shaders with variant defines
        // For now, return true as placeholder
        return true;
    }

    void ShaderVariantManager::UpdateVariantStats() {
        m_stats.totalVariants = m_variants.size();
        m_stats.activeVariants = 0;
        m_stats.memoryUsage = 0;
        
        float totalCreationTime = 0.0f;
        size_t creationTimeCount = 0;
        
        for (const auto& pair : m_variantUsage) {
            const auto& usage = pair.second;
            
            // Consider variant active if used in the last 60 seconds
            if (m_currentTime - usage.lastUsedTime < 60.0f) {
                m_stats.activeVariants++;
            }
            
            // Rough estimate of memory usage per variant
            m_stats.memoryUsage += 2048; // 2KB per variant (placeholder)
            
            if (usage.creationTime > 0.0f) {
                totalCreationTime += usage.creationTime;
                creationTimeCount++;
            }
        }
        
        if (creationTimeCount > 0) {
            m_stats.averageCreationTime = totalCreationTime / creationTimeCount;
        }
    }

    void ShaderVariantManager::CleanupOldVariants() {
        CleanupUnusedVariants();
    }

    bool ShaderVariantManager::ShouldEvictVariant(const VariantKey& key) const {
        auto it = m_variantUsage.find(key);
        if (it == m_variantUsage.end()) {
            return true; // No usage info, evict
        }
        
        const auto& usage = it->second;
        
        // Evict if not used recently and has low use count
        bool oldAndUnused = (m_currentTime - usage.lastUsedTime > m_variantLifetime) && (usage.useCount < 5);
        
        return oldAndUnused;
    }

    RenderContext ShaderVariantManager::CreateHardwareAwareContext(const RenderContext& baseContext) {
        RenderContext context = baseContext;
        PopulateHardwareCapabilities(context);
        return context;
    }

    void ShaderVariantManager::PopulateHardwareCapabilities(RenderContext& context) {
        if (!HardwareCapabilities::IsInitialized()) {
            LOG_WARNING("Hardware capabilities not initialized, using default values");
            return;
        }
        
        const auto& capabilities = HardwareCapabilities::GetInstance();
        
        // Update hardware capability flags
        context.supportsGeometryShaders = capabilities.SupportsGeometryShaders();
        context.supportsTessellation = capabilities.SupportsTessellation();
        context.supportsComputeShaders = capabilities.SupportsComputeShaders();
        context.supportsStorageBuffers = capabilities.SupportsStorageBuffers();
        context.supportsImageLoadStore = capabilities.SupportsImageLoadStore();
        context.supportsAtomicOperations = capabilities.SupportsAtomicOperations();
        context.performanceTier = capabilities.GetPerformanceTier();
        
        // Adjust limits based on hardware capabilities
        const auto& shaderLimits = capabilities.GetShaderLimits();
        
        // Limit texture units based on hardware
        int maxSafeTextureUnits = std::min(32, shaderLimits.maxCombinedTextureUnits / 2);
        
        // Adjust bone count for skinning based on uniform limits
        if (context.hasSkinning) {
            int maxSafeBones = std::min(context.maxBones, shaderLimits.maxVertexUniforms / 16); // Rough estimate
            context.maxBones = maxSafeBones;
        }
        
        // Adjust light counts based on performance tier
        switch (context.performanceTier) {
            case 0: // Low performance
                context.maxPointLights = std::min(context.maxPointLights, 2);
                context.maxSpotLights = std::min(context.maxSpotLights, 1);
                break;
            case 1: // Medium performance
                context.maxPointLights = std::min(context.maxPointLights, 4);
                context.maxSpotLights = std::min(context.maxSpotLights, 2);
                break;
            case 2: // High performance
                context.maxPointLights = std::min(context.maxPointLights, 8);
                context.maxSpotLights = std::min(context.maxSpotLights, 4);
                break;
            case 3: // Ultra performance
                // No additional limits
                break;
        }
        
        if (m_debugMode) {
            LOG_INFO("Hardware-aware context created:");
            LOG_INFO("  Performance Tier: " + std::to_string(context.performanceTier));
            LOG_INFO("  Compute Shaders: " + std::string(context.supportsComputeShaders ? "Yes" : "No"));
            LOG_INFO("  Geometry Shaders: " + std::string(context.supportsGeometryShaders ? "Yes" : "No"));
            LOG_INFO("  Max Point Lights: " + std::to_string(context.maxPointLights));
            LOG_INFO("  Max Bones: " + std::to_string(context.maxBones));
        }
    }
}