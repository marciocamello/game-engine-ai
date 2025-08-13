#pragma once

#include "Core/Math.h"
#include <vector>

namespace GameEngine {
namespace Animation {

    /**
     * Interpolation types for keyframe animation
     */
    enum class InterpolationType {
        Linear,     // Linear interpolation
        Step,       // No interpolation (step function)
        Cubic,      // Cubic spline interpolation
        Bezier      // Bezier curve interpolation
    };

    /**
     * Template keyframe structure for different data types
     */
    template<typename T>
    struct Keyframe {
        float time;                    // Time in seconds
        T value;                       // Keyframe value
        InterpolationType interpolation = InterpolationType::Linear;
        
        // Tangent data for cubic/bezier interpolation
        T inTangent{};
        T outTangent{};

        Keyframe() : time(0.0f) {}
        Keyframe(float t, const T& v) : time(t), value(v) {}
        Keyframe(float t, const T& v, InterpolationType interp) 
            : time(t), value(v), interpolation(interp) {}
    };

    // Common keyframe types
    using PositionKeyframe = Keyframe<Math::Vec3>;
    using RotationKeyframe = Keyframe<Math::Quat>;
    using ScaleKeyframe = Keyframe<Math::Vec3>;
    using FloatKeyframe = Keyframe<float>;

    /**
     * Animation track containing keyframes for a specific property
     */
    template<typename T>
    class AnimationTrack {
    public:
        AnimationTrack(const std::string& targetBone = "", const std::string& property = "")
            : m_targetBone(targetBone), m_property(property) {}

        // Keyframe management
        void AddKeyframe(const Keyframe<T>& keyframe);
        void AddKeyframe(float time, const T& value, InterpolationType interpolation = InterpolationType::Linear);
        void RemoveKeyframe(size_t index);
        void ClearKeyframes() { m_keyframes.clear(); }

        // Keyframe access
        const std::vector<Keyframe<T>>& GetKeyframes() const { return m_keyframes; }
        size_t GetKeyframeCount() const { return m_keyframes.size(); }
        bool IsEmpty() const { return m_keyframes.empty(); }

        // Time range
        float GetStartTime() const;
        float GetEndTime() const;
        float GetDuration() const { return GetEndTime() - GetStartTime(); }

        // Sampling
        T SampleAt(float time) const;
        T SampleAtNormalized(float normalizedTime) const; // 0.0 to 1.0

        // Target information
        const std::string& GetTargetBone() const { return m_targetBone; }
        const std::string& GetProperty() const { return m_property; }
        void SetTargetBone(const std::string& bone) { m_targetBone = bone; }
        void SetProperty(const std::string& property) { m_property = property; }

        // Optimization
        void OptimizeKeyframes(float tolerance = 0.001f);
        void SortKeyframes();

    private:
        std::vector<Keyframe<T>> m_keyframes;
        std::string m_targetBone;
        std::string m_property;

        // Helper methods
        size_t FindKeyframeIndex(float time) const;
        T InterpolateLinear(const Keyframe<T>& k1, const Keyframe<T>& k2, float t) const;
        T InterpolateCubic(const Keyframe<T>& k0, const Keyframe<T>& k1, 
                          const Keyframe<T>& k2, const Keyframe<T>& k3, float t) const;
    };

    // Specialized interpolation functions
    template<>
    Math::Quat AnimationTrack<Math::Quat>::InterpolateLinear(const Keyframe<Math::Quat>& k1, 
                                                             const Keyframe<Math::Quat>& k2, float t) const;

    // Common track types
    using PositionTrack = AnimationTrack<Math::Vec3>;
    using RotationTrack = AnimationTrack<Math::Quat>;
    using ScaleTrack = AnimationTrack<Math::Vec3>;
    using FloatTrack = AnimationTrack<float>;

} // namespace Animation
} // namespace GameEngine