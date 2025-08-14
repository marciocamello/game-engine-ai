#pragma once

#include "Animation/AnimationSkeleton.h"
#include "Animation/SkeletalAnimation.h"
#include "Core/Math.h"
#include <string>
#include <vector>
#include <memory>

namespace GameEngine {
namespace Animation {

    /**
     * Types of validation issues that can be found
     */
    enum class ValidationIssueType {
        Warning,
        Error,
        Critical
    };

    /**
     * Specific validation issue categories
     */
    enum class ValidationCategory {
        SkeletonHierarchy,
        AnimationData,
        KeyframeData,
        BoneMapping,
        CoordinateSystem,
        Performance,
        Compatibility
    };

    /**
     * Represents a validation issue found during animation data validation
     */
    struct ValidationIssue {
        ValidationIssueType type;
        ValidationCategory category;
        std::string description;
        std::string suggestion;
        std::string location; // Bone name, animation name, etc.
        
        // Additional context
        float severity = 1.0f; // 0.0 = minor, 1.0 = major
        bool canAutoFix = false;
        std::string autoFixDescription;
    };

    /**
     * Result of validation operation
     */
    struct ValidationResult {
        bool isValid = true;
        std::vector<ValidationIssue> issues;
        
        // Statistics
        size_t warningCount = 0;
        size_t errorCount = 0;
        size_t criticalCount = 0;
        
        // Performance metrics
        float validationTimeMs = 0.0f;
        
        bool HasCriticalIssues() const { return criticalCount > 0; }
        bool HasErrors() const { return errorCount > 0; }
        bool HasWarnings() const { return warningCount > 0; }
        bool HasIssues() const { return !issues.empty(); }
    };

    /**
     * Configuration for animation data validation
     */
    struct ValidationConfig {
        // Skeleton validation
        bool validateBoneHierarchy = true;
        bool checkForCyclicDependencies = true;
        bool validateBindPoses = true;
        bool checkBoneNaming = true;
        
        // Animation validation
        bool validateKeyframeData = true;
        bool checkAnimationDuration = true;
        bool validateFrameRate = true;
        bool checkForRedundantKeyframes = true;
        
        // Performance validation
        bool checkBoneCount = true;
        bool checkKeyframeCount = true;
        bool validateMemoryUsage = true;
        
        // Tolerance settings
        float keyframeRedundancyTolerance = 0.001f;
        float durationTolerance = 0.001f;
        size_t maxRecommendedBones = 256;
        size_t maxRecommendedKeyframes = 10000;
        
        // Auto-fix settings
        bool enableAutoFix = false;
        bool fixRedundantKeyframes = true;
        bool fixInvalidDurations = true;
        bool fixBoneNaming = false;
    };

    /**
     * Handles validation and error correction of animation data
     */
    class AnimationValidator {
    public:
        AnimationValidator();
        ~AnimationValidator() = default;

        // Main validation interface
        ValidationResult ValidateSkeleton(std::shared_ptr<AnimationSkeleton> skeleton,
                                         const ValidationConfig& config = ValidationConfig{});
        
        ValidationResult ValidateAnimation(std::shared_ptr<SkeletalAnimation> animation,
                                         const ValidationConfig& config = ValidationConfig{});
        
        ValidationResult ValidateAnimationWithSkeleton(std::shared_ptr<SkeletalAnimation> animation,
                                                      std::shared_ptr<AnimationSkeleton> skeleton,
                                                      const ValidationConfig& config = ValidationConfig{});

        // Batch validation
        ValidationResult ValidateMultipleAnimations(const std::vector<std::shared_ptr<SkeletalAnimation>>& animations,
                                                   std::shared_ptr<AnimationSkeleton> skeleton,
                                                   const ValidationConfig& config = ValidationConfig{});

        // Error correction
        bool FixValidationIssues(std::shared_ptr<AnimationSkeleton> skeleton,
                               const ValidationResult& result);
        
        bool FixValidationIssues(std::shared_ptr<SkeletalAnimation> animation,
                               const ValidationResult& result);

        // Configuration
        void SetDefaultConfig(const ValidationConfig& config) { m_defaultConfig = config; }
        const ValidationConfig& GetDefaultConfig() const { return m_defaultConfig; }

        // Utility methods
        std::string GetValidationReport(const ValidationResult& result) const;
        void PrintValidationReport(const ValidationResult& result) const;

    private:
        ValidationConfig m_defaultConfig;

        // Skeleton validation methods
        void ValidateSkeletonHierarchy(std::shared_ptr<AnimationSkeleton> skeleton,
                                     ValidationResult& result,
                                     const ValidationConfig& config);
        
        void ValidateBindPoses(std::shared_ptr<AnimationSkeleton> skeleton,
                             ValidationResult& result,
                             const ValidationConfig& config);
        
        void ValidateBoneNaming(std::shared_ptr<AnimationSkeleton> skeleton,
                              ValidationResult& result,
                              const ValidationConfig& config);

        // Animation validation methods
        void ValidateAnimationData(std::shared_ptr<SkeletalAnimation> animation,
                                 ValidationResult& result,
                                 const ValidationConfig& config);
        
        void ValidateKeyframes(std::shared_ptr<SkeletalAnimation> animation,
                             ValidationResult& result,
                             const ValidationConfig& config);
        
        void ValidateBoneMapping(std::shared_ptr<SkeletalAnimation> animation,
                               std::shared_ptr<AnimationSkeleton> skeleton,
                               ValidationResult& result,
                               const ValidationConfig& config);

        // Performance validation
        void ValidatePerformance(std::shared_ptr<AnimationSkeleton> skeleton,
                               std::shared_ptr<SkeletalAnimation> animation,
                               ValidationResult& result,
                               const ValidationConfig& config);

        // Auto-fix methods
        bool FixSkeletonIssues(std::shared_ptr<AnimationSkeleton> skeleton,
                             const std::vector<ValidationIssue>& issues);
        
        bool FixAnimationIssues(std::shared_ptr<SkeletalAnimation> animation,
                              const std::vector<ValidationIssue>& issues);

        // Helper methods
        void AddIssue(ValidationResult& result,
                     ValidationIssueType type,
                     ValidationCategory category,
                     const std::string& description,
                     const std::string& location = "",
                     const std::string& suggestion = "",
                     bool canAutoFix = false);
        
        void UpdateStatistics(ValidationResult& result);
        
        bool IsValidBoneName(const std::string& name) const;
        bool HasCyclicDependency(std::shared_ptr<AnimationSkeleton> skeleton) const;
        bool HasCyclicDependencyDFS(std::shared_ptr<Bone> bone,
                                   std::unordered_set<int32_t>& visited,
                                   std::unordered_set<int32_t>& recursionStack) const;
        bool AreKeyframesRedundant(const std::vector<float>& times,
                                 const std::vector<Math::Vec3>& values,
                                 float tolerance) const;
    };

} // namespace Animation
} // namespace GameEngine