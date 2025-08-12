cmake_minimum_required(VERSION 3.16)
project(BasicExample VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project configuration
set(PROJECT_NAME "BasicExample")
set(PROJECT_VERSION "1.0.0")
set(PROJECT_TYPE "Game")

# Required engine modules for this project
set(REQUIRED_ENGINE_MODULES
    graphics-opengl
)

# Optional engine modules for this project
set(OPTIONAL_ENGINE_MODULES
    physics-bullet
    audio-openal
)

# Project source files
set(PROJECT_SOURCES
    src/basic_example.cpp
)

# Project header files (if any)
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

# Deploy assets and configurations using the new system
# This function is defined in the root CMakeLists.txt and handles:
# 1. Project-specific assets (highest priority)
# 2. Shared engine assets (medium priority)
# 3. Legacy assets (lowest priority, backward compatibility)
if(COMMAND deploy_project_assets)
    deploy_project_assets(${PROJECT_NAME} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
else()
    # Fallback for when function is not available
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/../../assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMENT "Copying assets (fallback method)"
    )
endif()

# Deploy configurations using the new system
if(COMMAND deploy_project_configs)
    deploy_project_configs(${PROJECT_NAME} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
else()
    # Fallback for when function is not available
    message(STATUS "Configuration deployment function not available for ${PROJECT_NAME}")
endif()