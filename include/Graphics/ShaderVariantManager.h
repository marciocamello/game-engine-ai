#pragma once

#include "Graphics/ShaderVariant.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include <vector>
#include <string>

namespace GameEngine {
    class Shader;
    class ShaderManager;

    struct RenderContext {
        // Lighting information
        bool hasDirectionalLight = false;
        int pointLightCount = 0;
        int spotLightCount = 0;
        bool hasShadows = false;
        
        // Material information
        bool hasAlbedoMap = false;
        bool hasNormalMap = false;
        bool hasMetallicRoughnessMap = false;
        bool hasEmissionMap = false;
        bool hasAOMap = false;
        
        // Rendering features
        bool hasSkinning = false;
        bool hasInstancing = false;
        bool useDebugMode = false;
        bool useOptimizedPath = true;
        
        // Hardware capabilities (will be auto-detected)
        bool supportsGeometryShaders = true;
        bool supportsTessellation = true;
        bool supportsComputeShaders = true;
        bool supportsStorageBuffers = true;
        bool supportsImageLoadStore = true;
        bool supportsAtomicOperations = true;
        int performanceTier = 2; // 0=low, 1=medium, 2=high, 3=ultra
        
        // Performance settings
        int maxBones = 64;
        int maxPointLights = 8;
        int maxSpotLights = 4;
    };

    struct VariantKey {
        std::string baseName;
        std::string variantHash;

        bool operator==(const VariantKey& other) const {
            return baseName == other.baseName && variantHash == other.variantHash;
        }
    };

    struct VariantKeyHash {
        size_t operator()(const VariantKey& key) const {
            std::hash<std::string> hasher;
            return hasher(key.baseName + "_" + key.variantHash);
        }
    };

    struct VariantStats {
        size_t totalVariants = 0;
        size_t activeVariants = 0;
        size_t memoryUsage = 0;
        size_t cacheHits = 0;
        size_t cacheMisses = 0;
        float averageCreationTime = 0.0f;
    };

    class ShaderVariantManager {
    public:
        // Singleton access
        static ShaderVariantManager& GetInstance();
        
        // Lifecycle
        bool Initialize();
        void Shutdown();
        void Update(float deltaTime);

        // Variant creation and management
        std::shared_ptr<Shader> CreateVariant(const std::string& baseName, const ShaderVariant& variant);
        std::shared_ptr<Shader> GetVariant(const std::string& baseName, const ShaderVariant& variant);
        std::shared_ptr<Shader> GetOrCreateVariant(const std::string& baseName, const ShaderVariant& variant);
        void RemoveVariant(const std::string& baseName, const ShaderVariant& variant);
        void RemoveAllVariants(const std::string& baseName);

        // Variant selection
        std::shared_ptr<Shader> SelectBestVariant(const std::string& baseName, const RenderContext& context);
        void SetVariantSelectionCallback(std::function<ShaderVariant(const RenderContext&)> callback);
        ShaderVariant GenerateVariantFromContext(const RenderContext& context);
        
        // Hardware capability integration
        RenderContext CreateHardwareAwareContext(const RenderContext& baseContext);
        void PopulateHardwareCapabilities(RenderContext& context);

        // Cache management
        void ClearVariantCache();
        void OptimizeVariantCache();
        void SetMaxCacheSize(size_t maxSize);
        size_t GetVariantCount() const;
        size_t GetVariantCount(const std::string& baseName) const;

        // Variant enumeration
        std::vector<std::string> GetBaseShaderNames() const;
        std::vector<ShaderVariant> GetVariants(const std::string& baseName) const;
        std::vector<VariantKey> GetAllVariantKeys() const;

        // Memory management and cleanup
        void CleanupUnusedVariants();
        void SetVariantLifetime(float lifetimeSeconds);
        void MarkVariantUsed(const std::string& baseName, const ShaderVariant& variant);

        // Performance and debugging
        VariantStats GetVariantStats() const;
        void SetDebugMode(bool enabled) { m_debugMode = enabled; }
        bool IsDebugMode() const { return m_debugMode; }
        void LogVariantInfo(const std::string& baseName) const;

        // Precompilation and optimization
        void PrecompileCommonVariants(const std::string& baseName);
        void PrecompileVariantsFromContext(const std::string& baseName, const std::vector<RenderContext>& contexts);
        void SetAsyncCompilation(bool enabled) { m_asyncCompilation = enabled; }

    private:
        ShaderVariantManager() = default;
        ~ShaderVariantManager() = default;
        ShaderVariantManager(const ShaderVariantManager&) = delete;
        ShaderVariantManager& operator=(const ShaderVariantManager&) = delete;

        // Internal variant management
        std::shared_ptr<Shader> CreateVariantInternal(const std::string& baseName, const ShaderVariant& variant);
        bool ValidateVariant(const ShaderVariant& variant) const;
        VariantKey CreateVariantKey(const std::string& baseName, const ShaderVariant& variant) const;
        
        // Shader compilation with variants
        std::string InjectVariantDefines(const std::string& shaderSource, const ShaderVariant& variant) const;
        bool CompileShaderWithVariant(std::shared_ptr<Shader> shader, const std::string& baseName, const ShaderVariant& variant);
        
        // Cache management internals
        void UpdateVariantStats();
        void CleanupOldVariants();
        bool ShouldEvictVariant(const VariantKey& key) const;
        
        // Variant usage tracking
        struct VariantUsageInfo {
            float lastUsedTime = 0.0f;
            size_t useCount = 0;
            float creationTime = 0.0f;
        };

        // Member variables
        std::unordered_map<VariantKey, std::shared_ptr<Shader>, VariantKeyHash> m_variants;
        std::unordered_map<VariantKey, VariantUsageInfo, VariantKeyHash> m_variantUsage;
        std::function<ShaderVariant(const RenderContext&)> m_selectionCallback;

        bool m_initialized = false;
        bool m_debugMode = false;
        bool m_asyncCompilation = false;
        
        size_t m_maxCacheSize = 1000; // Maximum number of variants to cache
        float m_variantLifetime = 300.0f; // 5 minutes default lifetime for unused variants
        float m_currentTime = 0.0f;
        
        VariantStats m_stats;
        ShaderManager* m_shaderManager = nullptr; // Reference to shader manager for base shaders
    };
}