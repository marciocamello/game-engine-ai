#include "Animation/Keyframe.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {
namespace Animation {

    template<typename T>
    void AnimationTrack<T>::AddKeyframe(const Keyframe<T>& keyframe) {
        m_keyframes.push_back(keyframe);
        SortKeyframes();
    }

    template<typename T>
    void AnimationTrack<T>::AddKeyframe(float time, const T& value, InterpolationType interpolation) {
        Keyframe<T> keyframe(time, value, interpolation);
        AddKeyframe(keyframe);
    }

    template<typename T>
    void AnimationTrack<T>::RemoveKeyframe(size_t index) {
        if (index < m_keyframes.size()) {
            m_keyframes.erase(m_keyframes.begin() + index);
        }
    }

    template<typename T>
    float AnimationTrack<T>::GetStartTime() const {
        if (m_keyframes.empty()) {
            return 0.0f;
        }
        return m_keyframes.front().time;
    }

    template<typename T>
    float AnimationTrack<T>::GetEndTime() const {
        if (m_keyframes.empty()) {
            return 0.0f;
        }
        return m_keyframes.back().time;
    }

    template<typename T>
    T AnimationTrack<T>::SampleAt(float time) const {
        if (m_keyframes.empty()) {
            return T{};
        }

        // Clamp time to valid range
        if (time <= GetStartTime()) {
            return m_keyframes.front().value;
        }
        if (time >= GetEndTime()) {
            return m_keyframes.back().value;
        }

        // Find surrounding keyframes
        size_t index = FindKeyframeIndex(time);
        if (index >= m_keyframes.size() - 1) {
            return m_keyframes.back().value;
        }

        const auto& k1 = m_keyframes[index];
        const auto& k2 = m_keyframes[index + 1];

        // Calculate interpolation factor
        float t = (time - k1.time) / (k2.time - k1.time);

        // Interpolate based on type
        switch (k1.interpolation) {
            case InterpolationType::Step:
                return k1.value;
            
            case InterpolationType::Linear:
                return InterpolateLinear(k1, k2, t);
            
            case InterpolationType::Cubic:
                // Need 4 points for cubic interpolation
                if (index > 0 && index < m_keyframes.size() - 2) {
                    return InterpolateCubic(m_keyframes[index - 1], k1, k2, m_keyframes[index + 2], t);
                } else {
                    return InterpolateLinear(k1, k2, t);
                }
            
            case InterpolationType::Bezier:
                // For now, fall back to linear (Bezier requires tangent implementation)
                return InterpolateLinear(k1, k2, t);
            
            default:
                return InterpolateLinear(k1, k2, t);
        }
    }

    template<typename T>
    T AnimationTrack<T>::SampleAtNormalized(float normalizedTime) const {
        float duration = GetDuration();
        if (duration <= 0.0f) {
            return m_keyframes.empty() ? T{} : m_keyframes.front().value;
        }
        
        float time = GetStartTime() + normalizedTime * duration;
        return SampleAt(time);
    }

    template<typename T>
    void AnimationTrack<T>::OptimizeKeyframes(float tolerance) {
        if (m_keyframes.size() <= 2) {
            return;
        }

        std::vector<Keyframe<T>> optimized;
        optimized.push_back(m_keyframes.front());

        for (size_t i = 1; i < m_keyframes.size() - 1; ++i) {
            const auto& prev = m_keyframes[i - 1];
            const auto& current = m_keyframes[i];
            const auto& next = m_keyframes[i + 1];

            // Sample the linear interpolation between prev and next at current time
            float t = (current.time - prev.time) / (next.time - prev.time);
            T interpolated = InterpolateLinear(prev, next, t);

            // Check if current keyframe is significantly different from interpolated value
            // This is a simplified check - in practice, you'd want type-specific distance functions
            bool keepKeyframe = true;
            
            // For now, always keep keyframes (proper optimization would need type-specific logic)
            if (keepKeyframe) {
                optimized.push_back(current);
            }
        }

        optimized.push_back(m_keyframes.back());
        m_keyframes = std::move(optimized);
    }

    template<typename T>
    void AnimationTrack<T>::SortKeyframes() {
        std::sort(m_keyframes.begin(), m_keyframes.end(), 
                  [](const Keyframe<T>& a, const Keyframe<T>& b) {
                      return a.time < b.time;
                  });
    }

    template<typename T>
    size_t AnimationTrack<T>::FindKeyframeIndex(float time) const {
        for (size_t i = 0; i < m_keyframes.size(); ++i) {
            if (m_keyframes[i].time > time) {
                return i > 0 ? i - 1 : 0;
            }
        }
        return m_keyframes.size() > 0 ? m_keyframes.size() - 1 : 0;
    }

    template<typename T>
    T AnimationTrack<T>::InterpolateLinear(const Keyframe<T>& k1, const Keyframe<T>& k2, float t) const {
        // Generic linear interpolation (works for Vec3, float, etc.)
        return k1.value + t * (k2.value - k1.value);
    }

    template<typename T>
    T AnimationTrack<T>::InterpolateCubic(const Keyframe<T>& k0, const Keyframe<T>& k1, 
                                         const Keyframe<T>& k2, const Keyframe<T>& k3, float t) const {
        // Catmull-Rom spline interpolation
        float t2 = t * t;
        float t3 = t2 * t;

        T result = k0.value * (-0.5f * t3 + t2 - 0.5f * t) +
                   k1.value * (1.5f * t3 - 2.5f * t2 + 1.0f) +
                   k2.value * (-1.5f * t3 + 2.0f * t2 + 0.5f * t) +
                   k3.value * (0.5f * t3 - 0.5f * t2);

        return result;
    }

    // Specialized quaternion interpolation
    template<>
    Math::Quat AnimationTrack<Math::Quat>::InterpolateLinear(const Keyframe<Math::Quat>& k1, 
                                                             const Keyframe<Math::Quat>& k2, float t) const {
        return glm::slerp(k1.value, k2.value, t);
    }

    // Explicit template instantiations
    template class AnimationTrack<Math::Vec3>;
    template class AnimationTrack<Math::Quat>;
    template class AnimationTrack<float>;

} // namespace Animation
} // namespace GameEngine