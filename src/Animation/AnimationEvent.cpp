#include "Animation/AnimationEvent.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <chrono>
#include <cmath>

namespace GameEngine {
namespace Animation {

    // AnimationEvent implementation
    AnimationEvent::AnimationEvent(const std::string& eventName, float eventTime, AnimationEventType eventType)
        : name(eventName), time(eventTime), type(eventType) {
    }

    bool AnimationEvent::IsValid() const {
        return !name.empty() && IsTimeValid() && enabled;
    }

    bool AnimationEvent::operator==(const AnimationEvent& other) const {
        return name == other.name && 
               std::abs(time - other.time) < 0.001f && 
               type == other.type;
    }

    AnimationEvent::EventData AnimationEvent::Serialize() const {
        EventData data;
        data.name = name;
        data.time = time;
        data.type = static_cast<int>(type);
        data.stringParameter = stringParameter;
        data.floatParameter = floatParameter;
        data.intParameter = intParameter;
        data.boolParameter = boolParameter;
        data.description = description;
        data.priority = priority;
        data.enabled = enabled;
        return data;
    }

    bool AnimationEvent::Deserialize(const EventData& data) {
        name = data.name;
        time = data.time;
        type = static_cast<AnimationEventType>(data.type);
        stringParameter = data.stringParameter;
        floatParameter = data.floatParameter;
        intParameter = data.intParameter;
        boolParameter = data.boolParameter;
        description = data.description;
        priority = data.priority;
        enabled = data.enabled;
        
        return IsValid();
    }

    // AnimationEventManager implementation
    void AnimationEventManager::AddEvent(const AnimationEvent& event) {
        if (!event.IsValid()) {
            return;
        }
        
        // Check for duplicate events
        auto it = std::find_if(m_events.begin(), m_events.end(),
            [&event](const AnimationEvent& existing) {
                return existing.name == event.name && 
                       std::abs(existing.time - event.time) < 0.001f;
            });
        
        if (it != m_events.end()) {
            // Update existing event
            *it = event;
        } else {
            // Add new event
            m_events.push_back(event);
            SortEventsByTime();
        }
    }

    void AnimationEventManager::RemoveEvent(const std::string& eventName, float time) {
        m_events.erase(
            std::remove_if(m_events.begin(), m_events.end(),
                [&eventName, time](const AnimationEvent& event) {
                    return event.name == eventName && 
                           std::abs(event.time - time) < 0.001f;
                }),
            m_events.end());
    }

    void AnimationEventManager::RemoveAllEvents(const std::string& eventName) {
        m_events.erase(
            std::remove_if(m_events.begin(), m_events.end(),
                [&eventName](const AnimationEvent& event) {
                    return event.name == eventName;
                }),
            m_events.end());
    }

    void AnimationEventManager::ClearAllEvents() {
        m_events.clear();
    }

    std::vector<AnimationEvent> AnimationEventManager::GetEvents() const {
        return m_events;
    }

    std::vector<AnimationEvent> AnimationEventManager::GetEventsInTimeRange(float startTime, float endTime) const {
        std::vector<AnimationEvent> result;
        
        for (const auto& event : m_events) {
            if (event.enabled && event.time >= startTime && event.time <= endTime) {
                result.push_back(event);
            }
        }
        
        return result;
    }

    std::vector<AnimationEvent> AnimationEventManager::GetEventsByName(const std::string& eventName) const {
        std::vector<AnimationEvent> result;
        
        for (const auto& event : m_events) {
            if (event.enabled && event.name == eventName) {
                result.push_back(event);
            }
        }
        
        return result;
    }

    std::vector<AnimationEvent> AnimationEventManager::GetEventsByType(AnimationEventType type) const {
        std::vector<AnimationEvent> result;
        
        for (const auto& event : m_events) {
            if (event.enabled && event.type == type) {
                result.push_back(event);
            }
        }
        
        return result;
    }

    bool AnimationEventManager::HasEvent(const std::string& eventName, float time) const {
        return std::any_of(m_events.begin(), m_events.end(),
            [&eventName, time](const AnimationEvent& event) {
                return event.enabled && 
                       event.name == eventName && 
                       std::abs(event.time - time) < 0.001f;
            });
    }

    bool AnimationEventManager::HasEventsInRange(float startTime, float endTime) const {
        return std::any_of(m_events.begin(), m_events.end(),
            [startTime, endTime](const AnimationEvent& event) {
                return event.enabled && 
                       event.time >= startTime && 
                       event.time <= endTime;
            });
    }

    std::vector<AnimationEvent> AnimationEventManager::GetTriggeredEvents(float previousTime, float currentTime, bool looping) const {
        std::vector<AnimationEvent> triggeredEvents;
        
        for (const auto& event : m_events) {
            if (IsEventTriggered(event, previousTime, currentTime, looping)) {
                triggeredEvents.push_back(event);
            }
        }
        
        // Sort by priority (higher priority first)
        std::sort(triggeredEvents.begin(), triggeredEvents.end(),
            [](const AnimationEvent& a, const AnimationEvent& b) {
                return a.priority > b.priority;
            });
        
        return triggeredEvents;
    }

    void AnimationEventManager::ProcessEvents(float previousTime, float currentTime, const AnimationEventCallback& callback, bool looping) const {
        if (!callback) {
            return;
        }
        
        if (looping) {
            ProcessLoopingEvents(previousTime, currentTime, callback);
        } else {
            ProcessLinearEvents(previousTime, currentTime, callback);
        }
    }

    void AnimationEventManager::SetEventEnabled(const std::string& eventName, float time, bool enabled) {
        for (auto& event : m_events) {
            if (event.name == eventName && std::abs(event.time - time) < 0.001f) {
                event.enabled = enabled;
            }
        }
    }

    void AnimationEventManager::SetAllEventsEnabled(bool enabled) {
        for (auto& event : m_events) {
            event.enabled = enabled;
        }
    }

    void AnimationEventManager::SortEventsByTime() {
        std::sort(m_events.begin(), m_events.end(),
            [](const AnimationEvent& a, const AnimationEvent& b) {
                if (std::abs(a.time - b.time) < 0.001f) {
                    return a.priority > b.priority; // Higher priority first for same time
                }
                return a.time < b.time;
            });
    }

    void AnimationEventManager::OptimizeEvents() {
        // Remove invalid events
        m_events.erase(
            std::remove_if(m_events.begin(), m_events.end(),
                [](const AnimationEvent& event) {
                    return !event.IsValid();
                }),
            m_events.end());
        
        // Sort events
        SortEventsByTime();
    }

    bool AnimationEventManager::ValidateEvents() const {
        for (const auto& event : m_events) {
            if (!event.IsValid()) {
                return false;
            }
        }
        return true;
    }

    void AnimationEventManager::PrintEventInfo() const {
        std::cout << "Animation Event Manager Info:\n";
        std::cout << "Total Events: " << m_events.size() << "\n";
        
        for (size_t i = 0; i < m_events.size(); ++i) {
            const auto& event = m_events[i];
            std::cout << "[" << i << "] " << EventUtils::EventToString(event) << "\n";
        }
    }

    std::vector<std::string> AnimationEventManager::GetValidationErrors() const {
        std::vector<std::string> errors;
        
        for (size_t i = 0; i < m_events.size(); ++i) {
            const auto& event = m_events[i];
            
            if (event.name.empty()) {
                errors.push_back("Event " + std::to_string(i) + ": Empty event name");
            }
            
            if (!event.IsTimeValid()) {
                errors.push_back("Event " + std::to_string(i) + " (" + event.name + "): Invalid time " + std::to_string(event.time));
            }
        }
        
        return errors;
    }

    AnimationEventManager::EventManagerData AnimationEventManager::Serialize() const {
        EventManagerData data;
        
        for (const auto& event : m_events) {
            data.events.push_back(event.Serialize());
        }
        
        return data;
    }

    bool AnimationEventManager::Deserialize(const EventManagerData& data) {
        m_events.clear();
        
        for (const auto& eventData : data.events) {
            AnimationEvent event;
            if (event.Deserialize(eventData)) {
                m_events.push_back(event);
            }
        }
        
        SortEventsByTime();
        return ValidateEvents();
    }

    bool AnimationEventManager::IsEventTriggered(const AnimationEvent& event, float previousTime, float currentTime, bool looping) const {
        if (!event.enabled) {
            return false;
        }
        
        if (looping) {
            // Handle looping case where time might wrap around
            if (currentTime < previousTime) {
                // Time wrapped around (0.9 -> 0.1)
                return (event.time >= previousTime && event.time <= 1.0f) ||
                       (event.time >= 0.0f && event.time <= currentTime);
            }
        }
        
        // Normal case: check if event time is between previous and current time
        return event.time > previousTime && event.time <= currentTime;
    }

    void AnimationEventManager::ProcessLoopingEvents(float previousTime, float currentTime, const AnimationEventCallback& callback) const {
        std::vector<AnimationEvent> triggeredEvents;
        
        if (currentTime < previousTime) {
            // Time wrapped around - check events from previousTime to 1.0 and from 0.0 to currentTime
            for (const auto& event : m_events) {
                if (event.enabled) {
                    if ((event.time > previousTime && event.time <= 1.0f) ||
                        (event.time >= 0.0f && event.time <= currentTime)) {
                        triggeredEvents.push_back(event);
                    }
                }
            }
        } else {
            // Normal case
            for (const auto& event : m_events) {
                if (event.enabled && event.time > previousTime && event.time <= currentTime) {
                    triggeredEvents.push_back(event);
                }
            }
        }
        
        // Sort by time, then by priority
        std::sort(triggeredEvents.begin(), triggeredEvents.end(),
            [](const AnimationEvent& a, const AnimationEvent& b) {
                if (std::abs(a.time - b.time) < 0.001f) {
                    return a.priority > b.priority;
                }
                return a.time < b.time;
            });
        
        // Trigger events
        for (const auto& event : triggeredEvents) {
            callback(event);
        }
    }

    void AnimationEventManager::ProcessLinearEvents(float previousTime, float currentTime, const AnimationEventCallback& callback) const {
        for (const auto& event : m_events) {
            if (event.enabled && event.time > previousTime && event.time <= currentTime) {
                callback(event);
            }
        }
    }

    // AnimationEventHistory implementation
    void AnimationEventHistory::AddTriggeredEvent(const AnimationEvent& event, float actualTime, float animationTime, const std::string& animationName) {
        TriggeredEvent triggeredEvent;
        triggeredEvent.event = event;
        triggeredEvent.actualTime = actualTime;
        triggeredEvent.animationTime = animationTime;
        triggeredEvent.animationName = animationName;
        
        // Get current timestamp
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = now.time_since_epoch();
        triggeredEvent.timestamp = std::chrono::duration<double>(duration).count();
        
        triggeredEvents.push_back(triggeredEvent);
        
        // Maintain maximum history size
        if (triggeredEvents.size() > maxHistorySize) {
            triggeredEvents.erase(triggeredEvents.begin());
        }
    }

    void AnimationEventHistory::ClearHistory() {
        triggeredEvents.clear();
    }

    std::vector<AnimationEventHistory::TriggeredEvent> AnimationEventHistory::GetRecentEvents(size_t count) const {
        if (count >= triggeredEvents.size()) {
            return triggeredEvents;
        }
        
        return std::vector<TriggeredEvent>(
            triggeredEvents.end() - count,
            triggeredEvents.end()
        );
    }

    std::vector<AnimationEventHistory::TriggeredEvent> AnimationEventHistory::GetEventsByName(const std::string& eventName) const {
        std::vector<TriggeredEvent> result;
        
        for (const auto& triggeredEvent : triggeredEvents) {
            if (triggeredEvent.event.name == eventName) {
                result.push_back(triggeredEvent);
            }
        }
        
        return result;
    }

    void AnimationEventHistory::PrintHistory() const {
        std::cout << "Animation Event History (" << triggeredEvents.size() << " events):\n";
        
        for (size_t i = 0; i < triggeredEvents.size(); ++i) {
            const auto& triggered = triggeredEvents[i];
            std::cout << "[" << i << "] " << triggered.event.name 
                      << " at " << triggered.actualTime 
                      << " (anim: " << triggered.animationName << ")\n";
        }
    }

    // EventUtils implementation
    namespace EventUtils {
        AnimationEvent CreateSoundEvent(const std::string& soundName, float time, const std::string& soundFile) {
            AnimationEvent event(soundName, time, AnimationEventType::Sound);
            event.stringParameter = soundFile;
            event.description = "Sound event: " + soundName;
            return event;
        }

        AnimationEvent CreateEffectEvent(const std::string& effectName, float time, const std::string& effectType) {
            AnimationEvent event(effectName, time, AnimationEventType::Effect);
            event.stringParameter = effectType;
            event.description = "Effect event: " + effectName;
            return event;
        }

        AnimationEvent CreateFootstepEvent(float time, const std::string& surface, float volume) {
            AnimationEvent event("Footstep", time, AnimationEventType::Footstep);
            event.stringParameter = surface;
            event.floatParameter = volume;
            event.description = "Footstep on " + surface;
            return event;
        }

        AnimationEvent CreateCombatEvent(const std::string& actionName, float time, int damage) {
            AnimationEvent event(actionName, time, AnimationEventType::Combat);
            event.intParameter = damage;
            event.description = "Combat action: " + actionName;
            return event;
        }

        bool ValidateEventTime(float time) {
            return time >= 0.0f && time <= 1.0f;
        }

        bool ValidateEventName(const std::string& name) {
            return !name.empty() && name.length() <= 64; // Reasonable name length limit
        }

        std::string GetEventTypeString(AnimationEventType type) {
            switch (type) {
                case AnimationEventType::Generic: return "Generic";
                case AnimationEventType::Sound: return "Sound";
                case AnimationEventType::Effect: return "Effect";
                case AnimationEventType::Footstep: return "Footstep";
                case AnimationEventType::Combat: return "Combat";
                case AnimationEventType::Custom: return "Custom";
                default: return "Unknown";
            }
        }

        AnimationEventType ParseEventType(const std::string& typeString) {
            if (typeString == "Generic") return AnimationEventType::Generic;
            if (typeString == "Sound") return AnimationEventType::Sound;
            if (typeString == "Effect") return AnimationEventType::Effect;
            if (typeString == "Footstep") return AnimationEventType::Footstep;
            if (typeString == "Combat") return AnimationEventType::Combat;
            if (typeString == "Custom") return AnimationEventType::Custom;
            return AnimationEventType::Generic;
        }

        std::vector<AnimationEvent> FilterEventsByType(const std::vector<AnimationEvent>& events, AnimationEventType type) {
            std::vector<AnimationEvent> result;
            
            for (const auto& event : events) {
                if (event.type == type) {
                    result.push_back(event);
                }
            }
            
            return result;
        }

        std::vector<AnimationEvent> FilterEventsByTimeRange(const std::vector<AnimationEvent>& events, float startTime, float endTime) {
            std::vector<AnimationEvent> result;
            
            for (const auto& event : events) {
                if (event.time >= startTime && event.time <= endTime) {
                    result.push_back(event);
                }
            }
            
            return result;
        }

        void SortEventsByTime(std::vector<AnimationEvent>& events) {
            std::sort(events.begin(), events.end(),
                [](const AnimationEvent& a, const AnimationEvent& b) {
                    return a.time < b.time;
                });
        }

        void PrintEvent(const AnimationEvent& event) {
            std::cout << EventToString(event) << std::endl;
        }

        void PrintEventList(const std::vector<AnimationEvent>& events) {
            std::cout << "Events (" << events.size() << "):\n";
            for (size_t i = 0; i < events.size(); ++i) {
                std::cout << "[" << i << "] " << EventToString(events[i]) << "\n";
            }
        }

        std::string EventToString(const AnimationEvent& event) {
            std::stringstream ss;
            ss << event.name << " (" << GetEventTypeString(event.type) << ") at " << event.time;
            
            if (!event.stringParameter.empty()) {
                ss << " [str: " << event.stringParameter << "]";
            }
            if (event.floatParameter != 0.0f) {
                ss << " [float: " << event.floatParameter << "]";
            }
            if (event.intParameter != 0) {
                ss << " [int: " << event.intParameter << "]";
            }
            if (event.boolParameter) {
                ss << " [bool: true]";
            }
            
            if (!event.enabled) {
                ss << " [DISABLED]";
            }
            
            return ss.str();
        }
    }

} // namespace Animation
} // namespace GameEngine