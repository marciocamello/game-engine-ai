# Engine Module Template CMakeLists.txt
# This template can be used to create new engine modules

cmake_minimum_required(VERSION 3.16)

# Module information
# set(MODULE_NAME "your-module-name")        # Set this to your module name (kebab-case)
# set(MODULE_VERSION "1.0.0")               # Set this to your module version
# set(MODULE_TYPE "YourModuleType")          # Set this to module type (Graphics, Physics, Audio, etc.)

# Module source files
# Add your module source files here
set(MODULE_SOURCES
    YourModule.cpp
    # Add more source files here
)

# Module header files
set(MODULE_HEADERS
    YourModule.h
    # Add more header files here
)

# Module dependencies
# List required packages that must be found
set(MODULE_REQUIRED_PACKAGES
    # Example: OpenGL
    # Example: Bullet
)

# List optional packages that enhance functionality if available
set(MODULE_OPTIONAL_PACKAGES
    # Example: Lua
    # Example: nlohmann_json
)

# Check required dependencies
foreach(package ${MODULE_REQUIRED_PACKAGES})
    find_package(${package} REQUIRED)
    if(NOT ${package}_FOUND)
        message(FATAL_ERROR "Required package ${package} not found for module ${MODULE_NAME}")
    endif()
endforeach()

# Check optional dependencies
foreach(package ${MODULE_OPTIONAL_PACKAGES})
    find_package(${package} QUIET)
    if(${package}_FOUND)
        message(STATUS "Optional package ${package} found for module ${MODULE_NAME}")
    else()
        message(STATUS "Optional package ${package} not found for module ${MODULE_NAME}")
    endif()
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
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core
)

# Link required dependencies
# Example for different package types:
# if(OpenGL_FOUND)
#     target_link_libraries(${MODULE_NAME} PUBLIC OpenGL::GL)
#     target_compile_definitions(${MODULE_NAME} PUBLIC GAMEENGINE_HAS_OPENGL)
# endif()
#
# if(Bullet_FOUND)
#     target_link_libraries(${MODULE_NAME} PUBLIC 
#         BulletDynamics
#         BulletCollision
#         LinearMath
#     )
#     target_compile_definitions(${MODULE_NAME} PUBLIC GAMEENGINE_HAS_BULLET)
# endif()

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
    target_sources(GameEngineKiro PRIVATE ${MODULE_SOURCES} ${MODULE_HEADERS})
    
    # Add module-specific compile definitions to parent
    string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)
    string(REPLACE "-" "_" MODULE_NAME_UPPER ${MODULE_NAME_UPPER})
    target_compile_definitions(GameEngineKiro PUBLIC GAMEENGINE_MODULE_${MODULE_NAME_UPPER})
    
    # Add dependency-specific definitions
    # Example:
    # if(OpenGL_FOUND)
    #     target_compile_definitions(GameEngineKiro PUBLIC GAMEENGINE_MODULE_${MODULE_NAME_UPPER}_OPENGL)
    # endif()
endif()

message(STATUS "Configured module: ${MODULE_NAME} v${MODULE_VERSION} (${MODULE_TYPE})")