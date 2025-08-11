#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace GameEngine {
    namespace Tools {
        
        struct TemplateConfig {
            std::string projectName;
            std::string targetDirectory;
            std::vector<std::string> requiredModules;
            std::vector<std::string> optionalModules;
            bool includeExampleCode = false;
            std::string templateType = "basic";
        };

        struct ProjectConfig {
            std::string projectName;
            std::string projectVersion = "1.0.0";
            std::vector<std::string> requiredModules;
            std::vector<std::string> optionalModules;
            std::unordered_map<std::string, std::string> projectSettings;
            std::string assetPath = "assets/";
            std::string configPath = "config/";
        };

        class ProjectTemplate {
        public:
            // Template creation
            static bool CreateProject(const TemplateConfig& config);
            static std::vector<std::string> GetAvailableTemplates();
            
            // Template validation
            static bool ValidateTemplateConfig(const TemplateConfig& config);
            static bool ValidateProjectName(const std::string& projectName);
            static bool ValidateTargetDirectory(const std::string& targetDirectory);
            
            // Template utilities
            static bool CreateDirectoryStructure(const std::string& projectPath);
            static bool CreateCMakeListsFile(const std::string& projectPath, const TemplateConfig& config);
            static bool CreateProjectConfigFile(const std::string& projectPath, const TemplateConfig& config);
            static bool CreateReadmeFile(const std::string& projectPath, const TemplateConfig& config);
            static bool CreateExampleCode(const std::string& projectPath, const TemplateConfig& config);
            
            // Module validation
            static bool IsValidModule(const std::string& moduleName);
            static std::vector<std::string> GetAvailableModules();
            
        private:
            // Internal template creation helpers
            static bool CreateBasicGameTemplate(const std::string& projectPath, const TemplateConfig& config);
            static bool CreateAdvancedGameTemplate(const std::string& projectPath, const TemplateConfig& config);
            
            // File creation helpers
            static bool WriteFileContent(const std::string& filePath, const std::string& content);
            static std::string GenerateCMakeContent(const TemplateConfig& config);
            static std::string GenerateProjectConfigContent(const TemplateConfig& config);
            static std::string GenerateReadmeContent(const TemplateConfig& config);
            static std::string GenerateMainCppContent(const TemplateConfig& config);
            
            // Directory utilities
            static bool CreateDirectory(const std::string& path);
            static bool DirectoryExists(const std::string& path);
            static bool FileExists(const std::string& path);
            
            // Available templates and modules
            static const std::vector<std::string> s_availableTemplates;
            static const std::vector<std::string> s_availableModules;
        };
    }
}