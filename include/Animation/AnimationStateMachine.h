#pragma once

#include "Animation/Animation.h"
#include "Animation/Pose.h"
#include "Animation/AnimationTransition.h"
#include "Animation/BlendTree.h"
#include "Core/Math.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

namespace GameEngine {
namespace Animation {

    // Forward declarations
    class AnimationController;
    class AnimationState;
    class BlendTree;

    /**
     * Debug information for state machine
     */
    struct StateMachineDebugInfo {
        std::string currentStateName;
        float currentStateTime = 0.0f;
        std::string previousStateName;
        bool isTransitioning = false;
        std::string transitionToState;
        float transitionProgress = 0.0f;
        float transitionDuration = 0.0f;
        std::vector<std::string> availableStates;
        std::unordered_map<std::string, std::vector<std::string>> transitions;
    };

    /**
     * Animation State Machine for managing complex animation logic
     */
    class AnimationStateMachine {
    public:
        // Lifecycle
        AnimationStateMachine();
        ~AnimationStateMachine();

        // State management
        void AddState(std::shared_ptr<AnimationState> state);
        void RemoveState(const std::string& name);
        std::shared_ptr<AnimationState> GetState(const std::string& name) const;
        std::vector<std::shared_ptr<AnimationState>> GetAllStates() const;
        std::vector<std::string> GetStateNames() const;
        bool HasState(const std::string& name) const;

        // Transitions
        void AddTransition(const std::string& fromState, const std::string& toState,
                          std::shared_ptr<AnimationTransition> transition);
        void RemoveTransition(const std::string& fromState, const std::string& toState);
        void RemoveAllTransitionsFrom(const std::string& fromState);
        void RemoveAllTransitionsTo(const std::string& toState);
        std::vector<std::shared_ptr<AnimationTransition>> GetTransitions(const std::string& fromState) const;
        bool HasTransition(const std::string& fromState, const std::string& toState) const;

        // Entry and default states
        void SetEntryState(const std::string& name);
        void SetDefaultState(const std::string& name);
        const std::string& GetEntryState() const { return m_entryState; }
        const std::string& GetDefaultState() const { return m_defaultState; }

        // Execution
        void Start();
        void Update(float deltaTime, AnimationController* controller);
        void Stop();
        void Reset(); // Reset to entry state

        // Current state
        std::shared_ptr<AnimationState> GetCurrentState() const { return m_currentState; }
        std::string GetCurrentStateName() const;
        float GetCurrentStateTime() const { return m_currentStateTime; }
        bool IsRunning() const { return m_isRunning; }

        // Transition state
        bool IsTransitioning() const { return m_activeTransition != nullptr; }
        std::shared_ptr<AnimationTransition> GetActiveTransition() const { return m_activeTransition; }
        float GetTransitionProgress() const { return m_transitionProgress; }
        std::string GetTransitionTargetState() const;

        // Manual state changes
        void ForceTransitionTo(const std::string& stateName, float transitionTime = 0.3f);
        void ForceSetState(const std::string& stateName); // Immediate state change

        // Parameter access (delegates to controller)
        void SetParameter(const std::string& name, float value, AnimationController* controller);
        void SetParameter(const std::string& name, int value, AnimationController* controller);
        void SetParameter(const std::string& name, bool value, AnimationController* controller);
        void SetTrigger(const std::string& name, AnimationController* controller);

        // Pose evaluation
        void EvaluatePose(Pose& outPose, AnimationController* controller);

        // Validation
        bool ValidateStateMachine() const;
        std::vector<std::string> GetValidationErrors() const;

        // Debugging
        StateMachineDebugInfo GetDebugInfo() const;
        void PrintStateMachineInfo() const;

        // Events
        using StateChangeCallback = std::function<void(const std::string& fromState, const std::string& toState)>;
        using TransitionStartCallback = std::function<void(const std::string& fromState, const std::string& toState, float duration)>;
        using TransitionCompleteCallback = std::function<void(const std::string& fromState, const std::string& toState)>;

        void SetStateChangeCallback(StateChangeCallback callback) { m_stateChangeCallback = callback; }
        void SetTransitionStartCallback(TransitionStartCallback callback) { m_transitionStartCallback = callback; }
        void SetTransitionCompleteCallback(TransitionCompleteCallback callback) { m_transitionCompleteCallback = callback; }

    private:
        // State storage
        std::unordered_map<std::string, std::shared_ptr<AnimationState>> m_states;
        std::unordered_map<std::string, std::vector<std::shared_ptr<AnimationTransition>>> m_transitions;

        // Current execution state
        std::shared_ptr<AnimationState> m_currentState;
        std::shared_ptr<AnimationState> m_previousState;
        std::shared_ptr<AnimationTransition> m_activeTransition;

        // State machine configuration
        std::string m_entryState;
        std::string m_defaultState;

        // Runtime state
        bool m_isRunning = false;
        float m_currentStateTime = 0.0f;
        float m_transitionTime = 0.0f;
        float m_transitionProgress = 0.0f;

        // Event callbacks
        StateChangeCallback m_stateChangeCallback;
        TransitionStartCallback m_transitionStartCallback;
        TransitionCompleteCallback m_transitionCompleteCallback;

        // Helper methods
        void EvaluateTransitions(AnimationController* controller);
        void ProcessTransition(float deltaTime, AnimationController* controller);
        void CompleteTransition(AnimationController* controller);
        void ChangeState(std::shared_ptr<AnimationState> newState, AnimationController* controller);
        std::shared_ptr<AnimationState> FindStateByName(const std::string& name) const;
    };

    /**
     * Individual animation state that can contain single animations, blend trees, or sub-state machines
     */
    class AnimationState {
    public:
        enum class Type { 
            Single,           // Single animation
            BlendTree,        // Blend tree for parameter-driven blending
            SubStateMachine   // Nested state machine
        };

        // Lifecycle
        AnimationState(const std::string& name, Type type = Type::Single);
        ~AnimationState();

        // Properties
        void SetName(const std::string& name) { m_name = name; }
        void SetType(Type type) { m_type = type; }
        void SetSpeed(float speed) { m_speed = std::max(0.0f, speed); }
        void SetLooping(bool looping) { m_looping = looping; }

        const std::string& GetName() const { return m_name; }
        Type GetType() const { return m_type; }
        float GetSpeed() const { return m_speed; }
        bool IsLooping() const { return m_looping; }

        // Single animation state
        void SetAnimation(std::shared_ptr<Animation> animation);
        std::shared_ptr<Animation> GetAnimation() const { return m_animation; }

        // Blend tree state
        void SetBlendTree(std::shared_ptr<BlendTree> blendTree);
        std::shared_ptr<BlendTree> GetBlendTree() const { return m_blendTree; }

        // Sub-state machine
        void SetSubStateMachine(std::shared_ptr<AnimationStateMachine> subStateMachine);
        std::shared_ptr<AnimationStateMachine> GetSubStateMachine() const { return m_subStateMachine; }

        // State callbacks
        using StateCallback = std::function<void(AnimationController*)>;
        void SetOnEnterCallback(StateCallback callback) { m_onEnterCallback = callback; }
        void SetOnUpdateCallback(StateCallback callback) { m_onUpdateCallback = callback; }
        void SetOnExitCallback(StateCallback callback) { m_onExitCallback = callback; }

        // Execution
        void OnEnter(AnimationController* controller);
        void OnUpdate(float deltaTime, AnimationController* controller);
        void OnExit(AnimationController* controller);

        // Pose evaluation
        void EvaluatePose(float time, Pose& pose, AnimationController* controller) const;
        float GetStateDuration() const;

        // State information
        bool IsValid() const;
        std::string GetStateInfo() const;

        // Time management
        float NormalizeTime(float time) const;
        bool IsTimeAtEnd(float time) const;

    private:
        std::string m_name;
        Type m_type;
        float m_speed = 1.0f;
        bool m_looping = true;

        // State content (only one should be set based on type)
        std::shared_ptr<Animation> m_animation;
        std::shared_ptr<BlendTree> m_blendTree;
        std::shared_ptr<AnimationStateMachine> m_subStateMachine;

        // State callbacks
        StateCallback m_onEnterCallback;
        StateCallback m_onUpdateCallback;
        StateCallback m_onExitCallback;

        // Helper methods
        void ValidateStateContent() const;
    };

} // namespace Animation
} // namespace GameEngine