#include "Animation/AnimationImporter.h"
#include "Animation/AnimationValidator.h"
#include "Core/Logger.h"
#include <filesystem>
#include <algorithm>
#include <sstream>

#ifdef GAMEENGINE_HAS_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

namespace GameEngine {
namespace Animation {

AnimationImporter::AnimationImporter() {
    // Set default configuration
    m_defaultConfig.convertCoordinateSystem = true;
    m_defaultConfig.optimizeKeyframes = true;
    m_defaultConfig.keyframeOptimizationTolerance = 0.001f;
    m_defaultConfig.removeRedundantTracks = true;
    m_defaultConfig.validateBoneHierarchy = true;
    m_defaultConfig.generateMissingBindPoses = true;
    m_defaultConfig.preserveAnimationMetadata = true;
    m_defaultConfig.preserveCustomProperties = true;
}

AnimationImporter::~AnimationImporter() = default;

AnimationImportResult AnimationImporter::ImportFromFile(const std::string& filepath, 
                                                       const AnimationImportConfig& config) {
    AnimationImportResult result;
    
    try {
        // Validate file existence
        if (!std::filesystem::exists(filepath)) {
            result.errorMessage = "File does not exist: " + filepath;
            LOG_ERROR("AnimationImporter::ImportFromFile: " + result.errorMessage);
            return result;
        }

        // Detect file format
        std::string extension = std::filesystem::path(filepath).extension().string();
        if (extension.empty()) {
            result.errorMessage = "Cannot determine file format (no extension): " + filepath;
            LOG_ERROR("AnimationImporter::ImportFromFile: " + result.errorMessage);
            return result;
        }

        result.sourceFormat = NormalizeExtension(extension);

        // Check if format supports animations
        if (!IsAnimationFormatSupported(result.sourceFormat)) {
            result.errorMessage = "Format does not support animations: " + result.sourceFormat;
            LOG_WARNING("AnimationImporter::ImportFromFile: " + result.errorMessage);
            return result;
        }

#ifdef GAMEENGINE_HAS_ASSIMP
        // Load scene using Assimp
        Assimp::Importer importer;
        
        // Set post-processing flags for animation import
        uint32_t flags = aiProcess_Triangulate |
                        aiProcess_ValidateDataStructure |
                        aiProcess_LimitBoneWeights;
        
        // Don't pre-transform vertices as it breaks animations
        // Don't optimize graph as it might remove animation nodes
        
        const aiScene* scene = importer.ReadFile(filepath, flags);
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            result.errorMessage = "Assimp error: " + std::string(importer.GetErrorString());
            LOG_ERROR("AnimationImporter::ImportFromFile: " + result.errorMessage);
            return result;
        }

        // Import from Assimp scene
        result = ImportFromAssimpScene(scene, result.sourceFormat, config);
        
#else
        result.errorMessage = "Assimp not available - cannot import animations";
        LOG_ERROR("AnimationImporter::ImportFromFile: " + result.errorMessage);
#endif

    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = "Exception during animation import: " + std::string(e.what());
        LOG_ERROR("AnimationImporter::ImportFromFile: " + result.errorMessage);
    }

    return result;
}

AnimationImportResult AnimationImporter::ImportFromScene(const void* scene, 
                                                        const std::string& sourceFormat,
                                                        const AnimationImportConfig& config) {
    AnimationImportResult result;
    result.sourceFormat = sourceFormat;

#ifdef GAMEENGINE_HAS_ASSIMP
    const aiScene* assimpScene = static_cast<const aiScene*>(scene);
    if (!assimpScene) {
        result.errorMessage = "Invalid scene pointer";
        LOG_ERROR("AnimationImporter::ImportFromScene: " + result.errorMessage);
        return result;
    }

    result = ImportFromAssimpScene(assimpScene, sourceFormat, config);
#else
    result.errorMessage = "Assimp not available - cannot import animations";
    LOG_ERROR("AnimationImporter::ImportFromScene: " + result.errorMessage);
#endif

    return result;
}

std::shared_ptr<AnimationSkeleton> AnimationImporter::ImportSkeletonFromFile(const std::string& filepath,
                                                                            const AnimationImportConfig& config) {
    auto result = ImportFromFile(filepath, config);
    return result.skeleton;
}

std::vector<std::shared_ptr<SkeletalAnimation>> AnimationImporter::ImportAnimationsFromFile(
    const std::string& filepath,
    std::shared_ptr<AnimationSkeleton> targetSkeleton,
    const AnimationImportConfig& config) {
    
    if (!targetSkeleton) {
        LOG_ERROR("AnimationImporter::ImportAnimationsFromFile: Target skeleton is null");
        return {};
    }

    auto result = ImportFromFile(filepath, config);
    if (!result.success) {
        LOG_ERROR("AnimationImporter::ImportAnimationsFromFile: Failed to import from file: " + result.errorMessage);
        return {};
    }

    // TODO: Implement skeleton compatibility checking and animation retargeting
    // For now, return the imported animations as-is
    return result.animations;
}

bool AnimationImporter::ValidateAnimationData(const std::string& filepath) const {
#ifdef GAMEENGINE_HAS_ASSIMP
    try {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath, aiProcess_ValidateDataStructure);
        
        if (!scene) {
            return false;
        }

        // Check if scene has animations
        if (scene->mNumAnimations == 0) {
            LOG_INFO("AnimationImporter::ValidateAnimationData: File has no animations: " + filepath);
            return true; // Valid but no animations
        }

        // Basic validation of animation data
        for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
            const aiAnimation* anim = scene->mAnimations[i];
            if (!anim || anim->mDuration <= 0.0 || anim->mTicksPerSecond <= 0.0) {
                return false;
            }
        }

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("AnimationImporter::ValidateAnimationData: Exception: " + std::string(e.what()));
        return false;
    }
#else
    LOG_WARNING("AnimationImporter::ValidateAnimationData: Assimp not available");
    return false;
#endif
}

std::vector<std::string> AnimationImporter::GetAnimationNames(const std::string& filepath) const {
    std::vector<std::string> names;

#ifdef GAMEENGINE_HAS_ASSIMP
    try {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath, aiProcess_ValidateDataStructure);
        
        if (scene && scene->mNumAnimations > 0) {
            names.reserve(scene->mNumAnimations);
            for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
                const aiAnimation* anim = scene->mAnimations[i];
                if (anim && anim->mName.length > 0) {
                    names.emplace_back(anim->mName.C_Str());
                } else {
                    names.emplace_back("Animation_" + std::to_string(i));
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("AnimationImporter::GetAnimationNames: Exception: " + std::string(e.what()));
    }
#endif

    return names;
}

std::vector<std::string> AnimationImporter::GetBoneNames(const std::string& filepath) const {
    std::vector<std::string> names;

#ifdef GAMEENGINE_HAS_ASSIMP
    try {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath, aiProcess_ValidateDataStructure);
        
        if (scene && scene->mRootNode) {
            // Collect bone names from meshes
            std::unordered_set<std::string> uniqueNames;
            
            for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
                const aiMesh* mesh = scene->mMeshes[i];
                if (mesh && mesh->mNumBones > 0) {
                    for (uint32_t j = 0; j < mesh->mNumBones; ++j) {
                        const aiBone* bone = mesh->mBones[j];
                        if (bone && bone->mName.length > 0) {
                            uniqueNames.insert(bone->mName.C_Str());
                        }
                    }
                }
            }
            
            names.assign(uniqueNames.begin(), uniqueNames.end());
            std::sort(names.begin(), names.end());
        }
    } catch (const std::exception& e) {
        LOG_ERROR("AnimationImporter::GetBoneNames: Exception: " + std::string(e.what()));
    }
#endif

    return names;
}

bool AnimationImporter::IsAnimationFormatSupported(const std::string& extension) const {
    std::string normalizedExt = NormalizeExtension(extension);
    
    // Common formats that support animations
    static const std::unordered_set<std::string> supportedFormats = {
        "fbx", "dae", "gltf", "glb", "blend", "x", "bvh", "md5anim"
    };
    
    return supportedFormats.find(normalizedExt) != supportedFormats.end();
}

std::vector<std::string> AnimationImporter::GetSupportedAnimationFormats() const {
    return {"fbx", "dae", "gltf", "glb", "blend", "x", "bvh", "md5anim"};
}

#ifdef GAMEENGINE_HAS_ASSIMP

AnimationImportResult AnimationImporter::ImportFromAssimpScene(const aiScene* scene, 
                                                              const std::string& sourceFormat,
                                                              const AnimationImportConfig& config) {
    AnimationImportResult result;
    result.sourceFormat = sourceFormat;

    try {
        LOG_INFO("AnimationImporter: Starting import from " + sourceFormat + " scene");
        LOG_INFO("Scene contains " + std::to_string(scene->mNumAnimations) + " animations");

        // Import skeleton first
        result.skeleton = ImportSkeletonFromAssimp(scene, config);
        if (!result.skeleton) {
            result.errorMessage = "Failed to import skeleton";
            LOG_ERROR("AnimationImporter::ImportFromAssimpScene: " + result.errorMessage);
            return result;
        }

        result.boneCount = result.skeleton->GetBoneCount();
        LOG_INFO("Imported skeleton with " + std::to_string(result.boneCount) + " bones");

        // Import animations
        if (scene->mNumAnimations > 0) {
            result.animations = ImportAnimationsFromAssimp(scene, result.skeleton, config);
            result.animationCount = result.animations.size();
            
            // Calculate total duration
            for (const auto& anim : result.animations) {
                if (anim) {
                    result.totalDuration += anim->GetDuration();
                }
            }
            
            LOG_INFO("Imported " + std::to_string(result.animationCount) + " animations");
        } else {
            LOG_INFO("No animations found in scene");
        }

        // Apply post-processing
        if (config.convertCoordinateSystem) {
            ApplyCoordinateSystemConversion(result, config);
        }

        if (config.optimizeKeyframes && !result.animations.empty()) {
            OptimizeImportedAnimations(result.animations, config);
        }

        // Validate imported data
        ValidateImportedData(result);
        
        // Apply error correction if enabled
        if (config.validateBoneHierarchy || config.generateMissingBindPoses) {
            ApplyErrorCorrection(result, config);
        }

        result.success = true;
        LogImportStatistics(result);

    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = "Exception during Assimp scene import: " + std::string(e.what());
        LOG_ERROR("AnimationImporter::ImportFromAssimpScene: " + result.errorMessage);
    }

    return result;
}

std::shared_ptr<AnimationSkeleton> AnimationImporter::ImportSkeletonFromAssimp(const aiScene* scene,
                                                                             const AnimationImportConfig& config) {
    if (!scene || !scene->mRootNode) {
        LOG_ERROR("AnimationImporter::ImportSkeletonFromAssimp: Invalid scene");
        return nullptr;
    }

    auto skeleton = std::make_shared<AnimationSkeleton>("ImportedSkeleton");
    
    try {
        // Build bone mapping from meshes first
        BuildBoneMapping(scene, skeleton);
        
        // Process the node hierarchy to create bones
        ProcessAssimpNode(scene->mRootNode, skeleton, nullptr, config);
        
        // Set bind poses and calculate inverse bind poses
        if (config.generateMissingBindPoses) {
            skeleton->SetBindPose();
        }
        
        // Validate skeleton
        if (config.validateBoneHierarchy) {
            if (!ValidateSkeletonConsistency(skeleton)) {
                LOG_WARNING("AnimationImporter::ImportSkeletonFromAssimp: Skeleton validation failed");
            }
        }
        
        skeleton->RebuildBoneMaps();
        
        LOG_INFO("Successfully imported skeleton with " + std::to_string(skeleton->GetBoneCount()) + " bones");
        
    } catch (const std::exception& e) {
        LOG_ERROR("AnimationImporter::ImportSkeletonFromAssimp: Exception: " + std::string(e.what()));
        return nullptr;
    }

    return skeleton;
}

std::vector<std::shared_ptr<SkeletalAnimation>> AnimationImporter::ImportAnimationsFromAssimp(
    const aiScene* scene,
    std::shared_ptr<AnimationSkeleton> skeleton,
    const AnimationImportConfig& config) {
    
    std::vector<std::shared_ptr<SkeletalAnimation>> animations;
    
    if (!scene || !skeleton || scene->mNumAnimations == 0) {
        return animations;
    }

    try {
        animations.reserve(scene->mNumAnimations);
        
        for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
            const aiAnimation* assimpAnim = scene->mAnimations[i];
            if (!assimpAnim) {
                continue;
            }

            // Check animation name filter
            std::string animName = assimpAnim->mName.length > 0 ? 
                                 assimpAnim->mName.C_Str() : 
                                 ("Animation_" + std::to_string(i));
            
            if (!config.animationNameFilter.empty()) {
                bool found = std::find(config.animationNameFilter.begin(), 
                                     config.animationNameFilter.end(), 
                                     animName) != config.animationNameFilter.end();
                if (!found) {
                    LOG_INFO("Skipping animation '" + animName + "' due to name filter");
                    continue;
                }
            }

            auto animation = ProcessAssimpAnimation(assimpAnim, skeleton, config);
            if (animation) {
                animations.push_back(animation);
                LOG_INFO("Imported animation '" + animation->GetName() + 
                        "' (duration: " + std::to_string(animation->GetDuration()) + "s)");
            }
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("AnimationImporter::ImportAnimationsFromAssimp: Exception: " + std::string(e.what()));
    }

    return animations;
}

void AnimationImporter::ProcessAssimpNode(const aiNode* node, 
                                         std::shared_ptr<AnimationSkeleton> skeleton,
                                         std::shared_ptr<Bone> parentBone,
                                         const AnimationImportConfig& config) {
    if (!node || !skeleton) {
        return;
    }

    std::string nodeName = node->mName.C_Str();
    
    // Check if this node corresponds to a bone
    bool isBone = m_boneNameMapping.find(nodeName) != m_boneNameMapping.end();
    
    std::shared_ptr<Bone> currentBone = parentBone;
    
    if (isBone) {
        // Create bone from this node
        currentBone = CreateBoneFromAssimpNode(node, config);
        if (currentBone) {
            skeleton->AddBone(currentBone, parentBone ? parentBone->GetName() : "");
            
            if (!parentBone) {
                skeleton->SetRootBone(currentBone);
            }
        }
    }
    
    // Process child nodes
    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        ProcessAssimpNode(node->mChildren[i], skeleton, currentBone, config);
    }
}

std::shared_ptr<Bone> AnimationImporter::CreateBoneFromAssimpNode(const aiNode* node,
                                                                 const AnimationImportConfig& config) {
    if (!node) {
        return nullptr;
    }

    std::string boneName = node->mName.C_Str();
    
    // Create bone with unique ID
    static int32_t boneIdCounter = 0;
    auto bone = std::make_shared<Bone>(boneName, boneIdCounter++);
    
    // Convert and set local transform
    Math::Mat4 localTransform = ConvertAssimpMatrix(node->mTransformation, config);
    bone->SetLocalTransform(localTransform);
    
    // Set bind pose (same as local transform initially)
    bone->SetBindPose(localTransform);
    
    return bone;
}

std::shared_ptr<SkeletalAnimation> AnimationImporter::ProcessAssimpAnimation(const aiAnimation* animation,
                                                                           std::shared_ptr<AnimationSkeleton> skeleton,
                                                                           const AnimationImportConfig& config) {
    if (!animation || !skeleton) {
        return nullptr;
    }

    std::string animName = animation->mName.length > 0 ? 
                          animation->mName.C_Str() : 
                          "UnnamedAnimation";
    
    auto skeletalAnim = std::make_shared<SkeletalAnimation>(animName);
    
    // Set basic properties
    double duration = animation->mDuration;
    double ticksPerSecond = animation->mTicksPerSecond > 0 ? animation->mTicksPerSecond : 25.0;
    
    skeletalAnim->SetDuration(static_cast<float>(duration / ticksPerSecond));
    skeletalAnim->SetFrameRate(static_cast<float>(ticksPerSecond));
    
    // Process node animations (bone tracks)
    for (uint32_t i = 0; i < animation->mNumChannels; ++i) {
        const aiNodeAnim* nodeAnim = animation->mChannels[i];
        if (nodeAnim) {
            ProcessAssimpNodeAnimation(nodeAnim, skeletalAnim, config);
        }
    }
    
    // Validate and optimize
    if (config.removeRedundantTracks) {
        // TODO: Implement redundant track removal
    }
    
    return skeletalAnim;
}

void AnimationImporter::ProcessAssimpNodeAnimation(const aiNodeAnim* nodeAnim,
                                                  std::shared_ptr<SkeletalAnimation> animation,
                                                  const AnimationImportConfig& config) {
    if (!nodeAnim || !animation) {
        return;
    }

    std::string boneName = nodeAnim->mNodeName.C_Str();
    
    // Check bone name filter
    if (!config.boneNameFilter.empty()) {
        bool found = std::find(config.boneNameFilter.begin(), 
                             config.boneNameFilter.end(), 
                             boneName) != config.boneNameFilter.end();
        if (!found) {
            return;
        }
    }

    // Create position track
    if (nodeAnim->mNumPositionKeys > 0) {
        auto posTrack = animation->CreatePositionTrack(boneName);
        if (posTrack) {
            for (uint32_t i = 0; i < nodeAnim->mNumPositionKeys; ++i) {
                const aiVectorKey& key = nodeAnim->mPositionKeys[i];
                float time = static_cast<float>(key.mTime / animation->GetFrameRate());
                Math::Vec3 position = ConvertAssimpVector3(key.mValue, config);
                
                animation->AddPositionKeyframe(boneName, time, position);
            }
        }
    }

    // Create rotation track
    if (nodeAnim->mNumRotationKeys > 0) {
        auto rotTrack = animation->CreateRotationTrack(boneName);
        if (rotTrack) {
            for (uint32_t i = 0; i < nodeAnim->mNumRotationKeys; ++i) {
                const aiQuatKey& key = nodeAnim->mRotationKeys[i];
                float time = static_cast<float>(key.mTime / animation->GetFrameRate());
                Math::Quat rotation = ConvertAssimpQuaternion(key.mValue, config);
                
                animation->AddRotationKeyframe(boneName, time, rotation);
            }
        }
    }

    // Create scale track
    if (nodeAnim->mNumScalingKeys > 0) {
        auto scaleTrack = animation->CreateScaleTrack(boneName);
        if (scaleTrack) {
            for (uint32_t i = 0; i < nodeAnim->mNumScalingKeys; ++i) {
                const aiVectorKey& key = nodeAnim->mScalingKeys[i];
                float time = static_cast<float>(key.mTime / animation->GetFrameRate());
                Math::Vec3 scale = ConvertAssimpVector3(key.mValue, config);
                
                animation->AddScaleKeyframe(boneName, time, scale);
            }
        }
    }
}

Math::Mat4 AnimationImporter::ConvertAssimpMatrix(const aiMatrix4x4& matrix, const AnimationImportConfig& config) const {
    Math::Mat4 result(
        matrix.a1, matrix.b1, matrix.c1, matrix.d1,
        matrix.a2, matrix.b2, matrix.c2, matrix.d2,
        matrix.a3, matrix.b3, matrix.c3, matrix.d3,
        matrix.a4, matrix.b4, matrix.c4, matrix.d4
    );
    
    if (config.convertCoordinateSystem) {
        // Apply coordinate system conversion if needed
        if (config.flipYZ) {
            // Flip Y and Z axes
            Math::Mat4 flipMatrix(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );
            result = flipMatrix * result * flipMatrix;
        }
        
        // Apply scale
        if (config.coordinateSystemScale != Math::Vec3(1.0f)) {
            Math::Mat4 scaleMatrix = glm::scale(Math::Mat4(1.0f), config.coordinateSystemScale);
            result = scaleMatrix * result;
        }
    }
    
    return result;
}

Math::Vec3 AnimationImporter::ConvertAssimpVector3(const aiVector3D& vector, const AnimationImportConfig& config) const {
    Math::Vec3 result(vector.x, vector.y, vector.z);
    
    if (config.convertCoordinateSystem) {
        if (config.flipYZ) {
            std::swap(result.y, result.z);
        }
        result *= config.coordinateSystemScale;
    }
    
    return result;
}

Math::Quat AnimationImporter::ConvertAssimpQuaternion(const aiQuaternion& quat, const AnimationImportConfig& config) const {
    Math::Quat result(quat.w, quat.x, quat.y, quat.z);
    
    if (config.convertCoordinateSystem && config.flipYZ) {
        // Adjust quaternion for Y-Z flip
        std::swap(result.y, result.z);
        result.y = -result.y; // Negate to maintain handedness
    }
    
    return result;
}

void AnimationImporter::BuildBoneMapping(const aiScene* scene, std::shared_ptr<AnimationSkeleton> skeleton) {
    m_boneNameMapping.clear();
    
    if (!scene || !skeleton) {
        return;
    }

    // Collect bone names from all meshes
    for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[i];
        if (mesh && mesh->mNumBones > 0) {
            for (uint32_t j = 0; j < mesh->mNumBones; ++j) {
                const aiBone* bone = mesh->mBones[j];
                if (bone && bone->mName.length > 0) {
                    std::string boneName = bone->mName.C_Str();
                    m_boneNameMapping[boneName] = boneName; // Direct mapping for now
                }
            }
        }
    }
    
    LOG_INFO("Built bone mapping with " + std::to_string(m_boneNameMapping.size()) + " bones");
}

bool AnimationImporter::ValidateSkeletonConsistency(std::shared_ptr<AnimationSkeleton> skeleton) const {
    if (!skeleton) {
        return false;
    }

    return skeleton->ValidateHierarchy();
}

void AnimationImporter::MapAnimationTracks(const aiAnimation* assimpAnim,
                                          std::shared_ptr<SkeletalAnimation> animation,
                                          std::shared_ptr<AnimationSkeleton> skeleton,
                                          const AnimationImportConfig& config) {
    // This method can be used for more sophisticated track mapping
    // For now, the basic mapping is handled in ProcessAssimpNodeAnimation
}

#endif // GAMEENGINE_HAS_ASSIMP

void AnimationImporter::ApplyCoordinateSystemConversion(AnimationImportResult& result, 
                                                       const AnimationImportConfig& config) {
    // Coordinate system conversion is applied during import in the Convert* methods
    LOG_INFO("Applied coordinate system conversion");
}

void AnimationImporter::OptimizeImportedAnimations(std::vector<std::shared_ptr<SkeletalAnimation>>& animations,
                                                  const AnimationImportConfig& config) {
    for (auto& animation : animations) {
        if (animation) {
            animation->OptimizeKeyframes(config.keyframeOptimizationTolerance);
        }
    }
    
    LOG_INFO("Optimized " + std::to_string(animations.size()) + " animations");
}

void AnimationImporter::ValidateImportedData(AnimationImportResult& result) const {
    if (!result.skeleton) {
        result.success = false;
        result.errorMessage = "No skeleton imported";
        return;
    }

    if (!result.skeleton->ValidateHierarchy()) {
        LOG_WARNING("AnimationImporter::ValidateImportedData: Skeleton hierarchy validation failed");
    }

    for (const auto& animation : result.animations) {
        if (animation && !animation->ValidateAnimation()) {
            LOG_WARNING("AnimationImporter::ValidateImportedData: Animation '" + 
                       animation->GetName() + "' validation failed");
        }
    }
}

std::string AnimationImporter::NormalizeExtension(const std::string& extension) const {
    std::string normalized = extension;
    
    // Remove leading dot if present
    if (!normalized.empty() && normalized[0] == '.') {
        normalized = normalized.substr(1);
    }
    
    // Convert to lowercase
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    
    return normalized;
}

void AnimationImporter::ApplyErrorCorrection(AnimationImportResult& result, const AnimationImportConfig& config) {
    if (!result.success || (!result.skeleton && result.animations.empty())) {
        return;
    }

    try {
        AnimationValidator validator;
        ValidationConfig validationConfig;
        validationConfig.enableAutoFix = true;
        validationConfig.fixRedundantKeyframes = config.optimizeKeyframes;
        validationConfig.fixInvalidDurations = true;
        validationConfig.fixBoneNaming = true;

        // Validate and fix skeleton if present
        if (result.skeleton) {
            auto skeletonValidation = validator.ValidateSkeleton(result.skeleton, validationConfig);
            if (skeletonValidation.HasIssues()) {
                LOG_INFO("AnimationImporter: Found " + std::to_string(skeletonValidation.issues.size()) + 
                        " issues in imported skeleton, attempting fixes...");
                
                bool fixed = validator.FixValidationIssues(result.skeleton, skeletonValidation);
                if (fixed) {
                    LOG_INFO("AnimationImporter: Successfully fixed skeleton issues");
                } else {
                    LOG_WARNING("AnimationImporter: Could not fix all skeleton issues");
                }
            }
        }

        // Validate and fix animations
        for (auto& animation : result.animations) {
            if (!animation) continue;

            ValidationResult animValidation;
            if (result.skeleton) {
                animValidation = validator.ValidateAnimationWithSkeleton(animation, result.skeleton, validationConfig);
            } else {
                animValidation = validator.ValidateAnimation(animation, validationConfig);
            }

            if (animValidation.HasIssues()) {
                LOG_INFO("AnimationImporter: Found " + std::to_string(animValidation.issues.size()) + 
                        " issues in animation '" + animation->GetName() + "', attempting fixes...");
                
                bool fixed = validator.FixValidationIssues(animation, animValidation);
                if (fixed) {
                    LOG_INFO("AnimationImporter: Successfully fixed animation issues");
                } else {
                    LOG_WARNING("AnimationImporter: Could not fix all animation issues");
                }
            }
        }

    } catch (const std::exception& e) {
        LOG_WARNING("AnimationImporter::ApplyErrorCorrection: Exception: " + std::string(e.what()));
    }
}

void AnimationImporter::LogImportStatistics(const AnimationImportResult& result) const {
    std::stringstream ss;
    ss << "Animation import statistics:\n";
    ss << "  Source format: " << result.sourceFormat << "\n";
    ss << "  Success: " << (result.success ? "Yes" : "No") << "\n";
    ss << "  Bones: " << result.boneCount << "\n";
    ss << "  Animations: " << result.animationCount << "\n";
    ss << "  Total duration: " << result.totalDuration << "s";
    
    if (!result.success) {
        ss << "\n  Error: " << result.errorMessage;
    }
    
    LOG_INFO(ss.str());
}

} // namespace Animation
} // namespace GameEngine