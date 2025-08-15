#include "Animation/AnimationHotReloader.h"
#include "Core/Logger.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace GameEngine {
namespace Animation {

    // AnimationHotReloader Implementation

    bool AnimationHotReloader::Initialize() {
        if (m_initialized) {
            LOG_WARNING("AnimationHotReloader already initialized");
            return true;
        }

        LOG_INFO("Initializing Animation Hot Reloader");
        
        m_watchedFiles.clear();
        m_validationResults.clear();
        m_timeSinceLastCheck = 0.0f;
        m_initialized = true;
        
        LOG_INFO("Animation Hot Reloader initialized successfully");
        return true;
    }

    void AnimationHotReloader::Shutdown() {
        if (!m_initialized) {
            return;
        }

        LOG_INFO("Shutting down Animation Hot Reloader");
        
        m_watchedFiles.clear();
        m_validationResults.clear();
        m_reloadCallback = nullptr;
        m_errorCallback = nullptr;
        m_validationCallback = nullptr;
        
        m_initialized = false;
        m_enabled = false;
        
        LOG_INFO("Animation Hot Reloader shut down");
    }

    void AnimationHotReloader::Update(float deltaTime) {
        if (!m_initialized || !m_enabled) {
            return;
        }

        m_timeSinceLastCheck += deltaTime;
        
        if (m_timeSinceLastCheck >= m_checkInterval) {
            CheckFileChanges();
            m_timeSinceLastCheck = 0.0f;
        }
    }

    void AnimationHotReloader::WatchAnimationDirectory(const std::string& directory) {
        if (!m_initialized) {
            LOG_ERROR("AnimationHotReloader not initialized");
            return;
        }

        if (!std::filesystem::exists(directory)) {
            LOG_ERROR("Animation directory does not exist: " + directory);
            return;
        }

        LOG_INFO("Watching animation directory: " + directory);
        ProcessDirectoryRecursively(directory);
    }

    void AnimationHotReloader::WatchAnimationFile(const std::string& filepath) {
        if (!m_initialized) {
            LOG_ERROR("AnimationHotReloader not initialized");
            return;
        }

        if (!std::filesystem::exists(filepath)) {
            LOG_ERROR("Animation file does not exist: " + filepath);
            return;
        }

        if (!IsAnimationFile(filepath)) {
            LOG_WARNING("File is not a recognized animation file: " + filepath);
            return;
        }

        WatchedAnimationFile watchedFile;
        watchedFile.filepath = filepath;
        watchedFile.assetType = DetectAssetType(filepath);
        watchedFile.lastWriteTime = std::filesystem::last_write_time(filepath);
        watchedFile.needsReload = false;
        watchedFile.isValid = false; // Will be set to true after validation

        // Perform initial validation if enabled
        if (m_autoValidation) {
            auto result = ValidateAnimationFile(filepath);
            m_validationResults[filepath] = result;
            
            watchedFile.isValid = result.isValid;
            if (!result.isValid && !result.errors.empty()) {
                watchedFile.lastError = result.errors[0];
            } else {
                watchedFile.lastError.clear();
            }
            
            LogValidationResult(filepath, result);
            
            if (m_validationCallback) {
                m_validationCallback(filepath, result);
            }
        }
        
        m_watchedFiles[filepath] = watchedFile;

        LOG_INFO("Now watching animation file: " + GetRelativePath(filepath) + " (type: " + watchedFile.assetType + ")");
    }

    void AnimationHotReloader::UnwatchAnimationFile(const std::string& filepath) {
        auto it = m_watchedFiles.find(filepath);
        if (it != m_watchedFiles.end()) {
            LOG_INFO("Stopped watching animation file: " + GetRelativePath(filepath));
            m_watchedFiles.erase(it);
            m_validationResults.erase(filepath);
        }
    }

    void AnimationHotReloader::ClearWatchedFiles() {
        LOG_INFO("Clearing all watched animation files");
        m_watchedFiles.clear();
        m_validationResults.clear();
    }

    void AnimationHotReloader::ReloadAnimation(const std::string& filepath) {
        auto it = m_watchedFiles.find(filepath);
        if (it == m_watchedFiles.end()) {
            LOG_WARNING("Attempted to reload unwatched file: " + filepath);
            return;
        }

        ProcessReloadedFile(filepath);
    }

    void AnimationHotReloader::ReloadAllAnimations() {
        LOG_INFO("Reloading all watched animations");
        
        for (auto& [filepath, watchedFile] : m_watchedFiles) {
            ProcessReloadedFile(filepath);
        }
    }

    void AnimationHotReloader::ValidateAnimation(const std::string& filepath) {
        auto result = ValidateAnimationFile(filepath);
        m_validationResults[filepath] = result;
        
        LogValidationResult(filepath, result);
        
        if (m_validationCallback) {
            m_validationCallback(filepath, result);
        }
    }

    void AnimationHotReloader::ValidateAllAnimations() {
        LOG_INFO("Validating all watched animations");
        
        for (const auto& [filepath, watchedFile] : m_watchedFiles) {
            ValidateAnimation(filepath);
        }
    }

    void AnimationHotReloader::OptimizeAnimation(const std::string& filepath) {
        if (!m_optimizationEnabled) {
            LOG_INFO("Animation optimization is disabled");
            return;
        }

        LOG_INFO("Optimizing animation: " + GetRelativePath(filepath));
        
        if (OptimizeAnimationFile(filepath)) {
            LOG_INFO("Animation optimization completed: " + GetRelativePath(filepath));
        } else {
            LOG_ERROR("Animation optimization failed: " + GetRelativePath(filepath));
        }
    }

    void AnimationHotReloader::OptimizeAllAnimations() {
        if (!m_optimizationEnabled) {
            LOG_INFO("Animation optimization is disabled");
            return;
        }

        LOG_INFO("Optimizing all watched animations");
        
        for (const auto& [filepath, watchedFile] : m_watchedFiles) {
            OptimizeAnimation(filepath);
        }
    }

    std::vector<std::string> AnimationHotReloader::GetWatchedFiles() const {
        std::vector<std::string> files;
        files.reserve(m_watchedFiles.size());
        
        for (const auto& [filepath, watchedFile] : m_watchedFiles) {
            files.push_back(filepath);
        }
        
        return files;
    }

    std::vector<std::string> AnimationHotReloader::GetInvalidFiles() const {
        std::vector<std::string> invalidFiles;
        
        for (const auto& [filepath, watchedFile] : m_watchedFiles) {
            if (!watchedFile.isValid) {
                invalidFiles.push_back(filepath);
            }
        }
        
        return invalidFiles;
    }

    bool AnimationHotReloader::IsFileWatched(const std::string& filepath) const {
        return m_watchedFiles.find(filepath) != m_watchedFiles.end();
    }

    bool AnimationHotReloader::IsFileValid(const std::string& filepath) const {
        auto it = m_watchedFiles.find(filepath);
        return it != m_watchedFiles.end() && it->second.isValid;
    }

    std::string AnimationHotReloader::GetFileError(const std::string& filepath) const {
        auto it = m_watchedFiles.find(filepath);
        return it != m_watchedFiles.end() ? it->second.lastError : "";
    }

    std::string AnimationHotReloader::GetAssetType(const std::string& filepath) const {
        auto it = m_watchedFiles.find(filepath);
        return it != m_watchedFiles.end() ? it->second.assetType : "";
    }

    AnimationValidationResult AnimationHotReloader::GetValidationResult(const std::string& filepath) const {
        auto it = m_validationResults.find(filepath);
        return it != m_validationResults.end() ? it->second : AnimationValidationResult{};
    }

    void AnimationHotReloader::GenerateAssetReport(const std::string& outputPath) const {
        std::ofstream report(outputPath);
        if (!report.is_open()) {
            LOG_ERROR("Failed to create asset report file: " + outputPath);
            return;
        }

        report << "Animation Asset Report\n";
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        report << "Generated: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\n";
        report << "==============================================\n\n";

        report << "Summary:\n";
        report << "  Total watched files: " << m_watchedFiles.size() << "\n";
        report << "  Valid files: " << (m_watchedFiles.size() - GetInvalidFiles().size()) << "\n";
        report << "  Invalid files: " << GetInvalidFiles().size() << "\n\n";

        report << "File Details:\n";
        for (const auto& [filepath, watchedFile] : m_watchedFiles) {
            report << "  File: " << GetRelativePath(filepath) << "\n";
            report << "    Type: " << watchedFile.assetType << "\n";
            report << "    Valid: " << (watchedFile.isValid ? "Yes" : "No") << "\n";
            
            if (!watchedFile.isValid && !watchedFile.lastError.empty()) {
                report << "    Error: " << watchedFile.lastError << "\n";
            }
            
            auto validationIt = m_validationResults.find(filepath);
            if (validationIt != m_validationResults.end()) {
                const auto& result = validationIt->second;
                report << "    File Size: " << result.fileSize << " bytes\n";
                report << "    Version: " << result.version << "\n";
                
                if (!result.warnings.empty()) {
                    report << "    Warnings:\n";
                    for (const auto& warning : result.warnings) {
                        report << "      - " << warning << "\n";
                    }
                }
            }
            
            report << "\n";
        }

        report.close();
        LOG_INFO("Asset report generated: " + outputPath);
    }

    void AnimationHotReloader::ExportAssetStatistics(const std::string& outputPath) const {
        std::ofstream stats(outputPath);
        if (!stats.is_open()) {
            LOG_ERROR("Failed to create asset statistics file: " + outputPath);
            return;
        }

        // Export as JSON for easy parsing
        stats << "{\n";
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        stats << "  \"timestamp\": \"" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\",\n";
        stats << "  \"totalFiles\": " << m_watchedFiles.size() << ",\n";
        stats << "  \"validFiles\": " << (m_watchedFiles.size() - GetInvalidFiles().size()) << ",\n";
        stats << "  \"invalidFiles\": " << GetInvalidFiles().size() << ",\n";
        stats << "  \"assetTypes\": {\n";

        // Count asset types
        std::unordered_map<std::string, size_t> typeCounts;
        for (const auto& [filepath, watchedFile] : m_watchedFiles) {
            typeCounts[watchedFile.assetType]++;
        }

        bool first = true;
        for (const auto& [type, count] : typeCounts) {
            if (!first) stats << ",\n";
            stats << "    \"" << type << "\": " << count;
            first = false;
        }

        stats << "\n  }\n";
        stats << "}\n";

        stats.close();
        LOG_INFO("Asset statistics exported: " + outputPath);
    }

    // Private methods

    void AnimationHotReloader::CheckFileChanges() {
        for (auto& [filepath, watchedFile] : m_watchedFiles) {
            if (HasFileChanged(watchedFile)) {
                watchedFile.needsReload = true;
                UpdateFileTimestamp(filepath);
                ProcessReloadedFile(filepath);
            }
        }
    }

    bool AnimationHotReloader::HasFileChanged(const WatchedAnimationFile& file) {
        if (!std::filesystem::exists(file.filepath)) {
            return false;
        }

        auto currentWriteTime = std::filesystem::last_write_time(file.filepath);
        return currentWriteTime != file.lastWriteTime;
    }

    void AnimationHotReloader::UpdateFileTimestamp(const std::string& filepath) {
        auto it = m_watchedFiles.find(filepath);
        if (it != m_watchedFiles.end() && std::filesystem::exists(filepath)) {
            it->second.lastWriteTime = std::filesystem::last_write_time(filepath);
        }
    }

    void AnimationHotReloader::ProcessDirectoryRecursively(const std::string& directory) {
        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file() && IsAnimationFile(entry.path().string())) {
                    WatchAnimationFile(entry.path().string());
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            LOG_ERROR("Error processing directory " + directory + ": " + e.what());
        }
    }

    bool AnimationHotReloader::IsAnimationFile(const std::string& filepath) const {
        std::string extension = GetFileExtension(filepath);
        
        // Support common animation file formats
        return extension == ".json" || 
               extension == ".anim" || 
               extension == ".fbx" || 
               extension == ".gltf" || 
               extension == ".glb";
    }

    std::string AnimationHotReloader::DetectAssetType(const std::string& filepath) const {
        // Try to detect asset type from file content
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return "unknown";
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // Simple heuristic based on content - check for exact matches first
        if (content.find("\"type\": \"skeletal_animation\"") != std::string::npos ||
            content.find("\"type\":\"skeletal_animation\"") != std::string::npos) {
            return "skeletal_animation";
        } else if (content.find("\"type\": \"state_machine\"") != std::string::npos ||
                   content.find("\"type\":\"state_machine\"") != std::string::npos) {
            return "state_machine";
        } else if (content.find("\"type\": \"blend_tree\"") != std::string::npos ||
                   content.find("\"type\":\"blend_tree\"") != std::string::npos) {
            return "blend_tree";
        }
        
        // Fallback heuristics based on content structure
        if (content.find("\"boneAnimations\"") != std::string::npos || 
            content.find("\"keyframes\"") != std::string::npos ||
            content.find("\"duration\"") != std::string::npos) {
            return "skeletal_animation";
        } else if (content.find("\"states\"") != std::string::npos || 
                   content.find("\"transitions\"") != std::string::npos ||
                   content.find("\"entryState\"") != std::string::npos) {
            return "state_machine";
        } else if (content.find("\"blendType\"") != std::string::npos || 
                   content.find("\"motions\"") != std::string::npos ||
                   content.find("\"parameterX\"") != std::string::npos) {
            return "blend_tree";
        }

        return "unknown";
    }

    AnimationValidationResult AnimationHotReloader::ValidateAnimationFile(const std::string& filepath) {
        AnimationValidationResult result;
        
        try {
            // Get file size
            if (std::filesystem::exists(filepath)) {
                result.fileSize = std::filesystem::file_size(filepath);
            }

            // Read and parse file
            std::ifstream file(filepath);
            if (!file.is_open()) {
                result.errors.push_back("Cannot open file for reading");
                return result;
            }

            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();

            // Detect asset type
            result.assetType = DetectAssetType(filepath);
            
            if (result.assetType == "unknown") {
                result.errors.push_back("Unknown or unsupported asset type");
                return result;
            }

            // Validate using AnimationSerialization
            bool isValidData = AnimationSerialization::ValidateAnimationData(content, result.assetType);
            
            if (!isValidData) {
                result.errors.push_back("Invalid animation data format");
                return result;
            }

            // Additional validation based on asset type
            if (result.assetType == "skeletal_animation") {
                auto animation = AnimationSerialization::DeserializeSkeletalAnimation(content);
                if (!animation) {
                    result.errors.push_back("Failed to deserialize skeletal animation");
                    return result;
                }
                
                // Check for common issues
                if (animation->GetDuration() <= 0.0f) {
                    result.warnings.push_back("Animation has zero or negative duration");
                }
                
                if (animation->GetFrameRate() <= 0.0f) {
                    result.warnings.push_back("Animation has invalid frame rate");
                }
                
                if (animation->GetBoneAnimations().empty()) {
                    result.warnings.push_back("Animation has no bone animations");
                }
            }
            else if (result.assetType == "state_machine") {
                auto stateMachine = AnimationSerialization::DeserializeStateMachine(content);
                if (!stateMachine) {
                    result.errors.push_back("Failed to deserialize state machine");
                    return result;
                }
                
                // Check for common issues
                if (stateMachine->GetAllStates().empty()) {
                    result.warnings.push_back("State machine has no states");
                }
                
                if (stateMachine->GetEntryState().empty()) {
                    result.warnings.push_back("State machine has no entry state");
                }
            }
            else if (result.assetType == "blend_tree") {
                auto blendTree = AnimationSerialization::DeserializeBlendTree(content);
                if (!blendTree) {
                    result.errors.push_back("Failed to deserialize blend tree");
                    return result;
                }
                
                // Check for common issues
                if (blendTree->IsEmpty()) {
                    result.warnings.push_back("Blend tree has no motions");
                }
                
                if (blendTree->GetParameterX().empty()) {
                    result.warnings.push_back("Blend tree has no parameter X");
                }
            }

            result.isValid = true;
            
        } catch (const std::exception& e) {
            result.errors.push_back("Validation exception: " + std::string(e.what()));
        }

        return result;
    }

    bool AnimationHotReloader::OptimizeAnimationFile(const std::string& filepath) {
        // Placeholder for animation optimization
        // This could include:
        // - Keyframe reduction
        // - Compression
        // - Format conversion
        // - Validation fixes
        
        LOG_INFO("Animation optimization not yet implemented for: " + GetRelativePath(filepath));
        return true;
    }

    void AnimationHotReloader::ProcessReloadedFile(const std::string& filepath) {
        auto it = m_watchedFiles.find(filepath);
        if (it == m_watchedFiles.end()) {
            return;
        }

        auto& watchedFile = it->second;
        
        // Validate the reloaded file
        if (m_autoValidation) {
            auto result = ValidateAnimationFile(filepath);
            m_validationResults[filepath] = result;
            
            watchedFile.isValid = result.isValid;
            if (!result.isValid && !result.errors.empty()) {
                watchedFile.lastError = result.errors[0];
            } else {
                watchedFile.lastError.clear();
            }
            
            LogValidationResult(filepath, result);
            
            if (m_validationCallback) {
                m_validationCallback(filepath, result);
            }
        }

        // Trigger reload callback
        if (m_reloadCallback) {
            m_reloadCallback(filepath, watchedFile.assetType);
        }

        // Log reload event
        LogReloadEvent(filepath, watchedFile.assetType);

        watchedFile.needsReload = false;
    }

    std::string AnimationHotReloader::GetFileExtension(const std::string& filepath) const {
        size_t dotPos = filepath.find_last_of('.');
        if (dotPos != std::string::npos) {
            std::string ext = filepath.substr(dotPos);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            return ext;
        }
        return "";
    }

    std::string AnimationHotReloader::GetRelativePath(const std::string& filepath) const {
        // Simple relative path calculation
        std::filesystem::path path(filepath);
        return path.filename().string();
    }

    void AnimationHotReloader::LogReloadEvent(const std::string& filepath, const std::string& assetType) {
        LOG_INFO("Reloaded animation asset: " + GetRelativePath(filepath) + " (type: " + assetType + ")");
    }

    void AnimationHotReloader::LogValidationResult(const std::string& filepath, const AnimationValidationResult& result) {
        if (result.isValid) {
            LOG_INFO("Animation validation passed: " + GetRelativePath(filepath));
            
            if (!result.warnings.empty()) {
                for (const auto& warning : result.warnings) {
                    LOG_WARNING("Animation warning in " + GetRelativePath(filepath) + ": " + warning);
                }
            }
        } else {
            LOG_ERROR("Animation validation failed: " + GetRelativePath(filepath));
            
            for (const auto& error : result.errors) {
                LOG_ERROR("Animation error in " + GetRelativePath(filepath) + ": " + error);
            }
        }
    }



    // AnimationDevelopmentWorkflow Implementation

    bool AnimationDevelopmentWorkflow::Initialize() {
        if (m_initialized) {
            LOG_WARNING("AnimationDevelopmentWorkflow already initialized");
            return true;
        }

        LOG_INFO("Initializing Animation Development Workflow");

        if (!m_hotReloader.Initialize()) {
            LOG_ERROR("Failed to initialize animation hot reloader");
            return false;
        }

        // Set up callbacks
        m_hotReloader.SetReloadCallback([this](const std::string& filepath, const std::string& assetType) {
            OnAnimationReloaded(filepath, assetType);
        });

        m_hotReloader.SetErrorCallback([this](const std::string& filepath, const std::string& error) {
            OnAnimationError(filepath, error);
        });

        m_hotReloader.SetValidationCallback([this](const std::string& filepath, const AnimationValidationResult& result) {
            OnAnimationValidated(filepath, result);
        });

        m_initialized = true;
        ResetStatistics();

        LOG_INFO("Animation Development Workflow initialized successfully");
        return true;
    }

    void AnimationDevelopmentWorkflow::Shutdown() {
        if (!m_initialized) {
            return;
        }

        LOG_INFO("Shutting down Animation Development Workflow");

        StopLivePreview();
        m_hotReloader.Shutdown();
        m_assetImporters.clear();

        m_initialized = false;
        LOG_INFO("Animation Development Workflow shut down");
    }

    void AnimationDevelopmentWorkflow::Update(float deltaTime) {
        if (!m_initialized) {
            return;
        }

        m_hotReloader.Update(deltaTime);
        UpdateStatistics();
    }

    void AnimationDevelopmentWorkflow::SetProjectDirectory(const std::string& directory) {
        m_projectDirectory = directory;
        
        if (m_assetWatchingEnabled && !directory.empty()) {
            m_hotReloader.WatchAnimationDirectory(directory);
        }
    }

    void AnimationDevelopmentWorkflow::SetOutputDirectory(const std::string& directory) {
        m_outputDirectory = directory;
    }

    void AnimationDevelopmentWorkflow::SetSourceDirectory(const std::string& directory) {
        m_sourceDirectory = directory;
        
        if (m_assetWatchingEnabled && !directory.empty()) {
            m_hotReloader.WatchAnimationDirectory(directory);
        }
    }

    void AnimationDevelopmentWorkflow::RegisterAssetImporter(const std::string& extension, 
                                                           std::function<bool(const std::string&, const std::string&)> importer) {
        m_assetImporters[extension] = importer;
        LOG_INFO("Registered asset importer for extension: " + extension);
    }

    void AnimationDevelopmentWorkflow::ImportAsset(const std::string& sourcePath, const std::string& outputPath) {
        std::string extension = std::filesystem::path(sourcePath).extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        auto it = m_assetImporters.find(extension);
        if (it != m_assetImporters.end()) {
            LOG_INFO("Importing asset: " + sourcePath + " -> " + outputPath);
            
            if (it->second(sourcePath, outputPath)) {
                LOG_INFO("Asset import successful: " + outputPath);
                m_statistics.totalAssets++;
            } else {
                LOG_ERROR("Asset import failed: " + sourcePath);
            }
        } else {
            LOG_WARNING("No importer registered for extension: " + extension);
        }
    }

    void AnimationDevelopmentWorkflow::ImportAllAssets() {
        if (m_sourceDirectory.empty()) {
            LOG_WARNING("Source directory not set for asset import");
            return;
        }

        LOG_INFO("Importing all assets from: " + m_sourceDirectory);

        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(m_sourceDirectory)) {
                if (entry.is_regular_file()) {
                    std::string sourcePath = entry.path().string();
                    std::string extension = entry.path().extension().string();
                    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                    if (m_assetImporters.find(extension) != m_assetImporters.end()) {
                        std::string relativePath = std::filesystem::relative(entry.path(), m_sourceDirectory).string();
                        std::string outputPath = m_outputDirectory + "/" + relativePath;
                        
                        // Change extension to .json for processed assets
                        std::filesystem::path outputFilePath(outputPath);
                        outputFilePath.replace_extension(".json");
                        
                        ImportAsset(sourcePath, outputFilePath.string());
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            LOG_ERROR("Error during asset import: " + std::string(e.what()));
        }
    }

    void AnimationDevelopmentWorkflow::StartLivePreview() {
        if (m_livePreviewActive) {
            LOG_INFO("Live preview already active");
            return;
        }

        LOG_INFO("Starting animation live preview");
        m_livePreviewActive = true;
        m_hotReloader.SetEnabled(true);
    }

    void AnimationDevelopmentWorkflow::StopLivePreview() {
        if (!m_livePreviewActive) {
            return;
        }

        LOG_INFO("Stopping animation live preview");
        m_livePreviewActive = false;
        m_hotReloader.SetEnabled(false);
    }

    void AnimationDevelopmentWorkflow::EnableAssetWatching(bool enabled) {
        m_assetWatchingEnabled = enabled;
        
        if (enabled) {
            if (!m_projectDirectory.empty()) {
                m_hotReloader.WatchAnimationDirectory(m_projectDirectory);
            }
            if (!m_sourceDirectory.empty()) {
                m_hotReloader.WatchAnimationDirectory(m_sourceDirectory);
            }
        } else {
            m_hotReloader.ClearWatchedFiles();
        }
    }

    void AnimationDevelopmentWorkflow::RunAssetValidation() {
        LOG_INFO("Running asset validation");
        m_hotReloader.ValidateAllAnimations();
    }

    void AnimationDevelopmentWorkflow::RunAssetOptimization() {
        LOG_INFO("Running asset optimization");
        m_hotReloader.OptimizeAllAnimations();
    }

    void AnimationDevelopmentWorkflow::GenerateAssetReport() {
        std::string reportPath = m_outputDirectory + "/animation_asset_report.txt";
        m_hotReloader.GenerateAssetReport(reportPath);
        
        std::string statsPath = m_outputDirectory + "/animation_asset_stats.json";
        m_hotReloader.ExportAssetStatistics(statsPath);
    }

    void AnimationDevelopmentWorkflow::ResetStatistics() {
        m_statistics = WorkflowStatistics{};
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        m_statistics.lastUpdate = ss.str();
    }

    // Private methods

    void AnimationDevelopmentWorkflow::OnAnimationReloaded(const std::string& filepath, const std::string& assetType) {
        m_statistics.reloadedAssets++;
        ProcessAssetChange(filepath);
    }

    void AnimationDevelopmentWorkflow::OnAnimationError(const std::string& filepath, const std::string& error) {
        LOG_ERROR("Animation workflow error for " + filepath + ": " + error);
    }

    void AnimationDevelopmentWorkflow::OnAnimationValidated(const std::string& filepath, const AnimationValidationResult& result) {
        if (result.isValid) {
            m_statistics.validAssets++;
        } else {
            m_statistics.invalidAssets++;
        }
    }

    void AnimationDevelopmentWorkflow::ProcessAssetChange(const std::string& filepath) {
        // Process asset change for live preview
        if (m_livePreviewActive) {
            LOG_INFO("Processing asset change for live preview: " + filepath);
            // Additional live preview logic would go here
        }
    }

    void AnimationDevelopmentWorkflow::UpdateStatistics() {
        m_statistics.totalAssets = m_hotReloader.GetWatchedFileCount();
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        m_statistics.lastUpdate = ss.str();
    }

    std::string AnimationDevelopmentWorkflow::GetCurrentTimeString() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

} // namespace Animation
} // namespace GameEngine