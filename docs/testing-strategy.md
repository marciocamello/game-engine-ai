# 🧪 Game Engine Kiro - Testing Strategy

## 📋 Overview

This document outlines our comprehensive testing strategy following industry best practices for game engine development.

## 🏗️ Testing Architecture

### 🔄 Two-Tier Testing System

#### 1. **Integration Tests** (`tests/`)

- **Purpose**: System validation and component interaction testing
- **Location**: Part of main project build
- **Build**: Compiles with main engine
- **CI/CD**: Integrated with main build pipeline
- **Coverage**: Physics integration, system interactions

#### 2. **Internal Tests** (Embedded in engine)

- **Purpose**: Runtime visual validation and debugging
- **Location**: Built into engine via `TestRunner` system
- **Usage**: `--run-tests` flag or debug console
- **Coverage**: Rendering, gameplay, AI, multiplayer, editor tools

## 🎯 What to Test Where

| System                  | Test Type       | Location              | Rationale                       |
| ----------------------- | --------------- | --------------------- | ------------------------------- |
| **Physics Integration** | 🔄 Integration  | `tests/integration/`  | System interaction testing      |
| **Bullet Physics**      | 🔄 Integration  | `tests/integration/`  | Physics engine validation       |
| **Character Movement**  | 🔄 Integration  | `tests/integration/`  | Gameplay system testing         |
| **Rendering**           | 🧰 Visual Debug | Internal `TestRunner` | Visual inspection required      |
| **Gameplay/AI**         | 🧰 Visual Debug | Internal `TestRunner` | Behavior observation needed     |
| **Physics (Visual)**    | 🧰 Visual Debug | Internal `TestRunner` | Non-deterministic visualization |
| **Editor Tools**        | 🔁 Manual Tests | Internal `TestRunner` | UI/UX validation                |
| **Multiplayer**         | 🔁 Interactive  | Internal `TestRunner` | Network behavior                |

## 🚀 Quick Start

### Running Tests

```bash
# Build and run tests
./build.bat

# Run specific integration test
./build/Release/BulletIntegrationTest.exe
```

### Running Internal Tests

```bash
# Run specific visual test
./GameExample --run-test "Physics Debug Visualization"

# Run all internal tests
./GameExample --run-all-tests

# Enable debug overlay
./GameExample --debug-overlay
```

## 📁 Directory Structure

```
GameEngineKiro/
├── tests/                 # 🔄 Testing structure
│   └── integration/       # Integration tests
├── src/Core/TestRunner.*  # 🧰 Internal visual test system
├── docs/                  # 📚 All documentation
└── README.md             # 📖 Main project readme
```

## 🧮 Integration Testing Examples

```cpp
// tests/integration/test_bullet_integration.cpp
int main() {
    // Test basic Bullet Physics initialization
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(/*...*/);

    std::cout << "Bullet Physics integration test successful!" << std::endl;
    return 0;
}
```

## 🎮 Visual Testing Examples

```cpp
// Internal visual test registration
REGISTER_VISUAL_TEST(PhysicsDebugVisualization, "Shows collision shapes and raycasts") {
    .setup = []() {
        // Create test scene with physics objects
    },
    .update = [](float dt) {
        // Update physics simulation
        TestRunner::GetInstance().DrawDebugSphere(position, radius);
    },
    .render = []() {
        // Render debug overlays
    }
}
```

## 🔧 Integration with Build System

### CMake Integration

- External tests integrate with CTest
- Internal tests compile with engine
- Coverage reports include both systems

### CI/CD Pipeline

1. Build engine library
2. Build and run external tests
3. Generate coverage reports
4. Build full engine with internal tests
5. Run smoke tests

## 📊 Coverage Goals

| System              | Target Coverage       | Test Type         |
| ------------------- | --------------------- | ----------------- |
| Physics Integration | 90%+                  | Integration tests |
| System Interactions | 85%+                  | Integration tests |
| Rendering           | Visual validation     | Manual inspection |
| Gameplay            | Behavioral validation | Manual testing    |

## 🎯 Best Practices

### ✅ Do:

- Test math operations extensively
- Use deterministic physics tests
- Create visual debug tools for rendering
- Test ECS component logic
- Validate input parsing

### ❌ Don't:

- Test rendering output automatically
- Automate non-deterministic physics
- Over-test trivial getters/setters
- Ignore visual validation tools

## 🔄 Workflow

1. **Start with fundamentals**: Math, core systems
2. **Add deterministic tests**: Physics, ECS logic
3. **Build visual tools**: Debug overlays, test runner
4. **Manual validation**: Rendering, gameplay, UI
5. **Iterate**: Refine based on development needs

## 📈 Metrics and Monitoring

- **Integration test pass rate**: Should be 100%
- **Coverage percentage**: Track by system
- **Test execution time**: Keep under 30s for integration tests
- **Visual test catalog**: Maintain list of manual tests
- **Bug detection rate**: Measure test effectiveness
