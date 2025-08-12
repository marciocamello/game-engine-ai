# Game Project CMakeLists.txt Template
# This template provides a standardized structure for game projects

cmake_minimum_required(VERSION 3.16)
project(ProjectName VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project configuration (customize per project)
set(PROJECT_NAME "ProjectName")     # Replace with actual project name
set(PROJECT_VERSION "1.0.0")
set(PROJECT_TYPE "Game")

# Required engine modules for this project (customize per project)
set(REQUIRED_ENGINE_MODULES
    # Add required modules here
    # Example: graphics-opengl, physics-bullet, audio-openal
)

# Optional engine modules for this project (customize per project)
set(OPTIONAL_ENGINE_MODULES
    # Add optional modules here
)

# Project source files (customize per project)
set(PROJECT_SOURCES
    src/main.cpp
    # Add additional source files here
)

# Project header files (customize per project)
set(PROJECT_HEADERS
    # Add project headers here
)

# Create the project executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS})

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    VERSION ${PROJECT_VERSION}
)

# Link with the engine library (target-based linking when built as subdirectory)
if(TARGET GameEngineKiro)
    target_link_libraries(${PROJECT_NAME} PRIVATE GameEngineKiro)
    message(STATUS "Linking ${PROJECT_NAME} with GameEngineKiro")
else()
    message(WARNING "GameEngineKiro target not found for ${PROJECT_NAME}")
endif()

# Validate required modules are available
foreach(module ${REQUIRED_ENGINE_MODULES})
    string(TOUPPER ${module} module_upper)
    string(REPLACE "-" "_" module_upper ${module_upper})
    set(module_define "GAMEENGINE_MODULE_${module_upper}")
    
    # This will be checked at compile time via preprocessor definitions
    message(STATUS "${PROJECT_NAME} requires module: ${module}")
endforeach()

# Check optional modules
foreach(module ${OPTIONAL_ENGINE_MODULES})
    string(TOUPPER ${module} module_upper)
    string(REPLACE "-" "_" module_upper ${module_upper})
    set(module_define "GAMEENGINE_MODULE_${module_upper}")
    
    message(STATUS "${PROJECT_NAME} optionally uses module: ${module}")
endforeach()

# Dependencies are inherited from the engine library target

# Copy shared assets from engine
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/../../assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying shared engine assets to build directory"
)

# Copy project-specific assets if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMENT "Copying ${PROJECT_NAME} project assets to build directory"
    )
endif()

# Copy configuration files if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/config.json)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config/config.json $<TARGET_FILE_DIR:${PROJECT_NAME}>/config.json
        COMMENT "Copying ${PROJECT_NAME} configuration"
    )
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/engine_config.json)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config/engine_config.json $<TARGET_FILE_DIR:${PROJECT_NAME}>/engine_config.json
        COMMENT "Copying ${PROJECT_NAME} engine configuration"
    )
endif()