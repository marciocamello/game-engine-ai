cmake_minimum_required(VERSION 3.16)
project(GameEngineKiro VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force compile commands generation for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(MSVC)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
endif()

# Engine configuration
option(ENABLE_VULKAN "Enable Vulkan renderer" OFF)
option(ENABLE_OPENGL "Enable OpenGL renderer" ON)
option(ENABLE_DLSS "Enable NVIDIA DLSS support" OFF)
option(ENABLE_FSR "Enable AMD FSR support" OFF)
option(USE_VCPKG "Use vcpkg for dependencies" ON)

# Setup vcpkg toolchain if available
if(USE_VCPKG AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg toolchain")
endif()

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(glad REQUIRED)

# Optional packages
find_package(assimp QUIET)
find_package(OpenAL QUIET)
find_package(Bullet QUIET)
find_package(Lua QUIET)
find_package(nlohmann_json QUIET)
find_package(fmt QUIET)

# Engine source files
file(GLOB_RECURSE ENGINE_SOURCES
    "src/Core/*.cpp"
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

file(GLOB_RECURSE ENGINE_HEADERS
    "include/Core/*.h"
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

# Create engine library
add_library(GameEngineKiro STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

target_include_directories(GameEngineKiro PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
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
    target_link_libraries(GameEngineKiro PUBLIC ${BULLET_LIBRARIES})
    target_include_directories(GameEngineKiro PUBLIC ${BULLET_INCLUDE_DIRS})
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

# Platform-specific libraries
if(WIN32)
    target_link_libraries(GameEngineKiro PUBLIC winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(GameEngineKiro PUBLIC pthread dl)
elseif(APPLE)
    target_link_libraries(GameEngineKiro PUBLIC "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(GameEngineKiro PRIVATE /W4)
    target_compile_definitions(GameEngineKiro PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(GameEngineKiro PRIVATE -Wall -Wextra -Wpedantic)
endif()

# GLM experimental features
target_compile_definitions(GameEngineKiro PUBLIC GLM_ENABLE_EXPERIMENTAL)

# Example game executable
add_executable(GameExample
    examples/main.cpp
)

target_link_libraries(GameExample PRIVATE GameEngineKiro)

# Copy assets to build directory
add_custom_command(TARGET GameExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:GameExample>/assets
    COMMENT "Copying assets to build directory"
)