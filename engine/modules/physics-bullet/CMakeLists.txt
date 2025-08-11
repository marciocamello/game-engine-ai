# Physics Bullet Module CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Module information
set(MODULE_NAME "physics-bullet")
set(MODULE_VERSION "1.0.0")
set(MODULE_TYPE "Physics")

# Module source files
set(MODULE_SOURCES
    ../BulletPhysicsModule.cpp
    ../PhysicsModuleFactory.cpp
)

set(MODULE_HEADERS
    ../BulletPhysicsModule.h
    ../PhysicsModuleFactory.h
)

# Module dependencies
set(MODULE_REQUIRED_PACKAGES
    Bullet
)

set(MODULE_OPTIONAL_PACKAGES
    # Add optional packages here
)

# Check required dependencies
foreach(package ${MODULE_REQUIRED_PACKAGES})
    find_package(${package} CONFIG REQUIRED)
    if(NOT ${package}_FOUND)
        message(FATAL_ERROR "Required package ${package} not found for module ${MODULE_NAME}")
    endif()
endforeach()

# Check optional dependencies
foreach(package ${MODULE_OPTIONAL_PACKAGES})
    find_package(${package} QUIET)
endforeach()

# Create module library
add_library(${MODULE_NAME} STATIC ${MODULE_SOURCES} ${MODULE_HEADERS})

# Set target properties
set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    VERSION ${MODULE_VERSION}
)

# Include directories
target_include_directories(${MODULE_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core
)

# Link dependencies
if(Bullet_FOUND)
    target_link_libraries(${MODULE_NAME} PUBLIC 
        BulletDynamics
        BulletCollision
        LinearMath
    )
    target_compile_definitions(${MODULE_NAME} PUBLIC GAMEENGINE_HAS_BULLET)
endif()

# Link with engine core (if available)
if(TARGET GameEngineKiro)
    target_link_libraries(${MODULE_NAME} PUBLIC GameEngineKiro)
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(${MODULE_NAME} PRIVATE /W4)
    target_compile_definitions(${MODULE_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(${MODULE_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Module registration (add sources to parent target if it exists)
if(TARGET GameEngineKiro)
    # Add module-specific compile definitions to parent
    if(Bullet_FOUND)
        target_compile_definitions(GameEngineKiro PUBLIC GAMEENGINE_MODULE_PHYSICS_BULLET)
    endif()
    
    message(STATUS "Physics Bullet module integrated with GameEngineKiro")
else()
    message(WARNING "GameEngineKiro target not found - creating standalone module")
endif()

message(STATUS "Configured module: ${MODULE_NAME} v${MODULE_VERSION} (${MODULE_TYPE})")