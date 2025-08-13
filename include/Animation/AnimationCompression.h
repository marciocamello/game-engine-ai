#pragma once

#include "Animation/SkeletalAnimation.h"
#include "Animation/Keyframe.h"
#include "Core/Math.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace GameEngine {
namespace Animation {

    /**
     * Compression settings for animation optimization
     */
    struct CompressionSettings {
        float positionTolerance = 0.001f;      // Position tolerance in units
        float rotationTolerance = 0.001f;      // Rotation tolerance in radians
        float scaleTolerance = 0.001f;         // Scale tolerance in units
        float timeTolerance = 0.001f;          // Time tolerance in seconds
        
        bool enableKeyframeReduction = true;   // Remove redundant keyframes
        bool enableCurveCompression = true;    // Compress animation curves
        bool enableQuantization = false;       // Quantize keyframe values (lossy)
        
        // Quantization settings (when enabled)
        int positionBits = 16;                 // Bits per position component
        int rotationBits = 16;                 // Bits per rotation component
        int scaleBits = 16;                    // Bits per scale component
        int timeBits = 16;                     // Bits for time values
    };

    /**
     * Statistics about animation compression
     */
    struct CompressionStats {
        size_t originalKeyframes = 0;
        size_t compressedKeyframes = 0;
        size_t originalMemoryBytes = 0;
        size_t compressedMemoryBytes = 0;
        float compressionRatio = 0.0f;
        float memoryReduction = 0.0f;
        
        void Calculate() {
            if (originalKeyframes > 0) {
                compressionRatio = static_cast<float>(compressedKeyframes) / static_cast<float>(originalKeyframes);
            }
            if (originalMemoryBytes > 0) {
                memoryReduction = 1.0f - (static_cast<float>(compressedMemoryBytes) / static_cast<float>(originalMemoryBytes));
            }
        }
    };

    /**
     * Compressed keyframe data structure
     */
    template<typename T>
    struct CompressedKeyframe {
        float time;
        T value;
        InterpolationType interpolation = InterpolationType::Linear;
        
        // Compression metadata
        bool isQuantized = false;
        uint32_t quantizedData = 0;
        
        CompressedKeyframe() = default;
        CompressedKeyframe(const Keyframe<T>& original) 
            : time(original.time), value(original.value), interpolation(original.interpolation) {}
    };

    /**
     * Animation compression utilities
     */
    class AnimationCompressor {
    public:
        AnimationCompressor() = default;
        ~AnimationCompressor() = default;

        // Main compression interface
        std::shared_ptr<SkeletalAnimation> CompressAnimation(const SkeletalAnimation& original, 
                                                   const CompressionSettings& settings = CompressionSettings{});
        
        // Individual track compression
        template<typename T>
        std::unique_ptr<AnimationTrack<T>> CompressTrack(const AnimationTrack<T>& original,
                                                        const CompressionSettings& settings);

        // Keyframe optimization
        template<typename T>
        std::vector<Keyframe<T>> OptimizeKeyframes(const std::vector<Keyframe<T>>& keyframes,
                                                  float tolerance);

        // Redundant keyframe removal
        template<typename T>
        std::vector<Keyframe<T>> RemoveRedundantKeyframes(const std::vector<Keyframe<T>>& keyframes,
                                                         float tolerance);

        // Curve compression
        template<typename T>
        std::vector<Keyframe<T>> CompressCurve(const std::vector<Keyframe<T>>& keyframes,
                                              float tolerance);

        // Statistics
        const CompressionStats& GetLastCompressionStats() const { return m_lastStats; }
        void ResetStats() { m_lastStats = CompressionStats{}; }

    private:
        CompressionStats m_lastStats;

        // Helper methods
        template<typename T>
        bool IsKeyframeRedundant(const Keyframe<T>& prev, const Keyframe<T>& current, 
                                const Keyframe<T>& next, float tolerance) const;

        template<typename T>
        T InterpolateValue(const Keyframe<T>& k1, const Keyframe<T>& k2, float time) const;

        template<typename T>
        float CalculateError(const T& original, const T& compressed) const;

        // Specialized error calculation for different types
        float CalculatePositionError(const Math::Vec3& original, const Math::Vec3& compressed) const;
        float CalculateRotationError(const Math::Quat& original, const Math::Quat& compressed) const;
        float CalculateScaleError(const Math::Vec3& original, const Math::Vec3& compressed) const;

        // Memory calculation
        template<typename T>
        size_t CalculateTrackMemoryUsage(const AnimationTrack<T>& track) const;
        
        size_t CalculateAnimationMemoryUsage(const SkeletalAnimation& animation) const;
    };

    /**
     * Animation curve fitting for compression
     */
    class AnimationCurveFitter {
    public:
        AnimationCurveFitter() = default;
        ~AnimationCurveFitter() = default;

        // Fit curves to keyframe data
        template<typename T>
        std::vector<Keyframe<T>> FitCurve(const std::vector<Keyframe<T>>& keyframes,
                                         float tolerance, int maxIterations = 10);

        // Douglas-Peucker algorithm for curve simplification
        template<typename T>
        std::vector<Keyframe<T>> SimplifyCurve(const std::vector<Keyframe<T>>& keyframes,
                                              float tolerance);

    private:
        // Helper methods for curve fitting
        template<typename T>
        float CalculatePointToLineDistance(const Keyframe<T>& point, 
                                          const Keyframe<T>& lineStart,
                                          const Keyframe<T>& lineEnd) const;

        template<typename T>
        std::pair<std::vector<Keyframe<T>>, std::vector<Keyframe<T>>> 
        SplitCurve(const std::vector<Keyframe<T>>& keyframes, size_t splitIndex);
    };

    /**
     * Animation data sharing for memory optimization
     */
    class AnimationDataSharer {
    public:
        AnimationDataSharer() = default;
        ~AnimationDataSharer() = default;

        // Share similar animation data between animations
        void OptimizeAnimationSet(std::vector<std::shared_ptr<SkeletalAnimation>>& animations,
                                 float similarityThreshold = 0.95f);

        // Find similar tracks between animations
        template<typename T>
        std::vector<std::pair<size_t, size_t>> FindSimilarTracks(
            const std::vector<std::shared_ptr<AnimationTrack<T>>>& tracks,
            float threshold) const;

        // Calculate similarity between tracks
        template<typename T>
        float CalculateTrackSimilarity(const AnimationTrack<T>& track1,
                                      const AnimationTrack<T>& track2) const;

    private:
        // Shared data storage
        std::unordered_map<size_t, std::shared_ptr<void>> m_sharedData;
        
        // Hash calculation for tracks
        template<typename T>
        size_t CalculateTrackHash(const AnimationTrack<T>& track) const;
    };

} // namespace Animation
} // namespace GameEngine