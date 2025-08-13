#pragma once

#include "Animation/Skeleton.h"
#include "Animation/Animation.h"
#include "Animation/Pose.h"
#include "Animation/BlendTree.h"
#include "Animation/AnimationEvent.h"
#include "Core/Math.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <variant>

namespace GameEngine {
namespace Animation {

    // Forward declarations
    class AnimationStateMachine;
    class BlendTree;

    /**
     * Animation parameter types for controlling animation behavior
     */
    class AnimationParameter {
    public:
        enum class Type { Float, Int, Bool, Trigger };

        AnimationParameter() = default;
        AnimationParameter(float value);
        AnimationParameter(int value);
        AnimationParameter(bool value);

        Type GetType() const { return m_type; }

        float AsFloat() const;
        int AsInt() const;
        bool AsBool() const;
        bool IsTrigger() const;

        void SetFloat(float value);
        void SetInt(int value);
        void SetBool(bool value);
        void SetTrigger();
        void ResetTrigger();

    private:
        Type m_type = Type::Float;
        std::variant<float, int, bool> m_value;
        bool m_triggerState = false;
    };



    /**
     * Debug information for animation controller
     */
    struct AnimationControllerDebugInfo {
        std::string currentStateName;
        float currentStateTime = 0.0f;
        std::unordered_map<std::string, AnimationParameter> parameters;
        std::vector<AnimationSample> activeSamples;
        size_t boneCount = 0;
        bool isPlaying = false;
        bool isPaused = false;
        float playbackSpeed = 1.0f;
    };

    /**
     * Main animation controller that manages animation playback, blending, and state machines
     */
    class AnimationController {
    public:
        // Lifecycle
        AnimationController();
        ~AnimationController();
        bool Initialize(std::shared_ptr<Skeleton> skeleton);
        void Shutdown();

        // State machine management
        void SetStateMachine(std::shared_ptr<AnimationStateMachine> stateMachine);
        std::shared_ptr<AnimationStateMachine> GetStateMachine() const;

        // Parameter system
        void SetFloat(const std::string& name, float value);
        void SetInt(const std::string& name, int value);
        void SetBool(const std::string& name, bool value);
        void SetTrigger(const std::string& name);

        float GetFloat(const std::string& name) const;
        int GetInt(const std::string& name) const;
        bool GetBool(const std::string& name) const;
        bool GetTrigger(const std::string& name) const;

        // Animation control
        void Play(const std::string& animationName, float fadeTime = 0.3f);
        void Stop(const std::string& animationName, float fadeTime = 0.3f);
        void Pause();
        void Resume();
        void SetPlaybackSpeed(float speed);
        float GetPlaybackSpeed() const { return m_playbackSpeed; }

        // Animation management
        void AddAnimation(const std::string& name, std::shared_ptr<Animation> animation);
        void RemoveAnimation(const std::string& name);
        std::shared_ptr<Animation> GetAnimation(const std::string& name) const;
        std::vector<std::string> GetAnimationNames() const;

        // Update and evaluation
        void Update(float deltaTime);
        void Evaluate(std::vector<Math::Mat4>& boneMatrices);
        Pose EvaluateCurrentPose();

        // Multi-animation blending
        void PlayBlended(const std::vector<AnimationSample>& samples);
        void SetBlendWeights(const std::unordered_map<std::string, float>& weights);
        void AddAnimationLayer(const std::string& animationName, float weight, float time = 0.0f, bool additive = false);
        void RemoveAnimationLayer(const std::string& animationName);
        void ClearAnimationLayers();

        // Events
        void SetEventCallback(std::function<void(const AnimationEvent&)> callback);
        void TriggerEvent(const AnimationEvent& event);
        
        // Event history and debugging
        const AnimationEventHistory& GetEventHistory() const { return m_eventHistory; }
        void ClearEventHistory() { m_eventHistory.ClearHistory(); }
        void SetEventHistorySize(size_t maxSize) { m_eventHistory.maxHistorySize = maxSize; }
        
        // Event processing control
        void SetEventProcessingEnabled(bool enabled) { m_eventProcessingEnabled = enabled; }
        bool IsEventProcessingEnabled() const { return m_eventProcessingEnabled; }

        // Skeleton access
        std::shared_ptr<Skeleton> GetSkeleton() const { return m_skeleton; }
        bool HasValidSkeleton() const { return m_skeleton != nullptr; }

        // State queries
        bool IsPlaying() const { return m_isPlaying; }
        bool IsPaused() const { return m_isPaused; }
        bool IsInitialized() const { return m_initialized; }

        // Debugging
        AnimationControllerDebugInfo GetDebugInfo() const;
        void SetDebugVisualization(bool enabled);
        bool IsDebugVisualizationEnabled() const { return m_debugVisualization; }

    private:
        // Core components
        std::shared_ptr<Skeleton> m_skeleton;
        std::shared_ptr<AnimationStateMachine> m_stateMachine;

        // Animation storage
        std::unordered_map<std::string, std::shared_ptr<Animation>> m_animations;

        // Parameter system
        std::unordered_map<std::string, AnimationParameter> m_parameters;

        // Animation layers for blending
        struct AnimationLayer {
            std::shared_ptr<Animation> animation;
            float weight = 1.0f;
            float time = 0.0f;
            bool additive = false;
            bool fadeIn = false;
            bool fadeOut = false;
            float fadeTime = 0.0f;
            float fadeProgress = 0.0f;
        };
        std::unordered_map<std::string, AnimationLayer> m_animationLayers;

        // Event system
        std::function<void(const AnimationEvent&)> m_eventCallback;
        AnimationEventHistory m_eventHistory;
        bool m_eventProcessingEnabled = true;

        // Playback state
        bool m_initialized = false;
        bool m_isPlaying = false;
        bool m_isPaused = false;
        float m_playbackSpeed = 1.0f;
        bool m_debugVisualization = false;

        // Current pose for evaluation
        Pose m_currentPose;
        
        // Cached bone matrices for performance
        std::vector<Math::Mat4> m_cachedBoneMatrices;
        bool m_boneMatricesDirty = true;

        // Helper methods
        void UpdateAnimationLayers(float deltaTime);
        void ProcessAnimationEvents(const AnimationLayer& layer, float previousTime, float currentTime);
        void BlendAnimationLayers(Pose& outPose);
        void OptimizeAnimationLayers(); // Remove layers with zero weight or finished animations
        void ValidateParameters();
        void ResetTriggers();
    };

} // namespace Animation
} // namespace GameEngine