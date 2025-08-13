#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>

namespace GameEngine {
namespace Animation {

    /**
     * Animation event types for categorizing different kinds of events
     */
    enum class AnimationEventType {
        Generic,    // General purpose event
        Sound,      // Audio trigger event
        Effect,     // Visual effect trigger
        Footstep,   // Footstep/movement event
        Combat,     // Combat-related event
        Custom      // User-defined event type
    };

    /**
     * Animation event structure for triggering game logic during animations
     */
    struct AnimationEvent {
        std::string name;                    // Event identifier
        float time = 0.0f;                  // Normalized time (0-1) when event should trigger
        AnimationEventType type = AnimationEventType::Generic;
        
        // Event parameters for passing data to callbacks
        std::string stringParameter;
        float floatParameter = 0.0f;
        int intParameter = 0;
        bool boolParameter = false;
        
        // Additional metadata
        std::string description;             // Optional description for debugging
        int priority = 0;                   // Event priority for ordering
        bool enabled = true;                // Whether this event is active
        
        // Constructors
        AnimationEvent() = default;
        AnimationEvent(const std::string& eventName, float eventTime, AnimationEventType eventType = AnimationEventType::Generic);
        
        // Validation
        bool IsValid() const;
        bool IsTimeValid() const { return time >= 0.0f && time <= 1.0f; }
        
        // Comparison operators for sorting
        bool operator<(const AnimationEvent& other) const { return time < other.time; }
        bool operator==(const AnimationEvent& other) const;
        
        // Utility methods
        void SetStringParameter(const std::string& value) { stringParameter = value; }
        void SetFloatParameter(float value) { floatParameter = value; }
        void SetIntParameter(int value) { intParameter = value; }
        void SetBoolParameter(bool value) { boolParameter = value; }
        
        // Serialization support
        struct EventData {
            std::string name;
            float time;
            int type; // AnimationEventType as int
            std::string stringParameter;
            float floatParameter;
            int intParameter;
            bool boolParameter;
            std::string description;
            int priority;
            bool enabled;
        };
        
        EventData Serialize() const;
        bool Deserialize(const EventData& data);
    };

    /**
     * Event callback function type
     */
    using AnimationEventCallback = std::function<void(const AnimationEvent&)>;

    /**
     * Animation event manager for handling event registration and triggering
     */
    class AnimationEventManager {
    public:
        AnimationEventManager() = default;
        ~AnimationEventManager() = default;

        // Event registration
        void AddEvent(const AnimationEvent& event);
        void RemoveEvent(const std::string& eventName, float time);
        void RemoveAllEvents(const std::string& eventName);
        void ClearAllEvents();
        
        // Event queries
        std::vector<AnimationEvent> GetEvents() const;
        std::vector<AnimationEvent> GetEventsInTimeRange(float startTime, float endTime) const;
        std::vector<AnimationEvent> GetEventsByName(const std::string& eventName) const;
        std::vector<AnimationEvent> GetEventsByType(AnimationEventType type) const;
        
        bool HasEvent(const std::string& eventName, float time) const;
        bool HasEventsInRange(float startTime, float endTime) const;
        size_t GetEventCount() const { return m_events.size(); }
        bool IsEmpty() const { return m_events.empty(); }
        
        // Event processing
        std::vector<AnimationEvent> GetTriggeredEvents(float previousTime, float currentTime, bool looping = false) const;
        void ProcessEvents(float previousTime, float currentTime, const AnimationEventCallback& callback, bool looping = false) const;
        
        // Event modification
        void SetEventEnabled(const std::string& eventName, float time, bool enabled);
        void SetAllEventsEnabled(bool enabled);
        
        // Sorting and optimization
        void SortEventsByTime();
        void OptimizeEvents(); // Remove duplicate or invalid events
        
        // Debugging and validation
        bool ValidateEvents() const;
        void PrintEventInfo() const;
        std::vector<std::string> GetValidationErrors() const;
        
        // Serialization
        struct EventManagerData {
            std::vector<AnimationEvent::EventData> events;
        };
        
        EventManagerData Serialize() const;
        bool Deserialize(const EventManagerData& data);

    private:
        std::vector<AnimationEvent> m_events;
        
        // Helper methods
        bool IsEventTriggered(const AnimationEvent& event, float previousTime, float currentTime, bool looping) const;
        void ProcessLoopingEvents(float previousTime, float currentTime, const AnimationEventCallback& callback) const;
        void ProcessLinearEvents(float previousTime, float currentTime, const AnimationEventCallback& callback) const;
    };

    /**
     * Event history tracker for debugging and analysis
     */
    struct AnimationEventHistory {
        struct TriggeredEvent {
            AnimationEvent event;
            float actualTime;      // Actual time when event was triggered
            float animationTime;   // Animation time when event was triggered
            double timestamp;      // System timestamp when event was triggered
            std::string animationName; // Name of animation that triggered the event
        };
        
        std::vector<TriggeredEvent> triggeredEvents;
        size_t maxHistorySize = 100; // Maximum number of events to keep in history
        
        void AddTriggeredEvent(const AnimationEvent& event, float actualTime, float animationTime, const std::string& animationName);
        void ClearHistory();
        std::vector<TriggeredEvent> GetRecentEvents(size_t count) const;
        std::vector<TriggeredEvent> GetEventsByName(const std::string& eventName) const;
        void PrintHistory() const;
    };

    /**
     * Utility functions for animation events
     */
    namespace EventUtils {
        // Event creation helpers
        AnimationEvent CreateSoundEvent(const std::string& soundName, float time, const std::string& soundFile = "");
        AnimationEvent CreateEffectEvent(const std::string& effectName, float time, const std::string& effectType = "");
        AnimationEvent CreateFootstepEvent(float time, const std::string& surface = "", float volume = 1.0f);
        AnimationEvent CreateCombatEvent(const std::string& actionName, float time, int damage = 0);
        
        // Event validation
        bool ValidateEventTime(float time);
        bool ValidateEventName(const std::string& name);
        std::string GetEventTypeString(AnimationEventType type);
        AnimationEventType ParseEventType(const std::string& typeString);
        
        // Event processing utilities
        std::vector<AnimationEvent> FilterEventsByType(const std::vector<AnimationEvent>& events, AnimationEventType type);
        std::vector<AnimationEvent> FilterEventsByTimeRange(const std::vector<AnimationEvent>& events, float startTime, float endTime);
        void SortEventsByTime(std::vector<AnimationEvent>& events);
        
        // Debugging utilities
        void PrintEvent(const AnimationEvent& event);
        void PrintEventList(const std::vector<AnimationEvent>& events);
        std::string EventToString(const AnimationEvent& event);
    }

} // namespace Animation
} // namespace GameEngine