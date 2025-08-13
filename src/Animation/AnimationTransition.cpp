#include "Animation/AnimationTransition.h"
#include "Animation/AnimationController.h"
#include "Core/Logger.h"
#include <algorithm>
#include <sstream>
#include <cmath>

namespace GameEngine {
namespace Animation {

    // TransitionCondition implementation
    TransitionCondition TransitionCondition::FloatGreater(const std::string& param, float value) {
        TransitionCondition condition(TransitionConditionType::FloatGreater, param);
        condition.floatValue = value;
        return condition;
    }

    TransitionCondition TransitionCondition::FloatLess(const std::string& param, float value) {
        TransitionCondition condition(TransitionConditionType::FloatLess, param);
        condition.floatValue = value;
        return condition;
    }

    TransitionCondition TransitionCondition::FloatEqual(const std::string& param, float value, float tol) {
        TransitionCondition condition(TransitionConditionType::FloatEqual, param);
        condition.floatValue = value;
        condition.tolerance = tol;
        return condition;
    }

    TransitionCondition TransitionCondition::IntEqual(const std::string& param, int value) {
        TransitionCondition condition(TransitionConditionType::IntEqual, param);
        condition.intValue = value;
        return condition;
    }

    TransitionCondition TransitionCondition::IntGreater(const std::string& param, int value) {
        TransitionCondition condition(TransitionConditionType::IntGreater, param);
        condition.intValue = value;
        return condition;
    }

    TransitionCondition TransitionCondition::IntLess(const std::string& param, int value) {
        TransitionCondition condition(TransitionConditionType::IntLess, param);
        condition.intValue = value;
        return condition;
    }

    TransitionCondition TransitionCondition::BoolTrue(const std::string& param) {
        TransitionCondition condition(TransitionConditionType::BoolTrue, param);
        condition.boolValue = true;
        return condition;
    }

    TransitionCondition TransitionCondition::BoolFalse(const std::string& param) {
        TransitionCondition condition(TransitionConditionType::BoolFalse, param);
        condition.boolValue = false;
        return condition;
    }

    TransitionCondition TransitionCondition::TriggerSet(const std::string& param) {
        return TransitionCondition(TransitionConditionType::TriggerSet, param);
    }

    TransitionCondition TransitionCondition::Custom(std::function<bool(AnimationController*)> condition) {
        TransitionCondition cond(TransitionConditionType::Custom, "");
        cond.customCondition = condition;
        return cond;
    }

    bool TransitionCondition::Evaluate(AnimationController* controller) const {
        if (!controller) {
            return false;
        }

        switch (type) {
            case TransitionConditionType::FloatGreater:
                return controller->GetFloat(parameterName) > floatValue;

            case TransitionConditionType::FloatLess:
                return controller->GetFloat(parameterName) < floatValue;

            case TransitionConditionType::FloatEqual: {
                float paramValue = controller->GetFloat(parameterName);
                return std::abs(paramValue - floatValue) <= tolerance;
            }

            case TransitionConditionType::IntEqual:
                return controller->GetInt(parameterName) == intValue;

            case TransitionConditionType::IntGreater:
                return controller->GetInt(parameterName) > intValue;

            case TransitionConditionType::IntLess:
                return controller->GetInt(parameterName) < intValue;

            case TransitionConditionType::BoolTrue:
                return controller->GetBool(parameterName) == true;

            case TransitionConditionType::BoolFalse:
                return controller->GetBool(parameterName) == false;

            case TransitionConditionType::TriggerSet:
                return controller->GetTrigger(parameterName);

            case TransitionConditionType::Custom:
                return customCondition ? customCondition(controller) : false;
        }

        return false;
    }

    std::string TransitionCondition::ToString() const {
        std::ostringstream oss;
        
        switch (type) {
            case TransitionConditionType::FloatGreater:
                oss << parameterName << " > " << floatValue;
                break;
            case TransitionConditionType::FloatLess:
                oss << parameterName << " < " << floatValue;
                break;
            case TransitionConditionType::FloatEqual:
                oss << parameterName << " == " << floatValue << " (±" << tolerance << ")";
                break;
            case TransitionConditionType::IntEqual:
                oss << parameterName << " == " << intValue;
                break;
            case TransitionConditionType::IntGreater:
                oss << parameterName << " > " << intValue;
                break;
            case TransitionConditionType::IntLess:
                oss << parameterName << " < " << intValue;
                break;
            case TransitionConditionType::BoolTrue:
                oss << parameterName << " == true";
                break;
            case TransitionConditionType::BoolFalse:
                oss << parameterName << " == false";
                break;
            case TransitionConditionType::TriggerSet:
                oss << parameterName << " (trigger)";
                break;
            case TransitionConditionType::Custom:
                oss << "Custom condition";
                break;
        }

        return oss.str();
    }

    // AnimationTransition implementation
    AnimationTransition::AnimationTransition(const std::string& fromState, const std::string& toState)
        : m_fromState(fromState), m_toState(toState) {
    }

    void AnimationTransition::AddCondition(const TransitionCondition& condition) {
        m_conditions.push_back(condition);
        LOG_INFO("AnimationTransition: Added condition '" + condition.ToString() + "' to transition " + 
                 m_fromState + " -> " + m_toState);
    }

    void AnimationTransition::RemoveCondition(size_t index) {
        if (index < m_conditions.size()) {
            std::string conditionStr = m_conditions[index].ToString();
            m_conditions.erase(m_conditions.begin() + index);
            LOG_INFO("AnimationTransition: Removed condition '" + conditionStr + "' from transition " + 
                     m_fromState + " -> " + m_toState);
        }
    }

    void AnimationTransition::ClearConditions() {
        m_conditions.clear();
        LOG_INFO("AnimationTransition: Cleared all conditions from transition " + m_fromState + " -> " + m_toState);
    }

    bool AnimationTransition::EvaluateConditions(AnimationController* controller) const {
        if (m_conditions.empty()) {
            return true; // No conditions means always true
        }

        if (m_logicOperator == TransitionLogicOperator::And) {
            // All conditions must be true
            for (const auto& condition : m_conditions) {
                if (!condition.Evaluate(controller)) {
                    return false;
                }
            }
            return true;
        } else {
            // At least one condition must be true (OR)
            for (const auto& condition : m_conditions) {
                if (condition.Evaluate(controller)) {
                    return true;
                }
            }
            return false;
        }
    }

    bool AnimationTransition::CanTransition(AnimationController* controller, float normalizedTime) const {
        // Check exit time if required
        if (m_hasExitTime && !EvaluateExitTime(normalizedTime)) {
            return false;
        }

        // Check conditions
        return EvaluateConditions(controller);
    }

    bool AnimationTransition::ShouldTransition(AnimationController* controller, float normalizedTime) const {
        return CanTransition(controller, normalizedTime);
    }

    void AnimationTransition::OnTransitionStart(AnimationController* controller) {
        LOG_INFO("AnimationTransition: Starting transition " + m_fromState + " -> " + m_toState);
        
        if (m_onStartCallback) {
            m_onStartCallback(controller);
        }
    }

    void AnimationTransition::OnTransitionUpdate(float deltaTime, float progress, AnimationController* controller) {
        if (m_onUpdateCallback) {
            m_onUpdateCallback(controller);
        }
    }

    void AnimationTransition::OnTransitionComplete(AnimationController* controller) {
        LOG_INFO("AnimationTransition: Completed transition " + m_fromState + " -> " + m_toState);
        
        if (m_onCompleteCallback) {
            m_onCompleteCallback(controller);
        }
    }

    float AnimationTransition::CalculateBlendWeight(float progress) const {
        // Clamp progress to [0, 1]
        progress = Math::Clamp(progress, 0.0f, 1.0f);
        
        return ApplyBlendCurve(progress);
    }

    bool AnimationTransition::IsValid() const {
        if (m_fromState.empty() || m_toState.empty()) {
            return false;
        }

        if (m_duration < 0.0f) {
            return false;
        }

        if (m_hasExitTime && (m_exitTime < 0.0f || m_exitTime > 1.0f)) {
            return false;
        }

        // Validate conditions
        for (const auto& condition : m_conditions) {
            if (condition.type != TransitionConditionType::Custom && condition.parameterName.empty()) {
                return false;
            }
            
            if (condition.type == TransitionConditionType::Custom && !condition.customCondition) {
                return false;
            }
        }

        return true;
    }

    std::vector<std::string> AnimationTransition::GetValidationErrors() const {
        std::vector<std::string> errors;

        if (m_fromState.empty()) {
            errors.push_back("From state is empty");
        }

        if (m_toState.empty()) {
            errors.push_back("To state is empty");
        }

        if (m_duration < 0.0f) {
            errors.push_back("Duration cannot be negative");
        }

        if (m_hasExitTime && (m_exitTime < 0.0f || m_exitTime > 1.0f)) {
            errors.push_back("Exit time must be between 0 and 1");
        }

        // Validate conditions
        for (size_t i = 0; i < m_conditions.size(); ++i) {
            const auto& condition = m_conditions[i];
            
            if (condition.type != TransitionConditionType::Custom && condition.parameterName.empty()) {
                errors.push_back("Condition " + std::to_string(i) + " has empty parameter name");
            }
            
            if (condition.type == TransitionConditionType::Custom && !condition.customCondition) {
                errors.push_back("Condition " + std::to_string(i) + " has null custom condition function");
            }
        }

        return errors;
    }

    std::string AnimationTransition::GetTransitionInfo() const {
        std::ostringstream oss;
        oss << "Transition: " << m_fromState << " -> " << m_toState << "\n";
        oss << "  Duration: " << m_duration << "s\n";
        oss << "  Offset: " << m_offset << "s\n";
        
        if (m_hasExitTime) {
            oss << "  Exit Time: " << m_exitTime << "\n";
        } else {
            oss << "  Exit Time: Not used\n";
        }

        oss << "  Interrupt Source: ";
        switch (m_interruptSource) {
            case TransitionInterruptSource::None:
                oss << "None";
                break;
            case TransitionInterruptSource::Source:
                oss << "Source";
                break;
            case TransitionInterruptSource::Destination:
                oss << "Destination";
                break;
            case TransitionInterruptSource::SourceAndDestination:
                oss << "Source and Destination";
                break;
        }
        oss << "\n";

        oss << "  Blend Mode: ";
        switch (m_blendMode) {
            case BlendMode::Linear:
                oss << "Linear";
                break;
            case BlendMode::EaseIn:
                oss << "Ease In";
                break;
            case BlendMode::EaseOut:
                oss << "Ease Out";
                break;
            case BlendMode::EaseInOut:
                oss << "Ease In-Out";
                break;
            case BlendMode::Custom:
                oss << "Custom";
                break;
        }
        oss << "\n";

        oss << "  Conditions (" << m_conditions.size() << "):\n";
        if (m_conditions.empty()) {
            oss << "    None (always true)\n";
        } else {
            oss << "    Logic: " << (m_logicOperator == TransitionLogicOperator::And ? "AND" : "OR") << "\n";
            for (size_t i = 0; i < m_conditions.size(); ++i) {
                oss << "    " << (i + 1) << ". " << m_conditions[i].ToString() << "\n";
            }
        }

        return oss.str();
    }

    void AnimationTransition::PrintTransitionInfo() const {
        LOG_INFO(GetTransitionInfo());
    }

    bool AnimationTransition::EvaluateExitTime(float normalizedTime) const {
        if (!m_hasExitTime) {
            return true;
        }

        return normalizedTime >= m_exitTime;
    }

    float AnimationTransition::ApplyBlendCurve(float t) const {
        switch (m_blendMode) {
            case BlendMode::Linear:
                return t;

            case BlendMode::EaseIn:
                return t * t;

            case BlendMode::EaseOut:
                return 1.0f - (1.0f - t) * (1.0f - t);

            case BlendMode::EaseInOut: {
                if (t < 0.5f) {
                    return 2.0f * t * t;
                } else {
                    float temp = 2.0f * t - 1.0f;
                    return 1.0f - 0.5f * (1.0f - temp) * (1.0f - temp);
                }
            }

            case BlendMode::Custom:
                return m_customBlendCurve ? m_customBlendCurve(t) : t;
        }

        return t;
    }

    // TransitionBuilder implementation
    TransitionBuilder::TransitionBuilder(const std::string& fromState, const std::string& toState)
        : m_transition(std::make_shared<AnimationTransition>(fromState, toState)) {
    }

    TransitionBuilder& TransitionBuilder::WithDuration(float duration) {
        m_transition->SetDuration(duration);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::WithOffset(float offset) {
        m_transition->SetOffset(offset);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::WithExitTime(float exitTime) {
        m_transition->SetExitTime(exitTime);
        m_transition->SetHasExitTime(true);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::WithoutExitTime() {
        m_transition->SetHasExitTime(false);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::WithInterruptSource(TransitionInterruptSource source) {
        m_transition->SetInterruptSource(source);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::WithBlendMode(AnimationTransition::BlendMode mode) {
        m_transition->SetBlendMode(mode);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::WithCustomBlendCurve(std::function<float(float)> curve) {
        m_transition->SetBlendMode(AnimationTransition::BlendMode::Custom);
        m_transition->SetCustomBlendCurve(curve);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::When(const TransitionCondition& condition) {
        m_transition->AddCondition(condition);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::WhenFloat(const std::string& param, TransitionConditionType type, float value) {
        TransitionCondition condition(type, param);
        condition.floatValue = value;
        m_transition->AddCondition(condition);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::WhenInt(const std::string& param, TransitionConditionType type, int value) {
        TransitionCondition condition(type, param);
        condition.intValue = value;
        m_transition->AddCondition(condition);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::WhenBool(const std::string& param, bool value) {
        TransitionCondition condition(value ? TransitionConditionType::BoolTrue : TransitionConditionType::BoolFalse, param);
        condition.boolValue = value;
        m_transition->AddCondition(condition);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::WhenTrigger(const std::string& param) {
        m_transition->AddCondition(TransitionCondition::TriggerSet(param));
        return *this;
    }

    TransitionBuilder& TransitionBuilder::WhenCustom(std::function<bool(AnimationController*)> condition) {
        m_transition->AddCondition(TransitionCondition::Custom(condition));
        return *this;
    }

    TransitionBuilder& TransitionBuilder::WithAnd() {
        m_transition->SetLogicOperator(TransitionLogicOperator::And);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::WithOr() {
        m_transition->SetLogicOperator(TransitionLogicOperator::Or);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::OnStart(AnimationTransition::TransitionCallback callback) {
        m_transition->SetOnStartCallback(callback);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::OnUpdate(AnimationTransition::TransitionCallback callback) {
        m_transition->SetOnUpdateCallback(callback);
        return *this;
    }

    TransitionBuilder& TransitionBuilder::OnComplete(AnimationTransition::TransitionCallback callback) {
        m_transition->SetOnCompleteCallback(callback);
        return *this;
    }

    std::shared_ptr<AnimationTransition> TransitionBuilder::Build() {
        if (!m_transition->IsValid()) {
            LOG_WARNING("TransitionBuilder: Built transition is not valid");
            auto errors = m_transition->GetValidationErrors();
            for (const auto& error : errors) {
                LOG_WARNING("  - " + error);
            }
        }

        return m_transition;
    }

} // namespace Animation
} // namespace GameEngine