#pragma once

#include "Graphics/Mesh.h"
#include "Resource/ResourceManager.h"
#include "Core/Math.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_set>

#ifdef GAMEENGINE_HAS_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

// Forward declarations
namespace GameEngine {
    class GLTFLoader;
}

namespace GameEngine {

    /**
     * @brief ModelLoader provides comprehensive 3D model loading capabilities using Assimp
     * 
     * Supports industry-standard formats including:
     * - GLTF 2.0 (.gltf, .glb)
     * - FBX (.fbx)
     * - Wavefront OBJ (.obj)
     * - Collada DAE (.dae)
     * - 3DS Max (.3ds)
     * - And many more formats supported by Assimp
     */
    class ModelLoader {
    public:
        /**
         * @brief Configuration flags for model loading
         */
        enum class LoadingFlags : uint32_t {
            None = 0,
            FlipUVs = 1 << 0,
            GenerateNormals = 1 << 1,
            GenerateTangents = 1 << 2,
            OptimizeMeshes = 1 << 3,
            OptimizeGraph = 1 << 4,
            FlipWindingOrder = 1 << 5,
            MakeLeftHanded = 1 << 6,
            RemoveDuplicateVertices = 1 << 7,
            SortByPrimitiveType = 1 << 8,
            ImproveCacheLocality = 1 << 9,
            LimitBoneWeights = 1 << 10,
            SplitLargeMeshes = 1 << 11,
            Triangulate = 1 << 12,
            ValidateDataStructure = 1 << 13,
            PreTransformVertices = 1 << 14,
            FixInfacingNormals = 1 << 15
        };

        /**
         * @brief Result of model loading operation
         */
        struct LoadResult {
            std::vector<std::shared_ptr<Mesh>> meshes;
            bool success = false;
            std::string errorMessage;
            
            // Statistics
            uint32_t totalVertices = 0;
            uint32_t totalTriangles = 0;
            float loadingTimeMs = 0.0f;
            std::string formatUsed;
        };

        /**
         * @brief Information about supported file formats
         */
        struct FormatInfo {
            std::string extension;
            std::string description;
            bool canRead = false;
            bool canWrite = false;
        };

    public:
        ModelLoader();
        ~ModelLoader();

        // Lifecycle
        bool Initialize();
        void Shutdown();
        bool IsInitialized() const { return m_initialized; }

        // Main loading interface
        LoadResult LoadModel(const std::string& filepath);
        LoadResult LoadModelFromMemory(const std::vector<uint8_t>& data, const std::string& format);

        // Format support queries
        bool IsFormatSupported(const std::string& extension) const;
        std::vector<std::string> GetSupportedExtensions() const;
        std::vector<FormatInfo> GetSupportedFormats() const;
        std::string DetectFormat(const std::string& filepath) const;

        // Configuration
        void SetLoadingFlags(LoadingFlags flags);
        LoadingFlags GetLoadingFlags() const { return m_loadingFlags; }
        void SetImportScale(float scale);
        float GetImportScale() const { return m_importScale; }

        // Utility methods
        static bool IsModelFile(const std::string& filepath);
        static std::string GetFileExtension(const std::string& filepath);

    private:
#ifdef GAMEENGINE_HAS_ASSIMP
        std::unique_ptr<Assimp::Importer> m_importer;
        
        // Internal processing methods
        LoadResult ProcessScene(const aiScene* scene, const std::string& filepath);
        std::shared_ptr<Mesh> ProcessMesh(const aiMesh* mesh, const aiScene* scene);
        void ProcessNode(const aiNode* node, const aiScene* scene, std::vector<std::shared_ptr<Mesh>>& meshes);
        
        // Conversion utilities
        Math::Vec3 ConvertVector3(const aiVector3D& vec) const;
        Math::Vec2 ConvertVector2(const aiVector3D& vec) const;
        
        // Post-processing configuration
        uint32_t GetAssimpPostProcessFlags() const;
        void ValidateScene(const aiScene* scene) const;
#endif

        // GLTF loader
        std::unique_ptr<GLTFLoader> m_gltfLoader;

        // Configuration
        LoadingFlags m_loadingFlags = LoadingFlags::None;
        float m_importScale = 1.0f;
        bool m_initialized = false;

        // Supported formats cache
        mutable std::unordered_set<std::string> m_supportedExtensions;
        mutable bool m_extensionsCached = false;
        
        // Helper methods
        void CacheSupportedExtensions() const;
        std::string NormalizeExtension(const std::string& extension) const;
        void LogLoadingStats(const LoadResult& result) const;
    };

    // Bitwise operators for LoadingFlags
    inline ModelLoader::LoadingFlags operator|(ModelLoader::LoadingFlags a, ModelLoader::LoadingFlags b) {
        return static_cast<ModelLoader::LoadingFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline ModelLoader::LoadingFlags operator&(ModelLoader::LoadingFlags a, ModelLoader::LoadingFlags b) {
        return static_cast<ModelLoader::LoadingFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    inline ModelLoader::LoadingFlags operator^(ModelLoader::LoadingFlags a, ModelLoader::LoadingFlags b) {
        return static_cast<ModelLoader::LoadingFlags>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
    }

    inline ModelLoader::LoadingFlags operator~(ModelLoader::LoadingFlags a) {
        return static_cast<ModelLoader::LoadingFlags>(~static_cast<uint32_t>(a));
    }

    inline ModelLoader::LoadingFlags& operator|=(ModelLoader::LoadingFlags& a, ModelLoader::LoadingFlags b) {
        return a = a | b;
    }

    inline ModelLoader::LoadingFlags& operator&=(ModelLoader::LoadingFlags& a, ModelLoader::LoadingFlags b) {
        return a = a & b;
    }

    inline ModelLoader::LoadingFlags& operator^=(ModelLoader::LoadingFlags& a, ModelLoader::LoadingFlags b) {
        return a = a ^ b;
    }

    // Helper function to check if a flag is set
    inline bool HasFlag(ModelLoader::LoadingFlags flags, ModelLoader::LoadingFlags flag) {
        return (flags & flag) == flag;
    }
}