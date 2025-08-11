cmake_minimum_required(VERSION 3.16)
project(GameExample VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the GameExample executable
add_executable(GameExample
    src/main.cpp
)

# Link with the engine library (target-based linking when built as subdirectory)
target_link_libraries(GameExample PRIVATE GameEngineKiro)

# Dependencies are inherited from the engine library target

# Copy shared assets from engine
add_custom_command(TARGET GameExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/../../assets $<TARGET_FILE_DIR:GameExample>/assets
    COMMENT "Copying shared engine assets to build directory"
)

# Copy project-specific assets if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    add_custom_command(TARGET GameExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:GameExample>/assets
        COMMENT "Copying GameExample project assets to build directory"
    )
endif()

# Copy configuration files
add_custom_command(TARGET GameExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/config/config.json $<TARGET_FILE_DIR:GameExample>/config.json
    COMMENT "Copying GameExample configuration"
)

add_custom_command(TARGET GameExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/config/engine_config.json $<TARGET_FILE_DIR:GameExample>/engine_config.json
    COMMENT "Copying GameExample engine configuration"
)