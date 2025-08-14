#pragma once

#include "Graphics/Model.h"
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/GraphicsAnimation.h"
#include "Graphics/RenderSkeleton.h"
#include "Animation/MorphTarget.h"
#include "Core/Math.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace GameEngine {

    /**
     * @brief GLTF 2.0 loader implementation
     * 
     * Supports both .gltf (JSON) and .glb (binary) formats with:
     * - Scene graph parsing with hierarchical nodes
     * - Mesh extraction with vertex attributes
     * - PBR material import with metallic-roughness workflow
     * - Embedded and external texture loading
     */
    class GLTFLoader {
    public:
        /**
         * @brief Result of GLTF loading operation
         */
        struct LoadResult {
            std::shared_ptr<Model> model;
            bool success = false;
            std::string errorMessage;
            
            // Statistics
            uint32_t nodeCount = 0;
            uint32_t meshCount = 0;
            uint32_t materialCount = 0;
            uint32_t textureCount = 0;
            uint32_t totalVertices = 0;
            uint32_t totalTriangles = 0;
            float loadingTimeMs = 0.0f;
        };

        /**
         * @brief GLTF accessor information for buffer data
         */
        struct AccessorInfo {
            uint32_t bufferView = 0;
            uint32_t byteOffset = 0;
            uint32_t componentType = 0;
            uint32_t count = 0;
            std::string type;
            bool normalized = false;
        };

        /**
         * @brief GLTF buffer view information
         */
        struct BufferViewInfo {
            uint32_t buffer = 0;
            uint32_t byteOffset = 0;
            uint32_t byteLength = 0;
            uint32_t byteStride = 0;
            uint32_t target = 0; // GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER
        };

        /**
         * @brief GLTF buffer information
         */
        struct BufferInfo {
            std::vector<uint8_t> data;
            uint32_t byteLength = 0;
            std::string uri;
        };

    public:
        GLTFLoader();
        ~GLTFLoader();

        // Main loading interface
        LoadResult LoadGLTF(const std::string& filepath);
        LoadResult LoadGLTFFromMemory(const std::vector<uint8_t>& data, const std::string& baseDir = "");

        // Format detection
        static bool IsGLTFFile(const std::string& filepath);
        static bool IsGLBFile(const std::string& filepath);

    private:
        // JSON document and base directory for relative paths
        nlohmann::json m_gltfJson;
        std::string m_baseDirectory;
        
        // Parsed GLTF data
        std::vector<BufferInfo> m_buffers;
        std::vector<BufferViewInfo> m_bufferViews;
        std::vector<AccessorInfo> m_accessors;
        std::vector<std::shared_ptr<Material>> m_materials;
        std::vector<std::shared_ptr<Mesh>> m_meshes;
        std::vector<std::shared_ptr<Animation>> m_animations;
        std::vector<std::shared_ptr<Skeleton>> m_skeletons;
        std::vector<std::shared_ptr<Skin>> m_skins;
        
        // Loading methods
        bool LoadGLTFJson(const std::string& filepath);
        bool LoadGLBBinary(const std::string& filepath);
        bool ParseGLTFJson(const nlohmann::json& json);
        
        // GLB binary format parsing
        bool ParseGLBHeader(const std::vector<uint8_t>& data, uint32_t& jsonLength, uint32_t& binaryLength);
        bool ExtractGLBChunks(const std::vector<uint8_t>& data, std::string& jsonChunk, std::vector<uint8_t>& binaryChunk);
        
        // GLTF component parsing
        bool ParseBuffers();
        bool ParseBufferViews();
        bool ParseAccessors();
        bool ParseMaterials();
        bool ParseMeshes();
        bool ParseAnimations();
        bool ParseSkins();
        std::shared_ptr<Model> ParseScene(uint32_t sceneIndex = 0);
        
        // Node parsing
        std::shared_ptr<ModelNode> ParseNode(const nlohmann::json& nodeJson, uint32_t nodeIndex);
        void ProcessNodeHierarchy(const nlohmann::json& nodeJson, std::shared_ptr<ModelNode> node);
        
        // Mesh parsing
        std::shared_ptr<Mesh> ParseMesh(const nlohmann::json& meshJson, uint32_t meshIndex);
        bool ParseMeshPrimitive(const nlohmann::json& primitiveJson, std::shared_ptr<Mesh> mesh);
        
        // Material parsing
        std::shared_ptr<Material> ParseMaterial(const nlohmann::json& materialJson, uint32_t materialIndex);
        void ParsePBRMetallicRoughness(const nlohmann::json& pbrJson, std::shared_ptr<Material> material);
        
        // Animation parsing
        std::shared_ptr<Graphics::GraphicsAnimation> ParseAnimation(const nlohmann::json& animationJson, uint32_t animationIndex);
        std::shared_ptr<Graphics::AnimationChannel> ParseAnimationChannel(const nlohmann::json& channelJson);
        template<typename T>
        std::shared_ptr<Graphics::AnimationSampler<T>> ParseAnimationSampler(const nlohmann::json& samplerJson);
        Graphics::InterpolationType ParseInterpolationType(const std::string& interpolation);
        
        // Skeleton and skin parsing
        std::shared_ptr<Graphics::RenderSkin> ParseSkin(const nlohmann::json& skinJson, uint32_t skinIndex);
        std::shared_ptr<Skeleton> CreateSkeletonFromSkin(const nlohmann::json& skinJson);
        
        // Morph target parsing
        std::shared_ptr<MorphTargetSet> ParseMorphTargets(const nlohmann::json& targetsJson);
        
        // Accessor data extraction
        template<typename T>
        std::vector<T> GetAccessorData(uint32_t accessorIndex);
        
        std::vector<Math::Vec3> GetVec3AccessorData(uint32_t accessorIndex);
        std::vector<Math::Vec2> GetVec2AccessorData(uint32_t accessorIndex);
        std::vector<uint32_t> GetScalarAccessorData(uint32_t accessorIndex);
        
        // Buffer operations
        bool LoadExternalBuffer(const std::string& uri, std::vector<uint8_t>& data);
        bool IsDataURI(const std::string& uri);
        bool DecodeDataURI(const std::string& uri, std::vector<uint8_t>& data);
        
        // Utility methods
        Math::Mat4 ParseMatrix(const nlohmann::json& matrixJson);
        Math::Vec3 ParseVec3(const nlohmann::json& vecJson, const Math::Vec3& defaultValue = Math::Vec3(0.0f));
        Math::Vec4 ParseVec4(const nlohmann::json& vecJson, const Math::Vec4& defaultValue = Math::Vec4(0.0f));
        
        // Component type utilities
        uint32_t GetComponentSize(uint32_t componentType);
        uint32_t GetTypeComponentCount(const std::string& type);
        
        // Error handling
        void LogError(const std::string& message);
        void LogWarning(const std::string& message);
        void LogInfo(const std::string& message);
        
        // Validation
        bool ValidateGLTF();
        bool ValidateAccessor(uint32_t accessorIndex);
        bool ValidateBufferView(uint32_t bufferViewIndex);
        bool ValidateBuffer(uint32_t bufferIndex);
    };

} // namespace GameEngine