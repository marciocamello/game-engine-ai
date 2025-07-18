# Game Engine Kiro - Testing Guide

This guide covers the testing framework and best practices for Game Engine Kiro, including unit tests, integration tests, and physics debugging.

## Testing Framework

Game Engine Kiro uses **GoogleTest** and **GoogleMock** as the primary testing framework, providing:

- **Unit Testing**: Isolated component testing with GoogleTest
- **Mocking**: Interface mocking with GoogleMock for dependency isolation
- **Integration Testing**: System interaction testing
- **Performance Testing**: Benchmarking and performance validation
- **Memory Testing**: Memory leak detection and usage validation

## Test Structure

```
tests/
├── unit/                    # Unit tests (isolated component testing)
│   ├── test_bullet_utils.cpp
│   ├── test_collision_shape_factory.cpp
│   └── test_physics_debug_drawer.cpp
└── integration/             # Integration tests (system interaction)
    ├── test_bullet_integration.cpp
    ├── test_physics_queries.cpp
    ├── test_character_behavior.cpp
    └── test_physics_performance.cpp
```

## Setting Up Tests

### Prerequisites

Ensure GoogleTest and GoogleMock are available via vcpkg:

```json
// vcpkg.json
{
  "dependencies": ["gtest", "gmock"]
}
```

### CMake Configuration

Tests are automatically configured when GoogleTest is found:

```cmake
# CMakeLists.txt
find_package(GTest QUIET)

if(GTest_FOUND)
    enable_testing()

    add_executable(MyTest tests/unit/test_my_component.cpp)
    target_link_libraries(MyTest PRIVATE
        GameEngineKiro
        GTest::gtest_main
        GTest::gmock_main
    )

    include(GoogleTest)
    gtest_discover_tests(MyTest)
endif()
```

## Writing Unit Tests

### Basic Test Structure

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "MyComponent.h"

using namespace GameEngine;
using ::testing::_;
using ::testing::Return;
using ::testing::InSequence;

class MyComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test fixtures
        component = std::make_unique<MyComponent>();
    }

    void TearDown() override {
        // Clean up test fixtures
        component.reset();
    }

    std::unique_ptr<MyComponent> component;
};

TEST_F(MyComponentTest, BasicFunctionality) {
    // Arrange
    int expectedValue = 42;

    // Act
    int result = component->DoSomething(expectedValue);

    // Assert
    EXPECT_EQ(expectedValue, result);
}
```

### Mock Objects

```cpp
class MockPhysicsEngine : public IPhysicsEngine {
public:
    MOCK_METHOD(bool, Initialize, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(uint32_t, CreateRigidBody, (const RigidBody&, const CollisionShape&), (override));
    MOCK_METHOD(void, DestroyRigidBody, (uint32_t), (override));
};

TEST_F(MyTest, UsesPhysicsEngine) {
    // Arrange
    auto mockEngine = std::make_shared<MockPhysicsEngine>();
    EXPECT_CALL(*mockEngine, Initialize())
        .WillOnce(Return(true));

    // Act & Assert
    MyComponent component(mockEngine);
    EXPECT_TRUE(component.Initialize());
}
```

## Physics Testing Examples

### Testing Bullet Physics Integration

```cpp
#include <gtest/gtest.h>
#include "Physics/PhysicsEngine.h"
#include "Physics/BulletUtils.h"

class BulletPhysicsTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::Initialize(LogLevel::DEBUG);
        engine = std::make_shared<PhysicsEngine>();
        ASSERT_TRUE(engine->Initialize());
    }

    void TearDown() override {
        engine->Shutdown();
        Logger::Shutdown();
    }

    std::shared_ptr<PhysicsEngine> engine;
};

TEST_F(BulletPhysicsTest, CreateRigidBodySucceeds) {
    // Arrange
    RigidBody bodyDesc;
    bodyDesc.mass = 1.0f;
    bodyDesc.position = Math::Vec3(0, 5, 0);

    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1, 1, 1);

    // Act
    uint32_t bodyId = engine->CreateRigidBody(bodyDesc, shape);

    // Assert
    EXPECT_GT(bodyId, 0);

    // Verify transform
    Math::Vec3 position;
    Math::Quat rotation;
    EXPECT_TRUE(engine->GetRigidBodyTransform(bodyId, position, rotation));
    EXPECT_NEAR(position.y, 5.0f, 0.001f);
}
```

### Testing Physics Debug Drawer

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Physics/PhysicsDebugDrawer.h"

class MockPhysicsDebugDrawer : public IPhysicsDebugDrawer {
public:
    MOCK_METHOD(void, DrawLine, (const Math::Vec3&, const Math::Vec3&, const Math::Vec3&), (override));
    MOCK_METHOD(void, DrawSphere, (const Math::Vec3&, float, const Math::Vec3&), (override));
    MOCK_METHOD(void, Clear, (), (override));
};

TEST(PhysicsDebugDrawerTest, MockDrawerReceivesCalls) {
    // Arrange
    auto mockDrawer = std::make_shared<MockPhysicsDebugDrawer>();
    auto engine = std::make_shared<PhysicsEngine>();
    engine->Initialize();
    engine->SetDebugDrawer(mockDrawer);
    engine->EnableDebugDrawing(true);

    // Expect Clear to be called
    EXPECT_CALL(*mockDrawer, Clear()).Times(1);

    // Act
    engine->DrawDebugWorld();
}
```

## Performance Testing

### Benchmarking Physics Operations

```cpp
#include <gtest/gtest.h>
#include <chrono>
#include "Physics/PhysicsEngine.h"

class PhysicsPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_shared<PhysicsEngine>();
        engine->Initialize();
    }

    std::shared_ptr<PhysicsEngine> engine;
};

TEST_F(PhysicsPerformanceTest, RigidBodyCreationPerformance) {
    const int numBodies = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    // Create many rigid bodies
    for (int i = 0; i < numBodies; ++i) {
        RigidBody bodyDesc;
        bodyDesc.position = Math::Vec3(i * 2.0f, 0, 0);

        CollisionShape shape;
        shape.type = CollisionShape::Box;

        uint32_t bodyId = engine->CreateRigidBody(bodyDesc, shape);
        EXPECT_GT(bodyId, 0);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    LOG_INFO("Created {} rigid bodies in {} ms", numBodies, duration.count());

    // Performance assertion (adjust based on target performance)
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
}
```

## Memory Testing

### Memory Leak Detection

```cpp
#include <gtest/gtest.h>
#include "Physics/PhysicsEngine.h"

class MemoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Record initial memory state
        initialMemory = GetCurrentMemoryUsage();
    }

    void TearDown() override {
        // Check for memory leaks
        size_t finalMemory = GetCurrentMemoryUsage();
        size_t memoryDiff = finalMemory - initialMemory;

        // Allow some tolerance for test framework overhead
        EXPECT_LT(memoryDiff, 1024 * 1024); // Less than 1MB increase
    }

private:
    size_t GetCurrentMemoryUsage() {
        // Platform-specific memory usage detection
        // This is a simplified example
        return 0; // Implement actual memory detection
    }

    size_t initialMemory = 0;
};

TEST_F(MemoryTest, PhysicsEngineNoMemoryLeaks) {
    // Create and destroy physics engine multiple times
    for (int i = 0; i < 10; ++i) {
        auto engine = std::make_shared<PhysicsEngine>();
        ASSERT_TRUE(engine->Initialize());

        // Create some objects
        RigidBody bodyDesc;
        CollisionShape shape;
        uint32_t bodyId = engine->CreateRigidBody(bodyDesc, shape);

        // Clean up
        engine->DestroyRigidBody(bodyId);
        engine->Shutdown();
    }

    // Memory leak check happens in TearDown()
}
```

## Integration Testing

### Character Physics Integration

```cpp
#include <gtest/gtest.h>
#include "Game/Character.h"
#include "Physics/PhysicsEngine.h"

class CharacterPhysicsTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::Initialize(LogLevel::DEBUG);
        engine = std::make_shared<PhysicsEngine>();
        ASSERT_TRUE(engine->Initialize());

        character = std::make_unique<Character>();
        character->Initialize(engine);
    }

    void TearDown() override {
        character.reset();
        engine->Shutdown();
        Logger::Shutdown();
    }

    std::shared_ptr<PhysicsEngine> engine;
    std::unique_ptr<Character> character;
};

TEST_F(CharacterPhysicsTest, CharacterFallsWithGravity) {
    // Arrange
    Math::Vec3 initialPosition = character->GetPosition();

    // Act - simulate for 1 second
    float deltaTime = 1.0f / 60.0f;
    for (int i = 0; i < 60; ++i) {
        character->Update(deltaTime);
        engine->Update(deltaTime);
    }

    // Assert - character should have fallen
    Math::Vec3 finalPosition = character->GetPosition();
    EXPECT_LT(finalPosition.y, initialPosition.y);
}
```

## Running Tests

### Command Line

```cmd
# Build and run all tests
.\build.bat

# Run specific test executable
build\Release\BulletUtilsTest.exe
build\Release\PhysicsDebugDrawerTest.exe

# Run with verbose output
build\Release\BulletUtilsTest.exe --gtest_verbose

# Run specific test case
build\Release\BulletUtilsTest.exe --gtest_filter="BulletUtilsTest.ConversionAccuracy"
```

### PowerShell Test Script

```powershell
# run_physics_tests.ps1
$testExecutables = @(
    "BulletUtilsTest.exe",
    "CollisionShapeFactoryTest.exe",
    "PhysicsDebugDrawerTest.exe"
)

foreach ($test in $testExecutables) {
    $testPath = "build\Release\$test"
    if (Test-Path $testPath) {
        Write-Host "Running $test..." -ForegroundColor Green
        & $testPath
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Test $test failed!" -ForegroundColor Red
            exit 1
        }
    } else {
        Write-Host "Test $test not found!" -ForegroundColor Yellow
    }
}

Write-Host "All tests completed successfully!" -ForegroundColor Green
```

## Best Practices

### Test Organization

1. **One Test Class Per Component**: Each component should have its own test class
2. **Descriptive Test Names**: Use clear, descriptive names that explain what is being tested
3. **Arrange-Act-Assert**: Structure tests with clear setup, execution, and verification phases
4. **Test Isolation**: Each test should be independent and not rely on other tests

### Mocking Guidelines

1. **Mock External Dependencies**: Use mocks for external systems (file I/O, network, etc.)
2. **Don't Mock Value Objects**: Avoid mocking simple data structures
3. **Verify Interactions**: Use mocks to verify that components interact correctly
4. **Keep Mocks Simple**: Don't over-complicate mock setups

### Performance Testing

1. **Baseline Measurements**: Establish performance baselines for critical operations
2. **Realistic Data**: Use realistic data sizes and scenarios
3. **Multiple Runs**: Average results over multiple runs for consistency
4. **Environment Consistency**: Run performance tests in consistent environments

### Memory Testing

1. **RAII Patterns**: Ensure proper resource cleanup with RAII
2. **Smart Pointers**: Use smart pointers to prevent memory leaks
3. **Leak Detection**: Use tools like Visual Studio diagnostics for leak detection
4. **Stress Testing**: Test with large numbers of objects to find memory issues

## Debugging Tests

### Visual Studio Integration

1. **Set Breakpoints**: Use breakpoints in test code for debugging
2. **Test Explorer**: Use Visual Studio Test Explorer for test management
3. **Debug Output**: Use LOG_DEBUG for detailed test output
4. **Memory Diagnostics**: Enable memory leak detection in debug builds

### Common Issues

1. **Test Isolation**: Ensure tests don't interfere with each other
2. **Resource Cleanup**: Always clean up resources in TearDown()
3. **Timing Issues**: Be careful with timing-dependent tests
4. **Platform Differences**: Consider platform-specific behavior

This testing guide provides a comprehensive foundation for testing Game Engine Kiro components with industry-standard practices using GoogleTest and GoogleMock.
