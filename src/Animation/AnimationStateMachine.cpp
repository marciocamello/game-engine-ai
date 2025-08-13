#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimationController.h"
#include "Animation/AnimationTransition.h"
#include "Core/Logger.h"
#include <algorithm>
#include <sstream>

namespace GameEngine {
namespace Animation {

    // AnimationStateMachine implementation
    AnimationStateMachine::AnimationStateMachine() = default;

    AnimationStateMachine::~AnimationStateMachine() {
        Stop();
    }

    void AnimationStateMachine::AddState(std::shared_ptr<AnimationState> state) {
        if (!state) {
            LOG_WARNING("AnimationStateMachine: Cannot add null state");
            return;
        }

        const std::string& name = state->GetName();
        if (name.empty()) {
            LOG_WARNING("AnimationStateMachine: Cannot add state with empty name");
            return;
        }

        if (m_states.find(name) != m_states.end()) {
            LOG_WARNING("AnimationStateMachine: State '" + name + "' already exists, replacing");
        }

        m_states[name] = state;
        LOG_INFO("AnimationStateMachine: Added state '" + name + "'");

        // Set as entry state if it's the first state
        if (m_entryState.empty()) {
            m_entryState = name;
            LOG_INFO("AnimationStateMachine: Set '" + name + "' as entry state");
        }
    }

    void AnimationStateMachine::RemoveState(const std::string& name) {
        auto it = m_states.find(name);
        if (it == m_states.end()) {
            LOG_WARNING("AnimationStateMachine: State '" + name + "' not found for removal");
            return;
        }

        // Remove all transitions involving this state
        RemoveAllTransitionsFrom(name);
        RemoveAllTransitionsTo(name);

        // Handle current state removal
        if (m_currentState && m_currentState->GetName() == name) {
            if (m_isRunning) {
                LOG_WARNING("AnimationStateMachine: Removing current state '" + name + "', stopping state machine");
                Stop();
            }
            m_currentState.reset();
        }

        // Update entry/default states if needed
        if (m_entryState == name) {
            m_entryState.clear();
            if (!m_states.empty()) {
                m_entryState = m_states.begin()->first;
                LOG_INFO("AnimationStateMachine: Changed entry state to '" + m_entryState + "'");
            }
        }

        if (m_defaultState == name) {
            m_defaultState.clear();
        }

        m_states.erase(it);
        LOG_INFO("AnimationStateMachine: Removed state '" + name + "'");
    }

    std::shared_ptr<AnimationState> AnimationStateMachine::GetState(const std::string& name) const {
        auto it = m_states.find(name);
        return it != m_states.end() ? it->second : nullptr;
    }

    std::vector<std::shared_ptr<AnimationState>> AnimationStateMachine::GetAllStates() const {
        std::vector<std::shared_ptr<AnimationState>> states;
        states.reserve(m_states.size());
        for (const auto& pair : m_states) {
            states.push_back(pair.second);
        }
        return states;
    }

    std::vector<std::string> AnimationStateMachine::GetStateNames() const {
        std::vector<std::string> names;
        names.reserve(m_states.size());
        for (const auto& pair : m_states) {
            names.push_back(pair.first);
        }
        return names;
    }

    bool AnimationStateMachine::HasState(const std::string& name) const {
        return m_states.find(name) != m_states.end();
    }

    void AnimationStateMachine::AddTransition(const std::string& fromState, const std::string& toState,
                                            std::shared_ptr<AnimationTransition> transition) {
        if (!transition) {
            LOG_WARNING("AnimationStateMachine: Cannot add null transition");
            return;
        }

        if (!HasState(fromState)) {
            LOG_WARNING("AnimationStateMachine: From state '" + fromState + "' does not exist");
            return;
        }

        if (!HasState(toState)) {
            LOG_WARNING("AnimationStateMachine: To state '" + toState + "' does not exist");
            return;
        }

        m_transitions[fromState].push_back(transition);
        LOG_INFO("AnimationStateMachine: Added transition from '" + fromState + "' to '" + toState + "'");
    }

    void AnimationStateMachine::RemoveTransition(const std::string& fromState, const std::string& toState) {
        auto it = m_transitions.find(fromState);
        if (it == m_transitions.end()) {
            return;
        }

        // Remove transitions that target the specified state
        auto& transitions = it->second;
        transitions.erase(
            std::remove_if(transitions.begin(), transitions.end(),
                [&toState](const std::shared_ptr<AnimationTransition>& transition) {
                    return transition && transition->GetToState() == toState;
                }),
            transitions.end()
        );

        if (transitions.empty()) {
            m_transitions.erase(it);
        }

        LOG_INFO("AnimationStateMachine: Removed transition from '" + fromState + "' to '" + toState + "'");
    }

    void AnimationStateMachine::RemoveAllTransitionsFrom(const std::string& fromState) {
        auto it = m_transitions.find(fromState);
        if (it != m_transitions.end()) {
            m_transitions.erase(it);
            LOG_INFO("AnimationStateMachine: Removed all transitions from '" + fromState + "'");
        }
    }

    void AnimationStateMachine::RemoveAllTransitionsTo(const std::string& toState) {
        for (auto& pair : m_transitions) {
            auto& transitions = pair.second;
            transitions.erase(
                std::remove_if(transitions.begin(), transitions.end(),
                    [&toState](const std::shared_ptr<AnimationTransition>& transition) {
                        return transition && transition->GetToState() == toState;
                    }),
                transitions.end()
            );
        }

        // Remove empty transition lists
        auto it = m_transitions.begin();
        while (it != m_transitions.end()) {
            if (it->second.empty()) {
                it = m_transitions.erase(it);
            } else {
                ++it;
            }
        }

        LOG_INFO("AnimationStateMachine: Removed all transitions to '" + toState + "'");
    }

    std::vector<std::shared_ptr<AnimationTransition>> AnimationStateMachine::GetTransitions(const std::string& fromState) const {
        auto it = m_transitions.find(fromState);
        return it != m_transitions.end() ? it->second : std::vector<std::shared_ptr<AnimationTransition>>();
    }

    bool AnimationStateMachine::HasTransition(const std::string& fromState, const std::string& toState) const {
        auto it = m_transitions.find(fromState);
        if (it == m_transitions.end()) {
            return false;
        }

        for (const auto& transition : it->second) {
            if (transition && transition->GetToState() == toState) {
                return true;
            }
        }

        return false;
    }

    void AnimationStateMachine::SetEntryState(const std::string& name) {
        if (!HasState(name)) {
            LOG_WARNING("AnimationStateMachine: Cannot set entry state to non-existent state '" + name + "'");
            return;
        }

        m_entryState = name;
        LOG_INFO("AnimationStateMachine: Set entry state to '" + name + "'");
    }

    void AnimationStateMachine::SetDefaultState(const std::string& name) {
        if (!name.empty() && !HasState(name)) {
            LOG_WARNING("AnimationStateMachine: Cannot set default state to non-existent state '" + name + "'");
            return;
        }

        m_defaultState = name;
        LOG_INFO("AnimationStateMachine: Set default state to '" + name + "'");
    }

    void AnimationStateMachine::Start() {
        if (m_states.empty()) {
            LOG_WARNING("AnimationStateMachine: Cannot start with no states");
            return;
        }

        if (m_entryState.empty()) {
            LOG_WARNING("AnimationStateMachine: No entry state set");
            return;
        }

        auto entryState = GetState(m_entryState);
        if (!entryState) {
            LOG_ERROR("AnimationStateMachine: Entry state '" + m_entryState + "' not found");
            return;
        }

        m_currentState = entryState;
        m_currentStateTime = 0.0f;
        m_isRunning = true;
        m_activeTransition.reset();
        m_transitionTime = 0.0f;
        m_transitionProgress = 0.0f;

        // Trigger state enter callback
        if (m_currentState) {
            m_currentState->OnEnter(nullptr); // Controller will be passed in Update
        }

        // Trigger state change callback
        if (m_stateChangeCallback) {
            m_stateChangeCallback("", m_entryState);
        }

        LOG_INFO("AnimationStateMachine: Started with entry state '" + m_entryState + "'");
    }

    void AnimationStateMachine::Update(float deltaTime, AnimationController* controller) {
        if (!m_isRunning || !m_currentState) {
            return;
        }

        // Process active transition
        if (m_activeTransition) {
            ProcessTransition(deltaTime, controller);
        } else {
            // Update current state
            m_currentStateTime += deltaTime;
            m_currentState->OnUpdate(deltaTime, controller);

            // Evaluate transitions
            EvaluateTransitions(controller);
        }
    }

    void AnimationStateMachine::Stop() {
        if (!m_isRunning) {
            return;
        }

        // Trigger exit callback for current state
        if (m_currentState) {
            m_currentState->OnExit(nullptr);
        }

        m_isRunning = false;
        m_currentState.reset();
        m_previousState.reset();
        m_activeTransition.reset();
        m_currentStateTime = 0.0f;
        m_transitionTime = 0.0f;
        m_transitionProgress = 0.0f;

        LOG_INFO("AnimationStateMachine: Stopped");
    }

    void AnimationStateMachine::Reset() {
        bool wasRunning = m_isRunning;
        Stop();
        if (wasRunning) {
            Start();
        }
        LOG_INFO("AnimationStateMachine: Reset");
    }

    std::string AnimationStateMachine::GetCurrentStateName() const {
        return m_currentState ? m_currentState->GetName() : "";
    }

    std::string AnimationStateMachine::GetTransitionTargetState() const {
        return m_activeTransition ? m_activeTransition->GetToState() : "";
    }

    void AnimationStateMachine::ForceTransitionTo(const std::string& stateName, float transitionTime) {
        if (!HasState(stateName)) {
            LOG_WARNING("AnimationStateMachine: Cannot force transition to non-existent state '" + stateName + "'");
            return;
        }

        if (!m_isRunning) {
            LOG_WARNING("AnimationStateMachine: Cannot force transition when state machine is not running");
            return;
        }

        auto targetState = GetState(stateName);
        if (!targetState) {
            return;
        }

        if (transitionTime <= 0.0f) {
            // Immediate transition
            ForceSetState(stateName);
        } else {
            // Create a forced transition
            auto forcedTransition = std::make_shared<AnimationTransition>(
                m_currentState ? m_currentState->GetName() : "", stateName);
            forcedTransition->SetDuration(transitionTime);

            m_previousState = m_currentState;
            m_currentState = targetState;
            m_activeTransition = forcedTransition;
            m_transitionTime = 0.0f;
            m_transitionProgress = 0.0f;
            m_currentStateTime = 0.0f;

            // Trigger transition start
            m_activeTransition->OnTransitionStart(nullptr);

            if (m_transitionStartCallback) {
                m_transitionStartCallback(m_previousState ? m_previousState->GetName() : "", stateName, transitionTime);
            }
        }

        LOG_INFO("AnimationStateMachine: Forced transition to '" + stateName + "' with time " + std::to_string(transitionTime));
    }

    void AnimationStateMachine::ForceSetState(const std::string& stateName) {
        if (!HasState(stateName)) {
            LOG_WARNING("AnimationStateMachine: Cannot force set to non-existent state '" + stateName + "'");
            return;
        }

        auto newState = GetState(stateName);
        if (!newState) {
            return;
        }

        // Exit current state
        if (m_currentState) {
            m_currentState->OnExit(nullptr);
        }

        std::string previousStateName = m_currentState ? m_currentState->GetName() : "";

        // Set new state
        m_currentState = newState;
        m_currentStateTime = 0.0f;
        m_activeTransition.reset();
        m_transitionTime = 0.0f;
        m_transitionProgress = 0.0f;

        // Enter new state
        m_currentState->OnEnter(nullptr);

        // Trigger callbacks
        if (m_stateChangeCallback) {
            m_stateChangeCallback(previousStateName, stateName);
        }

        LOG_INFO("AnimationStateMachine: Force set state to '" + stateName + "'");
    }

    void AnimationStateMachine::SetParameter(const std::string& name, float value, AnimationController* controller) {
        if (controller) {
            controller->SetFloat(name, value);
        }
    }

    void AnimationStateMachine::SetParameter(const std::string& name, int value, AnimationController* controller) {
        if (controller) {
            controller->SetInt(name, value);
        }
    }

    void AnimationStateMachine::SetParameter(const std::string& name, bool value, AnimationController* controller) {
        if (controller) {
            controller->SetBool(name, value);
        }
    }

    void AnimationStateMachine::SetTrigger(const std::string& name, AnimationController* controller) {
        if (controller) {
            controller->SetTrigger(name);
        }
    }

    void AnimationStateMachine::EvaluatePose(Pose& outPose, AnimationController* controller) {
        if (!m_isRunning || !m_currentState) {
            return;
        }

        if (m_activeTransition && m_previousState) {
            // Blend between previous and current state during transition
            Pose previousPose(outPose.GetSkeleton());
            Pose currentPose(outPose.GetSkeleton());

            m_previousState->EvaluatePose(m_currentStateTime, previousPose, controller);
            m_currentState->EvaluatePose(m_currentStateTime, currentPose, controller);

            outPose = Pose::Blend(previousPose, currentPose, m_transitionProgress);
        } else {
            // Evaluate current state only
            m_currentState->EvaluatePose(m_currentStateTime, outPose, controller);
        }
    }

    bool AnimationStateMachine::ValidateStateMachine() const {
        if (m_states.empty()) {
            return false;
        }

        if (m_entryState.empty() || !HasState(m_entryState)) {
            return false;
        }

        // Validate all states
        for (const auto& pair : m_states) {
            if (!pair.second || !pair.second->IsValid()) {
                return false;
            }
        }

        return true;
    }

    std::vector<std::string> AnimationStateMachine::GetValidationErrors() const {
        std::vector<std::string> errors;

        if (m_states.empty()) {
            errors.push_back("No states defined");
        }

        if (m_entryState.empty()) {
            errors.push_back("No entry state set");
        } else if (!HasState(m_entryState)) {
            errors.push_back("Entry state '" + m_entryState + "' does not exist");
        }

        if (!m_defaultState.empty() && !HasState(m_defaultState)) {
            errors.push_back("Default state '" + m_defaultState + "' does not exist");
        }

        // Validate individual states
        for (const auto& pair : m_states) {
            if (!pair.second) {
                errors.push_back("State '" + pair.first + "' is null");
            } else if (!pair.second->IsValid()) {
                errors.push_back("State '" + pair.first + "' is invalid: " + pair.second->GetStateInfo());
            }
        }

        return errors;
    }

    StateMachineDebugInfo AnimationStateMachine::GetDebugInfo() const {
        StateMachineDebugInfo info;
        info.currentStateName = GetCurrentStateName();
        info.currentStateTime = m_currentStateTime;
        info.previousStateName = m_previousState ? m_previousState->GetName() : "";
        info.isTransitioning = IsTransitioning();
        info.transitionToState = GetTransitionTargetState();
        info.transitionProgress = m_transitionProgress;
        info.transitionDuration = m_transitionTime;
        info.availableStates = GetStateNames();

        // Build transition map
        for (const auto& pair : m_transitions) {
            std::vector<std::string> targets;
            for (const auto& transition : pair.second) {
                if (transition) {
                    targets.push_back(transition->GetToState());
                }
            }
            info.transitions[pair.first] = targets;
        }

        return info;
    }

    void AnimationStateMachine::PrintStateMachineInfo() const {
        std::ostringstream oss;
        oss << "AnimationStateMachine Info:\n";
        oss << "  States: " << m_states.size() << "\n";
        oss << "  Entry State: " << m_entryState << "\n";
        oss << "  Default State: " << m_defaultState << "\n";
        oss << "  Current State: " << GetCurrentStateName() << "\n";
        oss << "  Is Running: " << (m_isRunning ? "Yes" : "No") << "\n";
        oss << "  Is Transitioning: " << (IsTransitioning() ? "Yes" : "No") << "\n";

        LOG_INFO(oss.str());
    }

    void AnimationStateMachine::EvaluateTransitions(AnimationController* controller) {
        if (!m_currentState || m_activeTransition) {
            return;
        }

        const std::string& currentStateName = m_currentState->GetName();
        auto transitionsIt = m_transitions.find(currentStateName);
        
        if (transitionsIt == m_transitions.end()) {
            return; // No transitions from current state
        }

        // Calculate normalized time for current state
        float normalizedTime = 0.0f;
        float stateDuration = m_currentState->GetStateDuration();
        if (stateDuration > 0.0f) {
            normalizedTime = m_currentStateTime / stateDuration;
        }

        // Evaluate each transition condition
        for (const auto& transition : transitionsIt->second) {
            if (!transition) {
                continue;
            }

            if (transition->ShouldTransition(controller, normalizedTime)) {
                // Start transition
                auto targetState = GetState(transition->GetToState());
                if (targetState) {
                    m_previousState = m_currentState;
                    m_currentState = targetState;
                    m_activeTransition = transition;
                    m_transitionTime = 0.0f;
                    m_transitionProgress = 0.0f;

                    // Trigger transition start
                    m_activeTransition->OnTransitionStart(controller);

                    if (m_transitionStartCallback) {
                        m_transitionStartCallback(m_previousState->GetName(), targetState->GetName(), 
                                                transition->GetDuration());
                    }

                    LOG_INFO("AnimationStateMachine: Started transition from '" + m_previousState->GetName() + 
                             "' to '" + targetState->GetName() + "'");
                }
                break; // Only process one transition per frame
            }
        }
    }

    void AnimationStateMachine::ProcessTransition(float deltaTime, AnimationController* controller) {
        if (!m_activeTransition) {
            return;
        }

        m_transitionTime += deltaTime;
        float transitionDuration = m_activeTransition->GetDuration();
        
        if (m_transitionTime >= transitionDuration) {
            CompleteTransition(controller);
        } else {
            m_transitionProgress = transitionDuration > 0.0f ? (m_transitionTime / transitionDuration) : 1.0f;
            
            // Update transition
            m_activeTransition->OnTransitionUpdate(deltaTime, m_transitionProgress, controller);
        }

        // Also update current state time during transition
        m_currentStateTime += deltaTime;
    }

    void AnimationStateMachine::CompleteTransition(AnimationController* controller) {
        if (!m_activeTransition || !m_currentState) {
            return;
        }

        std::string fromState = m_previousState ? m_previousState->GetName() : "";
        std::string toState = m_currentState->GetName();

        // Exit previous state
        if (m_previousState) {
            m_previousState->OnExit(controller);
        }

        // Enter current state
        m_currentState->OnEnter(controller);

        // Trigger transition complete callback
        m_activeTransition->OnTransitionComplete(controller);

        // Clean up transition
        m_activeTransition.reset();
        m_previousState.reset();
        m_transitionTime = 0.0f;
        m_transitionProgress = 0.0f;
        m_currentStateTime = 0.0f;

        // Trigger callbacks
        if (m_transitionCompleteCallback) {
            m_transitionCompleteCallback(fromState, toState);
        }

        if (m_stateChangeCallback) {
            m_stateChangeCallback(fromState, toState);
        }

        LOG_INFO("AnimationStateMachine: Completed transition from '" + fromState + "' to '" + toState + "'");
    }

    void AnimationStateMachine::ChangeState(std::shared_ptr<AnimationState> newState, AnimationController* controller) {
        if (!newState) {
            return;
        }

        std::string previousStateName = m_currentState ? m_currentState->GetName() : "";
        std::string newStateName = newState->GetName();

        // Exit current state
        if (m_currentState) {
            m_currentState->OnExit(controller);
        }

        // Set new state
        m_currentState = newState;
        m_currentStateTime = 0.0f;

        // Enter new state
        m_currentState->OnEnter(controller);

        // Trigger callback
        if (m_stateChangeCallback) {
            m_stateChangeCallback(previousStateName, newStateName);
        }

        LOG_INFO("AnimationStateMachine: Changed state from '" + previousStateName + "' to '" + newStateName + "'");
    }

    std::shared_ptr<AnimationState> AnimationStateMachine::FindStateByName(const std::string& name) const {
        return GetState(name);
    }

    // AnimationState implementation
    AnimationState::AnimationState(const std::string& name, Type type)
        : m_name(name), m_type(type) {
    }

    AnimationState::~AnimationState() = default;

    void AnimationState::SetAnimation(std::shared_ptr<Animation> animation) {
        if (m_type != Type::Single) {
            LOG_WARNING("AnimationState: Cannot set animation on non-single state '" + m_name + "'");
            return;
        }

        m_animation = animation;
        m_blendTree.reset();
        m_subStateMachine.reset();

        if (animation) {
            LOG_INFO("AnimationState: Set animation '" + animation->GetName() + "' on state '" + m_name + "'");
        }
    }

    void AnimationState::SetBlendTree(std::shared_ptr<BlendTree> blendTree) {
        if (m_type != Type::BlendTree) {
            LOG_WARNING("AnimationState: Cannot set blend tree on non-blend-tree state '" + m_name + "'");
            return;
        }

        m_blendTree = blendTree;
        m_animation.reset();
        m_subStateMachine.reset();

        if (blendTree) {
            LOG_INFO("AnimationState: Set blend tree on state '" + m_name + "'");
        }
    }

    void AnimationState::SetSubStateMachine(std::shared_ptr<AnimationStateMachine> subStateMachine) {
        if (m_type != Type::SubStateMachine) {
            LOG_WARNING("AnimationState: Cannot set sub-state machine on non-sub-state-machine state '" + m_name + "'");
            return;
        }

        m_subStateMachine = subStateMachine;
        m_animation.reset();
        m_blendTree.reset();

        if (subStateMachine) {
            LOG_INFO("AnimationState: Set sub-state machine on state '" + m_name + "'");
        }
    }

    void AnimationState::OnEnter(AnimationController* controller) {
        LOG_INFO("AnimationState: Entering state '" + m_name + "'");

        // Start sub-state machine if present
        if (m_type == Type::SubStateMachine && m_subStateMachine) {
            m_subStateMachine->Start();
        }

        // Call custom callback
        if (m_onEnterCallback) {
            m_onEnterCallback(controller);
        }
    }

    void AnimationState::OnUpdate(float deltaTime, AnimationController* controller) {
        // Update sub-state machine if present
        if (m_type == Type::SubStateMachine && m_subStateMachine) {
            m_subStateMachine->Update(deltaTime * m_speed, controller);
        }

        // Call custom callback
        if (m_onUpdateCallback) {
            m_onUpdateCallback(controller);
        }
    }

    void AnimationState::OnExit(AnimationController* controller) {
        LOG_INFO("AnimationState: Exiting state '" + m_name + "'");

        // Stop sub-state machine if present
        if (m_type == Type::SubStateMachine && m_subStateMachine) {
            m_subStateMachine->Stop();
        }

        // Call custom callback
        if (m_onExitCallback) {
            m_onExitCallback(controller);
        }
    }

    void AnimationState::EvaluatePose(float time, Pose& pose, AnimationController* controller) const {
        float adjustedTime = time * m_speed;

        switch (m_type) {
            case Type::Single:
                if (m_animation) {
                    pose = PoseEvaluator::EvaluateAnimation(*m_animation, adjustedTime, pose.GetSkeleton());
                }
                break;

            case Type::BlendTree:
                if (m_blendTree) {
                    // This would need to be implemented when BlendTree is available
                    LOG_WARNING("AnimationState: BlendTree evaluation not yet implemented");
                }
                break;

            case Type::SubStateMachine:
                if (m_subStateMachine) {
                    m_subStateMachine->EvaluatePose(pose, controller);
                }
                break;
        }
    }

    float AnimationState::GetStateDuration() const {
        switch (m_type) {
            case Type::Single:
                return m_animation ? m_animation->GetDuration() : 0.0f;

            case Type::BlendTree:
                // This would need to be implemented when BlendTree is available
                return 0.0f;

            case Type::SubStateMachine:
                // Sub-state machines don't have a fixed duration
                return 0.0f;
        }
        return 0.0f;
    }

    bool AnimationState::IsValid() const {
        switch (m_type) {
            case Type::Single:
                return m_animation != nullptr;

            case Type::BlendTree:
                return m_blendTree != nullptr;

            case Type::SubStateMachine:
                return m_subStateMachine != nullptr && m_subStateMachine->ValidateStateMachine();
        }
        return false;
    }

    std::string AnimationState::GetStateInfo() const {
        std::ostringstream oss;
        oss << "State '" << m_name << "' (";
        
        switch (m_type) {
            case Type::Single:
                oss << "Single";
                if (m_animation) {
                    oss << ", Animation: " << m_animation->GetName();
                } else {
                    oss << ", No Animation";
                }
                break;

            case Type::BlendTree:
                oss << "BlendTree";
                if (m_blendTree) {
                    oss << ", BlendTree Set";
                } else {
                    oss << ", No BlendTree";
                }
                break;

            case Type::SubStateMachine:
                oss << "SubStateMachine";
                if (m_subStateMachine) {
                    oss << ", SubStateMachine Set";
                } else {
                    oss << ", No SubStateMachine";
                }
                break;
        }

        oss << ", Speed: " << m_speed;
        oss << ", Looping: " << (m_looping ? "Yes" : "No");
        oss << ")";

        return oss.str();
    }

    float AnimationState::NormalizeTime(float time) const {
        float duration = GetStateDuration();
        if (duration <= 0.0f) {
            return 0.0f;
        }

        if (m_looping) {
            return std::fmod(time, duration);
        } else {
            return std::min(time, duration);
        }
    }

    bool AnimationState::IsTimeAtEnd(float time) const {
        float duration = GetStateDuration();
        if (duration <= 0.0f) {
            return true;
        }

        return time >= duration;
    }

    void AnimationState::ValidateStateContent() const {
        int contentCount = 0;
        if (m_animation) contentCount++;
        if (m_blendTree) contentCount++;
        if (m_subStateMachine) contentCount++;

        if (contentCount > 1) {
            LOG_WARNING("AnimationState: State '" + m_name + "' has multiple content types set");
        } else if (contentCount == 0) {
            LOG_WARNING("AnimationState: State '" + m_name + "' has no content set");
        }
    }

} // namespace Animation
} // namespace GameEngine