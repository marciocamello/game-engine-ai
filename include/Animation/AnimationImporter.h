#pragma once

#include "Animation/AnimationSkeleton.h"
#include "Animation/SkeletalAnimation.h"
#include "Core/Math.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#ifdef GAMEENGINE_HAS_ASSIMP
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#endif

namespace GameEngine {
namespace Animation {

    /**
     * Result of animation import operation
     */
    struct AnimationImportResult {
        std::shared_ptr<AnimationSkeleton> skeleton;
        std::vector<std::shared_ptr<SkeletalAnimation>> animations;
        bool success = false;
        std::string errorMessage;
        
        // Statistics
        size_t boneCount = 0;
        size_t animationCount = 0;
        float totalDuration = 0.0f;
        std::string sourceFormat;
    };

    /**
     * Configuration for animation import
     */
    struct AnimationImportConfig {
        // Coordinate system conversion
        bool convertCoordinateSystem = true;
        Math::Vec3 coordinateSystemScale = Math::Vec3(1.0f, 1.0f, 1.0f);
        bool flipYZ = false;
        
        // Animation processing
        bool optimizeKeyframes = true;
        float keyframeOptimizationTolerance = 0.001f;
        bool removeRedundantTracks = true;
        
        // Skeleton processing
        bool validateBoneHierarchy = true;
        bool generateMissingBindPoses = true;
        
        // Import filtering
        std::vector<std::string> animationNameFilter; // Empty = import all
        std::vector<std::string> boneNameFilter;      // Empty = import all
        
        // Metadata preservation
        bool preserveAnimationMetadata = true;
        bool preserveCustomProperties = true;
    };

    /**
     * Handles importing animations and skeletons from 3D model files
     */
    class AnimationImporter {
    public:
        AnimationImporter();
        ~AnimationImporter();

        // Main import interface
        AnimationImportResult ImportFromFile(const std::string& filepath, 
                                           const AnimationImportConfig& config = AnimationImportConfig{});
        
        AnimationImportResult ImportFromScene(const void* scene, 
                                            const std::string& sourceFormat,
                                            const AnimationImportConfig& config = AnimationImportConfig{});

        // Skeleton-only import
        std::shared_ptr<AnimationSkeleton> ImportSkeletonFromFile(const std::string& filepath,
                                                                const AnimationImportConfig& config = AnimationImportConfig{});

        // Animation-only import (requires existing skeleton)
        std::vector<std::shared_ptr<SkeletalAnimation>> ImportAnimationsFromFile(
            const std::string& filepath,
            std::shared_ptr<AnimationSkeleton> targetSkeleton,
            const AnimationImportConfig& config = AnimationImportConfig{});

        // Validation and error checking
        bool ValidateAnimationData(const std::string& filepath) const;
        std::vector<std::string> GetAnimationNames(const std::string& filepath) const;
        std::vector<std::string> GetBoneNames(const std::string& filepath) const;

        // Configuration
        void SetDefaultConfig(const AnimationImportConfig& config) { m_defaultConfig = config; }
        const AnimationImportConfig& GetDefaultConfig() const { return m_defaultConfig; }

        // Format support
        bool IsAnimationFormatSupported(const std::string& extension) const;
        std::vector<std::string> GetSupportedAnimationFormats() const;

    private:
#ifdef GAMEENGINE_HAS_ASSIMP
        // Assimp-based import methods
        AnimationImportResult ImportFromAssimpScene(const aiScene* scene, 
                                                   const std::string& sourceFormat,
                                                   const AnimationImportConfig& config);
        
        std::shared_ptr<AnimationSkeleton> ImportSkeletonFromAssimp(const aiScene* scene,
                                                                  const AnimationImportConfig& config);
        
        std::vector<std::shared_ptr<SkeletalAnimation>> ImportAnimationsFromAssimp(
            const aiScene* scene,
            std::shared_ptr<AnimationSkeleton> skeleton,
            const AnimationImportConfig& config);

        // Skeleton processing
        void ProcessAssimpNode(const aiNode* node, 
                             std::shared_ptr<AnimationSkeleton> skeleton,
                             std::shared_ptr<Bone> parentBone,
                             const AnimationImportConfig& config);
        
        std::shared_ptr<Bone> CreateBoneFromAssimpNode(const aiNode* node,
                                                     const AnimationImportConfig& config);

        // Animation processing
        std::shared_ptr<SkeletalAnimation> ProcessAssimpAnimation(const aiAnimation* animation,
                                                                std::shared_ptr<AnimationSkeleton> skeleton,
                                                                const AnimationImportConfig& config);
        
        void ProcessAssimpNodeAnimation(const aiNodeAnim* nodeAnim,
                                      std::shared_ptr<SkeletalAnimation> animation,
                                      const AnimationImportConfig& config);

        // Coordinate system conversion
        Math::Mat4 ConvertAssimpMatrix(const aiMatrix4x4& matrix, const AnimationImportConfig& config) const;
        Math::Vec3 ConvertAssimpVector3(const aiVector3D& vector, const AnimationImportConfig& config) const;
        Math::Quat ConvertAssimpQuaternion(const aiQuaternion& quat, const AnimationImportConfig& config) const;
        
        // Bone mapping and validation
        void BuildBoneMapping(const aiScene* scene, std::shared_ptr<AnimationSkeleton> skeleton);
        bool ValidateSkeletonConsistency(std::shared_ptr<AnimationSkeleton> skeleton) const;
        
        // Animation track mapping
        void MapAnimationTracks(const aiAnimation* assimpAnim,
                              std::shared_ptr<SkeletalAnimation> animation,
                              std::shared_ptr<AnimationSkeleton> skeleton,
                              const AnimationImportConfig& config);

        std::unordered_map<std::string, std::string> m_boneNameMapping; // Assimp name -> Engine name
#endif

        // Configuration
        AnimationImportConfig m_defaultConfig;
        
        // Helper methods
        void ApplyCoordinateSystemConversion(AnimationImportResult& result, 
                                           const AnimationImportConfig& config);
        
        void OptimizeImportedAnimations(std::vector<std::shared_ptr<SkeletalAnimation>>& animations,
                                      const AnimationImportConfig& config);
        
        void ValidateImportedData(AnimationImportResult& result) const;
        void ApplyErrorCorrection(AnimationImportResult& result, const AnimationImportConfig& config);
        
        std::string NormalizeExtension(const std::string& extension) const;
        void LogImportStatistics(const AnimationImportResult& result) const;
    };

} // namespace Animation
} // namespace GameEngine