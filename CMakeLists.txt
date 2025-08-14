cmake_minimum_required(VERSION 3.16)
project(GameEngineKiro VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force compile commands generation for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(MSVC)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
endif()

# Ninja generator detection and optimization
if(CMAKE_GENERATOR STREQUAL "Ninja" OR CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
    set(USING_NINJA TRUE)
    message(STATUS "Using Ninja generator - enabling build optimizations")
    
    # Ninja-specific optimizations
    set(CMAKE_JOB_POOLS "compile=8;link=2" CACHE STRING "Ninja job pools for parallel builds")
    set(CMAKE_JOB_POOL_COMPILE "compile" CACHE STRING "Job pool for compilation")
    set(CMAKE_JOB_POOL_LINK "link" CACHE STRING "Job pool for linking")
    
    # Enable build diagnostics for Ninja
    set(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "Verbose output disabled for cleaner Ninja builds")
    
    # Optimize dependency scanning
    set(CMAKE_DEPENDS_USE_COMPILER TRUE CACHE BOOL "Use compiler for dependency scanning")
    
    # Enable colored output for Ninja builds
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # MSVC doesn't support colored output in the same way, but we can enable diagnostics
        add_compile_options(/diagnostics:column)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-fcolor-diagnostics)
    endif()
    
    message(STATUS "Ninja optimizations applied:")
    message(STATUS "  - Parallel compilation jobs: 8")
    message(STATUS "  - Parallel linking jobs: 2")
    message(STATUS "  - Compiler-based dependency scanning: enabled")
    message(STATUS "  - Colored diagnostics: enabled")
else()
    set(USING_NINJA FALSE)
    message(STATUS "Using ${CMAKE_GENERATOR} generator")
endif()

# Build configuration options
option(BUILD_ENGINE_ONLY "Build only the engine library" OFF)
option(BUILD_PROJECTS_ONLY "Build only game projects" OFF)
option(BUILD_TESTS_ONLY "Build only test suites" OFF)
option(BUILD_ENGINE_AND_TESTS_ONLY "Build engine and tests only" OFF)
option(BUILD_SPECIFIC_PROJECT "Build a specific project only" "")
option(BUILD_SPECIFIC_TEST "Build a specific test only" "")

# Engine configuration
option(ENABLE_VULKAN "Enable Vulkan renderer" OFF)
option(ENABLE_OPENGL "Enable OpenGL renderer" ON)
option(ENABLE_DLSS "Enable NVIDIA DLSS support" OFF)
option(ENABLE_FSR "Enable AMD FSR support" OFF)
option(USE_VCPKG "Use vcpkg for dependencies" ON)
option(ENABLE_COVERAGE "Enable test coverage analysis" OFF)

# Module configuration options
option(ENABLE_GRAPHICS_MODULE "Enable graphics module" ON)
option(ENABLE_PHYSICS_MODULE "Enable physics module" ON)
option(ENABLE_AUDIO_MODULE "Enable audio module" ON)
option(ENABLE_SCRIPTING_MODULE "Enable scripting module" OFF)
option(ENABLE_NETWORK_MODULE "Enable network module" OFF)

# Setup vcpkg toolchain if available
if(USE_VCPKG AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg toolchain")
endif()

# Global variables for module and project management
set_property(GLOBAL PROPERTY ENGINE_MODULES "")
set_property(GLOBAL PROPERTY ENGINE_PROJECTS "")
set_property(GLOBAL PROPERTY ENGINE_TESTS "")

# Function to register an engine module
function(register_engine_module module_name module_path)
    get_property(current_modules GLOBAL PROPERTY ENGINE_MODULES)
    list(APPEND current_modules "${module_name}")
    set_property(GLOBAL PROPERTY ENGINE_MODULES "${current_modules}")
    
    # Store path separately
    set_property(GLOBAL PROPERTY "ENGINE_MODULE_${module_name}_PATH" "${module_path}")
    message(STATUS "Registered engine module: ${module_name} at ${module_path}")
endfunction()

# Function to register a game project
function(register_game_project project_name project_path)
    get_property(current_projects GLOBAL PROPERTY ENGINE_PROJECTS)
    list(APPEND current_projects "${project_name}")
    set_property(GLOBAL PROPERTY ENGINE_PROJECTS "${current_projects}")
    
    # Store path separately
    set_property(GLOBAL PROPERTY "ENGINE_PROJECT_${project_name}_PATH" "${project_path}")
    message(STATUS "Registered game project: ${project_name} at ${project_path}")
endfunction()

# Function to discover engine modules
function(discover_engine_modules)
    file(GLOB module_dirs "${CMAKE_CURRENT_SOURCE_DIR}/engine/modules/*")
    foreach(module_dir ${module_dirs})
        if(IS_DIRECTORY ${module_dir})
            get_filename_component(module_name ${module_dir} NAME)
            if(EXISTS "${module_dir}/CMakeLists.txt")
                register_engine_module(${module_name} ${module_dir})
            endif()
        endif()
    endforeach()
endfunction()

# Function to discover game projects
function(discover_game_projects)
    file(GLOB project_dirs "${CMAKE_CURRENT_SOURCE_DIR}/projects/*")
    foreach(project_dir ${project_dirs})
        if(IS_DIRECTORY ${project_dir})
            get_filename_component(project_name ${project_dir} NAME)
            # Skip Tests directory as it's handled separately
            if(NOT project_name STREQUAL "Tests" AND EXISTS "${project_dir}/CMakeLists.txt")
                register_game_project(${project_name} ${project_dir})
            endif()
        endif()
    endforeach()
endfunction()

# Function to validate build configuration and prevent conflicts
function(validate_build_configuration)
    # Check for conflicting old class names in headers
    set(CONFLICTING_HEADERS
        "include/Animation/Animation.h"
        "include/Graphics/Animation.h"
        "include/Animation/Skeleton.h"
        "include/Graphics/Skeleton.h"
    )
    
    foreach(header ${CONFLICTING_HEADERS})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${header}")
            message(FATAL_ERROR 
                "Conflicting header file found: ${header}\n"
                "This indicates incomplete namespace refactoring. Please ensure all conflicting class names have been resolved.\n"
                "Expected new headers:\n"
                "  - include/Animation/SkeletalAnimation.h\n"
                "  - include/Animation/AnimationSkeleton.h\n"
                "  - include/Graphics/GraphicsAnimation.h\n"
                "  - include/Graphics/RenderSkeleton.h"
            )
        endif()
    endforeach()
    
    # Check for conflicting old implementation files
    set(CONFLICTING_SOURCES
        "src/Animation/Animation.cpp"
        "src/Graphics/Animation.cpp"
        "src/Animation/Skeleton.cpp"
        "src/Graphics/Skeleton.cpp"
    )
    
    foreach(source ${CONFLICTING_SOURCES})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
            message(FATAL_ERROR 
                "Conflicting source file found: ${source}\n"
                "This indicates incomplete namespace refactoring. Please ensure all conflicting class names have been resolved.\n"
                "Expected new source files:\n"
                "  - src/Animation/SkeletalAnimation.cpp\n"
                "  - src/Animation/AnimationSkeleton.cpp\n"
                "  - src/Graphics/GraphicsAnimation.cpp\n"
                "  - src/Graphics/RenderSkeleton.cpp"
            )
        endif()
    endforeach()
    
    # Verify that projects directory structure is properly configured
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/projects/GameExample/CMakeLists.txt")
        message(WARNING "GameExample project not found in projects/ directory")
    endif()
    
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/projects/BasicExample/CMakeLists.txt")
        message(WARNING "BasicExample project not found in projects/ directory")
    endif()
    
    # Ensure examples/ directory is not being used for builds
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
        message(STATUS "Legacy examples/ directory detected - this is kept for reference only and will not be built")
        message(STATUS "All example applications are now in the projects/ directory structure")
    endif()
    
    message(STATUS "Build configuration validation completed successfully")
endfunction()

# Function to print build summary
function(print_build_summary)
    message(STATUS "=== Build Configuration Summary ===")
    
    # Build system information
    message(STATUS "Build System:")
    message(STATUS "  - Generator: ${CMAKE_GENERATOR}")
    if(USING_NINJA)
        message(STATUS "  - Ninja Optimizations: Enabled")
        message(STATUS "  - Parallel Compile Jobs: 8")
        message(STATUS "  - Parallel Link Jobs: 2")
        message(STATUS "  - Dependency Scanning: Compiler-based")
    endif()
    
    get_property(modules GLOBAL PROPERTY ENGINE_MODULES)
    if(modules)
        message(STATUS "Engine Modules:")
        foreach(module_name ${modules})
            message(STATUS "  - ${module_name}")
        endforeach()
    endif()
    
    get_property(projects GLOBAL PROPERTY ENGINE_PROJECTS)
    if(projects)
        message(STATUS "Game Projects:")
        foreach(project_name ${projects})
            message(STATUS "  - ${project_name}")
        endforeach()
    endif()
    
    message(STATUS "Build Options:")
    message(STATUS "  - Engine Only: ${BUILD_ENGINE_ONLY}")
    message(STATUS "  - Projects Only: ${BUILD_PROJECTS_ONLY}")
    message(STATUS "  - Tests Only: ${BUILD_TESTS_ONLY}")
    message(STATUS "  - Engine + Tests Only: ${BUILD_ENGINE_AND_TESTS_ONLY}")
    if(BUILD_SPECIFIC_PROJECT)
        message(STATUS "  - Specific Project: ${BUILD_SPECIFIC_PROJECT}")
    endif()
    if(BUILD_SPECIFIC_TEST)
        message(STATUS "  - Specific Test: ${BUILD_SPECIFIC_TEST}")
    endif()
    
    # Ninja-specific build tips
    if(USING_NINJA)
        message(STATUS "")
        message(STATUS "Ninja Build Tips:")
        message(STATUS "  - Use 'ninja -d explain' to see why files are rebuilt")
        message(STATUS "  - Use 'ninja -t targets' to list all available targets")
        message(STATUS "  - Use 'ninja -t graph' to visualize build dependencies")
        message(STATUS "  - Use 'ninja -j N' to override parallel job count")
    endif()
    
    message(STATUS "===================================")
endfunction()

# Build engine core and modules (always needed)
if(TRUE)
    # Find required packages for engine
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    find_package(glm REQUIRED)
    find_package(glad REQUIRED)

    # Optional packages for engine
    find_package(assimp QUIET)
    find_package(OpenAL QUIET)
    find_package(Bullet CONFIG REQUIRED)
    find_package(Lua QUIET)
    find_package(nlohmann_json QUIET)
    find_package(fmt QUIET)
    find_package(Stb QUIET)
endif()

# Build engine core and modules (always needed)
if(TRUE)
    # Engine core source files (always included)
    file(GLOB_RECURSE ENGINE_CORE_SOURCES
        "engine/core/*.cpp"
        "src/Core/*.cpp"
    )

    file(GLOB_RECURSE ENGINE_CORE_HEADERS
        "engine/core/*.h"
        "engine/interfaces/*.h"
        "include/Core/*.h"
    )

    # Legacy source files (to be gradually moved to modules)
    file(GLOB_RECURSE ENGINE_LEGACY_SOURCES
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

    file(GLOB_RECURSE ENGINE_LEGACY_HEADERS
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

    # Module source files (until fully modularized)
    file(GLOB_RECURSE ENGINE_MODULE_SOURCES
        "engine/modules/*.cpp"
    )

    file(GLOB_RECURSE ENGINE_MODULE_HEADERS
        "engine/modules/*.h"
    )

    # Create engine core library
    add_library(GameEngineKiro STATIC 
        ${ENGINE_CORE_SOURCES} 
        ${ENGINE_CORE_HEADERS}
        ${ENGINE_LEGACY_SOURCES}
        ${ENGINE_LEGACY_HEADERS}
        ${ENGINE_MODULE_SOURCES}
        ${ENGINE_MODULE_HEADERS}
    )

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
        
        # Ninja-specific MSVC optimizations
        if(USING_NINJA)
            # Enable faster PDB generation for Ninja builds
            target_compile_options(GameEngineKiro PRIVATE /FS)
            # Enable parallel compilation within each translation unit
            target_compile_options(GameEngineKiro PRIVATE /MP)
            # Optimize for build speed in debug builds
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                target_compile_options(GameEngineKiro PRIVATE /JMC)
            endif()
        endif()
        
        # Coverage support for MSVC (debug symbols and optimization control)
        if(ENABLE_COVERAGE OR CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(GameEngineKiro PRIVATE /Zi /Od)
            target_link_options(GameEngineKiro PRIVATE /DEBUG:FULL)
        endif()
    else()
        target_compile_options(GameEngineKiro PRIVATE -Wall -Wextra -Wpedantic)
        
        # Ninja-specific GCC/Clang optimizations
        if(USING_NINJA)
            # Enable colored diagnostics for better Ninja output
            target_compile_options(GameEngineKiro PRIVATE -fcolor-diagnostics)
            # Optimize build times
            target_compile_options(GameEngineKiro PRIVATE -pipe)
        endif()
        
        # Coverage support for GCC/Clang
        if(ENABLE_COVERAGE)
            target_compile_options(GameEngineKiro PRIVATE --coverage -g -O0)
            target_link_options(GameEngineKiro PRIVATE --coverage)
        endif()
    endif()

    # GLM experimental features
    target_compile_definitions(GameEngineKiro PUBLIC GLM_ENABLE_EXPERIMENTAL)

    # Discover and include engine modules
    discover_engine_modules()
    
    # Validate build configuration to prevent conflicts with old files
    validate_build_configuration()
    
    # Function to include engine modules hierarchically
    function(include_engine_modules)
        get_property(modules GLOBAL PROPERTY ENGINE_MODULES)
        foreach(module_name ${modules})
            get_property(module_path GLOBAL PROPERTY "ENGINE_MODULE_${module_name}_PATH")
            
            # Check if module should be enabled
            string(TOUPPER ${module_name} module_name_upper)
            string(REPLACE "-" "_" module_name_upper ${module_name_upper})
            set(module_option_name "ENABLE_${module_name_upper}_MODULE")
            
            # Default enable for known modules
            if(module_name STREQUAL "graphics-opengl" OR 
               module_name STREQUAL "physics-bullet" OR 
               module_name STREQUAL "audio-openal")
                set(${module_option_name} ON CACHE BOOL "Enable ${module_name} module" FORCE)
            endif()
            
            if(${module_option_name})
                message(STATUS "Module enabled: ${module_name}")
                # TODO: Include module subdirectory when module system is fully implemented
                # For now, using legacy integration to avoid compilation conflicts
                # add_subdirectory(${module_path})
            else()
                message(STATUS "Module disabled: ${module_name}")
            endif()
        endforeach()
    endfunction()
    
    # Include engine modules
    include_engine_modules()

endif() # End of engine build block

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

# Function to deploy assets for a project
function(deploy_project_assets project_name target_dir)
    set(project_asset_dir "${CMAKE_CURRENT_SOURCE_DIR}/projects/${project_name}/assets")
    set(shared_asset_dir "${CMAKE_CURRENT_SOURCE_DIR}/shared/assets")
    set(legacy_asset_dir "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    
    # Copy project-specific assets (highest priority)
    if(EXISTS ${project_asset_dir})
        add_custom_command(TARGET ${project_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${project_asset_dir} ${target_dir}/assets
            COMMENT "Copying ${project_name} project assets"
        )
    endif()
    
    # Copy shared assets (medium priority, don't overwrite project assets)
    if(EXISTS ${shared_asset_dir})
        add_custom_command(TARGET ${project_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${shared_asset_dir} ${target_dir}/assets
            COMMENT "Copying shared engine assets"
        )
    endif()
    
    # Copy legacy assets (lowest priority, don't overwrite existing)
    if(EXISTS ${legacy_asset_dir})
        add_custom_command(TARGET ${project_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${legacy_asset_dir} ${target_dir}/assets
            COMMENT "Copying legacy assets for backward compatibility"
        )
    endif()
endfunction()

# Function to deploy configurations for a project
function(deploy_project_configs project_name target_dir)
    set(project_config_dir "${CMAKE_CURRENT_SOURCE_DIR}/projects/${project_name}/config")
    set(shared_config_dir "${CMAKE_CURRENT_SOURCE_DIR}/shared/configs")
    
    # Copy project-specific configurations
    if(EXISTS ${project_config_dir}/engine_config.json)
        add_custom_command(TARGET ${project_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${project_config_dir}/engine_config.json ${target_dir}/engine_config.json
            COMMENT "Copying ${project_name} engine configuration"
        )
    elseif(EXISTS ${shared_config_dir}/default_engine_config.json)
        add_custom_command(TARGET ${project_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${shared_config_dir}/default_engine_config.json ${target_dir}/engine_config.json
            COMMENT "Copying default engine configuration"
        )
    endif()
    
    if(EXISTS ${project_config_dir}/project_config.json)
        add_custom_command(TARGET ${project_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${project_config_dir}/project_config.json ${target_dir}/project_config.json
            COMMENT "Copying ${project_name} project configuration"
        )
    elseif(EXISTS ${shared_config_dir}/default_project_config.json)
        add_custom_command(TARGET ${project_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${shared_config_dir}/default_project_config.json ${target_dir}/project_config.json
            COMMENT "Copying default project configuration"
        )
    endif()
    
    # Copy any additional config files
    if(EXISTS ${project_config_dir}/config.json)
        add_custom_command(TARGET ${project_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${project_config_dir}/config.json ${target_dir}/config.json
            COMMENT "Copying ${project_name} additional configuration"
        )
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
        
        # Add prefix based on test type to avoid conflicts
        if(test_type STREQUAL "integration")
            set(test_executable_name "${first_char_upper}${rest_chars_clean}IntegrationTest")
        else()
            set(test_executable_name "${first_char_upper}${rest_chars_clean}Test")
        endif()
        
        # Check if building specific test
        if(BUILD_SPECIFIC_TEST)
            if(NOT test_executable_name STREQUAL BUILD_SPECIFIC_TEST)
                continue()
            endif()
            message(STATUS "Building specific test: ${test_executable_name}")
        endif()
        
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

# Build engine tests (only when --tests flag is used)
if(BUILD_TESTS)
    # Find packages needed for tests if not already found
    if(BUILD_TESTS_ONLY)
        find_package(OpenGL REQUIRED)
        find_package(glfw3 REQUIRED)
        find_package(glm REQUIRED)
        find_package(glad REQUIRED)
        find_package(assimp QUIET)
        find_package(OpenAL QUIET)
        find_package(Bullet CONFIG REQUIRED)
        find_package(Lua QUIET)
        find_package(nlohmann_json QUIET)
        find_package(fmt QUIET)
        find_package(Stb REQUIRED)
    endif()
    
    # Function to include tests hierarchically
    function(include_engine_tests)
        # Enhanced Test Runner executable
        add_executable(EnhancedTestRunner
            engine/core/TestRunner.cpp
            engine/core/TestFramework.cpp
        )

        target_link_libraries(EnhancedTestRunner PRIVATE GameEngineKiro)
        target_include_directories(EnhancedTestRunner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/engine)

        # Apply coverage settings to enhanced test runner
        apply_coverage_settings(EnhancedTestRunner)

        # Test Configuration Manager executable
        add_executable(TestConfigManager
            engine/core/TestConfigManager.cpp
            engine/core/TestFramework.cpp
        )

        target_link_libraries(TestConfigManager PRIVATE GameEngineKiro)
        target_include_directories(TestConfigManager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/engine)

        # Apply coverage settings to test config manager
        apply_coverage_settings(TestConfigManager)

        # Discover and add unit tests automatically
        discover_and_add_tests("tests/unit" "unit")
    endfunction()
    
    # Include engine tests
    include_engine_tests()
    
    # Discover and add integration tests automatically
    discover_and_add_tests("tests/integration" "integration")


    
endif() # End of tests build block

# Build game projects (when projects requested and not building specific test only)
if((BUILD_PROJECTS OR BUILD_SPECIFIC_PROJECT) AND (NOT BUILD_SPECIFIC_TEST OR BUILD_SPECIFIC_PROJECT))
    # Find packages needed for projects if not already found
    if(BUILD_PROJECTS_ONLY)
        find_package(OpenGL REQUIRED)
        find_package(glfw3 REQUIRED)
        find_package(glm REQUIRED)
        find_package(glad REQUIRED)
        find_package(assimp QUIET)
        find_package(OpenAL QUIET)
        find_package(Bullet CONFIG REQUIRED)
        find_package(Lua QUIET)
        find_package(nlohmann_json QUIET)
        find_package(fmt QUIET)
        find_package(Stb REQUIRED)
    endif()
    

    # Function to include game projects hierarchically
    function(include_game_projects)
        # Discover game projects
        discover_game_projects()
        
        # Include game projects
        get_property(projects GLOBAL PROPERTY ENGINE_PROJECTS)
        foreach(project_name ${projects})
            get_property(project_path GLOBAL PROPERTY "ENGINE_PROJECT_${project_name}_PATH")
            
            # Check if building specific project
            if(BUILD_SPECIFIC_PROJECT)
                if(project_name STREQUAL BUILD_SPECIFIC_PROJECT)
                    message(STATUS "Building specific project: ${project_name}")
                    add_subdirectory(${project_path})
                endif()
            else()
                message(STATUS "Including game project: ${project_name}")
                add_subdirectory(${project_path})
            endif()
        endforeach()
        
        # Legacy examples/ directory has been completely removed from build system
        # All example applications have been moved to the projects/ directory structure:
        # - projects/GameExample/ - Main game example with full engine features
        # - projects/BasicExample/ - Simple example for learning and testing
        # - projects/Tests/ - Comprehensive test suite
        # 
        # The examples/ directory is kept for reference only and is not built
    endfunction()
    
    # Include game projects
    include_game_projects()

endif() # End of projects build block

# Print build summary
print_build_summary()

