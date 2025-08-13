#include "TestUtils.h"
#include "Animation/AnimationEvent.h"
#include "Animation/SkeletalAnimation.h"
#include "Animation/AnimationController.h"

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Animation;

/**
 * Test AnimationEvent structure creation and validation
 * Requirements: 6.1, 6.2 (Event structure with time and parameter data)
 */
bool TestAnimationEventCreation() {
    TestOutput::PrintTestStart("animation event creation");

    // Test basic event creation
    AnimationEvent event("TestEvent", 0.5f, AnimationEventType::Generic);
    EXPECT_TRUE(event.IsValid());
    EXPECT_EQUAL(event.name, "TestEvent");
    EXPECT_NEARLY_EQUAL(event.time, 0.5f);
    EXPECT_EQUAL(static_cast<int>(event.type), static_cast<int>(AnimationEventType::Generic));

    // Test event with parameters
    AnimationEvent soundEvent("FootstepSound", 0.3f, AnimationEventType::Sound);
    soundEvent.SetStringParameter("grass");
    soundEvent.SetFloatParameter(0.8f);
    soundEvent.SetIntParameter(1);
    soundEvent.SetBoolParameter(true);

    EXPECT_TRUE(soundEvent.IsValid());
    EXPECT_EQUAL(soundEvent.stringParameter, "grass");
    EXPECT_NEARLY_EQUAL(soundEvent.floatParameter, 0.8f);
    EXPECT_EQUAL(soundEvent.intParameter, 1);
    EXPECT_TRUE(soundEvent.boolParameter);

    // Test invalid event (empty name)
    AnimationEvent invalidEvent("", 0.5f);
    EXPECT_FALSE(invalidEvent.IsValid());

    // Test invalid event (time out of range)
    AnimationEvent invalidTimeEvent("Test", 1.5f);
    EXPECT_FALSE(invalidTimeEvent.IsValid());

    TestOutput::PrintTestPass("animation event creation");
    return true;
}

/**
 * Test AnimationEventManager functionality
 * Requirements: 6.1, 6.2 (Event registration and callback system)
 */
bool TestAnimationEventManager() {
    TestOutput::PrintTestStart("animation event manager");

    AnimationEventManager manager;

    // Test adding events
    AnimationEvent event1("Event1", 0.2f, AnimationEventType::Generic);
    AnimationEvent event2("Event2", 0.5f, AnimationEventType::Sound);
    AnimationEvent event3("Event3", 0.8f, AnimationEventType::Effect);

    manager.AddEvent(event1);
    manager.AddEvent(event2);
    manager.AddEvent(event3);

    EXPECT_EQUAL(manager.GetEventCount(), static_cast<size_t>(3));
    EXPECT_FALSE(manager.IsEmpty());

    // Test event queries
    EXPECT_TRUE(manager.HasEvent("Event1", 0.2f));
    EXPECT_FALSE(manager.HasEvent("Event1", 0.3f));
    EXPECT_TRUE(manager.HasEventsInRange(0.1f, 0.6f));

    // Test getting events by type
    auto soundEvents = manager.GetEventsByType(AnimationEventType::Sound);
    EXPECT_EQUAL(soundEvents.size(), static_cast<size_t>(1));
    EXPECT_EQUAL(soundEvents[0].name, "Event2");

    // Test getting events in time range
    auto rangeEvents = manager.GetEventsInTimeRange(0.4f, 0.9f);
    EXPECT_EQUAL(rangeEvents.size(), static_cast<size_t>(2)); // Event2 and Event3

    // Test removing events
    manager.RemoveEvent("Event2", 0.5f);
    EXPECT_EQUAL(manager.GetEventCount(), static_cast<size_t>(2));
    EXPECT_FALSE(manager.HasEvent("Event2", 0.5f));

    // Test clearing all events
    manager.ClearAllEvents();
    EXPECT_TRUE(manager.IsEmpty());
    EXPECT_EQUAL(manager.GetEventCount(), static_cast<size_t>(0));

    TestOutput::PrintTestPass("animation event manager");
    return true;
}

/**
 * Test event triggering during animation playback
 * Requirements: 6.4 (Event triggering during animation playback)
 */
bool TestEventTriggering() {
    TestOutput::PrintTestStart("event triggering");

    AnimationEventManager manager;

    // Add events at different times
    AnimationEvent event1("Early", 0.2f);
    AnimationEvent event2("Middle", 0.5f);
    AnimationEvent event3("Late", 0.8f);

    manager.AddEvent(event1);
    manager.AddEvent(event2);
    manager.AddEvent(event3);

    // Test linear playback (0.0 -> 0.6)
    auto triggeredEvents = manager.GetTriggeredEvents(0.0f, 0.6f, false);
    EXPECT_EQUAL(triggeredEvents.size(), static_cast<size_t>(2)); // Early and Middle

    // Test partial playback (0.3 -> 0.7)
    triggeredEvents = manager.GetTriggeredEvents(0.3f, 0.7f, false);
    EXPECT_EQUAL(triggeredEvents.size(), static_cast<size_t>(1)); // Only Middle

    // Test looping playback (0.9 -> 0.3, wrapping around)
    triggeredEvents = manager.GetTriggeredEvents(0.9f, 0.3f, true);
    EXPECT_EQUAL(triggeredEvents.size(), static_cast<size_t>(1)); // Only Early

    // Test callback processing
    std::vector<std::string> triggeredEventNames;
    auto callback = [&triggeredEventNames](const AnimationEvent& event) {
        triggeredEventNames.push_back(event.name);
    };

    manager.ProcessEvents(0.0f, 1.0f, callback, false);
    EXPECT_EQUAL(triggeredEventNames.size(), static_cast<size_t>(3));
    EXPECT_EQUAL(triggeredEventNames[0], "Early");
    EXPECT_EQUAL(triggeredEventNames[1], "Middle");
    EXPECT_EQUAL(triggeredEventNames[2], "Late");

    TestOutput::PrintTestPass("event triggering");
    return true;
}

/**
 * Test Animation class event integration
 * Requirements: 6.1, 6.2 (Event registration and callback system to animations)
 */
bool TestAnimationEventIntegration() {
    TestOutput::PrintTestStart("animation event integration");

    GameEngine::Animation::SkeletalAnimation animation("TestAnimation");
    animation.SetDuration(2.0f);

    // Add events to animation (using normalized time 0-1)
    AnimationEvent startEvent("AnimationStart", 0.0f, AnimationEventType::Generic);
    AnimationEvent midEvent("AnimationMid", 0.5f, AnimationEventType::Sound);
    AnimationEvent endEvent("AnimationEnd", 1.0f, AnimationEventType::Effect);

    animation.AddEvent(startEvent);
    animation.AddEvent(midEvent);
    animation.AddEvent(endEvent);

    EXPECT_EQUAL(animation.GetEventCount(), static_cast<size_t>(3));
    EXPECT_TRUE(animation.HasEvent("AnimationStart", 0.0f));
    EXPECT_TRUE(animation.HasEvent("AnimationMid", 1.0f)); // 0.5 normalized time = 1.0 absolute time
    EXPECT_TRUE(animation.HasEvent("AnimationEnd", 2.0f)); // 1.0 normalized time = 2.0 absolute time

    // Test getting events by type
    auto soundEvents = animation.GetEventsByType(AnimationEventType::Sound);
    EXPECT_EQUAL(soundEvents.size(), static_cast<size_t>(1));
    EXPECT_EQUAL(soundEvents[0].name, "AnimationMid");

    // Test event processing with callback
    std::vector<std::string> triggeredEvents;
    auto callback = [&triggeredEvents](const AnimationEvent& event) {
        triggeredEvents.push_back(event.name);
    };

    // Process events using normalized time (0.0 to 1.0)
    // Events are triggered when time > previousTime and time <= currentTime
    // So events at 0.5 and 1.0 should trigger, but not the one at 0.0
    animation.ProcessEvents(0.0f, 1.0f, callback, false);
    EXPECT_EQUAL(triggeredEvents.size(), static_cast<size_t>(1)); // Only endEvent should trigger (at 1.0)

    TestOutput::PrintTestPass("animation event integration");
    return true;
}

/**
 * Test AnimationEventHistory for debugging
 * Requirements: 6.5, 6.7 (Event history and debugging information)
 */
bool TestAnimationEventHistory() {
    TestOutput::PrintTestStart("animation event history");

    AnimationEventHistory history;
    history.maxHistorySize = 5;

    // Add some triggered events
    AnimationEvent event1("Event1", 0.2f);
    AnimationEvent event2("Event2", 0.5f);
    AnimationEvent event3("Event3", 0.8f);

    history.AddTriggeredEvent(event1, 0.2f, 0.2f, "TestAnimation");
    history.AddTriggeredEvent(event2, 0.5f, 0.5f, "TestAnimation");
    history.AddTriggeredEvent(event3, 0.8f, 0.8f, "TestAnimation");

    EXPECT_EQUAL(history.triggeredEvents.size(), static_cast<size_t>(3));

    // Test getting recent events
    auto recentEvents = history.GetRecentEvents(2);
    EXPECT_EQUAL(recentEvents.size(), static_cast<size_t>(2));
    EXPECT_EQUAL(recentEvents[0].event.name, "Event2");
    EXPECT_EQUAL(recentEvents[1].event.name, "Event3");

    // Test getting events by name
    auto event1History = history.GetEventsByName("Event1");
    EXPECT_EQUAL(event1History.size(), static_cast<size_t>(1));
    EXPECT_EQUAL(event1History[0].event.name, "Event1");

    // Test history size limit
    for (int i = 0; i < 10; ++i) {
        AnimationEvent extraEvent("Extra" + std::to_string(i), 0.1f);
        history.AddTriggeredEvent(extraEvent, 0.1f, 0.1f, "TestAnimation");
    }

    EXPECT_EQUAL(history.triggeredEvents.size(), static_cast<size_t>(5)); // Limited to maxHistorySize

    // Test clearing history
    history.ClearHistory();
    EXPECT_EQUAL(history.triggeredEvents.size(), static_cast<size_t>(0));

    TestOutput::PrintTestPass("animation event history");
    return true;
}

/**
 * Test EventUtils helper functions
 * Requirements: 6.1, 6.2 (Event creation and validation utilities)
 */
bool TestEventUtils() {
    TestOutput::PrintTestStart("event utils");

    // Test event creation helpers
    auto soundEvent = EventUtils::CreateSoundEvent("Footstep", 0.3f, "footstep.wav");
    EXPECT_EQUAL(soundEvent.name, "Footstep");
    EXPECT_NEARLY_EQUAL(soundEvent.time, 0.3f);
    EXPECT_EQUAL(static_cast<int>(soundEvent.type), static_cast<int>(AnimationEventType::Sound));
    EXPECT_EQUAL(soundEvent.stringParameter, "footstep.wav");

    auto effectEvent = EventUtils::CreateEffectEvent("Explosion", 0.7f, "fire");
    EXPECT_EQUAL(effectEvent.name, "Explosion");
    EXPECT_EQUAL(static_cast<int>(effectEvent.type), static_cast<int>(AnimationEventType::Effect));
    EXPECT_EQUAL(effectEvent.stringParameter, "fire");

    auto footstepEvent = EventUtils::CreateFootstepEvent(0.5f, "grass", 0.8f);
    EXPECT_EQUAL(footstepEvent.name, "Footstep");
    EXPECT_EQUAL(static_cast<int>(footstepEvent.type), static_cast<int>(AnimationEventType::Footstep));
    EXPECT_EQUAL(footstepEvent.stringParameter, "grass");
    EXPECT_NEARLY_EQUAL(footstepEvent.floatParameter, 0.8f);

    auto combatEvent = EventUtils::CreateCombatEvent("Sword Strike", 0.6f, 25);
    EXPECT_EQUAL(combatEvent.name, "Sword Strike");
    EXPECT_EQUAL(static_cast<int>(combatEvent.type), static_cast<int>(AnimationEventType::Combat));
    EXPECT_EQUAL(combatEvent.intParameter, 25);

    // Test validation functions
    EXPECT_TRUE(EventUtils::ValidateEventTime(0.5f));
    EXPECT_FALSE(EventUtils::ValidateEventTime(1.5f));
    EXPECT_FALSE(EventUtils::ValidateEventTime(-0.1f));

    EXPECT_TRUE(EventUtils::ValidateEventName("ValidName"));
    EXPECT_FALSE(EventUtils::ValidateEventName(""));

    // Test type string conversion
    EXPECT_EQUAL(EventUtils::GetEventTypeString(AnimationEventType::Sound), "Sound");
    EXPECT_EQUAL(EventUtils::GetEventTypeString(AnimationEventType::Effect), "Effect");
    EXPECT_EQUAL(static_cast<int>(EventUtils::ParseEventType("Sound")), static_cast<int>(AnimationEventType::Sound));
    EXPECT_EQUAL(static_cast<int>(EventUtils::ParseEventType("Effect")), static_cast<int>(AnimationEventType::Effect));

    TestOutput::PrintTestPass("event utils");
    return true;
}

/**
 * Test event handling for non-linear playback and scrubbing
 * Requirements: 6.7 (Event handling for non-linear playback and scrubbing)
 */
bool TestNonLinearEventHandling() {
    TestOutput::PrintTestStart("non-linear event handling");

    AnimationEventManager manager;

    // Add events throughout the timeline
    for (int i = 0; i < 10; ++i) {
        float time = i * 0.1f;
        AnimationEvent event("Event" + std::to_string(i), time);
        manager.AddEvent(event);
    }

    // Test scrubbing backwards (should not trigger events)
    auto triggeredEvents = manager.GetTriggeredEvents(0.8f, 0.2f, false);
    EXPECT_EQUAL(triggeredEvents.size(), static_cast<size_t>(0));

    // Test scrubbing forwards (should trigger events in range)
    // Events at 0.3, 0.4, 0.5, 0.6, 0.7, 0.8 (events 3-8) should trigger
    triggeredEvents = manager.GetTriggeredEvents(0.2f, 0.8f, false);
    EXPECT_EQUAL(triggeredEvents.size(), static_cast<size_t>(6)); // Events 3-8

    // Test looping with wrap-around (0.8 -> 0.2)
    // Should trigger events 9 (0.9), 0 (0.0), 1 (0.1), 2 (0.2)
    triggeredEvents = manager.GetTriggeredEvents(0.8f, 0.2f, true);
    EXPECT_EQUAL(triggeredEvents.size(), static_cast<size_t>(5)); // Events 9, 0, 1, 2 + event at 0.9

    // Test event processing with scrubbing callback
    std::vector<std::string> triggeredEventNames;
    auto callback = [&triggeredEventNames](const AnimationEvent& event) {
        triggeredEventNames.push_back(event.name);
    };

    // Process events for a jump in time (0.1 -> 0.7)
    // Should trigger events at 0.2, 0.3, 0.4, 0.5, 0.6, 0.7 (events 2-7)
    manager.ProcessEvents(0.1f, 0.7f, callback, false);
    EXPECT_EQUAL(triggeredEventNames.size(), static_cast<size_t>(6)); // Events 2-7

    TestOutput::PrintTestPass("non-linear event handling");
    return true;
}

int main() {
    TestOutput::PrintHeader("AnimationEvent");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("AnimationEvent Tests");

        // Run all tests
        allPassed &= suite.RunTest("Animation Event Creation", TestAnimationEventCreation);
        allPassed &= suite.RunTest("Animation Event Manager", TestAnimationEventManager);
        allPassed &= suite.RunTest("Event Triggering", TestEventTriggering);
        allPassed &= suite.RunTest("Animation Event Integration", TestAnimationEventIntegration);
        allPassed &= suite.RunTest("Animation Event History", TestAnimationEventHistory);
        allPassed &= suite.RunTest("Event Utils", TestEventUtils);
        allPassed &= suite.RunTest("Non-Linear Event Handling", TestNonLinearEventHandling);

        // Print detailed summary
        suite.PrintSummary();

        TestOutput::PrintFooter(allPassed);
        return allPassed ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
        return 1;
    } catch (...) {
        TestOutput::PrintError("UNKNOWN TEST ERROR!");
        return 1;
    }
}