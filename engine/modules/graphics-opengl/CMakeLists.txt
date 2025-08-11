# Graphics OpenGL Module CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Module information
set(MODULE_NAME "graphics-opengl")
set(MODULE_VERSION "1.0.0")
set(MODULE_TYPE "Graphics")

# Module source files
set(MODULE_SOURCES
    ../OpenGLGraphicsModule.cpp
    ../GraphicsModuleFactory.cpp
)

set(MODULE_HEADERS
    ../OpenGLGraphicsModule.h
    ../GraphicsModuleFactory.h
)

# Module dependencies
set(MODULE_REQUIRED_PACKAGES
    OpenGL
    glfw3
    glad
    glm
)

set(MODULE_OPTIONAL_PACKAGES
    # Add optional packages here
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
target_link_libraries(${MODULE_NAME} PUBLIC
    OpenGL::GL
    glfw
    glad::glad
    glm::glm
)

# Compile definitions
target_compile_definitions(${MODULE_NAME} PUBLIC 
    GAMEENGINE_HAS_OPENGL
    GLM_ENABLE_EXPERIMENTAL
)

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
    target_compile_definitions(GameEngineKiro PUBLIC GAMEENGINE_MODULE_GRAPHICS_OPENGL)
    
    message(STATUS "Graphics OpenGL module integrated with GameEngineKiro")
else()
    message(WARNING "GameEngineKiro target not found - creating standalone module")
endif()

message(STATUS "Configured module: ${MODULE_NAME} v${MODULE_VERSION} (${MODULE_TYPE})")