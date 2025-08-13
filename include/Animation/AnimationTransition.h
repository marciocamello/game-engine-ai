#pragma once

#include "Core/Math.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace GameEngine {
namespace Animation {

    // Forward declarations
    class AnimationController;

    /**
     * Transition condition types for evaluating when transitions should occur
     */
    enum class TransitionConditionType {
        FloatGreater,       // Float parameter > value
        FloatLess,          // Float parameter < value
        FloatEqual,         // Float parameter == value (with tolerance)
        IntEqual,           // Int parameter == value
        IntGreater,         // Int parameter > value
        IntLess,            // Int parameter < value
        BoolTrue,           // Bool parameter == true
        BoolFalse,          // Bool parameter == false
        TriggerSet,         // Trigger parameter is set
        Custom              // Custom condition function
    };

    /**
     * Individual transition condition
     */
    struct TransitionCondition {
        TransitionConditionType type;
        std::string parameterName;
        
        // Condition values (only relevant ones are used based on type)
        float floatValue = 0.0f;
        int intValue = 0;
        bool boolValue = false;
        float tolerance = 0.001f; // For float equality comparisons
        
        // Custom condition function (for TransitionConditionType::Custom)
        std::function<bool(AnimationController*)> customCondition;

        TransitionCondition() = default;
        TransitionCondition(TransitionConditionType t, const std::string& param)
            : type(t), parameterName(param) {}

        // Factory methods for common conditions
        static TransitionCondition FloatGreater(const std::string& param, float value);
        static TransitionCondition FloatLess(const std::string& param, float value);
        static TransitionCondition FloatEqual(const std::string& param, float value, float tol = 0.001f);
        static TransitionCondition IntEqual(const std::string& param, int value);
        static TransitionCondition IntGreater(const std::string& param, int value);
        static TransitionCondition IntLess(const std::string& param, int value);
        static TransitionCondition BoolTrue(const std::string& param);
        static TransitionCondition BoolFalse(const std::string& param);
        static TransitionCondition TriggerSet(const std::string& param);
        static TransitionCondition Custom(std::function<bool(AnimationController*)> condition);

        // Evaluation
        bool Evaluate(AnimationController* controller) const;
        std::string ToString() const;
    };

    /**
     * Logical operators for combining multiple conditions
     */
    enum class TransitionLogicOperator {
        And,    // All conditions must be true
        Or      // At least one condition must be true
    };

    /**
     * Transition interrupt settings
     */
    enum class TransitionInterruptSource {
        None,           // Cannot be interrupted
        Source,         // Can be interrupted by transitions from source state
        Destination,    // Can be interrupted by transitions from destination state
        SourceAndDestination // Can be interrupted from either state
    };

    /**
     * Animation transition with condition-based triggering and smooth blending
     */
    class AnimationTransition {
    public:
        // Lifecycle
        AnimationTransition(const std::string& fromState, const std::string& toState);
        ~AnimationTransition() = default;

        // Basic properties
        const std::string& GetFromState() const { return m_fromState; }
        const std::string& GetToState() const { return m_toState; }
        void SetToState(const std::string& toState) { m_toState = toState; }

        // Transition timing
        void SetDuration(float duration) { m_duration = std::max(0.0f, duration); }
        float GetDuration() const { return m_duration; }

        void SetOffset(float offset) { m_offset = offset; }
        float GetOffset() const { return m_offset; }

        // Exit time (normalized time in source animation when transition can start)
        void SetExitTime(float exitTime) { m_exitTime = Math::Clamp(exitTime, 0.0f, 1.0f); }
        float GetExitTime() const { return m_exitTime; }
        void SetHasExitTime(bool hasExitTime) { m_hasExitTime = hasExitTime; }
        bool HasExitTime() const { return m_hasExitTime; }

        // Interruption settings
        void SetInterruptSource(TransitionInterruptSource source) { m_interruptSource = source; }
        TransitionInterruptSource GetInterruptSource() const { return m_interruptSource; }

        // Condition management
        void AddCondition(const TransitionCondition& condition);
        void RemoveCondition(size_t index);
        void ClearConditions();
        const std::vector<TransitionCondition>& GetConditions() const { return m_conditions; }
        size_t GetConditionCount() const { return m_conditions.size(); }

        void SetLogicOperator(TransitionLogicOperator op) { m_logicOperator = op; }
        TransitionLogicOperator GetLogicOperator() const { return m_logicOperator; }

        // Condition evaluation
        bool EvaluateConditions(AnimationController* controller) const;
        bool CanTransition(AnimationController* controller, float normalizedTime) const;

        // Transition execution
        bool ShouldTransition(AnimationController* controller, float normalizedTime) const;
        void OnTransitionStart(AnimationController* controller);
        void OnTransitionUpdate(float deltaTime, float progress, AnimationController* controller);
        void OnTransitionComplete(AnimationController* controller);

        // Blending
        enum class BlendMode {
            Linear,         // Linear interpolation
            EaseIn,         // Ease in curve
            EaseOut,        // Ease out curve
            EaseInOut,      // Ease in-out curve
            Custom          // Custom curve
        };

        void SetBlendMode(BlendMode mode) { m_blendMode = mode; }
        BlendMode GetBlendMode() const { return m_blendMode; }

        void SetCustomBlendCurve(std::function<float(float)> curve) { m_customBlendCurve = curve; }
        float CalculateBlendWeight(float progress) const;

        // Validation
        bool IsValid() const;
        std::vector<std::string> GetValidationErrors() const;

        // Debugging
        std::string GetTransitionInfo() const;
        void PrintTransitionInfo() const;

        // Events
        using TransitionCallback = std::function<void(AnimationController*)>;
        void SetOnStartCallback(TransitionCallback callback) { m_onStartCallback = callback; }
        void SetOnUpdateCallback(TransitionCallback callback) { m_onUpdateCallback = callback; }
        void SetOnCompleteCallback(TransitionCallback callback) { m_onCompleteCallback = callback; }

    private:
        // Transition identity
        std::string m_fromState;
        std::string m_toState;

        // Timing properties
        float m_duration = 0.3f;        // Transition duration in seconds
        float m_offset = 0.0f;          // Start offset in destination animation
        float m_exitTime = 0.0f;        // Normalized exit time (0-1)
        bool m_hasExitTime = false;     // Whether to use exit time

        // Interruption
        TransitionInterruptSource m_interruptSource = TransitionInterruptSource::None;

        // Conditions
        std::vector<TransitionCondition> m_conditions;
        TransitionLogicOperator m_logicOperator = TransitionLogicOperator::And;

        // Blending
        BlendMode m_blendMode = BlendMode::Linear;
        std::function<float(float)> m_customBlendCurve;

        // Event callbacks
        TransitionCallback m_onStartCallback;
        TransitionCallback m_onUpdateCallback;
        TransitionCallback m_onCompleteCallback;

        // Helper methods
        bool EvaluateExitTime(float normalizedTime) const;
        float ApplyBlendCurve(float t) const;
    };

    /**
     * Transition builder for easy transition creation
     */
    class TransitionBuilder {
    public:
        TransitionBuilder(const std::string& fromState, const std::string& toState);

        // Chaining methods for fluent interface
        TransitionBuilder& WithDuration(float duration);
        TransitionBuilder& WithOffset(float offset);
        TransitionBuilder& WithExitTime(float exitTime);
        TransitionBuilder& WithoutExitTime();
        TransitionBuilder& WithInterruptSource(TransitionInterruptSource source);
        TransitionBuilder& WithBlendMode(AnimationTransition::BlendMode mode);
        TransitionBuilder& WithCustomBlendCurve(std::function<float(float)> curve);

        // Condition methods
        TransitionBuilder& When(const TransitionCondition& condition);
        TransitionBuilder& WhenFloat(const std::string& param, TransitionConditionType type, float value);
        TransitionBuilder& WhenInt(const std::string& param, TransitionConditionType type, int value);
        TransitionBuilder& WhenBool(const std::string& param, bool value);
        TransitionBuilder& WhenTrigger(const std::string& param);
        TransitionBuilder& WhenCustom(std::function<bool(AnimationController*)> condition);

        // Logic operators
        TransitionBuilder& WithAnd();
        TransitionBuilder& WithOr();

        // Event callbacks
        TransitionBuilder& OnStart(AnimationTransition::TransitionCallback callback);
        TransitionBuilder& OnUpdate(AnimationTransition::TransitionCallback callback);
        TransitionBuilder& OnComplete(AnimationTransition::TransitionCallback callback);

        // Build the transition
        std::shared_ptr<AnimationTransition> Build();

    private:
        std::shared_ptr<AnimationTransition> m_transition;
    };

} // namespace Animation
} // namespace GameEngine