#include "Graphics/Animation.h"
#include "Graphics/ModelNode.h"
#include "Core/Logger.h"
#include <algorithm>

namespace GameEngine {

// AnimationSampler implementation
template<typename T>
void AnimationSampler<T>::AddKeyframe(const Keyframe<T>& keyframe) {
    m_keyframes.push_back(keyframe);
    // Keep keyframes sorted by time
    std::sort(m_keyframes.begin(), m_keyframes.end(), 
              [](const Keyframe<T>& a, const Keyframe<T>& b) {
                  return a.time < b.time;
              });
}

template<typename T>
void AnimationSampler<T>::SetKeyframes(const std::vector<Keyframe<T>>& keyframes) {
    m_keyframes = keyframes;
    // Ensure keyframes are sorted by time
    std::sort(m_keyframes.begin(), m_keyframes.end(), 
              [](const Keyframe<T>& a, const Keyframe<T>& b) {
                  return a.time < b.time;
              });
}

template<typename T>
T AnimationSampler<T>::Sample(float time) const {
    if (m_keyframes.empty()) {
        return T{};
    }
    
    if (m_keyframes.size() == 1) {
        return m_keyframes[0].value;
    }
    
    // Clamp time to animation range
    if (time <= m_keyframes.front().time) {
        return m_keyframes.front().value;
    }
    if (time >= m_keyframes.back().time) {
        return m_keyframes.back().value;
    }
    
    // Find keyframe pair for interpolation
    size_t index = FindKeyframeIndex(time);
    if (index >= m_keyframes.size() - 1) {
        return m_keyframes.back().value;
    }
    
    const auto& k1 = m_keyframes[index];
    const auto& k2 = m_keyframes[index + 1];
    
    float t = (time - k1.time) / (k2.time - k1.time);
    
    switch (m_interpolationType) {
        case InterpolationType::Linear:
            return InterpolateLinear(k1, k2, t);
        case InterpolationType::Step:
            return InterpolateStep(k1, k2, t);
        case InterpolationType::CubicSpline:
            return InterpolateCubicSpline(k1, k2, t);
        default:
            return InterpolateLinear(k1, k2, t);
    }
}

template<typename T>
float AnimationSampler<T>::GetDuration() const {
    if (m_keyframes.empty()) {
        return 0.0f;
    }
    return m_keyframes.back().time - m_keyframes.front().time;
}

template<typename T>
size_t AnimationSampler<T>::FindKeyframeIndex(float time) const {
    for (size_t i = 0; i < m_keyframes.size() - 1; ++i) {
        if (time >= m_keyframes[i].time && time < m_keyframes[i + 1].time) {
            return i;
        }
    }
    return m_keyframes.size() - 1;
}

template<typename T>
T AnimationSampler<T>::InterpolateLinear(const Keyframe<T>& k1, const Keyframe<T>& k2, float t) const {
    if constexpr (std::is_same_v<T, std::vector<float>>) {
        // Special handling for vector<float> - element-wise interpolation
        T result = k1.value;
        size_t size = std::min(k1.value.size(), k2.value.size());
        result.resize(size);
        
        for (size_t i = 0; i < size; ++i) {
            result[i] = k1.value[i] * (1.0f - t) + k2.value[i] * t;
        }
        return result;
    } else {
        return glm::mix(k1.value, k2.value, t);
    }
}

template<typename T>
T AnimationSampler<T>::InterpolateStep(const Keyframe<T>& k1, const Keyframe<T>& k2, float t) const {
    return k1.value; // Step interpolation returns the first keyframe value
}

template<typename T>
T AnimationSampler<T>::InterpolateCubicSpline(const Keyframe<T>& k1, const Keyframe<T>& k2, float t) const {
    // Cubic spline interpolation using Hermite interpolation
    float t2 = t * t;
    float t3 = t2 * t;
    
    float h1 = 2.0f * t3 - 3.0f * t2 + 1.0f;
    float h2 = -2.0f * t3 + 3.0f * t2;
    float h3 = t3 - 2.0f * t2 + t;
    float h4 = t3 - t2;
    
    float dt = k2.time - k1.time;
    
    if constexpr (std::is_same_v<T, std::vector<float>>) {
        // Special handling for vector<float> - element-wise interpolation
        T result = k1.value;
        size_t size = std::min({k1.value.size(), k2.value.size(), k1.outTangent.size(), k2.inTangent.size()});
        result.resize(size);
        
        for (size_t i = 0; i < size; ++i) {
            result[i] = h1 * k1.value[i] + h2 * k2.value[i] + h3 * dt * k1.outTangent[i] + h4 * dt * k2.inTangent[i];
        }
        return result;
    } else {
        return h1 * k1.value + h2 * k2.value + h3 * dt * k1.outTangent + h4 * dt * k2.inTangent;
    }
}

// Explicit template instantiations
template class AnimationSampler<Math::Vec3>;
template class AnimationSampler<Math::Quat>;
template class AnimationSampler<float>;
template class AnimationSampler<std::vector<float>>;

// AnimationChannel implementation
Math::Vec3 AnimationChannel::SampleTranslation(float time) const {
    if (m_translationSampler) {
        return m_translationSampler->Sample(time);
    }
    return Math::Vec3(0.0f);
}

Math::Quat AnimationChannel::SampleRotation(float time) const {
    if (m_rotationSampler) {
        return m_rotationSampler->Sample(time);
    }
    return Math::Quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
}

Math::Vec3 AnimationChannel::SampleScale(float time) const {
    if (m_scaleSampler) {
        return m_scaleSampler->Sample(time);
    }
    return Math::Vec3(1.0f);
}

std::vector<float> AnimationChannel::SampleWeights(float time) const {
    if (m_weightsSampler) {
        return m_weightsSampler->Sample(time);
    }
    return {};
}

float AnimationChannel::GetDuration() const {
    float maxDuration = 0.0f;
    
    if (m_translationSampler && !m_translationSampler->IsEmpty()) {
        maxDuration = std::max(maxDuration, m_translationSampler->GetDuration());
    }
    if (m_rotationSampler && !m_rotationSampler->IsEmpty()) {
        maxDuration = std::max(maxDuration, m_rotationSampler->GetDuration());
    }
    if (m_scaleSampler && !m_scaleSampler->IsEmpty()) {
        maxDuration = std::max(maxDuration, m_scaleSampler->GetDuration());
    }
    if (m_weightsSampler && !m_weightsSampler->IsEmpty()) {
        maxDuration = std::max(maxDuration, m_weightsSampler->GetDuration());
    }
    
    return maxDuration;
}

// Animation implementation
Animation::Animation(const std::string& name) : m_name(name) {
}

void Animation::AddChannel(std::shared_ptr<AnimationChannel> channel) {
    if (channel) {
        m_channels.push_back(channel);
    }
}

void Animation::SetChannels(const std::vector<std::shared_ptr<AnimationChannel>>& channels) {
    m_channels = channels;
}

float Animation::GetDuration() const {
    float maxDuration = 0.0f;
    
    for (const auto& channel : m_channels) {
        if (channel) {
            maxDuration = std::max(maxDuration, channel->GetDuration());
        }
    }
    
    return maxDuration;
}

void Animation::Update(float deltaTime) {
    m_currentTime += deltaTime * m_playbackSpeed;
    
    float duration = GetDuration();
    if (duration > 0.0f) {
        if (m_looping) {
            m_currentTime = fmod(m_currentTime, duration);
        } else {
            m_currentTime = std::min(m_currentTime, duration);
        }
    }
}

void Animation::Reset() {
    m_currentTime = 0.0f;
}

void Animation::ApplyToNodes(const std::vector<std::shared_ptr<ModelNode>>& nodes) const {
    for (const auto& channel : m_channels) {
        if (!channel) continue;
        
        uint32_t nodeIndex = channel->GetTargetNode();
        if (nodeIndex >= nodes.size()) continue;
        
        auto node = nodes[nodeIndex];
        if (!node) continue;
        
        // Sample animation data at current time
        Math::Vec3 translation = channel->SampleTranslation(m_currentTime);
        Math::Quat rotation = channel->SampleRotation(m_currentTime);
        Math::Vec3 scale = channel->SampleScale(m_currentTime);
        
        // Build transform matrix
        Math::Mat4 transform = Math::Mat4(1.0f);
        transform = glm::translate(transform, translation);
        transform = transform * glm::mat4_cast(rotation);
        transform = glm::scale(transform, scale);
        
        node->SetLocalTransform(transform);
        
        // Handle morph target weights
        auto weights = channel->SampleWeights(m_currentTime);
        if (!weights.empty()) {
            // TODO: Apply morph target weights to node's mesh
            // This will be implemented when we integrate with the mesh system
        }
    }
}

} // namespace GameEngine