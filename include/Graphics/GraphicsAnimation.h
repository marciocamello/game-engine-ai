#pragma once

#include "Core/Math.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace GameEngine {
namespace Graphics {

    /**
     * @brief Interpolation types for animation keyframes
     */
    enum class InterpolationType {
        Linear,
        Step,
        CubicSpline
    };

    /**
     * @brief Animation channel target types
     */
    enum class AnimationTarget {
        Translation,
        Rotation,
        Scale,
        Weights // For morph targets
    };

    /**
     * @brief Keyframe data for different animation properties
     */
    template<typename T>
    struct Keyframe {
        float time;
        T value;
        T inTangent;  // For cubic spline interpolation
        T outTangent; // For cubic spline interpolation
        
        Keyframe() : time(0.0f) {}
        Keyframe(float t, const T& v) : time(t), value(v) {}
        Keyframe(float t, const T& v, const T& inTan, const T& outTan) 
            : time(t), value(v), inTangent(inTan), outTangent(outTan) {}
    };

    /**
     * @brief Animation sampler containing keyframe data and interpolation method
     */
    template<typename T>
    class AnimationSampler {
    public:
        AnimationSampler() = default;
        ~AnimationSampler() = default;

        void SetInterpolationType(InterpolationType type) { m_interpolationType = type; }
        InterpolationType GetInterpolationType() const { return m_interpolationType; }

        void AddKeyframe(const Keyframe<T>& keyframe);
        void SetKeyframes(const std::vector<Keyframe<T>>& keyframes);
        const std::vector<Keyframe<T>>& GetKeyframes() const { return m_keyframes; }

        T Sample(float time) const;
        float GetDuration() const;
        bool IsEmpty() const { return m_keyframes.empty(); }

    private:
        std::vector<Keyframe<T>> m_keyframes;
        InterpolationType m_interpolationType = InterpolationType::Linear;

        size_t FindKeyframeIndex(float time) const;
        T InterpolateLinear(const Keyframe<T>& k1, const Keyframe<T>& k2, float t) const;
        T InterpolateStep(const Keyframe<T>& k1, const Keyframe<T>& k2, float t) const;
        T InterpolateCubicSpline(const Keyframe<T>& k1, const Keyframe<T>& k2, float t) const;
    };

    /**
     * @brief Animation channel targeting a specific node and property
     */
    class AnimationChannel {
    public:
        AnimationChannel() = default;
        ~AnimationChannel() = default;

        void SetTargetNode(uint32_t nodeIndex) { m_targetNodeIndex = nodeIndex; }
        uint32_t GetTargetNode() const { return m_targetNodeIndex; }

        void SetTargetProperty(AnimationTarget target) { m_targetProperty = target; }
        AnimationTarget GetTargetProperty() const { return m_targetProperty; }

        void SetTranslationSampler(std::shared_ptr<AnimationSampler<Math::Vec3>> sampler) { m_translationSampler = sampler; }
        void SetRotationSampler(std::shared_ptr<AnimationSampler<Math::Quat>> sampler) { m_rotationSampler = sampler; }
        void SetScaleSampler(std::shared_ptr<AnimationSampler<Math::Vec3>> sampler) { m_scaleSampler = sampler; }
        void SetWeightsSampler(std::shared_ptr<AnimationSampler<std::vector<float>>> sampler) { m_weightsSampler = sampler; }

        std::shared_ptr<AnimationSampler<Math::Vec3>> GetTranslationSampler() const { return m_translationSampler; }
        std::shared_ptr<AnimationSampler<Math::Quat>> GetRotationSampler() const { return m_rotationSampler; }
        std::shared_ptr<AnimationSampler<Math::Vec3>> GetScaleSampler() const { return m_scaleSampler; }
        std::shared_ptr<AnimationSampler<std::vector<float>>> GetWeightsSampler() const { return m_weightsSampler; }

        Math::Vec3 SampleTranslation(float time) const;
        Math::Quat SampleRotation(float time) const;
        Math::Vec3 SampleScale(float time) const;
        std::vector<float> SampleWeights(float time) const;

        float GetDuration() const;

    private:
        uint32_t m_targetNodeIndex = 0;
        AnimationTarget m_targetProperty = AnimationTarget::Translation;

        std::shared_ptr<AnimationSampler<Math::Vec3>> m_translationSampler;
        std::shared_ptr<AnimationSampler<Math::Quat>> m_rotationSampler;
        std::shared_ptr<AnimationSampler<Math::Vec3>> m_scaleSampler;
        std::shared_ptr<AnimationSampler<std::vector<float>>> m_weightsSampler;
    };

    /**
     * @brief Complete graphics animation containing multiple channels
     */
    class GraphicsAnimation {
    public:
        GraphicsAnimation(const std::string& name = "");
        ~GraphicsAnimation() = default;

        void SetName(const std::string& name) { m_name = name; }
        const std::string& GetName() const { return m_name; }

        void AddChannel(std::shared_ptr<AnimationChannel> channel);
        void SetChannels(const std::vector<std::shared_ptr<AnimationChannel>>& channels);
        const std::vector<std::shared_ptr<AnimationChannel>>& GetChannels() const { return m_channels; }

        float GetDuration() const;
        size_t GetChannelCount() const { return m_channels.size(); }

        // Animation playback state
        void SetCurrentTime(float time) { m_currentTime = time; }
        float GetCurrentTime() const { return m_currentTime; }

        void SetLooping(bool loop) { m_looping = loop; }
        bool IsLooping() const { return m_looping; }

        void SetPlaybackSpeed(float speed) { m_playbackSpeed = speed; }
        float GetPlaybackSpeed() const { return m_playbackSpeed; }

        // Update animation state
        void Update(float deltaTime);
        void Reset();

        // Apply animation to scene nodes
        void ApplyToNodes(const std::vector<std::shared_ptr<class GameEngine::ModelNode>>& nodes) const;

    private:
        std::string m_name;
        std::vector<std::shared_ptr<AnimationChannel>> m_channels;

        // Playback state
        float m_currentTime = 0.0f;
        float m_playbackSpeed = 1.0f;
        bool m_looping = true;
    };

    // Type aliases for common sampler types
    using Vec3Sampler = AnimationSampler<Math::Vec3>;
    using QuatSampler = AnimationSampler<Math::Quat>;
    using FloatSampler = AnimationSampler<float>;
    using WeightsSampler = AnimationSampler<std::vector<float>>;

} // namespace Graphics
} // namespace GameEngine