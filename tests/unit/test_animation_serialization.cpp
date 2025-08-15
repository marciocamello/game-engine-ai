#include "TestUtils.h"
#include "Animation/AnimationSerialization.h"
#include "Animation/SkeletalAnimation.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/BlendTree.h"
#include "Animation/AnimationTransition.h"

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Animation;

/**
 * Test skeletal animation serialization and deserialization
 * Requirements: 7.3, 8.6, 8.7
 */
bool TestSkeletalAnimationSerialization() {
    TestOutput::PrintTestStart("skeletal animation serialization");

    // Create a test animation
    SkeletalAnimation animation("TestAnimation");
    animation.SetDuration(2.0f);
    animation.SetFrameRate(30.0f);
    animation.SetLoopMode(LoopMode::Loop);

    // Add some keyframes
    animation.AddPositionKeyframe("root", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    animation.AddPositionKeyframe("root", 1.0f, Math::Vec3(1.0f, 0.0f, 0.0f));
    animation.AddPositionKeyframe("root", 2.0f, Math::Vec3(2.0f, 0.0f, 0.0f));

    animation.AddRotationKeyframe("root", 0.0f, Math::Quat(1.0f, 0.0f, 0.0f, 0.0f));
    animation.AddRotationKeyframe("root", 2.0f, Math::Quat(0.707f, 0.0f, 0.707f, 0.0f));

    // Add an event
    AnimationEvent event;
    event.name = "TestEvent";
    event.time = 1.0f;
    event.stringParameter = "test";
    event.floatParameter = 42.0f;
    event.type = AnimationEventType::Generic;
    animation.AddEvent(event);

    // Serialize
    std::string serializedData = AnimationSerialization::SerializeSkeletalAnimation(animation);
    EXPECT_FALSE(serializedData.empty());

    // Deserialize
    auto deserializedAnimation = AnimationSerialization::DeserializeSkeletalAnimation(serializedData);
    EXPECT_TRUE(deserializedAnimation != nullptr);

    if (deserializedAnimation) {
        EXPECT_EQUAL(deserializedAnimation->GetName(), "TestAnimation");
        EXPECT_NEARLY_EQUAL(deserializedAnimation->GetDuration(), 2.0f);
        EXPECT_NEARLY_EQUAL(deserializedAnimation->GetFrameRate(), 30.0f);
        EXPECT_EQUAL(static_cast<int>(deserializedAnimation->GetLoopMode()), static_cast<int>(LoopMode::Loop));

        // Check if bone animation was preserved
        EXPECT_TRUE(deserializedAnimation->HasBone("root"));

        // Check events
        auto events = deserializedAnimation->GetEvents();
        EXPECT_EQUAL(events.size(), static_cast<size_t>(1));
        if (!events.empty()) {
            EXPECT_EQUAL(events[0].name, "TestEvent");
            EXPECT_NEARLY_EQUAL(events[0].time, 1.0f);
            EXPECT_EQUAL(events[0].stringParameter, "test");
            EXPECT_NEARLY_EQUAL(events[0].floatParameter, 42.0f);
        }
    }

    TestOutput::PrintTestPass("skeletal animation serialization");
    return true;
}

/**
 * Test animation asset serialization
 * Requirements: 7.3, 8.6, 8.7
 */
bool TestAnimationAssetSerialization() {
    TestOutput::PrintTestStart("animation asset serialization");

    // Create test asset
    AnimationSerialization::AnimationAsset asset;
    asset.name = "TestAsset";
    asset.type = "skeletal_animation";
    asset.version = "1.0.0";
    asset.sourceFile = "test.fbx";
    asset.data = "{\"test\": \"data\"}";
    asset.timestamp = 1234567890;
    asset.dataSize = asset.data.size();

    // Serialize
    std::string serializedData = AnimationSerialization::SerializeAnimationAsset(asset);
    EXPECT_FALSE(serializedData.empty());

    // Deserialize
    auto deserializedAsset = AnimationSerialization::DeserializeAnimationAsset(serializedData);
    EXPECT_EQUAL(deserializedAsset.name, "TestAsset");
    EXPECT_EQUAL(deserializedAsset.type, "skeletal_animation");
    EXPECT_EQUAL(deserializedAsset.version, "1.0.0");
    EXPECT_EQUAL(deserializedAsset.sourceFile, "test.fbx");
    EXPECT_EQUAL(deserializedAsset.data, "{\"test\": \"data\"}");
    EXPECT_EQUAL(deserializedAsset.timestamp, static_cast<uint64_t>(1234567890));
    EXPECT_EQUAL(deserializedAsset.dataSize, asset.data.size());

    TestOutput::PrintTestPass("animation asset serialization");
    return true;
}

/**
 * Test animation collection serialization
 * Requirements: 7.3, 8.6, 8.7
 */
bool TestAnimationCollectionSerialization() {
    TestOutput::PrintTestStart("animation collection serialization");

    // Create test collection
    AnimationSerialization::AnimationCollection collection;
    collection.name = "TestCollection";
    collection.version = "1.0.0";

    // Add test assets
    AnimationSerialization::AnimationAsset asset1;
    asset1.name = "Animation1";
    asset1.type = "skeletal_animation";
    asset1.version = "1.0.0";
    collection.animations.push_back(asset1);

    AnimationSerialization::AnimationAsset asset2;
    asset2.name = "StateMachine1";
    asset2.type = "state_machine";
    asset2.version = "1.0.0";
    collection.stateMachines.push_back(asset2);

    // Serialize
    std::string serializedData = AnimationSerialization::SerializeAnimationCollection(collection);
    EXPECT_FALSE(serializedData.empty());

    // Deserialize
    auto deserializedCollection = AnimationSerialization::DeserializeAnimationCollection(serializedData);
    EXPECT_EQUAL(deserializedCollection.name, "TestCollection");
    EXPECT_EQUAL(deserializedCollection.version, "1.0.0");
    EXPECT_EQUAL(deserializedCollection.animations.size(), static_cast<size_t>(1));
    EXPECT_EQUAL(deserializedCollection.stateMachines.size(), static_cast<size_t>(1));

    if (!deserializedCollection.animations.empty()) {
        EXPECT_EQUAL(deserializedCollection.animations[0].name, "Animation1");
        EXPECT_EQUAL(deserializedCollection.animations[0].type, "skeletal_animation");
    }

    if (!deserializedCollection.stateMachines.empty()) {
        EXPECT_EQUAL(deserializedCollection.stateMachines[0].name, "StateMachine1");
        EXPECT_EQUAL(deserializedCollection.stateMachines[0].type, "state_machine");
    }

    TestOutput::PrintTestPass("animation collection serialization");
    return true;
}

/**
 * Test file I/O operations
 * Requirements: 7.3, 8.6, 8.7
 */
bool TestFileIOOperations() {
    TestOutput::PrintTestStart("file I/O operations");

    // Create a test animation
    SkeletalAnimation animation("FileTestAnimation");
    animation.SetDuration(1.0f);
    animation.SetFrameRate(24.0f);
    animation.AddPositionKeyframe("bone1", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    animation.AddPositionKeyframe("bone1", 1.0f, Math::Vec3(1.0f, 1.0f, 1.0f));

    // Test save to file
    std::string testFilePath = "test_animation.json";
    bool saveResult = AnimationSerialization::SaveAnimationToFile(animation, testFilePath);
    EXPECT_TRUE(saveResult);

    // Test load from file
    auto loadedAnimation = AnimationSerialization::LoadAnimationFromFile(testFilePath);
    EXPECT_TRUE(loadedAnimation != nullptr);

    if (loadedAnimation) {
        EXPECT_EQUAL(loadedAnimation->GetName(), "FileTestAnimation");
        EXPECT_NEARLY_EQUAL(loadedAnimation->GetDuration(), 1.0f);
        EXPECT_NEARLY_EQUAL(loadedAnimation->GetFrameRate(), 24.0f);
        EXPECT_TRUE(loadedAnimation->HasBone("bone1"));
    }

    // Clean up test file
    std::remove(testFilePath.c_str());

    TestOutput::PrintTestPass("file I/O operations");
    return true;
}

/**
 * Test data validation
 * Requirements: 7.3, 8.6, 8.7
 */
bool TestDataValidation() {
    TestOutput::PrintTestStart("data validation");

    // Test valid data
    std::string validData = R"({
        "type": "skeletal_animation",
        "version": "1.0.0",
        "name": "TestAnimation"
    })";
    
    bool isValid = AnimationSerialization::ValidateAnimationData(validData, "skeletal_animation");
    EXPECT_TRUE(isValid);

    // Test invalid type
    std::string invalidTypeData = R"({
        "type": "invalid_type",
        "version": "1.0.0",
        "name": "TestAnimation"
    })";
    
    bool isInvalidType = AnimationSerialization::ValidateAnimationData(invalidTypeData, "skeletal_animation");
    EXPECT_FALSE(isInvalidType);

    // Test missing fields
    std::string missingFieldsData = R"({
        "name": "TestAnimation"
    })";
    
    bool isMissingFields = AnimationSerialization::ValidateAnimationData(missingFieldsData, "skeletal_animation");
    EXPECT_FALSE(isMissingFields);

    // Test version compatibility
    EXPECT_TRUE(AnimationSerialization::IsVersionCompatible("1.0.0"));
    EXPECT_FALSE(AnimationSerialization::IsVersionCompatible("2.0.0"));

    TestOutput::PrintTestPass("data validation");
    return true;
}

int main() {
    TestOutput::PrintHeader("AnimationSerialization");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("AnimationSerialization Tests");

        // Run all tests
        allPassed &= suite.RunTest("Skeletal Animation Serialization", TestSkeletalAnimationSerialization);
        allPassed &= suite.RunTest("Animation Asset Serialization", TestAnimationAssetSerialization);
        allPassed &= suite.RunTest("Animation Collection Serialization", TestAnimationCollectionSerialization);
        allPassed &= suite.RunTest("File I/O Operations", TestFileIOOperations);
        allPassed &= suite.RunTest("Data Validation", TestDataValidation);

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