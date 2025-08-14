#include "Animation/AnimationValidator.h"
#include "Core/Logger.h"
#include <chrono>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <regex>

namespace GameEngine {
namespace Animation {

AnimationValidator::AnimationValidator() {
    // Set default configuration
    m_defaultConfig.validateBoneHierarchy = true;
    m_defaultConfig.checkForCyclicDependencies = true;
    m_defaultConfig.validateBindPoses = true;
    m_defaultConfig.checkBoneNaming = true;
    m_defaultConfig.validateKeyframeData = true;
    m_defaultConfig.checkAnimationDuration = true;
    m_defaultConfig.validateFrameRate = true;
    m_defaultConfig.checkForRedundantKeyframes = true;
    m_defaultConfig.checkBoneCount = true;
    m_defaultConfig.checkKeyframeCount = true;
    m_defaultConfig.validateMemoryUsage = true;
    m_defaultConfig.keyframeRedundancyTolerance = 0.001f;
    m_defaultConfig.durationTolerance = 0.001f;
    m_defaultConfig.maxRecommendedBones = 256;
    m_defaultConfig.maxRecommendedKeyframes = 10000;
    m_defaultConfig.enableAutoFix = false;
    m_defaultConfig.fixRedundantKeyframes = true;
    m_defaultConfig.fixInvalidDurations = true;
    m_defaultConfig.fixBoneNaming = false;
}

ValidationResult AnimationValidator::ValidateSkeleton(std::shared_ptr<AnimationSkeleton> skeleton,
                                                     const ValidationConfig& config) {
    ValidationResult result;
    auto startTime = std::chrono::high_resolution_clock::now();

    if (!skeleton) {
        AddIssue(result, ValidationIssueType::Critical, ValidationCategory::SkeletonHierarchy,
                "Skeleton is null", "", "Provide a valid skeleton object");
        result.isValid = false;
        return result;
    }

    try {
        LOG_INFO("AnimationValidator: Starting skeleton validation for '" + skeleton->GetName() + "'");

        // Validate skeleton hierarchy
        if (config.validateBoneHierarchy) {
            ValidateSkeletonHierarchy(skeleton, result, config);
        }

        // Validate bind poses
        if (config.validateBindPoses) {
            ValidateBindPoses(skeleton, result, config);
        }

        // Validate bone naming
        if (config.checkBoneNaming) {
            ValidateBoneNaming(skeleton, result, config);
        }

        // Update statistics
        UpdateStatistics(result);

        // Calculate validation time
        auto endTime = std::chrono::high_resolution_clock::now();
        result.validationTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();

        result.isValid = !result.HasCriticalIssues();

        LOG_INFO("AnimationValidator: Skeleton validation completed in " + 
                std::to_string(result.validationTimeMs) + "ms");
        LOG_INFO("Issues found: " + std::to_string(result.criticalCount) + " critical, " +
                std::to_string(result.errorCount) + " errors, " +
                std::to_string(result.warningCount) + " warnings");

    } catch (const std::exception& e) {
        AddIssue(result, ValidationIssueType::Critical, ValidationCategory::SkeletonHierarchy,
                "Exception during skeleton validation: " + std::string(e.what()));
        result.isValid = false;
        LOG_ERROR("AnimationValidator::ValidateSkeleton: Exception: " + std::string(e.what()));
    }

    return result;
}

ValidationResult AnimationValidator::ValidateAnimation(std::shared_ptr<SkeletalAnimation> animation,
                                                      const ValidationConfig& config) {
    ValidationResult result;
    auto startTime = std::chrono::high_resolution_clock::now();

    if (!animation) {
        AddIssue(result, ValidationIssueType::Critical, ValidationCategory::AnimationData,
                "Animation is null", "", "Provide a valid animation object");
        result.isValid = false;
        return result;
    }

    try {
        LOG_INFO("AnimationValidator: Starting animation validation for '" + animation->GetName() + "'");

        // Validate animation data
        if (config.validateKeyframeData) {
            ValidateAnimationData(animation, result, config);
        }

        // Validate keyframes
        if (config.checkForRedundantKeyframes) {
            ValidateKeyframes(animation, result, config);
        }

        // Update statistics
        UpdateStatistics(result);

        // Calculate validation time
        auto endTime = std::chrono::high_resolution_clock::now();
        result.validationTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();

        result.isValid = !result.HasCriticalIssues();

        LOG_INFO("AnimationValidator: Animation validation completed in " + 
                std::to_string(result.validationTimeMs) + "ms");

    } catch (const std::exception& e) {
        AddIssue(result, ValidationIssueType::Critical, ValidationCategory::AnimationData,
                "Exception during animation validation: " + std::string(e.what()));
        result.isValid = false;
        LOG_ERROR("AnimationValidator::ValidateAnimation: Exception: " + std::string(e.what()));
    }

    return result;
}

ValidationResult AnimationValidator::ValidateAnimationWithSkeleton(std::shared_ptr<SkeletalAnimation> animation,
                                                                  std::shared_ptr<AnimationSkeleton> skeleton,
                                                                  const ValidationConfig& config) {
    ValidationResult result;
    auto startTime = std::chrono::high_resolution_clock::now();

    if (!animation || !skeleton) {
        AddIssue(result, ValidationIssueType::Critical, ValidationCategory::AnimationData,
                "Animation or skeleton is null", "", "Provide valid animation and skeleton objects");
        result.isValid = false;
        return result;
    }

    try {
        LOG_INFO("AnimationValidator: Starting combined validation for animation '" + 
                animation->GetName() + "' with skeleton '" + skeleton->GetName() + "'");

        // First validate skeleton
        auto skeletonResult = ValidateSkeleton(skeleton, config);
        result.issues.insert(result.issues.end(), skeletonResult.issues.begin(), skeletonResult.issues.end());

        // Then validate animation
        auto animationResult = ValidateAnimation(animation, config);
        result.issues.insert(result.issues.end(), animationResult.issues.begin(), animationResult.issues.end());

        // Validate bone mapping between animation and skeleton
        ValidateBoneMapping(animation, skeleton, result, config);

        // Validate performance
        ValidatePerformance(skeleton, animation, result, config);

        // Update statistics
        UpdateStatistics(result);

        // Calculate validation time
        auto endTime = std::chrono::high_resolution_clock::now();
        result.validationTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();

        result.isValid = !result.HasCriticalIssues();

        LOG_INFO("AnimationValidator: Combined validation completed in " + 
                std::to_string(result.validationTimeMs) + "ms");

    } catch (const std::exception& e) {
        AddIssue(result, ValidationIssueType::Critical, ValidationCategory::AnimationData,
                "Exception during combined validation: " + std::string(e.what()));
        result.isValid = false;
        LOG_ERROR("AnimationValidator::ValidateAnimationWithSkeleton: Exception: " + std::string(e.what()));
    }

    return result;
}

ValidationResult AnimationValidator::ValidateMultipleAnimations(const std::vector<std::shared_ptr<SkeletalAnimation>>& animations,
                                                               std::shared_ptr<AnimationSkeleton> skeleton,
                                                               const ValidationConfig& config) {
    ValidationResult result;
    auto startTime = std::chrono::high_resolution_clock::now();

    if (!skeleton) {
        AddIssue(result, ValidationIssueType::Critical, ValidationCategory::SkeletonHierarchy,
                "Skeleton is null", "", "Provide a valid skeleton object");
        result.isValid = false;
        return result;
    }

    try {
        LOG_INFO("AnimationValidator: Starting batch validation of " + 
                std::to_string(animations.size()) + " animations");

        // Validate skeleton once
        auto skeletonResult = ValidateSkeleton(skeleton, config);
        result.issues.insert(result.issues.end(), skeletonResult.issues.begin(), skeletonResult.issues.end());

        // Validate each animation
        for (size_t i = 0; i < animations.size(); ++i) {
            const auto& animation = animations[i];
            if (!animation) {
                AddIssue(result, ValidationIssueType::Error, ValidationCategory::AnimationData,
                        "Animation at index " + std::to_string(i) + " is null");
                continue;
            }

            auto animResult = ValidateAnimationWithSkeleton(animation, skeleton, config);
            result.issues.insert(result.issues.end(), animResult.issues.begin(), animResult.issues.end());
        }

        // Update statistics
        UpdateStatistics(result);

        // Calculate validation time
        auto endTime = std::chrono::high_resolution_clock::now();
        result.validationTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();

        result.isValid = !result.HasCriticalIssues();

        LOG_INFO("AnimationValidator: Batch validation completed in " + 
                std::to_string(result.validationTimeMs) + "ms");

    } catch (const std::exception& e) {
        AddIssue(result, ValidationIssueType::Critical, ValidationCategory::AnimationData,
                "Exception during batch validation: " + std::string(e.what()));
        result.isValid = false;
        LOG_ERROR("AnimationValidator::ValidateMultipleAnimations: Exception: " + std::string(e.what()));
    }

    return result;
}

bool AnimationValidator::FixValidationIssues(std::shared_ptr<AnimationSkeleton> skeleton,
                                            const ValidationResult& result) {
    if (!skeleton) {
        LOG_ERROR("AnimationValidator::FixValidationIssues: Skeleton is null");
        return false;
    }

    std::vector<ValidationIssue> fixableIssues;
    for (const auto& issue : result.issues) {
        if (issue.canAutoFix) {
            fixableIssues.push_back(issue);
        }
    }

    if (fixableIssues.empty()) {
        LOG_INFO("AnimationValidator: No auto-fixable issues found for skeleton");
        return true;
    }

    LOG_INFO("AnimationValidator: Attempting to fix " + std::to_string(fixableIssues.size()) + 
            " issues in skeleton '" + skeleton->GetName() + "'");

    return FixSkeletonIssues(skeleton, fixableIssues);
}

bool AnimationValidator::FixValidationIssues(std::shared_ptr<SkeletalAnimation> animation,
                                            const ValidationResult& result) {
    if (!animation) {
        LOG_ERROR("AnimationValidator::FixValidationIssues: Animation is null");
        return false;
    }

    std::vector<ValidationIssue> fixableIssues;
    for (const auto& issue : result.issues) {
        if (issue.canAutoFix) {
            fixableIssues.push_back(issue);
        }
    }

    if (fixableIssues.empty()) {
        LOG_INFO("AnimationValidator: No auto-fixable issues found for animation");
        return true;
    }

    LOG_INFO("AnimationValidator: Attempting to fix " + std::to_string(fixableIssues.size()) + 
            " issues in animation '" + animation->GetName() + "'");

    return FixAnimationIssues(animation, fixableIssues);
}

std::string AnimationValidator::GetValidationReport(const ValidationResult& result) const {
    std::stringstream ss;
    
    ss << "Animation Validation Report\n";
    ss << "==========================\n";
    ss << "Overall Status: " << (result.isValid ? "VALID" : "INVALID") << "\n";
    ss << "Validation Time: " << result.validationTimeMs << "ms\n";
    ss << "Issues Found: " << result.issues.size() << " total\n";
    ss << "  - Critical: " << result.criticalCount << "\n";
    ss << "  - Errors: " << result.errorCount << "\n";
    ss << "  - Warnings: " << result.warningCount << "\n\n";

    if (!result.issues.empty()) {
        ss << "Detailed Issues:\n";
        ss << "----------------\n";
        
        for (size_t i = 0; i < result.issues.size(); ++i) {
            const auto& issue = result.issues[i];
            
            std::string typeStr;
            switch (issue.type) {
                case ValidationIssueType::Warning: typeStr = "WARNING"; break;
                case ValidationIssueType::Error: typeStr = "ERROR"; break;
                case ValidationIssueType::Critical: typeStr = "CRITICAL"; break;
            }
            
            std::string categoryStr;
            switch (issue.category) {
                case ValidationCategory::SkeletonHierarchy: categoryStr = "Skeleton"; break;
                case ValidationCategory::AnimationData: categoryStr = "Animation"; break;
                case ValidationCategory::KeyframeData: categoryStr = "Keyframes"; break;
                case ValidationCategory::BoneMapping: categoryStr = "BoneMapping"; break;
                case ValidationCategory::CoordinateSystem: categoryStr = "Coordinates"; break;
                case ValidationCategory::Performance: categoryStr = "Performance"; break;
                case ValidationCategory::Compatibility: categoryStr = "Compatibility"; break;
            }
            
            ss << (i + 1) << ". [" << typeStr << "] " << categoryStr;
            if (!issue.location.empty()) {
                ss << " (" << issue.location << ")";
            }
            ss << "\n   " << issue.description << "\n";
            
            if (!issue.suggestion.empty()) {
                ss << "   Suggestion: " << issue.suggestion << "\n";
            }
            
            if (issue.canAutoFix) {
                ss << "   Auto-fix: Available";
                if (!issue.autoFixDescription.empty()) {
                    ss << " - " << issue.autoFixDescription;
                }
                ss << "\n";
            }
            
            ss << "\n";
        }
    }

    return ss.str();
}

void AnimationValidator::PrintValidationReport(const ValidationResult& result) const {
    std::string report = GetValidationReport(result);
    LOG_INFO(report);
}

// Private implementation methods

void AnimationValidator::ValidateSkeletonHierarchy(std::shared_ptr<AnimationSkeleton> skeleton,
                                                  ValidationResult& result,
                                                  const ValidationConfig& config) {
    // Check if skeleton has bones
    if (skeleton->GetBoneCount() == 0) {
        AddIssue(result, ValidationIssueType::Warning, ValidationCategory::SkeletonHierarchy,
                "Skeleton has no bones", skeleton->GetName(),
                "Add bones to the skeleton");
        return;
    }

    // Check for root bone
    auto rootBone = skeleton->GetRootBone();
    if (!rootBone) {
        AddIssue(result, ValidationIssueType::Error, ValidationCategory::SkeletonHierarchy,
                "Skeleton has no root bone", skeleton->GetName(),
                "Set a root bone for the skeleton", true);
    }

    // Check for cyclic dependencies
    if (config.checkForCyclicDependencies && HasCyclicDependency(skeleton)) {
        AddIssue(result, ValidationIssueType::Critical, ValidationCategory::SkeletonHierarchy,
                "Cyclic dependency detected in bone hierarchy", skeleton->GetName(),
                "Fix bone parent-child relationships");
    }

    // Validate hierarchy consistency
    if (!skeleton->ValidateHierarchy()) {
        AddIssue(result, ValidationIssueType::Error, ValidationCategory::SkeletonHierarchy,
                "Skeleton hierarchy validation failed", skeleton->GetName(),
                "Check bone parent-child relationships", true);
    }

    // Check bone count for performance
    if (config.checkBoneCount && skeleton->GetBoneCount() > config.maxRecommendedBones) {
        AddIssue(result, ValidationIssueType::Warning, ValidationCategory::Performance,
                "High bone count may impact performance", skeleton->GetName(),
                "Consider reducing bone count to " + std::to_string(config.maxRecommendedBones) + " or less");
    }
}

void AnimationValidator::ValidateBindPoses(std::shared_ptr<AnimationSkeleton> skeleton,
                                          ValidationResult& result,
                                          const ValidationConfig& config) {
    if (!skeleton->HasValidBindPose()) {
        AddIssue(result, ValidationIssueType::Warning, ValidationCategory::SkeletonHierarchy,
                "Skeleton has no valid bind pose", skeleton->GetName(),
                "Set bind pose for the skeleton", true);
    }

    // Check individual bone bind poses
    const auto& bones = skeleton->GetAllBones();
    for (const auto& bone : bones) {
        if (!bone) continue;

        // Check for identity matrices (might indicate missing bind pose)
        const auto& bindPose = bone->GetBindPose();
        bool isIdentity = true;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                float expected = (i == j) ? 1.0f : 0.0f;
                if (std::abs(bindPose[i][j] - expected) > 0.001f) {
                    isIdentity = false;
                    break;
                }
            }
            if (!isIdentity) break;
        }

        if (isIdentity && bone->GetName() != "Root") {
            AddIssue(result, ValidationIssueType::Warning, ValidationCategory::SkeletonHierarchy,
                    "Bone has identity bind pose", bone->GetName(),
                    "Verify bind pose is correct");
        }
    }
}

void AnimationValidator::ValidateBoneNaming(std::shared_ptr<AnimationSkeleton> skeleton,
                                           ValidationResult& result,
                                           const ValidationConfig& config) {
    const auto& bones = skeleton->GetAllBones();
    std::unordered_set<std::string> boneNames;

    for (const auto& bone : bones) {
        if (!bone) continue;

        const std::string& name = bone->GetName();

        // Check for empty names
        if (name.empty()) {
            AddIssue(result, ValidationIssueType::Error, ValidationCategory::SkeletonHierarchy,
                    "Bone has empty name", "Bone ID: " + std::to_string(bone->GetId()),
                    "Assign a valid name to the bone", true);
            continue;
        }

        // Check for duplicate names
        if (boneNames.find(name) != boneNames.end()) {
            AddIssue(result, ValidationIssueType::Error, ValidationCategory::SkeletonHierarchy,
                    "Duplicate bone name found", name,
                    "Ensure all bone names are unique", true);
        } else {
            boneNames.insert(name);
        }

        // Check for valid naming convention
        if (!IsValidBoneName(name)) {
            AddIssue(result, ValidationIssueType::Warning, ValidationCategory::SkeletonHierarchy,
                    "Bone name doesn't follow naming convention", name,
                    "Use alphanumeric characters and underscores only");
        }
    }
}

void AnimationValidator::ValidateAnimationData(std::shared_ptr<SkeletalAnimation> animation,
                                              ValidationResult& result,
                                              const ValidationConfig& config) {
    // Check animation duration
    if (config.checkAnimationDuration) {
        float duration = animation->GetDuration();
        if (duration <= config.durationTolerance) {
            AddIssue(result, ValidationIssueType::Warning, ValidationCategory::AnimationData,
                    "Animation has very short or zero duration", animation->GetName(),
                    "Verify animation duration is correct", true);
        }
    }

    // Check frame rate
    if (config.validateFrameRate) {
        float frameRate = animation->GetFrameRate();
        if (frameRate <= 0.0f) {
            AddIssue(result, ValidationIssueType::Error, ValidationCategory::AnimationData,
                    "Animation has invalid frame rate", animation->GetName(),
                    "Set a valid frame rate (e.g., 30 or 60 FPS)", true);
        } else if (frameRate < 10.0f || frameRate > 120.0f) {
            AddIssue(result, ValidationIssueType::Warning, ValidationCategory::AnimationData,
                    "Animation has unusual frame rate", animation->GetName(),
                    "Consider using standard frame rates (24, 30, 60 FPS)");
        }
    }

    // Check if animation is empty
    if (animation->IsEmpty()) {
        AddIssue(result, ValidationIssueType::Warning, ValidationCategory::AnimationData,
                "Animation has no bone tracks", animation->GetName(),
                "Add animation tracks for bones");
    }

    // Check keyframe count for performance
    if (config.checkKeyframeCount) {
        size_t keyframeCount = animation->GetKeyframeCount();
        if (keyframeCount > config.maxRecommendedKeyframes) {
            AddIssue(result, ValidationIssueType::Warning, ValidationCategory::Performance,
                    "High keyframe count may impact performance", animation->GetName(),
                    "Consider optimizing keyframes or using compression");
        }
    }
}

void AnimationValidator::ValidateKeyframes(std::shared_ptr<SkeletalAnimation> animation,
                                          ValidationResult& result,
                                          const ValidationConfig& config) {
    const auto& boneAnimations = animation->GetBoneAnimations();
    
    for (const auto& pair : boneAnimations) {
        const std::string& boneName = pair.first;
        const auto& boneAnim = pair.second;
        
        if (!boneAnim) continue;

        // Check for redundant keyframes in position tracks
        if (boneAnim->HasPositionTrack()) {
            // TODO: Implement redundant keyframe detection
            // This would require access to the actual keyframe data
        }

        // Check for redundant keyframes in rotation tracks
        if (boneAnim->HasRotationTrack()) {
            // TODO: Implement redundant keyframe detection
        }

        // Check for redundant keyframes in scale tracks
        if (boneAnim->HasScaleTrack()) {
            // TODO: Implement redundant keyframe detection
        }
    }
}

void AnimationValidator::ValidateBoneMapping(std::shared_ptr<SkeletalAnimation> animation,
                                            std::shared_ptr<AnimationSkeleton> skeleton,
                                            ValidationResult& result,
                                            const ValidationConfig& config) {
    auto animatedBones = animation->GetAnimatedBoneNames();
    auto skeletonBones = skeleton->GetBoneNames();
    
    std::unordered_set<std::string> skeletonBoneSet(skeletonBones.begin(), skeletonBones.end());

    // Check for animated bones that don't exist in skeleton
    for (const auto& boneName : animatedBones) {
        if (skeletonBoneSet.find(boneName) == skeletonBoneSet.end()) {
            AddIssue(result, ValidationIssueType::Warning, ValidationCategory::BoneMapping,
                    "Animation references bone not found in skeleton", boneName,
                    "Remove unused bone tracks or add missing bones to skeleton");
        }
    }

    // Check for skeleton bones that have no animation
    std::unordered_set<std::string> animatedBoneSet(animatedBones.begin(), animatedBones.end());
    size_t unanimatedBones = 0;
    
    for (const auto& boneName : skeletonBones) {
        if (animatedBoneSet.find(boneName) == animatedBoneSet.end()) {
            unanimatedBones++;
        }
    }

    if (unanimatedBones > 0) {
        AddIssue(result, ValidationIssueType::Warning, ValidationCategory::BoneMapping,
                std::to_string(unanimatedBones) + " skeleton bones have no animation data",
                animation->GetName(),
                "Consider adding animation tracks for all bones or verify this is intentional");
    }
}

void AnimationValidator::ValidatePerformance(std::shared_ptr<AnimationSkeleton> skeleton,
                                            std::shared_ptr<SkeletalAnimation> animation,
                                            ValidationResult& result,
                                            const ValidationConfig& config) {
    if (!config.validateMemoryUsage) return;

    // Estimate memory usage
    size_t skeletonMemory = skeleton ? (skeleton->GetBoneCount() * sizeof(Bone)) : 0;
    size_t animationMemory = animation ? animation->GetMemoryUsage() : 0;
    size_t totalMemory = skeletonMemory + animationMemory;

    // Warn about high memory usage (>10MB)
    const size_t highMemoryThreshold = 10 * 1024 * 1024; // 10MB
    if (totalMemory > highMemoryThreshold) {
        AddIssue(result, ValidationIssueType::Warning, ValidationCategory::Performance,
                "High memory usage detected", 
                "Total: " + std::to_string(totalMemory / 1024 / 1024) + "MB",
                "Consider using animation compression or optimization");
    }
}

bool AnimationValidator::FixSkeletonIssues(std::shared_ptr<AnimationSkeleton> skeleton,
                                          const std::vector<ValidationIssue>& issues) {
    bool allFixed = true;
    
    for (const auto& issue : issues) {
        bool fixed = false;
        
        if (issue.category == ValidationCategory::SkeletonHierarchy) {
            if (issue.description.find("no root bone") != std::string::npos) {
                // Try to set the first bone as root
                const auto& bones = skeleton->GetAllBones();
                if (!bones.empty() && bones[0]) {
                    skeleton->SetRootBone(bones[0]);
                    fixed = true;
                    LOG_INFO("Auto-fixed: Set '" + bones[0]->GetName() + "' as root bone");
                }
            }
        }
        
        if (!fixed) {
            allFixed = false;
            LOG_WARNING("Could not auto-fix issue: " + issue.description);
        }
    }
    
    return allFixed;
}

bool AnimationValidator::FixAnimationIssues(std::shared_ptr<SkeletalAnimation> animation,
                                           const std::vector<ValidationIssue>& issues) {
    bool allFixed = true;
    
    for (const auto& issue : issues) {
        bool fixed = false;
        
        if (issue.category == ValidationCategory::AnimationData) {
            if (issue.description.find("invalid frame rate") != std::string::npos) {
                animation->SetFrameRate(30.0f); // Set default frame rate
                fixed = true;
                LOG_INFO("Auto-fixed: Set frame rate to 30 FPS for animation '" + animation->GetName() + "'");
            }
            else if (issue.description.find("zero duration") != std::string::npos) {
                animation->RecalculateDuration();
                fixed = true;
                LOG_INFO("Auto-fixed: Recalculated duration for animation '" + animation->GetName() + "'");
            }
        }
        
        if (!fixed) {
            allFixed = false;
            LOG_WARNING("Could not auto-fix issue: " + issue.description);
        }
    }
    
    return allFixed;
}

void AnimationValidator::AddIssue(ValidationResult& result,
                                 ValidationIssueType type,
                                 ValidationCategory category,
                                 const std::string& description,
                                 const std::string& location,
                                 const std::string& suggestion,
                                 bool canAutoFix) {
    ValidationIssue issue;
    issue.type = type;
    issue.category = category;
    issue.description = description;
    issue.location = location;
    issue.suggestion = suggestion;
    issue.canAutoFix = canAutoFix;
    
    // Set severity based on type
    switch (type) {
        case ValidationIssueType::Warning: issue.severity = 0.3f; break;
        case ValidationIssueType::Error: issue.severity = 0.7f; break;
        case ValidationIssueType::Critical: issue.severity = 1.0f; break;
    }
    
    result.issues.push_back(issue);
}

void AnimationValidator::UpdateStatistics(ValidationResult& result) {
    result.warningCount = 0;
    result.errorCount = 0;
    result.criticalCount = 0;
    
    for (const auto& issue : result.issues) {
        switch (issue.type) {
            case ValidationIssueType::Warning: result.warningCount++; break;
            case ValidationIssueType::Error: result.errorCount++; break;
            case ValidationIssueType::Critical: result.criticalCount++; break;
        }
    }
}

bool AnimationValidator::IsValidBoneName(const std::string& name) const {
    if (name.empty()) return false;
    
    // Check if name contains only alphanumeric characters and underscores
    std::regex validNameRegex("^[a-zA-Z_][a-zA-Z0-9_]*$");
    return std::regex_match(name, validNameRegex);
}

bool AnimationValidator::HasCyclicDependency(std::shared_ptr<AnimationSkeleton> skeleton) const {
    // Simple cycle detection using DFS
    const auto& bones = skeleton->GetAllBones();
    std::unordered_set<int32_t> visited;
    std::unordered_set<int32_t> recursionStack;
    
    for (const auto& bone : bones) {
        if (!bone) continue;
        
        if (visited.find(bone->GetId()) == visited.end()) {
            if (HasCyclicDependencyDFS(bone, visited, recursionStack)) {
                return true;
            }
        }
    }
    
    return false;
}

bool AnimationValidator::HasCyclicDependencyDFS(std::shared_ptr<Bone> bone,
                                               std::unordered_set<int32_t>& visited,
                                               std::unordered_set<int32_t>& recursionStack) const {
    if (!bone) return false;
    
    int32_t boneId = bone->GetId();
    visited.insert(boneId);
    recursionStack.insert(boneId);
    
    // Check all children
    for (const auto& child : bone->GetChildren()) {
        if (!child) continue;
        
        int32_t childId = child->GetId();
        
        if (recursionStack.find(childId) != recursionStack.end()) {
            return true; // Cycle detected
        }
        
        if (visited.find(childId) == visited.end()) {
            if (HasCyclicDependencyDFS(child, visited, recursionStack)) {
                return true;
            }
        }
    }
    
    recursionStack.erase(boneId);
    return false;
}

bool AnimationValidator::AreKeyframesRedundant(const std::vector<float>& times,
                                              const std::vector<Math::Vec3>& values,
                                              float tolerance) const {
    if (times.size() != values.size() || times.size() < 3) {
        return false;
    }
    
    // Check if consecutive keyframes have similar values
    for (size_t i = 1; i < values.size() - 1; ++i) {
        Math::Vec3 diff1 = values[i] - values[i-1];
        Math::Vec3 diff2 = values[i+1] - values[i];
        
        float len1 = glm::length(diff1);
        float len2 = glm::length(diff2);
        
        if (len1 < tolerance && len2 < tolerance) {
            return true; // Found redundant keyframe
        }
    }
    
    return false;
}

} // namespace Animation
} // namespace GameEngine