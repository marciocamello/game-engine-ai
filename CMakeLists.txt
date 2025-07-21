cmake_minimum_required(VERSION 3.16)
project(GameEngineKiro VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force compile commands generation for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(MSVC)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
endif()

# Engine configuration
option(ENABLE_VULKAN "Enable Vulkan renderer" OFF)
option(ENABLE_OPENGL "Enable OpenGL renderer" ON)
option(ENABLE_DLSS "Enable NVIDIA DLSS support" OFF)
option(ENABLE_FSR "Enable AMD FSR support" OFF)
option(USE_VCPKG "Use vcpkg for dependencies" ON)
option(ENABLE_COVERAGE "Enable test coverage analysis" OFF)

# Setup vcpkg toolchain if available
if(USE_VCPKG AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg toolchain")
endif()

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(glad REQUIRED)

# Optional packages
find_package(assimp QUIET)
find_package(OpenAL QUIET)
find_package(Bullet CONFIG REQUIRED)
find_package(Lua QUIET)
find_package(nlohmann_json QUIET)
find_package(fmt QUIET)

# Engine source files
file(GLOB_RECURSE ENGINE_SOURCES
    "src/Core/*.cpp"
    "src/Graphics/*.cpp"
    "src/Resource/*.cpp"
    "src/Physics/*.cpp"
    "src/Audio/*.cpp"
    "src/Input/*.cpp"
    "src/Game/*.cpp"
    "src/UI/*.cpp"
    "src/Animation/*.cpp"
    "src/AI/*.cpp"
    "src/Scripting/*.cpp"
    "src/Network/*.cpp"
)

file(GLOB_RECURSE ENGINE_HEADERS
    "include/Core/*.h"
    "include/Graphics/*.h"
    "include/Resource/*.h"
    "include/Physics/*.h"
    "include/Audio/*.h"
    "include/Input/*.h"
    "include/Game/*.h"
    "include/UI/*.h"
    "include/Animation/*.h"
    "include/AI/*.h"
    "include/Scripting/*.h"
    "include/Network/*.h"
)

# Create engine library
add_library(GameEngineKiro STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

target_include_directories(GameEngineKiro PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Core dependencies
target_link_libraries(GameEngineKiro PUBLIC
    OpenGL::GL
    glfw
    glm::glm
    glad::glad
)

# Optional dependencies
if(assimp_FOUND)
    target_link_libraries(GameEngineKiro PUBLIC assimp::assimp)
    target_compile_definitions(GameEngineKiro PUBLIC GAMEENGINE_HAS_ASSIMP)
endif()

if(OpenAL_FOUND)
    target_link_libraries(GameEngineKiro PUBLIC OpenAL::OpenAL)
    target_compile_definitions(GameEngineKiro PUBLIC GAMEENGINE_HAS_OPENAL)
endif()

if(Bullet_FOUND)
    target_link_libraries(GameEngineKiro PUBLIC 
        BulletDynamics
        BulletCollision
        LinearMath
    )
    target_compile_definitions(GameEngineKiro PUBLIC GAMEENGINE_HAS_BULLET)
endif()

if(Lua_FOUND)
    target_link_libraries(GameEngineKiro PUBLIC ${LUA_LIBRARIES})
    target_include_directories(GameEngineKiro PUBLIC ${LUA_INCLUDE_DIR})
    target_compile_definitions(GameEngineKiro PUBLIC GAMEENGINE_HAS_LUA)
endif()

if(nlohmann_json_FOUND)
    target_link_libraries(GameEngineKiro PUBLIC nlohmann_json::nlohmann_json)
    target_compile_definitions(GameEngineKiro PUBLIC GAMEENGINE_HAS_JSON)
endif()

if(fmt_FOUND)
    target_link_libraries(GameEngineKiro PUBLIC fmt::fmt)
    target_compile_definitions(GameEngineKiro PUBLIC GAMEENGINE_HAS_FMT)
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(GameEngineKiro PUBLIC winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(GameEngineKiro PUBLIC pthread dl)
elseif(APPLE)
    target_link_libraries(GameEngineKiro PUBLIC "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(GameEngineKiro PRIVATE /W4)
    target_compile_definitions(GameEngineKiro PRIVATE _CRT_SECURE_NO_WARNINGS)
    
    # Coverage support for MSVC (debug symbols and optimization control)
    if(ENABLE_COVERAGE OR CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(GameEngineKiro PRIVATE /Zi /Od)
        target_link_options(GameEngineKiro PRIVATE /DEBUG:FULL)
    endif()
else()
    target_compile_options(GameEngineKiro PRIVATE -Wall -Wextra -Wpedantic)
    
    # Coverage support for GCC/Clang
    if(ENABLE_COVERAGE)
        target_compile_options(GameEngineKiro PRIVATE --coverage -g -O0)
        target_link_options(GameEngineKiro PRIVATE --coverage)
    endif()
endif()

# GLM experimental features
target_compile_definitions(GameEngineKiro PUBLIC GLM_ENABLE_EXPERIMENTAL)

# Function to apply coverage settings to test executables
function(apply_coverage_settings target_name)
    if(ENABLE_COVERAGE)
        if(MSVC)
            target_compile_options(${target_name} PRIVATE /Zi /Od)
            target_link_options(${target_name} PRIVATE /DEBUG:FULL)
        else()
            target_compile_options(${target_name} PRIVATE --coverage -g -O0)
            target_link_options(${target_name} PRIVATE --coverage)
        endif()
    endif()
endfunction()

# Example game executable
add_executable(GameExample
    examples/main.cpp
)

target_link_libraries(GameExample PRIVATE GameEngineKiro)

# Character comparison example (temporarily disabled due to interface issues)
# add_executable(CharacterComparison
#     examples/character_comparison.cpp
# )
# 
# target_link_libraries(CharacterComparison PRIVATE GameEngineKiro)

# Character controller test
add_executable(CharacterControllerTest
    examples/character_controller_test.cpp
)

target_link_libraries(CharacterControllerTest PRIVATE GameEngineKiro)

# Physics debug renderer example
add_executable(PhysicsDebugExample
    examples/physics_debug_example.cpp
)

target_link_libraries(PhysicsDebugExample PRIVATE GameEngineKiro)

# Integration test for Bullet Physics
add_executable(BulletIntegrationTest
    tests/integration/test_bullet_integration.cpp
)
apply_coverage_settings(BulletIntegrationTest)

# Integration test for Bullet conversion utilities
add_executable(BulletConversionTest
    tests/integration/test_bullet_conversion.cpp
)
apply_coverage_settings(BulletConversionTest)

# Simple integration test for Bullet conversion utilities
add_executable(BulletUtilsSimpleTest
    tests/integration/test_bullet_utils_simple.cpp
)
apply_coverage_settings(BulletUtilsSimpleTest)

# Simple integration test for CollisionShapeFactory
add_executable(CollisionShapeFactorySimpleTest
    tests/integration/test_collision_shape_factory_simple.cpp
)
apply_coverage_settings(CollisionShapeFactorySimpleTest)

# Physics queries integration test
add_executable(PhysicsQueriesTest
    tests/integration/test_physics_queries.cpp
)
apply_coverage_settings(PhysicsQueriesTest)

# Physics configuration integration test
add_executable(PhysicsConfigurationTest
    tests/integration/test_physics_configuration.cpp
)
apply_coverage_settings(PhysicsConfigurationTest)

# Movement component comparison test
add_executable(MovementComponentComparisonTest
    tests/integration/test_movement_component_comparison.cpp
)
apply_coverage_settings(MovementComponentComparisonTest)

# Simple physics performance test
add_executable(PhysicsPerformanceSimpleTest
    tests/integration/test_physics_performance_simple.cpp
)
apply_coverage_settings(PhysicsPerformanceSimpleTest)

# Simple memory usage test
add_executable(MemoryUsageSimpleTest
    tests/integration/test_memory_usage_simple.cpp
)
apply_coverage_settings(MemoryUsageSimpleTest)

# Simple character behavior test
add_executable(CharacterBehaviorSimpleTest
    tests/integration/test_character_behavior_simple.cpp
)
apply_coverage_settings(CharacterBehaviorSimpleTest)

# Unit test for Math utilities
add_executable(MathUnitTest
    tests/unit/test_math.cpp
)
apply_coverage_settings(MathUnitTest)



target_link_libraries(BulletIntegrationTest PRIVATE GameEngineKiro)
target_link_libraries(BulletConversionTest PRIVATE GameEngineKiro)
target_link_libraries(BulletUtilsSimpleTest PRIVATE GameEngineKiro)
target_link_libraries(CollisionShapeFactorySimpleTest PRIVATE GameEngineKiro)
target_link_libraries(PhysicsQueriesTest PRIVATE GameEngineKiro)
target_link_libraries(PhysicsConfigurationTest PRIVATE GameEngineKiro)
target_link_libraries(MovementComponentComparisonTest PRIVATE GameEngineKiro)
target_link_libraries(PhysicsPerformanceSimpleTest PRIVATE GameEngineKiro)
target_link_libraries(MemoryUsageSimpleTest PRIVATE GameEngineKiro)
target_link_libraries(CharacterBehaviorSimpleTest PRIVATE GameEngineKiro)
target_link_libraries(MathUnitTest PRIVATE GameEngineKiro)



if(Bullet_FOUND)
    target_link_libraries(BulletIntegrationTest PRIVATE 
        BulletDynamics
        BulletCollision
        LinearMath
    )
    
    target_link_libraries(BulletConversionTest PRIVATE 
        BulletDynamics
        BulletCollision
        LinearMath
    )
    
    target_link_libraries(BulletUtilsSimpleTest PRIVATE 
        BulletDynamics
        BulletCollision
        LinearMath
    )
    
    target_link_libraries(CollisionShapeFactorySimpleTest PRIVATE 
        BulletDynamics
        BulletCollision
        LinearMath
    )
    
    target_link_libraries(PhysicsQueriesTest PRIVATE 
        BulletDynamics
        BulletCollision
        LinearMath
    )
    
    target_link_libraries(PhysicsConfigurationTest PRIVATE 
        BulletDynamics
        BulletCollision
        LinearMath
    )
    
    target_link_libraries(MovementComponentComparisonTest PRIVATE 
        BulletDynamics
        BulletCollision
        LinearMath
    )
    
    target_link_libraries(PhysicsPerformanceSimpleTest PRIVATE 
        BulletDynamics
        BulletCollision
        LinearMath
    )
    
    target_link_libraries(MemoryUsageSimpleTest PRIVATE 
        BulletDynamics
        BulletCollision
        LinearMath
    )
    
    target_link_libraries(CharacterBehaviorSimpleTest PRIVATE 
        BulletDynamics
        BulletCollision
        LinearMath
    )
endif()

# Copy assets to build directory
add_custom_command(TARGET GameExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:GameExample>/assets
    COMMENT "Copying assets to build directory"
)

