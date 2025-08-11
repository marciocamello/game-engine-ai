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
find_package(Stb REQUIRED)

# Engine source files
file(GLOB_RECURSE ENGINE_SOURCES
    "engine/core/*.cpp"
    "engine/modules/*.cpp"
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
    "engine/core/*.h"
    "engine/modules/*.h"
    "engine/interfaces/*.h"
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
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
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

if(Stb_FOUND)
    target_include_directories(GameEngineKiro PUBLIC ${Stb_INCLUDE_DIR})
    target_compile_definitions(GameEngineKiro PUBLIC GAMEENGINE_HAS_STB)
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(GameEngineKiro PUBLIC winmm)
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

# Function to create a unit test executable with standard configuration
function(add_unit_test test_name source_file)
    # Create the executable
    add_executable(${test_name} ${source_file})
    
    # Apply coverage settings
    apply_coverage_settings(${test_name})
    
    # Link with engine library
    target_link_libraries(${test_name} PRIVATE GameEngineKiro)
    
    # Add test utilities include
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    
    # Add to test list for potential automation
    set_property(GLOBAL APPEND PROPERTY UNIT_TEST_TARGETS ${test_name})
endfunction()

# Function to create an integration test executable with standard configuration
function(add_integration_test test_name source_file)
    # Create the executable
    add_executable(${test_name} ${source_file})
    
    # Apply coverage settings
    apply_coverage_settings(${test_name})
    
    # Link with engine library
    target_link_libraries(${test_name} PRIVATE GameEngineKiro)
    
    # Add test utilities include
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    
    # Link with Bullet Physics if available (common for integration tests)
    if(Bullet_FOUND)
        target_link_libraries(${test_name} PRIVATE 
            BulletDynamics
            BulletCollision
            LinearMath
        )
    endif()
    
    # Add to test list for potential automation
    set_property(GLOBAL APPEND PROPERTY INTEGRATION_TEST_TARGETS ${test_name})
endfunction()

# Function to create a performance test executable with standard configuration
function(add_performance_test test_name source_file)
    # Create the executable
    add_executable(${test_name} ${source_file})
    
    # Apply coverage settings (usually disabled for performance tests)
    # apply_coverage_settings(${test_name})
    
    # Link with engine library
    target_link_libraries(${test_name} PRIVATE GameEngineKiro)
    
    # Add test utilities include
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    
    # Optimization flags for performance tests
    if(MSVC)
        target_compile_options(${test_name} PRIVATE /O2)
    else()
        target_compile_options(${test_name} PRIVATE -O2)
    endif()
    
    # Add to test list for potential automation
    set_property(GLOBAL APPEND PROPERTY PERFORMANCE_TEST_TARGETS ${test_name})
endfunction()

# Function to automatically discover and add test files
function(discover_and_add_tests test_directory test_type)
    file(GLOB test_files "${test_directory}/test_*.cpp")
    
    foreach(test_file ${test_files})
        # Extract test name from filename
        get_filename_component(test_name_with_ext ${test_file} NAME)
        string(REPLACE ".cpp" "" test_name_base ${test_name_with_ext})
        
        # Convert to PascalCase for executable name
        string(REGEX REPLACE "test_([a-z])([a-z_]*)" "\\1\\2" test_name_clean ${test_name_base})
        string(SUBSTRING ${test_name_clean} 0 1 first_char)
        string(TOUPPER ${first_char} first_char_upper)
        string(SUBSTRING ${test_name_clean} 1 -1 rest_chars)
        string(REPLACE "_" "" rest_chars_clean ${rest_chars})
        set(test_executable_name "${first_char_upper}${rest_chars_clean}Test")
        
        # Add the appropriate test type
        if(test_type STREQUAL "unit")
            add_unit_test(${test_executable_name} ${test_file})
        elseif(test_type STREQUAL "integration")
            add_integration_test(${test_executable_name} ${test_file})
        elseif(test_type STREQUAL "performance")
            add_performance_test(${test_executable_name} ${test_file})
        endif()
        
        message(STATUS "Added ${test_type} test: ${test_executable_name} from ${test_file}")
    endforeach()
endfunction()

# Function to print all discovered tests
function(print_test_summary)
    get_property(unit_tests GLOBAL PROPERTY UNIT_TEST_TARGETS)
    get_property(integration_tests GLOBAL PROPERTY INTEGRATION_TEST_TARGETS)
    get_property(performance_tests GLOBAL PROPERTY PERFORMANCE_TEST_TARGETS)
    
    if(unit_tests)
        message(STATUS "Unit Tests: ${unit_tests}")
    endif()
    
    if(integration_tests)
        message(STATUS "Integration Tests: ${integration_tests}")
    endif()
    
    if(performance_tests)
        message(STATUS "Performance Tests: ${performance_tests}")
    endif()
endfunction()

# Include example projects as subdirectories
add_subdirectory(projects/GameExample)
add_subdirectory(projects/BasicExample)

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

# FBX test example
add_executable(FBXTestSimple
    examples/test_fbx_simple.cpp
)

target_link_libraries(FBXTestSimple PRIVATE GameEngineKiro)

# FBX minimal test
add_executable(FBXTestMinimal
    examples/test_fbx_minimal.cpp
)

target_link_libraries(FBXTestMinimal PRIVATE GameEngineKiro)

# FBX via ModelLoader test
add_executable(FBXViaModelLoader
    examples/test_fbx_via_modelloader.cpp
)

target_link_libraries(FBXViaModelLoader PRIVATE GameEngineKiro)

# FBX Animation test
add_executable(FBXAnimationTest
    examples/test_fbx_animation.cpp
)

target_link_libraries(FBXAnimationTest PRIVATE GameEngineKiro)

# FBX Debug test
add_executable(DebugFBXLoading
    examples/debug_fbx_loading.cpp
)

target_link_libraries(DebugFBXLoading PRIVATE GameEngineKiro)

# FBX Isolated Debug test
add_executable(DebugFBXIsolated
    examples/debug_fbx_isolated.cpp
)

target_link_libraries(DebugFBXIsolated PRIVATE GameEngineKiro)

# Mesh Creation Debug test
add_executable(DebugMeshCreation
    examples/debug_mesh_creation.cpp
)

target_link_libraries(DebugMeshCreation PRIVATE GameEngineKiro)

# VertexLayout Debug test
add_executable(DebugVertexLayout
    examples/debug_vertex_layout.cpp
)

target_link_libraries(DebugVertexLayout PRIVATE GameEngineKiro)

# Direct Mesh Debug test
add_executable(DebugMeshDirect
    examples/debug_mesh_direct.cpp
)

target_link_libraries(DebugMeshDirect PRIVATE GameEngineKiro)

# Model Hot Reload example
add_executable(ModelHotReloadExample
    examples/model_hot_reload_example.cpp
)
target_link_libraries(ModelHotReloadExample PRIVATE GameEngineKiro)

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
add_integration_test(BulletUtilsSimpleTest tests/integration/test_bullet_utils_simple.cpp)

# Simple integration test for CollisionShapeFactory
add_integration_test(CollisionShapeFactorySimpleTest tests/integration/test_collision_shape_factory_simple.cpp)

# Physics queries integration test
add_integration_test(PhysicsQueriesTest tests/integration/test_physics_queries.cpp)

# Physics configuration integration test
add_integration_test(PhysicsConfigurationTest tests/integration/test_physics_configuration.cpp)

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

# OpenAL integration test
add_integration_test(OpenALIntegrationTest tests/integration/test_openal_integration.cpp)

# AudioLoader integration test
add_integration_test(AudioLoaderTest tests/integration/test_audio_loader.cpp)

# Audio Camera integration test
add_integration_test(AudioCameraIntegrationTest tests/integration/test_audio_camera_integration.cpp)

# Resource Statistics integration test
add_integration_test(ResourceStatisticsTest tests/integration/test_resource_statistics.cpp)

# Error Handling integration test
add_integration_test(ErrorHandlingTest tests/integration/test_error_handling.cpp)

# Final v1.0 Validation integration test
add_integration_test(FinalV1ValidationTest tests/integration/test_final_v1_validation.cpp)

# Final 3D Model Integration and Validation test
add_integration_test(Final3dModelIntegrationValidationTest tests/integration/test_final_3d_model_integration_validation.cpp)

# ModelLoader Assimp integration test
add_integration_test(ModelLoaderAssimpTest tests/integration/test_model_loader_assimp.cpp)

# MaterialImporter integration test
add_integration_test(MaterialImporterTest tests/integration/test_material_importer.cpp)

# GLTF Loader integration test
add_integration_test(GLTFLoaderTest tests/integration/test_gltf_loader.cpp)

# FBX Loader integration test
add_integration_test(FBXLoaderTest tests/integration/test_fbx_loader.cpp)

# Model Resource Integration test
add_integration_test(ModelResourceIntegrationTest tests/integration/test_model_resource_integration.cpp)

# Model Hot Reload integration test
add_integration_test(ModelHotReloadTest tests/integration/test_model_hot_reload.cpp)

# Resource Usage Tracking test
add_integration_test(ResourceUsageTrackingTest tests/integration/test_resource_usage_tracking.cpp)

# Model Debugger integration test
add_integration_test(ModelDebuggerTest tests/integration/test_model_debugger.cpp)

# Performance and Stability integration test
add_integration_test(PerformanceStabilityTest tests/integration/test_performance_stability.cpp)

# ShaderManager Integration test
add_integration_test(ShadermanagerintegrationTest tests/integration/test_shadermanager_integration.cpp)

# Advanced Shader System Integration test
add_integration_test(AdvancedshadersystemintegrationTest tests/integration/test_advanced_shader_system_integration.cpp)

# Final Shader System Validation test
add_integration_test(FinalShaderSystemValidationTest tests/integration/test_final_shader_system_validation.cpp)

# Bullet Physics Module integration test
add_integration_test(BulletPhysicsModuleTest tests/integration/test_bullet_physics_module.cpp)

# Engine Module Initialization integration test
add_integration_test(EngineModuleInitializationTest tests/integration/test_engine_module_initialization.cpp)

# Discover and add unit tests automatically
discover_and_add_tests("tests/unit" "unit")



target_link_libraries(BulletIntegrationTest PRIVATE GameEngineKiro)
target_link_libraries(BulletConversionTest PRIVATE GameEngineKiro)

target_link_libraries(CollisionShapeFactorySimpleTest PRIVATE GameEngineKiro)
target_link_libraries(PhysicsQueriesTest PRIVATE GameEngineKiro)
target_link_libraries(PhysicsConfigurationTest PRIVATE GameEngineKiro)
target_link_libraries(MovementComponentComparisonTest PRIVATE GameEngineKiro)
target_link_libraries(PhysicsPerformanceSimpleTest PRIVATE GameEngineKiro)
target_link_libraries(MemoryUsageSimpleTest PRIVATE GameEngineKiro)
target_link_libraries(CharacterBehaviorSimpleTest PRIVATE GameEngineKiro)
target_link_libraries(OpenALIntegrationTest PRIVATE GameEngineKiro)
# Print test summary
print_test_summary()



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

