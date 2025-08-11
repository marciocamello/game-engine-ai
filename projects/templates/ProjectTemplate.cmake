# Project Template CMakeLists.txt
# This template can be used to create new game projects

cmake_minimum_required(VERSION 3.16)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project configuration
# set(PROJECT_NAME "YourProjectName")        # Set this to your project name
# set(PROJECT_VERSION "1.0.0")              # Set this to your project version
# set(PROJECT_TYPE "Game")                   # Project type (Game, Tool, etc.)

# Required engine modules for this project
# Uncomment and modify as needed
set(REQUIRED_ENGINE_MODULES
    graphics-opengl    # Required for rendering
    # physics-bullet   # Required for physics simulation
    # audio-openal     # Required for audio
)

# Optional engine modules for this project
# These modules will be used if available but won't cause build failure if missing
set(OPTIONAL_ENGINE_MODULES
    # scripting-lua    # Optional scripting support
    # network          # Optional networking
)

# Project source files
# Add your source files here
set(PROJECT_SOURCES
    src/main.cpp
    # Add more source files here
)

# Project header files
# Add your header files here (optional, for IDE organization)
set(PROJECT_HEADERS
    # include/YourHeader.h
    # Add more header files here
)

# Create the project executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS})

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    VERSION ${PROJECT_VERSION}
)

# Include directories for project-specific headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link with the engine library
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

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy shared assets from engine
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/../../assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying shared engine assets to ${PROJECT_NAME} build directory"
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

message(STATUS "Configured project: ${PROJECT_NAME} v${PROJECT_VERSION} (${PROJECT_TYPE})")