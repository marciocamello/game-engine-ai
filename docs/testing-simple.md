# Game Engine Kiro - Simple Testing Guide

## Overview

We use a simple, lightweight testing approach focused on practical validation without external frameworks.

## Testing Philosophy

- **Simple**: No complex frameworks, just C++ and assertions
- **Focused**: Test what matters for game engine functionality
- **Visual**: Use TestRunner for visual debugging when needed
- **Practical**: Integration tests over isolated unit tests

## Test Structure

```
tests/
├── unit/               # Simple unit tests (no framework)
│   └── test_math.cpp   # Math operations testing
└── integration/        # System interaction tests
    └── (existing tests)
```

## Writing Tests

### Simple Test Format

```cpp
#include <iostream>
#include <cassert>
#include "Core/Math.h"

// Simple test function
bool TestVectorOperations() {
    // Test vector addition
    Math::Vec3 a(1.0f, 2.0f, 3.0f);
    Math::Vec3 b(4.0f, 5.0f, 6.0f);
    Math::Vec3 result = a + b;

    // Simple assertions
    assert(result.x == 5.0f);
    assert(result.y == 7.0f);
    assert(result.z == 9.0f);

    std::cout << "✅ Vector operations test passed" << std::endl;
    return true;
}

int main() {
    std::cout << "Running Math Tests..." << std::endl;

    if (TestVectorOperations()) {
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "Tests failed!" << std::endl;
        return 1;
    }
}
```

## Running Tests

```bash
# Build and run
./build.bat

# Run specific test
./build/Release/MathTest.exe
```

## Benefits

- **No Dependencies**: No external testing frameworks needed
- **Fast Build**: Compiles quickly with main project
- **Easy Debug**: Simple to debug with Visual Studio
- **Clear Output**: Easy to understand test results
