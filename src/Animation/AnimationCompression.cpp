#include "Animation/AnimationCompression.h"
#include "Core/Logger.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace GameEngine {
namespace Animation {

    std::shared_ptr<Animation> AnimationCompressor::CompressAnimation(const Animation& original, 
                                                                     const CompressionSettings& settings) {
        LOG_INFO("Starting compression of animation '" + original.GetName() + "'");
        
        // Reset statistics
        ResetStats();
        m_lastStats.originalMemoryBytes = CalculateAnimationMemoryUsage(original);
        
        // Create compressed animation
        auto compressed = std::make_shared<Animation>(original.GetName() + "_compressed");
        compressed->SetDuration(original.GetDuration());
        compressed->SetFrameRate(original.GetFrameRate());
        compressed->SetLoopMode(original.GetLoopMode());
        
        // Compress each bone animation
        const auto& boneAnimations = original.GetBoneAnimations();
        for (const auto& [boneName, boneAnim] : boneAnimations) {
            if (!boneAnim->HasAnyTracks()) {
                continue;
            }
            
            // Create compressed bone animation
            auto* compressedBoneAnim = compressed->CreateBoneAnimation(boneName);
            
            // Compress position track
            if (boneAnim->HasPositionTrack()) {
                auto compressedPosTrack = CompressTrack(*boneAnim->positionTrack, settings);
                compressedBoneAnim->positionTrack = std::move(compressedPosTrack);
                
                // Update statistics
                m_lastStats.originalKeyframes += boneAnim->positionTrack->GetKeyframeCount();
                m_lastStats.compressedKeyframes += compressedBoneAnim->positionTrack->GetKeyframeCount();
            }
            
            // Compress rotation track
            if (boneAnim->HasRotationTrack()) {
                auto compressedRotTrack = CompressTrack(*boneAnim->rotationTrack, settings);
                compressedBoneAnim->rotationTrack = std::move(compressedRotTrack);
                
                // Update statistics
                m_lastStats.originalKeyframes += boneAnim->rotationTrack->GetKeyframeCount();
                m_lastStats.compressedKeyframes += compressedBoneAnim->rotationTrack->GetKeyframeCount();
            }
            
            // Compress scale track
            if (boneAnim->HasScaleTrack()) {
                auto compressedScaleTrack = CompressTrack(*boneAnim->scaleTrack, settings);
                compressedBoneAnim->scaleTrack = std::move(compressedScaleTrack);
                
                // Update statistics
                m_lastStats.originalKeyframes += boneAnim->scaleTrack->GetKeyframeCount();
                m_lastStats.compressedKeyframes += compressedBoneAnim->scaleTrack->GetKeyframeCount();
            }
        }
        
        // Calculate final statistics
        m_lastStats.compressedMemoryBytes = CalculateAnimationMemoryUsage(*compressed);
        m_lastStats.Calculate();
        
        LOG_INFO("Animation compression completed:");
        LOG_INFO("  Original keyframes: " + std::to_string(m_lastStats.originalKeyframes));
        LOG_INFO("  Compressed keyframes: " + std::to_string(m_lastStats.compressedKeyframes));
        LOG_INFO("  Compression ratio: " + std::to_string(m_lastStats.compressionRatio));
        LOG_INFO("  Memory reduction: " + std::to_string(m_lastStats.memoryReduction * 100.0f) + "%");
        
        return compressed;
    }

    template<typename T>
    std::unique_ptr<AnimationTrack<T>> AnimationCompressor::CompressTrack(const AnimationTrack<T>& original,
                                                                         const CompressionSettings& settings) {
        auto compressed = std::make_unique<AnimationTrack<T>>(original.GetTargetBone(), original.GetProperty());
        
        const auto& originalKeyframes = original.GetKeyframes();
        if (originalKeyframes.empty()) {
            return compressed;
        }
        
        std::vector<Keyframe<T>> optimizedKeyframes = originalKeyframes;
        
        // Apply keyframe reduction if enabled
        if (settings.enableKeyframeReduction) {
            float tolerance = GetToleranceForType<T>(settings);
            optimizedKeyframes = RemoveRedundantKeyframes(optimizedKeyframes, tolerance);
        }
        
        // Apply curve compression if enabled
        if (settings.enableCurveCompression) {
            float tolerance = GetToleranceForType<T>(settings);
            optimizedKeyframes = CompressCurve(optimizedKeyframes, tolerance);
        }
        
        // Add optimized keyframes to compressed track
        for (const auto& keyframe : optimizedKeyframes) {
            compressed->AddKeyframe(keyframe);
        }
        
        return compressed;
    }

    template<typename T>
    std::vector<Keyframe<T>> AnimationCompressor::OptimizeKeyframes(const std::vector<Keyframe<T>>& keyframes,
                                                                   float tolerance) {
        if (keyframes.size() <= 2) {
            return keyframes;
        }
        
        std::vector<Keyframe<T>> optimized;
        optimized.reserve(keyframes.size());
        
        // Always keep first keyframe
        optimized.push_back(keyframes[0]);
        
        for (size_t i = 1; i < keyframes.size() - 1; ++i) {
            const auto& prev = keyframes[i - 1];
            const auto& current = keyframes[i];
            const auto& next = keyframes[i + 1];
            
            // Check if current keyframe is redundant
            if (!IsKeyframeRedundant(prev, current, next, tolerance)) {
                optimized.push_back(current);
            }
        }
        
        // Always keep last keyframe
        optimized.push_back(keyframes.back());
        
        return optimized;
    }

    template<typename T>
    std::vector<Keyframe<T>> AnimationCompressor::RemoveRedundantKeyframes(const std::vector<Keyframe<T>>& keyframes,
                                                                          float tolerance) {
        if (keyframes.size() <= 2) {
            return keyframes;
        }
        
        std::vector<Keyframe<T>> result;
        result.reserve(keyframes.size());
        
        // Always keep first keyframe
        result.push_back(keyframes[0]);
        
        for (size_t i = 1; i < keyframes.size() - 1; ++i) {
            const auto& prev = result.back();
            const auto& current = keyframes[i];
            const auto& next = keyframes[i + 1];
            
            // Calculate interpolated value at current time
            float t = (current.time - prev.time) / (next.time - prev.time);
            T interpolated = InterpolateValue(prev, next, current.time);
            
            // Check if current value differs significantly from interpolated value
            float error = CalculateError(current.value, interpolated);
            if (error > tolerance) {
                result.push_back(current);
            }
        }
        
        // Always keep last keyframe
        result.push_back(keyframes.back());
        
        return result;
    }

    template<typename T>
    std::vector<Keyframe<T>> AnimationCompressor::CompressCurve(const std::vector<Keyframe<T>>& keyframes,
                                                               float tolerance) {
        if (keyframes.size() <= 2) {
            return keyframes;
        }
        
        // Use Douglas-Peucker algorithm for curve simplification
        AnimationCurveFitter fitter;
        return fitter.SimplifyCurve(keyframes, tolerance);
    }

    template<typename T>
    bool AnimationCompressor::IsKeyframeRedundant(const Keyframe<T>& prev, const Keyframe<T>& current, 
                                                 const Keyframe<T>& next, float tolerance) const {
        // Calculate interpolated value at current time
        float t = (current.time - prev.time) / (next.time - prev.time);
        T interpolated = InterpolateValue(prev, next, current.time);
        
        // Check if current value is close enough to interpolated value
        float error = CalculateError(current.value, interpolated);
        return error <= tolerance;
    }

    template<typename T>
    T AnimationCompressor::InterpolateValue(const Keyframe<T>& k1, const Keyframe<T>& k2, float time) const {
        if (k2.time <= k1.time) {
            return k1.value;
        }
        
        float t = (time - k1.time) / (k2.time - k1.time);
        t = std::clamp(t, 0.0f, 1.0f);
        
        return k1.value + t * (k2.value - k1.value);
    }

    // Specialized interpolation for quaternions
    template<>
    Math::Quat AnimationCompressor::InterpolateValue(const Keyframe<Math::Quat>& k1, 
                                                     const Keyframe<Math::Quat>& k2, float time) const {
        if (k2.time <= k1.time) {
            return k1.value;
        }
        
        float t = (time - k1.time) / (k2.time - k1.time);
        t = std::clamp(t, 0.0f, 1.0f);
        
        return glm::slerp(k1.value, k2.value, t);
    }

    template<typename T>
    float AnimationCompressor::CalculateError(const T& original, const T& compressed) const {
        // Generic error calculation (works for Vec3, float)
        auto diff = original - compressed;
        return glm::length(diff);
    }

    // Specialized error calculations
    float AnimationCompressor::CalculatePositionError(const Math::Vec3& original, const Math::Vec3& compressed) const {
        return glm::distance(original, compressed);
    }

    float AnimationCompressor::CalculateRotationError(const Math::Quat& original, const Math::Quat& compressed) const {
        // Calculate angular difference between quaternions
        float dot = glm::dot(original, compressed);
        dot = std::clamp(std::abs(dot), 0.0f, 1.0f);
        return std::acos(dot) * 2.0f; // Angular difference in radians
    }

    float AnimationCompressor::CalculateScaleError(const Math::Vec3& original, const Math::Vec3& compressed) const {
        return glm::distance(original, compressed);
    }

    template<>
    float AnimationCompressor::CalculateError(const Math::Vec3& original, const Math::Vec3& compressed) const {
        return CalculatePositionError(original, compressed);
    }

    template<>
    float AnimationCompressor::CalculateError(const Math::Quat& original, const Math::Quat& compressed) const {
        return CalculateRotationError(original, compressed);
    }

    template<>
    float AnimationCompressor::CalculateError(const float& original, const float& compressed) const {
        return std::abs(original - compressed);
    }

    template<typename T>
    size_t AnimationCompressor::CalculateTrackMemoryUsage(const AnimationTrack<T>& track) const {
        size_t baseSize = sizeof(AnimationTrack<T>);
        size_t keyframeSize = sizeof(Keyframe<T>) * track.GetKeyframeCount();
        size_t stringSize = track.GetTargetBone().size() + track.GetProperty().size();
        
        return baseSize + keyframeSize + stringSize;
    }

    size_t AnimationCompressor::CalculateAnimationMemoryUsage(const Animation& animation) const {
        size_t totalSize = sizeof(Animation);
        totalSize += animation.GetName().size();
        
        const auto& boneAnimations = animation.GetBoneAnimations();
        for (const auto& [boneName, boneAnim] : boneAnimations) {
            totalSize += sizeof(BoneAnimation);
            totalSize += boneName.size();
            
            if (boneAnim->HasPositionTrack()) {
                totalSize += CalculateTrackMemoryUsage(*boneAnim->positionTrack);
            }
            if (boneAnim->HasRotationTrack()) {
                totalSize += CalculateTrackMemoryUsage(*boneAnim->rotationTrack);
            }
            if (boneAnim->HasScaleTrack()) {
                totalSize += CalculateTrackMemoryUsage(*boneAnim->scaleTrack);
            }
        }
        
        return totalSize;
    }

    // Helper function to get tolerance for specific types
    template<typename T>
    float GetToleranceForType(const CompressionSettings& settings) {
        return settings.positionTolerance; // Default
    }

    template<>
    float GetToleranceForType<Math::Vec3>(const CompressionSettings& settings) {
        return settings.positionTolerance;
    }

    template<>
    float GetToleranceForType<Math::Quat>(const CompressionSettings& settings) {
        return settings.rotationTolerance;
    }

    template<>
    float GetToleranceForType<float>(const CompressionSettings& settings) {
        return settings.scaleTolerance;
    }

    // Explicit template instantiations
    template std::unique_ptr<AnimationTrack<Math::Vec3>> AnimationCompressor::CompressTrack(
        const AnimationTrack<Math::Vec3>& original, const CompressionSettings& settings);
    template std::unique_ptr<AnimationTrack<Math::Quat>> AnimationCompressor::CompressTrack(
        const AnimationTrack<Math::Quat>& original, const CompressionSettings& settings);
    template std::unique_ptr<AnimationTrack<float>> AnimationCompressor::CompressTrack(
        const AnimationTrack<float>& original, const CompressionSettings& settings);

    template std::vector<Keyframe<Math::Vec3>> AnimationCompressor::OptimizeKeyframes(
        const std::vector<Keyframe<Math::Vec3>>& keyframes, float tolerance);
    template std::vector<Keyframe<Math::Quat>> AnimationCompressor::OptimizeKeyframes(
        const std::vector<Keyframe<Math::Quat>>& keyframes, float tolerance);
    template std::vector<Keyframe<float>> AnimationCompressor::OptimizeKeyframes(
        const std::vector<Keyframe<float>>& keyframes, float tolerance);

    template std::vector<Keyframe<Math::Vec3>> AnimationCompressor::RemoveRedundantKeyframes(
        const std::vector<Keyframe<Math::Vec3>>& keyframes, float tolerance);
    template std::vector<Keyframe<Math::Quat>> AnimationCompressor::RemoveRedundantKeyframes(
        const std::vector<Keyframe<Math::Quat>>& keyframes, float tolerance);
    template std::vector<Keyframe<float>> AnimationCompressor::RemoveRedundantKeyframes(
        const std::vector<Keyframe<float>>& keyframes, float tolerance);

    template std::vector<Keyframe<Math::Vec3>> AnimationCompressor::CompressCurve(
        const std::vector<Keyframe<Math::Vec3>>& keyframes, float tolerance);
    template std::vector<Keyframe<Math::Quat>> AnimationCompressor::CompressCurve(
        const std::vector<Keyframe<Math::Quat>>& keyframes, float tolerance);
    template std::vector<Keyframe<float>> AnimationCompressor::CompressCurve(
        const std::vector<Keyframe<float>>& keyframes, float tolerance);

    // AnimationCurveFitter implementation
    template<typename T>
    std::vector<Keyframe<T>> AnimationCurveFitter::FitCurve(const std::vector<Keyframe<T>>& keyframes,
                                                           float tolerance, int maxIterations) {
        if (keyframes.size() <= 2) {
            return keyframes;
        }
        
        // Start with simplified curve using Douglas-Peucker
        return SimplifyCurve(keyframes, tolerance);
    }

    template<typename T>
    std::vector<Keyframe<T>> AnimationCurveFitter::SimplifyCurve(const std::vector<Keyframe<T>>& keyframes,
                                                                float tolerance) {
        if (keyframes.size() <= 2) {
            return keyframes;
        }
        
        // Find the point with maximum distance from line between first and last points
        float maxDistance = 0.0f;
        size_t maxIndex = 0;
        
        for (size_t i = 1; i < keyframes.size() - 1; ++i) {
            float distance = CalculatePointToLineDistance(keyframes[i], keyframes[0], keyframes.back());
            if (distance > maxDistance) {
                maxDistance = distance;
                maxIndex = i;
            }
        }
        
        // If max distance is greater than tolerance, split the curve
        if (maxDistance > tolerance) {
            // Split curve at max distance point
            auto [leftCurve, rightCurve] = SplitCurve(keyframes, maxIndex);
            
            // Recursively simplify both parts
            auto leftSimplified = SimplifyCurve(leftCurve, tolerance);
            auto rightSimplified = SimplifyCurve(rightCurve, tolerance);
            
            // Combine results (remove duplicate point at split)
            std::vector<Keyframe<T>> result = leftSimplified;
            result.insert(result.end(), rightSimplified.begin() + 1, rightSimplified.end());
            
            return result;
        } else {
            // All points are within tolerance, return just endpoints
            return {keyframes.front(), keyframes.back()};
        }
    }

    template<typename T>
    float AnimationCurveFitter::CalculatePointToLineDistance(const Keyframe<T>& point, 
                                                           const Keyframe<T>& lineStart,
                                                           const Keyframe<T>& lineEnd) const {
        // Calculate distance from point to line in time-value space
        float t1 = lineStart.time;
        float t2 = lineEnd.time;
        float tp = point.time;
        
        if (t2 <= t1) {
            return glm::length(point.value - lineStart.value);
        }
        
        // Interpolate line value at point time
        float t = (tp - t1) / (t2 - t1);
        T lineValue = lineStart.value + t * (lineEnd.value - lineStart.value);
        
        // Return distance between actual and interpolated values
        return glm::length(point.value - lineValue);
    }

    // Specialized distance calculation for quaternions
    template<>
    float AnimationCurveFitter::CalculatePointToLineDistance(const Keyframe<Math::Quat>& point, 
                                                           const Keyframe<Math::Quat>& lineStart,
                                                           const Keyframe<Math::Quat>& lineEnd) const {
        float t1 = lineStart.time;
        float t2 = lineEnd.time;
        float tp = point.time;
        
        if (t2 <= t1) {
            float dot = glm::dot(point.value, lineStart.value);
            return std::acos(std::clamp(std::abs(dot), 0.0f, 1.0f));
        }
        
        // Interpolate quaternion at point time
        float t = (tp - t1) / (t2 - t1);
        Math::Quat lineValue = glm::slerp(lineStart.value, lineEnd.value, t);
        
        // Return angular distance
        float dot = glm::dot(point.value, lineValue);
        return std::acos(std::clamp(std::abs(dot), 0.0f, 1.0f));
    }

    template<typename T>
    std::pair<std::vector<Keyframe<T>>, std::vector<Keyframe<T>>> 
    AnimationCurveFitter::SplitCurve(const std::vector<Keyframe<T>>& keyframes, size_t splitIndex) {
        std::vector<Keyframe<T>> leftCurve(keyframes.begin(), keyframes.begin() + splitIndex + 1);
        std::vector<Keyframe<T>> rightCurve(keyframes.begin() + splitIndex, keyframes.end());
        
        return {leftCurve, rightCurve};
    }

    // Explicit template instantiations for AnimationCurveFitter
    template std::vector<Keyframe<Math::Vec3>> AnimationCurveFitter::FitCurve(
        const std::vector<Keyframe<Math::Vec3>>& keyframes, float tolerance, int maxIterations);
    template std::vector<Keyframe<Math::Quat>> AnimationCurveFitter::FitCurve(
        const std::vector<Keyframe<Math::Quat>>& keyframes, float tolerance, int maxIterations);
    template std::vector<Keyframe<float>> AnimationCurveFitter::FitCurve(
        const std::vector<Keyframe<float>>& keyframes, float tolerance, int maxIterations);

    template std::vector<Keyframe<Math::Vec3>> AnimationCurveFitter::SimplifyCurve(
        const std::vector<Keyframe<Math::Vec3>>& keyframes, float tolerance);
    template std::vector<Keyframe<Math::Quat>> AnimationCurveFitter::SimplifyCurve(
        const std::vector<Keyframe<Math::Quat>>& keyframes, float tolerance);
    template std::vector<Keyframe<float>> AnimationCurveFitter::SimplifyCurve(
        const std::vector<Keyframe<float>>& keyframes, float tolerance);

    // AnimationDataSharer implementation
    void AnimationDataSharer::OptimizeAnimationSet(std::vector<std::shared_ptr<Animation>>& animations,
                                                   float similarityThreshold) {
        if (animations.size() < 2) {
            return;
        }
        
        LOG_INFO("Optimizing animation set with " + std::to_string(animations.size()) + " animations");
        
        // Collect all tracks by type
        std::vector<std::shared_ptr<PositionTrack>> positionTracks;
        std::vector<std::shared_ptr<RotationTrack>> rotationTracks;
        std::vector<std::shared_ptr<ScaleTrack>> scaleTracks;
        
        for (auto& animation : animations) {
            const auto& boneAnimations = animation->GetBoneAnimations();
            for (const auto& [boneName, boneAnim] : boneAnimations) {
                if (boneAnim->HasPositionTrack()) {
                    positionTracks.push_back(std::shared_ptr<PositionTrack>(boneAnim->positionTrack.get(), [](PositionTrack*){}));
                }
                if (boneAnim->HasRotationTrack()) {
                    rotationTracks.push_back(std::shared_ptr<RotationTrack>(boneAnim->rotationTrack.get(), [](RotationTrack*){}));
                }
                if (boneAnim->HasScaleTrack()) {
                    scaleTracks.push_back(std::shared_ptr<ScaleTrack>(boneAnim->scaleTrack.get(), [](ScaleTrack*){}));
                }
            }
        }
        
        // Find and share similar tracks
        auto positionPairs = FindSimilarTracks(positionTracks, similarityThreshold);
        auto rotationPairs = FindSimilarTracks(rotationTracks, similarityThreshold);
        auto scalePairs = FindSimilarTracks(scaleTracks, similarityThreshold);
        
        LOG_INFO("Found " + std::to_string(positionPairs.size()) + " similar position track pairs");
        LOG_INFO("Found " + std::to_string(rotationPairs.size()) + " similar rotation track pairs");
        LOG_INFO("Found " + std::to_string(scalePairs.size()) + " similar scale track pairs");
    }

    template<typename T>
    std::vector<std::pair<size_t, size_t>> AnimationDataSharer::FindSimilarTracks(
        const std::vector<std::shared_ptr<AnimationTrack<T>>>& tracks,
        float threshold) const {
        
        std::vector<std::pair<size_t, size_t>> similarPairs;
        
        for (size_t i = 0; i < tracks.size(); ++i) {
            for (size_t j = i + 1; j < tracks.size(); ++j) {
                float similarity = CalculateTrackSimilarity(*tracks[i], *tracks[j]);
                if (similarity >= threshold) {
                    similarPairs.emplace_back(i, j);
                }
            }
        }
        
        return similarPairs;
    }

    template<typename T>
    float AnimationDataSharer::CalculateTrackSimilarity(const AnimationTrack<T>& track1,
                                                       const AnimationTrack<T>& track2) const {
        const auto& keyframes1 = track1.GetKeyframes();
        const auto& keyframes2 = track2.GetKeyframes();
        
        if (keyframes1.empty() || keyframes2.empty()) {
            return 0.0f;
        }
        
        // Sample both tracks at regular intervals and compare
        float startTime = std::max(track1.GetStartTime(), track2.GetStartTime());
        float endTime = std::min(track1.GetEndTime(), track2.GetEndTime());
        
        if (endTime <= startTime) {
            return 0.0f;
        }
        
        const int sampleCount = 20; // Number of samples to compare
        float timeStep = (endTime - startTime) / (sampleCount - 1);
        
        float totalError = 0.0f;
        for (int i = 0; i < sampleCount; ++i) {
            float time = startTime + i * timeStep;
            
            T value1 = track1.SampleAt(time);
            T value2 = track2.SampleAt(time);
            
            float error = glm::length(value1 - value2);
            totalError += error;
        }
        
        float averageError = totalError / sampleCount;
        
        // Convert error to similarity (0 = no similarity, 1 = identical)
        // This is a simple heuristic - could be improved based on data type
        float maxExpectedError = 1.0f; // Adjust based on data type
        float similarity = std::max(0.0f, 1.0f - (averageError / maxExpectedError));
        
        return similarity;
    }

    // Specialized similarity calculation for quaternions
    template<>
    float AnimationDataSharer::CalculateTrackSimilarity(const AnimationTrack<Math::Quat>& track1,
                                                       const AnimationTrack<Math::Quat>& track2) const {
        const auto& keyframes1 = track1.GetKeyframes();
        const auto& keyframes2 = track2.GetKeyframes();
        
        if (keyframes1.empty() || keyframes2.empty()) {
            return 0.0f;
        }
        
        float startTime = std::max(track1.GetStartTime(), track2.GetStartTime());
        float endTime = std::min(track1.GetEndTime(), track2.GetEndTime());
        
        if (endTime <= startTime) {
            return 0.0f;
        }
        
        const int sampleCount = 20;
        float timeStep = (endTime - startTime) / (sampleCount - 1);
        
        float totalAngularError = 0.0f;
        for (int i = 0; i < sampleCount; ++i) {
            float time = startTime + i * timeStep;
            
            Math::Quat quat1 = track1.SampleAt(time);
            Math::Quat quat2 = track2.SampleAt(time);
            
            float dot = glm::dot(quat1, quat2);
            float angularError = std::acos(std::clamp(std::abs(dot), 0.0f, 1.0f));
            totalAngularError += angularError;
        }
        
        float averageAngularError = totalAngularError / sampleCount;
        
        // Convert angular error to similarity
        float maxAngularError = 3.14159f; // 180 degrees
        float similarity = std::max(0.0f, 1.0f - (averageAngularError / maxAngularError));
        
        return similarity;
    }

    template<typename T>
    size_t AnimationDataSharer::CalculateTrackHash(const AnimationTrack<T>& track) const {
        size_t hash = 0;
        
        const auto& keyframes = track.GetKeyframes();
        for (const auto& keyframe : keyframes) {
            // Simple hash combining time and value
            hash ^= std::hash<float>{}(keyframe.time) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            
            // Hash value components (this is simplified - real implementation would be type-specific)
            if constexpr (std::is_same_v<T, Math::Vec3>) {
                hash ^= std::hash<float>{}(keyframe.value.x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                hash ^= std::hash<float>{}(keyframe.value.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                hash ^= std::hash<float>{}(keyframe.value.z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            } else if constexpr (std::is_same_v<T, Math::Quat>) {
                hash ^= std::hash<float>{}(keyframe.value.x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                hash ^= std::hash<float>{}(keyframe.value.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                hash ^= std::hash<float>{}(keyframe.value.z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                hash ^= std::hash<float>{}(keyframe.value.w) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            } else if constexpr (std::is_same_v<T, float>) {
                hash ^= std::hash<float>{}(keyframe.value) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
        }
        
        return hash;
    }

    // Explicit template instantiations for AnimationDataSharer
    template std::vector<std::pair<size_t, size_t>> AnimationDataSharer::FindSimilarTracks(
        const std::vector<std::shared_ptr<AnimationTrack<Math::Vec3>>>& tracks, float threshold) const;
    template std::vector<std::pair<size_t, size_t>> AnimationDataSharer::FindSimilarTracks(
        const std::vector<std::shared_ptr<AnimationTrack<Math::Quat>>>& tracks, float threshold) const;
    template std::vector<std::pair<size_t, size_t>> AnimationDataSharer::FindSimilarTracks(
        const std::vector<std::shared_ptr<AnimationTrack<float>>>& tracks, float threshold) const;

    template float AnimationDataSharer::CalculateTrackSimilarity(
        const AnimationTrack<Math::Vec3>& track1, const AnimationTrack<Math::Vec3>& track2) const;
    template float AnimationDataSharer::CalculateTrackSimilarity(
        const AnimationTrack<float>& track1, const AnimationTrack<float>& track2) const;

    template size_t AnimationDataSharer::CalculateTrackHash(const AnimationTrack<Math::Vec3>& track) const;
    template size_t AnimationDataSharer::CalculateTrackHash(const AnimationTrack<Math::Quat>& track) const;
    template size_t AnimationDataSharer::CalculateTrackHash(const AnimationTrack<float>& track) const;

} // namespace Animation
} // namespace GameEngine