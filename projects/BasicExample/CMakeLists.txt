cmake_minimum_required(VERSION 3.16)
project(BasicExample VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the BasicExample executable
add_executable(BasicExample
    src/basic_example.cpp
)

# Link with the engine library (target-based linking when built as subdirectory)
target_link_libraries(BasicExample PRIVATE GameEngineKiro)

# Dependencies are inherited from the engine library target

# Copy shared assets from engine
add_custom_command(TARGET BasicExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/../../assets $<TARGET_FILE_DIR:BasicExample>/assets
    COMMENT "Copying shared engine assets to build directory"
)

# Copy project-specific assets if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    add_custom_command(TARGET BasicExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:BasicExample>/assets
        COMMENT "Copying BasicExample project assets to build directory"
    )
endif()