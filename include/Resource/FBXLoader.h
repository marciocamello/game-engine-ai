#pragma once

#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/Skeleton.h"
#include "Graphics/Animation.h"
#include "Resource/ResourceManager.h"
#include "Core/Math.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#ifdef GAMEENGINE_HAS_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

namespace GameEngine {

    /**
     * @brief FBXLoader provides specialized FBX model loading capabilities
     * 
     * Handles FBX-specific features including:
     * - Proper coordinate system conversion (Maya/3ds Max to OpenGL)
     * - Material import with texture mapping
     * - Skeletal animation support
     * - Mesh optimization for FBX models
     */
    class FBXLoader {
    public:
        /**
         * @brief Result of FBX loading operation
         */
        struct FBXLoadResult {
            std::vector<std::shared_ptr<Mesh>> meshes;
            std::vector<std::shared_ptr<Material>> materials;
            std::shared_ptr<Skeleton> skeleton;
            std::vector<std::shared_ptr<Animation>> animations;
            bool success = false;
            std::string errorMessage;
            
            // Statistics
            uint32_t totalVertices = 0;
            uint32_t totalTriangles = 0;
            uint32_t materialCount = 0;
            uint32_t boneCount = 0;
            uint32_t animationCount = 0;
            float loadingTimeMs = 0.0f;
            std::string sourceApplication; // Maya, 3ds Max, Blender, etc.
            bool hasSkeleton = false;
            bool hasAnimations = false;
        };

        /**
         * @brief FBX-specific loading configuration
         */
        struct FBXLoadingConfig {
            bool convertToOpenGLCoordinates = true;  // Convert from Maya/Max coordinates
            bool importMaterials = true;             // Import material definitions
            bool importTextures = true;              // Load referenced textures
            bool importSkeleton = true;              // Import skeletal data
            bool importAnimations = true;            // Import animation data
            bool optimizeMeshes = true;              // Apply mesh optimization
            bool generateMissingNormals = true;      // Generate normals if missing
            bool generateTangents = true;            // Generate tangent vectors
            float importScale = 1.0f;                // Scale factor for import
            std::vector<std::string> textureSearchPaths; // Additional texture search paths
        };

    public:
        FBXLoader();
        ~FBXLoader();

        // Lifecycle
        bool Initialize();
        void Shutdown();
        bool IsInitialized() const { return m_initialized; }

        // Main loading interface
        FBXLoadResult LoadFBX(const std::string& filepath);
        FBXLoadResult LoadFBXFromMemory(const std::vector<uint8_t>& data);

        // Configuration
        void SetLoadingConfig(const FBXLoadingConfig& config);
        FBXLoadingConfig GetLoadingConfig() const { return m_config; }

        // Utility methods
        static bool IsFBXFile(const std::string& filepath);
        static std::string DetectSourceApplication(const std::string& filepath);

    private:
#ifdef GAMEENGINE_HAS_ASSIMP
        std::unique_ptr<Assimp::Importer> m_importer;
        
        // Internal processing methods
        FBXLoadResult ProcessFBXScene(const aiScene* scene, const std::string& filepath);
        std::shared_ptr<Mesh> ProcessFBXMesh(const aiMesh* mesh, const aiScene* scene);
        std::vector<std::shared_ptr<Material>> ProcessFBXMaterials(const aiScene* scene, const std::string& filepath);
        std::shared_ptr<Material> ProcessFBXMaterial(const aiMaterial* aiMat, const std::string& filepath);
        
        // Animation and rigging processing
        std::shared_ptr<Skeleton> ProcessFBXSkeleton(const aiScene* scene);
        std::vector<std::shared_ptr<Animation>> ProcessFBXAnimations(const aiScene* scene);
        std::shared_ptr<Animation> ProcessFBXAnimation(const aiAnimation* aiAnim);
        void ProcessBoneWeights(const aiMesh* mesh, const aiScene* scene, std::vector<Vertex>& vertices);
        void ExtractBoneData(const aiMesh* mesh, std::shared_ptr<Skeleton> skeleton);
        
        // FBX-specific processing
        void ApplyCoordinateSystemConversion(std::vector<Vertex>& vertices) const;
        Math::Mat4 GetFBXToOpenGLTransform() const;
        void ProcessFBXNode(const aiNode* node, const aiScene* scene, std::vector<std::shared_ptr<Mesh>>& meshes);
        
        // Material processing
        std::shared_ptr<Texture> LoadFBXTexture(const std::string& texturePath, const std::string& modelPath);
        std::string FindTexturePath(const std::string& texturePath, const std::string& modelPath) const;
        Math::Vec3 ConvertFBXColor(const aiColor3D& color) const;
        
        // Conversion utilities
        Math::Vec3 ConvertVector3(const aiVector3D& vec) const;
        Math::Vec2 ConvertVector2(const aiVector3D& vec) const;
        
        // Post-processing configuration
        uint32_t GetFBXPostProcessFlags() const;
        void ValidateFBXScene(const aiScene* scene) const;
        std::string DetectSourceApplicationFromScene(const aiScene* scene) const;
#endif

        // Configuration
        FBXLoadingConfig m_config;
        bool m_initialized = false;
        
        // Texture cache for shared textures
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;
        
        // Helper methods
        void LogFBXLoadingStats(const FBXLoadResult& result) const;
        void ClearTextureCache();
    };

} // namespace GameEngine