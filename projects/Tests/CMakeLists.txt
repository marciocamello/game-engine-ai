# Project Tests CMakeLists.txt
# This file manages testing for game projects (GameExample, BasicExample, etc.)
# Engine tests remain in the root tests/ directory

cmake_minimum_required(VERSION 3.16)
project(ProjectTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/engine/core)
include_directories(${CMAKE_SOURCE_DIR}/engine/interfaces)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/tests)  # For TestUtils.h

# Function to add project tests
function(add_project_test TEST_NAME TEST_SOURCE PROJECT_NAME)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} 
        GameEngineKiro
        ${OPENGL_LIBRARIES}
    )
    
    # Set output directory
    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    # Add to test suite
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# Enable testing
enable_testing()

# Discover and add unit tests
file(GLOB_RECURSE UNIT_TEST_SOURCES "unit/*.cpp")
foreach(TEST_SOURCE ${UNIT_TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    string(REPLACE "test_" "" TEST_NAME ${TEST_NAME})
    set(TEST_NAME "Project${TEST_NAME}Test")
    add_project_test(${TEST_NAME} ${TEST_SOURCE} "")
endforeach()

# Discover and add integration tests
file(GLOB_RECURSE INTEGRATION_TEST_SOURCES "integration/*.cpp")
foreach(TEST_SOURCE ${INTEGRATION_TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    string(REPLACE "test_" "" TEST_NAME ${TEST_NAME})
    set(TEST_NAME "Project${TEST_NAME}IntegrationTest")
    add_project_test(${TEST_NAME} ${TEST_SOURCE} "")
endforeach()

# Print configuration info
message(STATUS "Project Tests Configuration:")
message(STATUS "  Unit tests found: ${UNIT_TEST_SOURCES}")
message(STATUS "  Integration tests found: ${INTEGRATION_TEST_SOURCES}")