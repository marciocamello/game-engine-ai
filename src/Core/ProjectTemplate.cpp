#include "Core/ProjectTemplate.h"
#include "Core/Logger.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>

namespace GameEngine {
    namespace Tools {
        
        // Static member definitions
        const std::vector<std::string> ProjectTemplate::s_availableTemplates = {
            "basic",
            "advanced"
        };
        
        const std::vector<std::string> ProjectTemplate::s_availableModules = {
            "graphics-opengl",
            "physics-bullet",
            "physics-physx",
            "audio-openal",
            "scripting-lua",
            "resource-assimp",
            "input",
            "network"
        };

        bool ProjectTemplate::CreateProject(const TemplateConfig& config) {
            LOG_INFO("Creating project: " + config.projectName);
            
            // Validate configuration
            if (!ValidateTemplateConfig(config)) {
                LOG_ERROR("Invalid template configuration");
                return false;
            }
            
            // Create full project path
            std::string projectPath = config.targetDirectory + "/" + config.projectName;
            
            // Check if project already exists
            if (DirectoryExists(projectPath)) {
                LOG_ERROR("Project directory already exists: " + projectPath);
                return false;
            }
            
            // Create directory structure
            if (!CreateDirectoryStructure(projectPath)) {
                LOG_ERROR("Failed to create directory structure");
                return false;
            }
            
            // Create template-specific content
            bool success = false;
            if (config.templateType == "basic") {
                success = CreateBasicGameTemplate(projectPath, config);
            } else if (config.templateType == "advanced") {
                success = CreateAdvancedGameTemplate(projectPath, config);
            } else {
                LOG_ERROR("Unknown template type: " + config.templateType);
                return false;
            }
            
            if (!success) {
                LOG_ERROR("Failed to create template content");
                return false;
            }
            
            LOG_INFO("Project created successfully: " + projectPath);
            return true;
        }

        std::vector<std::string> ProjectTemplate::GetAvailableTemplates() {
            return s_availableTemplates;
        }

        bool ProjectTemplate::ValidateTemplateConfig(const TemplateConfig& config) {
            // Validate project name
            if (!ValidateProjectName(config.projectName)) {
                return false;
            }
            
            // Validate target directory
            if (!ValidateTargetDirectory(config.targetDirectory)) {
                return false;
            }
            
            // Validate template type
            auto it = std::find(s_availableTemplates.begin(), s_availableTemplates.end(), config.templateType);
            if (it == s_availableTemplates.end()) {
                LOG_ERROR("Invalid template type: " + config.templateType);
                return false;
            }
            
            // Validate required modules
            for (const auto& module : config.requiredModules) {
                if (!IsValidModule(module)) {
                    LOG_ERROR("Invalid required module: " + module);
                    return false;
                }
            }
            
            // Validate optional modules
            for (const auto& module : config.optionalModules) {
                if (!IsValidModule(module)) {
                    LOG_ERROR("Invalid optional module: " + module);
                    return false;
                }
            }
            
            return true;
        }

        bool ProjectTemplate::ValidateProjectName(const std::string& projectName) {
            if (projectName.empty()) {
                LOG_ERROR("Project name cannot be empty");
                return false;
            }
            
            // Check for valid characters (alphanumeric, underscore, hyphen)
            std::regex validName("^[a-zA-Z][a-zA-Z0-9_-]*$");
            if (!std::regex_match(projectName, validName)) {
                LOG_ERROR("Invalid project name format: " + projectName);
                return false;
            }
            
            return true;
        }

        bool ProjectTemplate::ValidateTargetDirectory(const std::string& targetDirectory) {
            if (targetDirectory.empty()) {
                LOG_ERROR("Target directory cannot be empty");
                return false;
            }
            
            // Check if target directory exists or can be created
            if (!DirectoryExists(targetDirectory)) {
                try {
                    std::filesystem::create_directories(targetDirectory);
                } catch (const std::exception& e) {
                    LOG_ERROR("Cannot create target directory: " + std::string(e.what()));
                    return false;
                }
            }
            
            return true;
        }

        bool ProjectTemplate::CreateDirectoryStructure(const std::string& projectPath) {
            std::vector<std::string> directories = {
                projectPath,
                projectPath + "/src",
                projectPath + "/include",
                projectPath + "/assets",
                projectPath + "/config"
            };
            
            for (const auto& dir : directories) {
                if (!CreateDirectory(dir)) {
                    LOG_ERROR("Failed to create directory: " + dir);
                    return false;
                }
            }
            
            return true;
        }

        bool ProjectTemplate::CreateCMakeListsFile(const std::string& projectPath, const TemplateConfig& config) {
            std::string filePath = projectPath + "/CMakeLists.txt";
            std::string content = GenerateCMakeContent(config);
            return WriteFileContent(filePath, content);
        }

        bool ProjectTemplate::CreateProjectConfigFile(const std::string& projectPath, const TemplateConfig& config) {
            std::string filePath = projectPath + "/config/project.json";
            std::string content = GenerateProjectConfigContent(config);
            return WriteFileContent(filePath, content);
        }

        bool ProjectTemplate::CreateReadmeFile(const std::string& projectPath, const TemplateConfig& config) {
            std::string filePath = projectPath + "/README.md";
            std::string content = GenerateReadmeContent(config);
            return WriteFileContent(filePath, content);
        }

        bool ProjectTemplate::CreateExampleCode(const std::string& projectPath, const TemplateConfig& config) {
            if (!config.includeExampleCode) {
                return true;
            }
            
            std::string filePath = projectPath + "/src/main.cpp";
            std::string content = GenerateMainCppContent(config);
            return WriteFileContent(filePath, content);
        }

        bool ProjectTemplate::IsValidModule(const std::string& moduleName) {
            auto it = std::find(s_availableModules.begin(), s_availableModules.end(), moduleName);
            return it != s_availableModules.end();
        }

        std::vector<std::string> ProjectTemplate::GetAvailableModules() {
            return s_availableModules;
        }

        bool ProjectTemplate::CreateBasicGameTemplate(const std::string& projectPath, const TemplateConfig& config) {
            // Create CMakeLists.txt
            if (!CreateCMakeListsFile(projectPath, config)) {
                return false;
            }
            
            // Create project configuration
            if (!CreateProjectConfigFile(projectPath, config)) {
                return false;
            }
            
            // Create README
            if (!CreateReadmeFile(projectPath, config)) {
                return false;
            }
            
            // Create example code if requested
            if (!CreateExampleCode(projectPath, config)) {
                return false;
            }
            
            return true;
        }

        bool ProjectTemplate::CreateAdvancedGameTemplate(const std::string& projectPath, const TemplateConfig& config) {
            // Create basic template first
            if (!CreateBasicGameTemplate(projectPath, config)) {
                return false;
            }
            
            // Add additional directories for advanced template
            std::vector<std::string> advancedDirs = {
                projectPath + "/include/Game",
                projectPath + "/src/Game",
                projectPath + "/assets/shaders",
                projectPath + "/assets/textures",
                projectPath + "/assets/models"
            };
            
            for (const auto& dir : advancedDirs) {
                if (!CreateDirectory(dir)) {
                    LOG_ERROR("Failed to create advanced directory: " + dir);
                    return false;
                }
            }
            
            return true;
        }

        bool ProjectTemplate::WriteFileContent(const std::string& filePath, const std::string& content) {
            try {
                std::ofstream file(filePath);
                if (!file.is_open()) {
                    LOG_ERROR("Failed to open file for writing: " + filePath);
                    return false;
                }
                
                file << content;
                file.close();
                return true;
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to write file: " + filePath + " - " + std::string(e.what()));
                return false;
            }
        }

        std::string ProjectTemplate::GenerateCMakeContent(const TemplateConfig& config) {
            std::stringstream ss;
            
            ss << "cmake_minimum_required(VERSION 3.16)\n";
            ss << "project(" << config.projectName << ")\n\n";
            
            ss << "set(CMAKE_CXX_STANDARD 20)\n";
            ss << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";
            
            ss << "# Find Game Engine Kiro\n";
            ss << "find_library(GAME_ENGINE_KIRO GameEngineKiro PATHS ${CMAKE_SOURCE_DIR}/../../build/Release)\n\n";
            
            ss << "# Include directories\n";
            ss << "include_directories(${CMAKE_SOURCE_DIR}/include)\n";
            ss << "include_directories(${CMAKE_SOURCE_DIR}/../../include)\n\n";
            
            ss << "# Source files\n";
            ss << "file(GLOB_RECURSE SOURCES \"src/*.cpp\")\n";
            ss << "file(GLOB_RECURSE HEADERS \"include/*.h\")\n\n";
            
            ss << "# Create executable\n";
            ss << "add_executable(" << config.projectName << " ${SOURCES} ${HEADERS})\n\n";
            
            ss << "# Link libraries\n";
            ss << "target_link_libraries(" << config.projectName << " ${GAME_ENGINE_KIRO})\n\n";
            
            // Add module-specific dependencies
            if (!config.requiredModules.empty() || !config.optionalModules.empty()) {
                ss << "# Module dependencies\n";
                for (const auto& module : config.requiredModules) {
                    ss << "# Required: " << module << "\n";
                }
                for (const auto& module : config.optionalModules) {
                    ss << "# Optional: " << module << "\n";
                }
                ss << "\n";
            }
            
            ss << "# Copy assets to build directory\n";
            ss << "add_custom_command(TARGET " << config.projectName << " POST_BUILD\n";
            ss << "    COMMAND ${CMAKE_COMMAND} -E copy_directory\n";
            ss << "    ${CMAKE_SOURCE_DIR}/assets\n";
            ss << "    $<TARGET_FILE_DIR:" << config.projectName << ">/assets\n";
            ss << "    COMMENT \"Copying assets to build directory\")\n";
            
            return ss.str();
        }

        std::string ProjectTemplate::GenerateProjectConfigContent(const TemplateConfig& config) {
            std::stringstream ss;
            
            ss << "{\n";
            ss << "  \"projectName\": \"" << config.projectName << "\",\n";
            ss << "  \"projectVersion\": \"1.0.0\",\n";
            ss << "  \"requiredModules\": [\n";
            
            for (size_t i = 0; i < config.requiredModules.size(); ++i) {
                ss << "    \"" << config.requiredModules[i] << "\"";
                if (i < config.requiredModules.size() - 1) {
                    ss << ",";
                }
                ss << "\n";
            }
            
            ss << "  ],\n";
            ss << "  \"optionalModules\": [\n";
            
            for (size_t i = 0; i < config.optionalModules.size(); ++i) {
                ss << "    \"" << config.optionalModules[i] << "\"";
                if (i < config.optionalModules.size() - 1) {
                    ss << ",";
                }
                ss << "\n";
            }
            
            ss << "  ],\n";
            ss << "  \"projectSettings\": {\n";
            ss << "    \"assetPath\": \"assets/\",\n";
            ss << "    \"configPath\": \"config/\"\n";
            ss << "  }\n";
            ss << "}\n";
            
            return ss.str();
        }

        std::string ProjectTemplate::GenerateReadmeContent(const TemplateConfig& config) {
            std::stringstream ss;
            
            ss << "# " << config.projectName << "\n\n";
            ss << "A game project created with Game Engine Kiro.\n\n";
            
            ss << "## Template Type\n";
            ss << "- **Type**: " << config.templateType << "\n";
            ss << "- **Created**: " << "Generated by ProjectTemplate system" << "\n\n";
            
            if (!config.requiredModules.empty()) {
                ss << "## Required Modules\n";
                for (const auto& module : config.requiredModules) {
                    ss << "- " << module << "\n";
                }
                ss << "\n";
            }
            
            if (!config.optionalModules.empty()) {
                ss << "## Optional Modules\n";
                for (const auto& module : config.optionalModules) {
                    ss << "- " << module << "\n";
                }
                ss << "\n";
            }
            
            ss << "## Building\n\n";
            ss << "```bash\n";
            ss << "mkdir build\n";
            ss << "cd build\n";
            ss << "cmake ..\n";
            ss << "cmake --build . --config Release\n";
            ss << "```\n\n";
            
            ss << "## Running\n\n";
            ss << "```bash\n";
            ss << "cd build/Release\n";
            ss << "./" << config.projectName << ".exe\n";
            ss << "```\n\n";
            
            ss << "## Project Structure\n\n";
            ss << "```\n";
            ss << config.projectName << "/\n";
            ss << "├── src/           # Source code\n";
            ss << "├── include/       # Header files\n";
            ss << "├── assets/        # Game assets\n";
            ss << "├── config/        # Configuration files\n";
            ss << "├── CMakeLists.txt # Build configuration\n";
            ss << "└── README.md      # This file\n";
            ss << "```\n";
            
            return ss.str();
        }

        std::string ProjectTemplate::GenerateMainCppContent(const TemplateConfig& config) {
            std::stringstream ss;
            
            ss << "#include \"Core/Engine.h\"\n";
            ss << "#include \"Core/Logger.h\"\n";
            ss << "#include <iostream>\n\n";
            
            ss << "using namespace GameEngine;\n\n";
            
            ss << "int main() {\n";
            ss << "    LOG_INFO(\"Starting " << config.projectName << "\");\n\n";
            
            ss << "    try {\n";
            ss << "        Engine engine;\n";
            ss << "        \n";
            ss << "        if (!engine.Initialize()) {\n";
            ss << "            LOG_ERROR(\"Failed to initialize engine\");\n";
            ss << "            return -1;\n";
            ss << "        }\n\n";
            
            ss << "        LOG_INFO(\"Engine initialized successfully\");\n";
            ss << "        \n";
            ss << "        // Main game loop\n";
            ss << "        engine.Run();\n";
            ss << "        \n";
            ss << "        engine.Shutdown();\n";
            ss << "        LOG_INFO(\"" << config.projectName << " completed successfully\");\n";
            ss << "        \n";
            ss << "    } catch (const std::exception& e) {\n";
            ss << "        LOG_ERROR(\"Exception: \" + std::string(e.what()));\n";
            ss << "        return -1;\n";
            ss << "    }\n\n";
            
            ss << "    return 0;\n";
            ss << "}\n";
            
            return ss.str();
        }

        bool ProjectTemplate::CreateDirectory(const std::string& path) {
            try {
                std::filesystem::create_directories(path);
                return true;
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to create directory: " + path + " - " + std::string(e.what()));
                return false;
            }
        }

        bool ProjectTemplate::DirectoryExists(const std::string& path) {
            return std::filesystem::exists(path) && std::filesystem::is_directory(path);
        }

        bool ProjectTemplate::FileExists(const std::string& path) {
            return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
        }
    }
}