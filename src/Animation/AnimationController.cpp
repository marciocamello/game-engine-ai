#include "Animation/AnimationController.h"
#include "Core/Logger.h"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace GameEngine {
namespace Animation {

    // AnimationParameter implementation
    AnimationParameter::AnimationParameter(float value) : m_type(Type::Float), m_value(value) {}
    AnimationParameter::AnimationParameter(int value) : m_type(Type::Int), m_value(value) {}
    AnimationParameter::AnimationParameter(bool value) : m_type(Type::Bool), m_value(value) {}

    float AnimationParameter::AsFloat() const {
        switch (m_type) {
            case Type::Float: return std::get<float>(m_value);
            case Type::Int: return static_cast<float>(std::get<int>(m_value));
            case Type::Bool: return std::get<bool>(m_value) ? 1.0f : 0.0f;
            case Type::Trigger: return m_triggerState ? 1.0f : 0.0f;
        }
        return 0.0f;
    }

    int AnimationParameter::AsInt() const {
        switch (m_type) {
            case Type::Float: return static_cast<int>(std::get<float>(m_value));
            case Type::Int: return std::get<int>(m_value);
            case Type::Bool: return std::get<bool>(m_value) ? 1 : 0;
            case Type::Trigger: return m_triggerState ? 1 : 0;
        }
        return 0;
    }

    bool AnimationParameter::AsBool() const {
        switch (m_type) {
            case Type::Float: return std::get<float>(m_value) != 0.0f;
            case Type::Int: return std::get<int>(m_value) != 0;
            case Type::Bool: return std::get<bool>(m_value);
            case Type::Trigger: return m_triggerState;
        }
        return false;
    }

    bool AnimationParameter::IsTrigger() const {
        return m_type == Type::Trigger;
    }

    void AnimationParameter::SetFloat(float value) {
        m_type = Type::Float;
        m_value = value;
        m_triggerState = false;
    }

    void AnimationParameter::SetInt(int value) {
        m_type = Type::Int;
        m_value = value;
        m_triggerState = false;
    }

    void AnimationParameter::SetBool(bool value) {
        m_type = Type::Bool;
        m_value = value;
        m_triggerState = false;
    }

    void AnimationParameter::SetTrigger() {
        m_type = Type::Trigger;
        m_triggerState = true;
    }

    void AnimationParameter::ResetTrigger() {
        if (m_type == Type::Trigger) {
            m_triggerState = false;
        }
    }

    // AnimationController implementation
    AnimationController::AnimationController() = default;

    AnimationController::~AnimationController() {
        Shutdown();
    }

    bool AnimationController::Initialize(std::shared_ptr<AnimationSkeleton> skeleton) {
        if (!skeleton) {
            LOG_ERROR("AnimationController: Cannot initialize with null skeleton");
            return false;
        }

        if (!skeleton->HasValidBindPose()) {
            LOG_WARNING("AnimationController: Skeleton does not have valid bind pose");
        }

        m_skeleton = skeleton;
        m_currentPose.SetSkeleton(skeleton);
        m_initialized = true;

        LOG_INFO("AnimationController: Initialized with skeleton '" + skeleton->GetName() + "'");
        return true;
    }

    void AnimationController::Shutdown() {
        if (!m_initialized) {
            return;
        }

        m_stateMachine.reset();
        m_animations.clear();
        m_parameters.clear();
        m_animationLayers.clear();
        m_eventCallback = nullptr;
        m_skeleton.reset();
        
        m_initialized = false;
        m_isPlaying = false;
        m_isPaused = false;
        m_playbackSpeed = 1.0f;
        m_debugVisualization = false;

        LOG_INFO("AnimationController: Shutdown complete");
    }

    void AnimationController::SetStateMachine(std::shared_ptr<AnimationStateMachine> stateMachine) {
        m_stateMachine = stateMachine;
        if (stateMachine) {
            LOG_INFO("AnimationController: State machine set");
        }
    }

    std::shared_ptr<AnimationStateMachine> AnimationController::GetStateMachine() const {
        return m_stateMachine;
    }

    // Parameter system implementation
    void AnimationController::SetFloat(const std::string& name, float value) {
        m_parameters[name].SetFloat(value);
    }

    void AnimationController::SetInt(const std::string& name, int value) {
        m_parameters[name].SetInt(value);
    }

    void AnimationController::SetBool(const std::string& name, bool value) {
        m_parameters[name].SetBool(value);
    }

    void AnimationController::SetTrigger(const std::string& name) {
        m_parameters[name].SetTrigger();
    }

    float AnimationController::GetFloat(const std::string& name) const {
        auto it = m_parameters.find(name);
        return it != m_parameters.end() ? it->second.AsFloat() : 0.0f;
    }

    int AnimationController::GetInt(const std::string& name) const {
        auto it = m_parameters.find(name);
        return it != m_parameters.end() ? it->second.AsInt() : 0;
    }

    bool AnimationController::GetBool(const std::string& name) const {
        auto it = m_parameters.find(name);
        return it != m_parameters.end() ? it->second.AsBool() : false;
    }

    bool AnimationController::GetTrigger(const std::string& name) const {
        auto it = m_parameters.find(name);
        return it != m_parameters.end() ? it->second.IsTrigger() && it->second.AsBool() : false;
    }

    // Animation control implementation
    void AnimationController::Play(const std::string& animationName, float fadeTime) {
        auto animIt = m_animations.find(animationName);
        if (animIt == m_animations.end()) {
            LOG_WARNING("AnimationController: Animation '" + animationName + "' not found");
            return;
        }

        AnimationLayer layer;
        layer.animation = animIt->second;
        layer.weight = 1.0f;
        layer.time = 0.0f;
        layer.additive = false;
        
        if (fadeTime > 0.0f) {
            layer.fadeIn = true;
            layer.fadeTime = fadeTime;
            layer.fadeProgress = 0.0f;
            layer.weight = 0.0f;
        }

        m_animationLayers[animationName] = layer;
        m_isPlaying = true;
        m_boneMatricesDirty = true;

        std::ostringstream oss;
        oss << "AnimationController: Playing animation '" << animationName << "' with fade time " << fadeTime;
        LOG_INFO(oss.str());
    }

    void AnimationController::Stop(const std::string& animationName, float fadeTime) {
        auto layerIt = m_animationLayers.find(animationName);
        if (layerIt == m_animationLayers.end()) {
            return;
        }

        if (fadeTime > 0.0f) {
            layerIt->second.fadeOut = true;
            layerIt->second.fadeTime = fadeTime;
            layerIt->second.fadeProgress = 0.0f;
        } else {
            m_animationLayers.erase(layerIt);
        }

        if (m_animationLayers.empty()) {
            m_isPlaying = false;
        }

        std::ostringstream oss;
        oss << "AnimationController: Stopping animation '" << animationName << "' with fade time " << fadeTime;
        LOG_INFO(oss.str());
    }

    void AnimationController::Pause() {
        m_isPaused = true;
        LOG_INFO("AnimationController: Paused");
    }

    void AnimationController::Resume() {
        m_isPaused = false;
        LOG_INFO("AnimationController: Resumed");
    }

    void AnimationController::SetPlaybackSpeed(float speed) {
        m_playbackSpeed = std::max(0.0f, speed);
        std::ostringstream oss;
        oss << "AnimationController: Playback speed set to " << m_playbackSpeed;
        LOG_INFO(oss.str());
    }

    // Animation management
    void AnimationController::AddAnimation(const std::string& name, std::shared_ptr<SkeletalAnimation> animation) {
        if (!animation) {
            LOG_WARNING("AnimationController: Cannot add null animation '" + name + "'");
            return;
        }

        m_animations[name] = animation;
        LOG_INFO("AnimationController: Added animation '" + name + "'");
    }

    void AnimationController::RemoveAnimation(const std::string& name) {
        auto it = m_animations.find(name);
        if (it != m_animations.end()) {
            // Stop the animation if it's currently playing
            Stop(name, 0.0f);
            m_animations.erase(it);
            LOG_INFO("AnimationController: Removed animation '" + name + "'");
        }
    }

    std::shared_ptr<SkeletalAnimation> AnimationController::GetAnimation(const std::string& name) const {
        auto it = m_animations.find(name);
        return it != m_animations.end() ? it->second : nullptr;
    }

    std::vector<std::string> AnimationController::GetAnimationNames() const {
        std::vector<std::string> names;
        names.reserve(m_animations.size());
        for (const auto& pair : m_animations) {
            names.push_back(pair.first);
        }
        return names;
    }

    // Update and evaluation
    void AnimationController::Update(float deltaTime) {
        if (!m_initialized || m_isPaused) {
            return;
        }

        float scaledDeltaTime = deltaTime * m_playbackSpeed;

        // Update animation layers
        UpdateAnimationLayers(scaledDeltaTime);

        // Optimize layers (remove zero-weight or finished layers)
        OptimizeAnimationLayers();

        // Mark bone matrices as dirty since animations have updated
        m_boneMatricesDirty = true;

        // Update state machine if present
        if (m_stateMachine) {
            // State machine update will be implemented when state machine is available
        }

        // Reset triggers after update
        ResetTriggers();
    }

    void AnimationController::Evaluate(std::vector<Math::Mat4>& boneMatrices) {
        if (!m_initialized || !m_skeleton) {
            boneMatrices.clear();
            return;
        }

        // Check if we need to recalculate bone matrices
        if (m_boneMatricesDirty || m_cachedBoneMatrices.empty()) {
            // Reset pose to bind pose
            m_currentPose.ResetToBindPose();

            // Blend all active animation layers
            BlendAnimationLayers(m_currentPose);

            // Apply pose to skeleton and get skinning matrices
            m_currentPose.ApplyToSkeleton();
            m_currentPose.GetSkinningMatrices(m_cachedBoneMatrices);
            
            m_boneMatricesDirty = false;
        }

        // Copy cached matrices to output
        boneMatrices = m_cachedBoneMatrices;
    }

    Pose AnimationController::EvaluateCurrentPose() {
        if (!m_initialized) {
            return Pose();
        }

        Pose pose(m_skeleton);
        pose.ResetToBindPose();
        BlendAnimationLayers(pose);
        return pose;
    }

    // Multi-animation blending
    void AnimationController::PlayBlended(const std::vector<AnimationSample>& samples) {
        ClearAnimationLayers();

        for (const auto& sample : samples) {
            if (!sample.IsValid()) {
                continue;
            }

            // Find animation name
            std::string animationName;
            for (const auto& pair : m_animations) {
                if (pair.second == sample.animation) {
                    animationName = pair.first;
                    break;
                }
            }

            if (animationName.empty()) {
                LOG_WARNING("AnimationController: Animation not found in controller for blended playback");
                continue;
            }

            AnimationLayer layer;
            layer.animation = sample.animation;
            layer.weight = sample.weight;
            layer.time = sample.time;
            layer.additive = false;

            m_animationLayers[animationName] = layer;
        }

        m_isPlaying = !m_animationLayers.empty();
        m_boneMatricesDirty = true;
    }

    void AnimationController::SetBlendWeights(const std::unordered_map<std::string, float>& weights) {
        for (const auto& pair : weights) {
            auto layerIt = m_animationLayers.find(pair.first);
            if (layerIt != m_animationLayers.end()) {
                layerIt->second.weight = std::max(0.0f, pair.second);
            }
        }
    }

    void AnimationController::AddAnimationLayer(const std::string& animationName, float weight, float time, bool additive) {
        auto animIt = m_animations.find(animationName);
        if (animIt == m_animations.end()) {
            LOG_WARNING("AnimationController: Cannot add layer for unknown animation '" + animationName + "'");
            return;
        }

        AnimationLayer layer;
        layer.animation = animIt->second;
        layer.weight = std::max(0.0f, weight);
        layer.time = time;
        layer.additive = additive;

        m_animationLayers[animationName] = layer;
        m_isPlaying = true;
        m_boneMatricesDirty = true;
    }

    void AnimationController::RemoveAnimationLayer(const std::string& animationName) {
        m_animationLayers.erase(animationName);
        if (m_animationLayers.empty()) {
            m_isPlaying = false;
        }
        m_boneMatricesDirty = true;
    }

    void AnimationController::ClearAnimationLayers() {
        m_animationLayers.clear();
        m_isPlaying = false;
        m_boneMatricesDirty = true;
    }

    // Events
    void AnimationController::SetEventCallback(std::function<void(const AnimationEvent&)> callback) {
        m_eventCallback = callback;
    }

    void AnimationController::TriggerEvent(const AnimationEvent& event) {
        if (!event.IsValid() || !m_eventProcessingEnabled) {
            return;
        }

        // Add to event history
        m_eventHistory.AddTriggeredEvent(event, 0.0f, 0.0f, "Manual Trigger");

        // Call user callback
        if (m_eventCallback) {
            m_eventCallback(event);
        }
    }

    // Debugging
    AnimationControllerDebugInfo AnimationController::GetDebugInfo() const {
        AnimationControllerDebugInfo info;
        info.parameters = m_parameters;
        info.boneCount = m_skeleton ? m_skeleton->GetBoneCount() : 0;
        info.isPlaying = m_isPlaying;
        info.isPaused = m_isPaused;
        info.playbackSpeed = m_playbackSpeed;

        // Collect active samples
        for (const auto& pair : m_animationLayers) {
            AnimationSample sample;
            sample.animation = pair.second.animation;
            sample.weight = pair.second.weight;
            sample.time = pair.second.time;
            info.activeSamples.push_back(sample);
        }

        return info;
    }

    void AnimationController::SetDebugVisualization(bool enabled) {
        m_debugVisualization = enabled;
    }

    // Private helper methods
    void AnimationController::UpdateAnimationLayers(float deltaTime) {
        auto it = m_animationLayers.begin();
        while (it != m_animationLayers.end()) {
            AnimationLayer& layer = it->second;
            float previousTime = layer.time;

            // Update animation time
            if (layer.animation) {
                layer.time += deltaTime;
                
                // Handle looping
                if (layer.animation->GetLoopMode() == LoopMode::Loop) {
                    layer.time = std::fmod(layer.time, layer.animation->GetDuration());
                } else if (layer.time > layer.animation->GetDuration()) {
                    layer.time = layer.animation->GetDuration();
                }
            }

            // Handle fade in
            if (layer.fadeIn) {
                layer.fadeProgress += deltaTime;
                if (layer.fadeProgress >= layer.fadeTime) {
                    layer.fadeIn = false;
                    layer.weight = 1.0f;
                } else {
                    layer.weight = layer.fadeProgress / layer.fadeTime;
                }
            }

            // Handle fade out
            if (layer.fadeOut) {
                layer.fadeProgress += deltaTime;
                if (layer.fadeProgress >= layer.fadeTime) {
                    it = m_animationLayers.erase(it);
                    continue;
                } else {
                    layer.weight = 1.0f - (layer.fadeProgress / layer.fadeTime);
                }
            }

            // Process animation events
            ProcessAnimationEvents(layer, previousTime, layer.time);

            ++it;
        }

        if (m_animationLayers.empty()) {
            m_isPlaying = false;
        }
    }

    void AnimationController::ProcessAnimationEvents(const AnimationLayer& layer, float previousTime, float currentTime) {
        if (!m_eventProcessingEnabled || !m_eventCallback || !layer.animation) {
            return;
        }

        // Determine if animation is looping
        bool isLooping = layer.animation->GetLoopMode() == LoopMode::Loop || 
                        layer.animation->GetLoopMode() == LoopMode::PingPong;

        // Create event callback that adds to history and calls user callback
        auto eventCallbackWithHistory = [this, &layer](const AnimationEvent& event) {
            // Add to event history
            m_eventHistory.AddTriggeredEvent(event, layer.time, layer.time, layer.animation->GetName());
            
            // Call user callback
            if (m_eventCallback) {
                m_eventCallback(event);
            }
        };

        // Process events from the animation
        layer.animation->ProcessEvents(previousTime, currentTime, eventCallbackWithHistory, isLooping);
    }

    void AnimationController::BlendAnimationLayers(Pose& outPose) {
        if (m_animationLayers.empty()) {
            return;
        }

        // Collect valid non-additive layers
        std::vector<std::pair<std::string, const AnimationLayer*>> validLayers;
        std::vector<std::pair<std::string, const AnimationLayer*>> additiveLayers;
        float totalWeight = 0.0f;

        for (const auto& pair : m_animationLayers) {
            const AnimationLayer& layer = pair.second;
            
            if (!layer.animation || layer.weight <= 0.0f) {
                continue;
            }

            if (layer.additive) {
                additiveLayers.emplace_back(pair.first, &layer);
            } else {
                validLayers.emplace_back(pair.first, &layer);
                totalWeight += layer.weight;
            }
        }

        // Handle case with no valid layers
        if (validLayers.empty()) {
            outPose.ResetToBindPose();
        } else if (validLayers.size() == 1 && totalWeight >= 1.0f) {
            // Single layer with full weight - direct evaluation
            const AnimationLayer& layer = *validLayers[0].second;
            outPose = PoseEvaluator::EvaluateAnimation(*layer.animation, layer.time, m_skeleton);
        } else {
            // Multi-layer blending with weight normalization
            bool firstLayer = true;
            
            for (const auto& layerPair : validLayers) {
                const AnimationLayer& layer = *layerPair.second;
                
                // Evaluate animation at current time
                Pose layerPose = PoseEvaluator::EvaluateAnimation(*layer.animation, layer.time, m_skeleton);

                if (firstLayer) {
                    // Start with first layer
                    if (totalWeight > 0.0f) {
                        float normalizedWeight = layer.weight / totalWeight;
                        if (normalizedWeight < 1.0f) {
                            // Blend with bind pose if weight is less than 1
                            Pose bindPose(m_skeleton);
                            bindPose.ResetToBindPose();
                            outPose = Pose::Blend(bindPose, layerPose, normalizedWeight);
                        } else {
                            outPose = layerPose;
                        }
                    } else {
                        outPose = layerPose;
                    }
                    firstLayer = false;
                } else {
                    // Blend with accumulated result
                    float normalizedWeight = totalWeight > 0.0f ? layer.weight / totalWeight : 0.0f;
                    outPose.BlendWith(layerPose, normalizedWeight);
                }
            }
        }

        // Apply additive layers
        for (const auto& layerPair : additiveLayers) {
            const AnimationLayer& layer = *layerPair.second;
            
            // Evaluate additive animation
            Pose additivePose = PoseEvaluator::EvaluateAnimation(*layer.animation, layer.time, m_skeleton);
            outPose.BlendAdditiveWith(additivePose, layer.weight);
        }

        // Ensure pose is valid
        if (!outPose.ValidatePose()) {
            LOG_WARNING("AnimationController: Generated invalid pose, resetting to bind pose");
            outPose.ResetToBindPose();
        }
    }

    void AnimationController::ValidateParameters() {
        // Remove invalid parameters if needed
        // This is a placeholder for parameter validation
    }

    void AnimationController::OptimizeAnimationLayers() {
        auto it = m_animationLayers.begin();
        while (it != m_animationLayers.end()) {
            const AnimationLayer& layer = it->second;
            
            // Remove layers with zero or negative weight
            if (layer.weight <= 0.0f) {
                it = m_animationLayers.erase(it);
                continue;
            }
            
            // Remove finished non-looping animations
            if (layer.animation && 
                layer.animation->GetLoopMode() == LoopMode::Once &&
                layer.time >= layer.animation->GetDuration()) {
                it = m_animationLayers.erase(it);
                continue;
            }
            
            ++it;
        }
        
        // Update playing state
        if (m_animationLayers.empty()) {
            m_isPlaying = false;
        }
    }

    void AnimationController::ResetTriggers() {
        for (auto& pair : m_parameters) {
            pair.second.ResetTrigger();
        }
    }

} // namespace Animation
} // namespace GameEngine